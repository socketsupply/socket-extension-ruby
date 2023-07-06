/**
 * Adapted from: https://github.com/mattn/mruby-thread/blob/master/src/mrb_thread.c
 */

#include <socket/ruby.h>

namespace Socket::Ruby::Migrate {
  mrb_value MigrateValue (
    mrb_state *source_state,
    mrb_value value,
    mrb_state *destination_state
  );

  mrb_sym MigrateSymbol (
    mrb_state *source_state,
    mrb_sym symbol,
    mrb_state *destination_state
  ) {
    mrb_int size;
    const char *name = mrb_sym2name_len(source_state, symbol, &size);
    return mrb_intern_static(destination_state, name, size);
  }

  void MigrateSymbols (
    mrb_state *mrb,
    mrb_state *destination_state
  ) {
    for (mrb_sym i = 1; i < mrb->symidx + 1; i++) {
      MigrateSymbol(mrb, i, destination_state);
    }
  }

  void MigrateInstanceVariables (
    mrb_state *source_state,
    mrb_value source_value,
    mrb_state *destination_state,
    mrb_value destination_value
  ) {
    mrb_value variables = mrb_obj_instance_variables(source_state, source_value);

    for (mrb_int i = 0; i < RARRAY_LEN(variables); ++i) {
      mrb_sym source_symbol = mrb_symbol(RARRAY_PTR(variables)[i]);
      mrb_sym destination_symbol = MigrateSymbol(
        source_state,
        source_symbol,
        destination_state
      );

      mrb_value value = mrb_iv_get(
        source_state,
        source_value,
        source_symbol
      );

      mrb_iv_set(
        destination_state,
        destination_value,
        destination_symbol,
        MigrateValue(source_state, value, destination_state)
      );
    }
  }

  mrb_bool IsSafeMigratableDataType (const mrb_data_type *type) {
    static const char **types = Socket::Ruby::Types::GetNames();
    static const char *internals[] = { "IO", "Time", nullptr };

    for (int i = 0; internals[i]; ++i) {
      if (strcmp(type->struct_name, internals[i]) == 0) {
        return true;
      }
    }

    for (int i = 0; types[i]; i++) {
      if (strcmp(type->struct_name, types[i]) == 0) {
        return true;
      }
    }

    return false;
  }

  mrb_bool IsSafeMigratableValue (
    mrb_state *source_state,
    mrb_value value,
    mrb_state *destination_state
  ) {
    switch (mrb_type(value)) {
      case MRB_TT_OBJECT:
      case MRB_TT_EXCEPTION: {
        struct RObject *object = mrb_obj_ptr(value);
        mrb_value path = mrb_class_path(source_state, object->c);

        if (mrb_nil_p(path) || !mrb_class_defined(destination_state, RSTRING_PTR(path))) {
          return false;
        }

        break;
      }

      case MRB_TT_PROC:
      case MRB_TT_FALSE:
      case MRB_TT_TRUE:
      case MRB_TT_FIXNUM:
      case MRB_TT_SYMBOL:
      case MRB_TT_STRING:
      #ifndef MRB_WITHOUT_FLOAT
      case MRB_TT_FLOAT:
      #endif
        break;

      case MRB_TT_RANGE: {
        struct RRange *range = MRB_RANGE_PTR(source_state, value);

        if (!IsSafeMigratableValue(source_state, RANGE_BEG(range), destination_state)) {
          return false;
        }

        if (!IsSafeMigratableValue(source_state, RANGE_END(range), destination_state)) {
          return false;
        }

        break;
      }

      case MRB_TT_ARRAY: {
        for (int i = 0; i < RARRAY_LEN(value); ++i) {
          if (!IsSafeMigratableValue(source_state, RARRAY_PTR(value)[i], destination_state)) {
            return false;
          }
        }
        break;
      }

      case MRB_TT_HASH: {
        mrb_value keys = mrb_hash_keys(source_state, value);
        for (int i = 0; i < RARRAY_LEN(keys); ++i) {
          mrb_value key = mrb_ary_entry(keys, i);

          if (!IsSafeMigratableValue(source_state, key, destination_state)) {
            return false;
          }

          if (
            !IsSafeMigratableValue(
              source_state,
              mrb_hash_get(source_state, value, key),
              destination_state
            )
          ) {
            return false;
          }
        }
        break;
      }

      case MRB_TT_DATA: {
        if (!IsSafeMigratableDataType(DATA_TYPE(value))) {
          return false;
        }
        break;
     }

      default: return false;
    }

    return true;
  }

