#include <socket/ruby.h>

namespace Socket::Ruby::Bindings {
  static void sapi_ipc_router_map_block (
    sapi_context_t* context,
    sapi_ipc_message_t* message,
    const sapi_ipc_router_t* router
  ) {
    Lock lock(mutex);
    auto state = State::GetRoot();
    auto proc = (struct RProc*) sapi_context_get_data(context);

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
    Lock lock(mutex);
    mrb_value block;
    char *route = nullptr;

    mrb_get_args(state, "z|&", &route, &block);

    if (mrb_block_given_p(state)) {
      if (mrb_obj_respond_to(state, mrb_class(state, block), mrb_intern_lit(state, "call"))) {
        ::sapi_ipc_router_map(
          reinterpret_cast<sapi_context_t*>(DATA_PTR(mrb_obj_value(state->top_self))),
          route,
          sapi_ipc_router_map_block,
          mrb_proc_ptr(block)
        );
      }
    }

    return self;
  }

  mrb_value sapi_ipc_router_unmap (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    char *route = nullptr;
    mrb_get_args(state, "z", &route);
    auto context = (sapi_context_t*) DATA_PTR(mrb_obj_value(state->top_self));
    if (context && route) {
      ::sapi_ipc_router_unmap(context, route);
    }
    return self;
  }

  mrb_value sapi_ipc_result_create (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[2] = {0};
    mrb_get_args(state, "o|o", &args[0], &args[1]);
    auto context = (sapi_context_t*) DATA_PTR(args[0]);
    auto message = (sapi_ipc_message_t*) DATA_PTR(args[1]);
    auto result = ::sapi_ipc_result_create(context, message);
    return Types::GetClassInstance(state, Types::IPC::Result, result);
  }

  mrb_value sapi_ipc_reply (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[1] = {0};
    mrb_get_args(state, "o", &args[0]);
    auto result = (sapi_ipc_result_t*) DATA_PTR(args[0]);
    ::sapi_ipc_reply(result);
    return self;
  }

  mrb_value sapi_ipc_result_set_bytes (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
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
    Lock lock(mutex);
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
    Lock lock(mutex);
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
    Lock lock(mutex);
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

  mrb_value sapi_ipc_result_set_header (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[2] = {0};
    char *key = nullptr;
    char *value = nullptr;
    mrb_get_args(state, "o|z|z", &args[0], &key, &value);
    auto result = (sapi_ipc_result_t*) DATA_PTR(args[0]);
    ::sapi_ipc_result_set_header(result, key, value);
    return self;
  }

  mrb_value sapi_context_create (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[2] = {0};
    mrb_get_args(state, "o|b", &args[0], &args[1]);
    auto context = ::sapi_context_create(
      (sapi_context_t*) DATA_PTR(args[0]),
      mrb_bool(args[1])
    );
    return Types::GetClassInstance(state, Types::Context, context);
  }

  mrb_value sapi_context_release (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[1] = {0};
    mrb_get_args(state, "o", &args[0]);
    ::sapi_context_release((sapi_context_t*) DATA_PTR(args[0]));
    return self;
  }

  mrb_value sapi_javascript_evaluate (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[1] = {0};
    char *name = nullptr;
    char *source = nullptr;
    mrb_get_args(state, "o|z|z", &args[0], &name, &source);
    ::sapi_javascript_evaluate(
      (sapi_context_t*) DATA_PTR(args[0]),
      name,
      source
    );
    return self;
  }

  static void sapi_context_dispatch_block (
    sapi_context_t* context,
    const void* data
  ) {
    Lock lock(mutex);

    auto state = State::GetRoot();
    auto proc = (struct RProc*) data;

    mrb_yield_with_class(
      state,
      mrb_obj_value(proc),
      0,
      nullptr,
      mrb_nil_value(),
      state->object_class
    );
  }

  mrb_value sapi_context_dispatch (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value block;
    mrb_value context;
    mrb_get_args(state, "o|&", &context, &block);

    if (mrb_block_given_p(state)) {
      if (mrb_obj_respond_to(state, mrb_class(state, block), mrb_intern_lit(state, "call"))) {
        ::sapi_context_dispatch(
          reinterpret_cast<sapi_context_t*>(DATA_PTR(context)),
          mrb_proc_ptr(block),
          sapi_context_dispatch_block
        );
      }
    }

    return self;
  }

  mrb_value sapi_ipc_send_bytes (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[3];
    char *headers = nullptr;
    mrb_get_args(state, "o|o|A|z", &args[0], &args[1], &args[2], &headers);

    auto array = mrb_ary_ptr(args[2]);
    auto size = ARY_LEN(array);
    auto bytes = new unsigned char[size]{0};

    for (int i = 0; i < size; ++i) {
      auto entry = mrb_ary_entry(args[2], i);
      auto byte = mrb_int(state, entry);
      bytes[i] = byte;
    }

    auto result = ::sapi_ipc_send_bytes(
      reinterpret_cast<sapi_context_t*>(DATA_PTR(args[0])),
      mrb_nil_p(args[1]) ? nullptr : reinterpret_cast<sapi_ipc_message_t*>(DATA_PTR(args[1])),
      size,
      bytes,
      headers
    );

    return self;
  }

  mrb_value sapi_ipc_send_json (mrb_state *state, mrb_value self) {
    Lock lock(mutex);
    mrb_value args[3];
    char *json = nullptr;
    mrb_get_args(state, "o|o|z", &args[0], &args[1], &json);

    ::sapi_ipc_send_json(
      reinterpret_cast<sapi_context_t*>(DATA_PTR(args[0])),
      mrb_nil_p(args[1]) ? nullptr : reinterpret_cast<sapi_ipc_message_t*>(DATA_PTR(args[1])),
      sapi_json_any(sapi_json_raw_from(
        reinterpret_cast<sapi_context_t*>(DATA_PTR(args[0])),
        json
      ))
    );

    return self;
  }

  void Init (mrb_state *state) {
    Lock lock(mutex);
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
      MRB_ARGS_REQ(1)
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
      "sapi_ipc_result_set_header",
      Bindings::sapi_ipc_result_set_header,
      MRB_ARGS_REQ(3)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_reply",
      Bindings::sapi_ipc_reply,
      MRB_ARGS_REQ(1)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_send_bytes",
      Bindings::sapi_ipc_send_bytes,
      MRB_ARGS_REQ(4)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_ipc_send_json",
      Bindings::sapi_ipc_send_json,
      MRB_ARGS_REQ(3)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_javascript_evaluate",
      Bindings::sapi_javascript_evaluate,
      MRB_ARGS_REQ(3)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "sapi_context_dispatch",
      Bindings::sapi_context_dispatch,
      MRB_ARGS_REQ(1) | MRB_ARGS_BLOCK()
    );
  }
}
