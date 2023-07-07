#include <socket/ruby.h>

namespace Socket::Ruby::Bindings {
  static void sapi_ipc_router_map_block (
    sapi_context_t* context,
    sapi_ipc_message_t* message,
    const sapi_ipc_router_t* router
  ) {
    auto proc_context = (Proc::Context*) sapi_context_get_data(context);
    auto state = proc_context->state;
    auto proc = proc_context->proc;

    mrb_value argv[] = {
      Types::GetClassInstance(state, Types::Context, context),
      Types::GetClassInstance(state, Types::IPC::Message, message),
      Types::GetClassInstance(state, Types::IPC::Router, (void*) router)
    };

    mrb_yield_with_class(
      state,
      mrb_obj_value(proc),
      3,
      argv,
      mrb_nil_value(),
      state->object_class
    );
  }

  mrb_value sapi_ipc_router_map (mrb_state *state, mrb_value self) {
    mrb_value block;
    char *route = nullptr;

    mrb_get_args(state, "z|&*", &route, &block);

    if (mrb_block_given_p(state)) {
      if (mrb_obj_respond_to(state, mrb_class(state, block), mrb_intern_lit(state, "call"))) {
        sapi_context_t* ctx = (sapi_context_t*) DATA_PTR(mrb_obj_value(state->top_self));
        mrb_state* destination_state = State::Clone(state, self);
        Proc::Context *proc_context = (Proc::Context*) mrb_malloc(
          destination_state,
          sizeof(Proc::Context)
        );

        proc_context->state = destination_state;
        proc_context->proc = Migrate::MigrateProc(
          state,
          mrb_proc_ptr(block),
          destination_state
        );

        do {
          mrb_state *mrb = state;
          MRB_PROC_SET_TARGET_CLASS(proc_context->proc, proc_context->state->object_class);
        } while (0);

        ::sapi_ipc_router_map(ctx, route, sapi_ipc_router_map_block, proc_context);
      }
    }

    return self;
  }

  mrb_value sapi_ipc_router_unmap (mrb_state *state, mrb_value self) {
    char *route = nullptr;
    mrb_get_args(state, "z", &route);
    auto context = (sapi_context_t*) DATA_PTR(mrb_obj_value(state->top_self));
    if (context && route) {
      ::sapi_ipc_router_unmap(context, route);
    }
    return self;
  }

  mrb_value sapi_ipc_result_create (mrb_state *state, mrb_value self) {
    mrb_value args[2] = {0};
    mrb_get_args(state, "o|o", &args[0], &args[1]);
    auto context = (sapi_context_t*) DATA_PTR(args[0]);
    auto message = (sapi_ipc_message_t*) DATA_PTR(args[1]);
    auto result = sapi_ipc_result_create(context, message);
    return Types::GetClassInstance(state, Types::IPC::Result, result);
  }

  mrb_value sapi_ipc_reply (mrb_state *state, mrb_value self) {
    mrb_value args[1] = {0};
    mrb_get_args(state, "o", &args[0]);
    auto result = (sapi_ipc_result_t*) DATA_PTR(args[0]);
    sapi_ipc_reply(result);
    return self;
  }

  mrb_value sapi_ipc_result_set_bytes (mrb_state *state, mrb_value self) {
    mrb_value args[2] = {0};
    mrb_get_args(state, "o|A", &args[0], &args[1]);
    auto result = (sapi_ipc_result_t*) DATA_PTR(args[0]);
    auto context = sapi_ipc_result_get_context(result);
    auto array = mrb_ary_ptr(args[1]);
    auto size = ARY_LEN(array);
    unsigned char *bytes = new unsigned char[size]{0};
    for (int i = 0; i < size; ++i) {
      auto entry = mrb_ary_entry(args[1], i);
      auto byte = mrb_int(state, entry);
      bytes[i] = byte;
    }
    ::sapi_ipc_result_set_bytes(result, size, bytes);
    return self;
  }

  mrb_value sapi_ipc_result_set_json (mrb_state *state, mrb_value self) {
    mrb_value args[1] = {0};
    char *json = nullptr;
    mrb_get_args(state, "o|z", &args[0], &json);
    auto result = (sapi_ipc_result_t*) DATA_PTR(args[0]);
    auto context = sapi_ipc_result_get_context(result);
    ::sapi_ipc_result_set_json(
      result,
      sapi_json_any(sapi_json_raw_from(context, json))
    );
    return self;
  }

  mrb_value sapi_ipc_result_set_json_data (mrb_state *state, mrb_value self) {
    mrb_value args[1] = {0};
    char *json = nullptr;
    mrb_get_args(state, "o|z", &args[0], &json);
    auto result = (sapi_ipc_result_t*) DATA_PTR(args[0]);
    auto context = sapi_ipc_result_get_context(result);
    ::sapi_ipc_result_set_json_data(
      result,
      sapi_json_any(sapi_json_raw_from(context, json))
    );
    return self;
  }

  mrb_value sapi_ipc_result_set_json_error (mrb_state *state, mrb_value self) {
    mrb_value args[1] = {0};
    char *json = nullptr;
    mrb_get_args(state, "o|z", &args[0], &json);
    auto result = (sapi_ipc_result_t*) DATA_PTR(args[0]);
    auto context = sapi_ipc_result_get_context(result);
    ::sapi_ipc_result_set_json_error(
      result,
      sapi_json_any(sapi_json_raw_from(context, json))
    );
    return self;
  }

  void Init (mrb_state *state) {
    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_router_map",
      Bindings::sapi_ipc_router_map,
      MRB_ARGS_REQ(2) | MRB_ARGS_BLOCK()
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_router_unmap",
      Bindings::sapi_ipc_router_unmap,
      MRB_ARGS_REQ(2)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_result_create",
      Bindings::sapi_ipc_result_create,
      MRB_ARGS_REQ(2)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_result_set_bytes",
      Bindings::sapi_ipc_result_set_bytes,
      MRB_ARGS_REQ(2)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_result_set_json",
      Bindings::sapi_ipc_result_set_json,
      MRB_ARGS_REQ(2)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_result_set_json_data",
      Bindings::sapi_ipc_result_set_json_data,
      MRB_ARGS_REQ(2)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_result_set_json_error",
      Bindings::sapi_ipc_result_set_json_error,
      MRB_ARGS_REQ(2)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_reply",
      Bindings::sapi_ipc_reply,
      MRB_ARGS_REQ(1)
    );
  }
}
