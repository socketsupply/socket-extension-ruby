#include <socket/ruby.h>
#include <mutex>

namespace Socket::Ruby::State {
  static mrb_state *state = nullptr;
  static Mutex mutex;

  mrb_state* Open () {
    Lock lock(mutex);

    if (state == nullptr) {
      state = mrb_open();
      if (state != nullptr) {
        Kernel::Init(state);
      }
    }

    return state;
  }

  void Close () {
    Lock lock(mutex);

    if (state != nullptr) {
      mrb_close(state);
      state = nullptr;
    }
  }

  mrb_state* Get () {
    Lock lock(mutex);
    return state;
  }

  mrb_state* Clone (mrb_value self) {
    return Clone(state, self);
  }

  mrb_state* Clone (mrb_state* source_state, mrb_value self) {
    mrb_state* destination_state = mrb_open();
    mrb_value globals = mrb_f_global_variables(source_state, self);

    Migrate::MigrateSymbols(source_state, destination_state);

    for (int i = 0; i < RARRAY_LEN(globals); i++) {
      int arena = mrb_gc_arena_save(source_state);
      mrb_value key = mrb_ary_entry(globals, i);
      mrb_value value = mrb_gv_get(source_state, mrb_symbol(key));

      if (
        Migrate::IsSafeMigratableValue(
          source_state,
          value,
          destination_state
        )
      ) {
        mrb_int size = 0;
        const char *name = mrb_sym2name_len(
          source_state,
          mrb_symbol(key),
          &size
        );

        mrb_gv_set(
          destination_state,
          mrb_intern_static(destination_state, name, size),
          Migrate::MigrateValue(source_state, value, destination_state)
        );
      }

      mrb_gc_arena_restore(source_state, arena);
    }

    auto top_self = source_state->top_self;
    auto variables = mrb_obj_instance_variables(
      source_state,
      mrb_obj_value(source_state->top_self)
    );

    for (int i = 0; i < RARRAY_LEN(variables); ++i) {
      int arena = mrb_gc_arena_save(source_state);
      mrb_int size = 0;
      mrb_value key = mrb_ary_entry(variables, i);
      mrb_value value = mrb_obj_iv_get(
        source_state,
        top_self,
        mrb_symbol(key)
      );

      if (
        Migrate::IsSafeMigratableValue(
          source_state,
          value,
          destination_state
        )
      ) {
        const char *name = mrb_sym2name_len(source_state, mrb_symbol(key), &size);
        mrb_obj_iv_set(
          destination_state,
          destination_state->top_self,
          mrb_intern_static(destination_state, name, size),
          Migrate::MigrateValue(source_state, value, destination_state)
        );
      }

      mrb_gc_arena_restore(source_state, arena);
    }

    return destination_state;
  }
}