  void MigrateInternalRepresentationChild (
    mrb_state *source_state,
    mrb_irep *representation,
    mrb_state *destination_state
  ) {
    // migrate pool
    // FIXME: broken with mruby3
    #ifndef IREP_TT_SFLAG
    for (int i = 0; i < representation->plen; ++i) {
      mrb_value v = ret->pool[i];
      if (mrb_type(v) == MRB_TT_STRING) {
        struct RString *s = mrb_str_ptr(v);
        if (RSTR_NOFREE_P(s) && RSTRING_LEN(v) > 0) {
          char *old = RSTRING_PTR(v);
          s->as.heap.ptr = (char*)mrb_malloc(destination_state, RSTRING_LEN(v));
          memcpy(s->as.heap.ptr, old, RSTRING_LEN(v));
          RSTR_UNSET_NOFREE_FLAG(s);
        }
      }
    }
    #endif

    // migrate iseq
    if (representation->flags & MRB_ISEQ_NO_FREE) {
      mrb_code *old_iseq = (mrb_code*) representation->iseq;
      representation->iseq = (mrb_code*) mrb_malloc(
        destination_state,
        sizeof(mrb_code) * representation->ilen
      );

      memcpy(
        (void*) representation->iseq,
        old_iseq,
        sizeof(mrb_code) * representation->ilen
      );

      representation->flags &= ~MRB_ISEQ_NO_FREE;
    }

    // migrate sub ireps
    for (int i = 0; i < representation->rlen; ++i) {
      MigrateInternalRepresentationChild(
        source_state,
        (struct mrb_irep*) representation->reps[i],
        destination_state
      );
    }
  }

  mrb_irep* MigrateInternalRepresentation (
    mrb_state *source_state,
    mrb_irep *source_representation,
    mrb_state *destination_state
  ) {
    uint8_t *bin = nullptr;
    size_t bin_size = 0;

    #ifdef DUMP_ENDIAN_NAT
    mrb_dump_irep(
      source_state,
      source_representation,
      DUMP_ENDIAN_NAT,
      &bin,
      &bin_size
    );
    #else
    mrb_dump_irep(
      source_state,
      source_representation,
      0,
      &bin,
      &bin_size
    );
    #endif

    mrb_irep* destination_representation = mrb_read_irep(
      destination_state,
      bin
    );

    MigrateInternalRepresentationChild(
      source_state,
      destination_representation,
      destination_state
    );

    mrb_free(source_state, bin);
    return destination_representation;
  }

  struct RProc* MigrateProc (
    mrb_state *source_state,
    struct RProc *source_proc,
    mrb_state *destination_state
  ) {
    struct RProc *destination_proc  = mrb_proc_new(
      destination_state,
      MigrateInternalRepresentation(
        source_state,
        (struct mrb_irep*) source_proc->body.irep,
        destination_state
      )
    );

    mrb_irep_decref(
      destination_state,
      (struct mrb_irep*) destination_proc->body.irep
    );

    if (_MRB_PROC_ENV(source_proc) && MRB_PROC_ENV_P(source_proc)) {
      mrb_int len = MRB_ENV_LEN(_MRB_PROC_ENV(source_proc));
      struct REnv *destination_env = (struct REnv*) mrb_obj_alloc(
        destination_state,
        MRB_TT_ENV,
        destination_state->object_class
      );

      destination_env->stack = (mrb_value*) mrb_malloc(
        source_state,
        sizeof(mrb_value) * len
      );

      MRB_ENV_CLOSE(destination_env);

      for (mrb_int i = 0; i < len; ++i) {
        mrb_value value = _MRB_PROC_ENV(source_proc)->stack[i];
        if (mrb_obj_ptr(value) == ((struct RObject*) source_proc)) {
          destination_env->stack[i] = mrb_obj_value(destination_proc);
        } else {
          destination_env->stack[i] = MigrateValue(
            source_state,
            value,
            destination_state
          );
        }
      }

      MRB_ENV_SET_LEN(destination_env, len);

      destination_proc->flags |= MRB_PROC_ENVSET;

      if (source_proc->upper) {
        destination_proc->upper = MigrateProc(
          source_state,
          (struct RProc*) source_proc->upper,
          destination_state
        );
      }
    }

    return destination_proc;
  }

  struct RClass* GetClass (
    mrb_state *source_state,
    char const *path_begin,
    mrb_int size
  ) {
    char const *pointer = path_begin;
    char const *begin = path_begin;
    char const *end = begin + size;
    struct RClass* class_type = source_state->object_class;

    while (true) {
      while (
        (pointer < end && pointer[0] != ':') ||
        (pointer + 1 < end && pointer[1] != ':')
      ) {
        pointer++;
      }

      mrb_sym class_symbol = mrb_intern(source_state, begin, pointer - begin);

      if (!mrb_mod_cv_defined(source_state, class_type, class_symbol)) {
        if (
          strncmp(
            "Socket::Context",
            path_begin,
            pointer - path_begin
          ) == 0
        ) {
          break;
        }

        mrb_raisef(
          source_state,
          mrb_class_get(source_state, "ArgumentError"),
          "Socket::Ruby::Migrate::GetClass: undefined class/module %S",
          mrb_str_new(
            source_state,
            path_begin,
            pointer - path_begin
          )
        );
      }

      mrb_value class_value = mrb_mod_cv_get(
        source_state,
        class_type,
        class_symbol
      );

      if (
        mrb_type(class_value) != MRB_TT_CLASS &&
        mrb_type(class_value) != MRB_TT_MODULE
      ) {
        mrb_raisef(
          source_state,
          mrb_class_get(source_state, "TypeError"),
          "Socket::Ruby::Migrate::GetClass: %S does not refer to class/module",
          mrb_str_new(source_state, path_begin, pointer - path_begin)
        );
      }

      class_type = mrb_class_ptr(class_value);

      if (pointer >= end) {
        break;
      }

      // advance
      pointer += 2;
      begin = pointer;
    }

    return class_type;
  }

  mrb_value MigrateValue (
    mrb_state *source_state,
    mrb_value const source_value,
    mrb_state *destination_state
  ) {
    if (source_state == destination_state) {
      return source_value;
    }

    switch (mrb_type(source_value)) {
      case MRB_TT_CLASS:
      case MRB_TT_MODULE: {
        mrb_value class_path = mrb_class_path(
          source_state,
          mrb_class_ptr(source_value)
        );

        if (mrb_nil_p(class_path)) {
          return mrb_nil_value();
        }

        struct RClass *class_type = GetClass(
          destination_state,
          RSTRING_PTR(class_path),
          RSTRING_LEN(class_path)
        );

        return mrb_obj_value(class_type);
      }

      case MRB_TT_OBJECT:
      case MRB_TT_EXCEPTION: {
        mrb_value class_path = mrb_class_path(
          source_state,
          mrb_class(source_state, source_value)
        );

        if (mrb_nil_p(class_path)) {
          return mrb_nil_value();
        }

        struct RClass *class_type = GetClass(
          destination_state,
          RSTRING_PTR(class_path),
          RSTRING_LEN(class_path)
        );

        mrb_value destination_value = mrb_obj_value(
          mrb_obj_alloc(
            destination_state,
            mrb_type(source_value),
            class_type
          )
        );

        MigrateInstanceVariables(
          source_state,
          source_value,
          destination_state,
          destination_value
        );

        if (mrb_type(source_value) == MRB_TT_EXCEPTION) {
          mrb_iv_set(
            destination_state,
            destination_value,
            mrb_intern_lit(destination_state, "mesg"),
            MigrateValue(
               source_state,
               mrb_iv_get(
                  source_state,
                  source_value,
                  mrb_intern_lit(source_state, "mesg")
                ),
               destination_state
             )
          );
        }

        return destination_value;
      }

      case MRB_TT_PROC: {
        return mrb_obj_value(
          MigrateProc(
            source_state,
            mrb_proc_ptr(source_value),
            destination_state
          )
        );
      }

      case MRB_TT_FALSE:
      case MRB_TT_TRUE:
      case MRB_TT_FIXNUM: {
        return source_value;
      }

      case MRB_TT_SYMBOL: {
        return mrb_symbol_value(
          MigrateSymbol(
            source_state,
            mrb_symbol(source_value),
            destination_state
            )
        );
      }

      #ifndef MRB_WITHOUT_FLOAT
      case MRB_TT_FLOAT: {
        return mrb_float_value(
          destination_state,
          mrb_float(source_value)
        );
       }
      #endif

      case MRB_TT_STRING: {
        return mrb_str_new(
          destination_state,
          RSTRING_PTR(source_value),
          RSTRING_LEN(source_value)
        );
      }

      case MRB_TT_RANGE: {
        struct RRange *range = MRB_RANGE_PTR(source_state, source_value);
        return mrb_range_new(
          destination_state,
          MigrateValue(source_state, RANGE_BEG(range), destination_state),
          MigrateValue(source_state, RANGE_END(range), destination_state),
          RANGE_EXCL(range)
        );
      }

      case MRB_TT_ARRAY: {
        mrb_value destination_value = mrb_ary_new_capa(
          destination_state,
          RARRAY_LEN(source_value)
        );

        int arena = mrb_gc_arena_save(destination_state);

        for (int i = 0; i < RARRAY_LEN(source_value); ++i) {
          mrb_ary_push(
            destination_state,
            destination_value,
            MigrateValue(
              source_state,
              RARRAY_PTR(source_value)[i],
              destination_state
            )
          );

          mrb_gc_arena_restore(destination_state, arena);
        }

        return destination_value;
      }

      case MRB_TT_HASH: {
        mrb_value destination_value = mrb_hash_new(destination_state);
        mrb_value keys = mrb_hash_keys(source_state, source_value);

        for (int i = 0; i < RARRAY_LEN(keys); ++i) {
          int arena = mrb_gc_arena_save(destination_state);
          mrb_value key = MigrateValue(
            source_state,
            mrb_ary_entry(keys, i),
            destination_state
          );

          mrb_value value = MigrateValue(
            source_state,
            mrb_hash_get(source_state, source_value, key),
            destination_state
          );

          mrb_hash_set(
            destination_state,
            destination_value,
            key,
            value
          );

          mrb_gc_arena_restore(destination_state, arena);
        }

        MigrateInstanceVariables(
          source_state,
          source_value,
          destination_state,
          destination_value
        );

        return destination_value;
      }

      case MRB_TT_DATA: {
        mrb_value class_path = mrb_class_path(
          source_state,
          mrb_class(source_state, source_value)
        );

        struct RClass *class_type = GetClass(
          destination_state,
          RSTRING_PTR(class_path),
          RSTRING_LEN(class_path)
        );

        if (!IsSafeMigratableDataType(DATA_TYPE(source_value))) {
          mrb_raisef(
            source_state,
            mrb_class_get(source_state, "TypeError"),
            "Socket::Ruby::Migrate::MigrateValue: cannot migrate object: %S(%S)",
            mrb_str_new_cstr(
              source_state,
              DATA_TYPE(source_value)->struct_name
            ),
            mrb_inspect(source_state, source_value)
          );
        }

        mrb_value destination_value = mrb_obj_value(
          mrb_obj_alloc(
            destination_state,
            mrb_type(source_value),
            class_type
          )
        );

        // handle `mruby-time`
        if (strcmp(DATA_TYPE(source_value)->struct_name, "Time") == 0) {
          DATA_PTR(destination_value) = mrb_malloc(source_state, sizeof(struct mrb_time));
          *((struct mrb_time*) DATA_PTR(destination_value)) = *((struct mrb_time*) DATA_PTR(source_value));
          DATA_TYPE(destination_value) = DATA_TYPE(source_value);
        } else {
          DATA_PTR(destination_value) = DATA_PTR(source_value);
          MigrateInstanceVariables(
            source_state,
            source_value,
            destination_state,
            destination_value
          );

        }
        return destination_value;
      }

      default: break;
    }

    mrb_raisef(
      source_state,
      mrb_class_get(source_state, "TypeError"),
      "Socket::Ruby::Migrate::MigrateValue: cannot migrate object: %S(%S)",
      mrb_inspect(source_state, source_value),
      mrb_fixnum_value(mrb_type(source_value))
    );

    return mrb_nil_value();
  }
}
