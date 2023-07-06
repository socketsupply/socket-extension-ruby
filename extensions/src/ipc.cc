#include <socket/ruby.h>

namespace Socket::Ruby::IPC {
  void Start (sapi_context_t* context) {
    sapi_ipc_router_map(context, "ruby.open", Routes::Open, nullptr);
  }

  void Stop (sapi_context_t* context) {
  }

  namespace Routes {
    void Open (
      sapi_context_t* context,
      sapi_ipc_message_t* message,
      const sapi_ipc_router_t* router
    ) {
      sapi_log(0, "BEGIN onopen");

      auto state = State::Get();
      auto result = sapi_ipc_result_create(context, message);
      auto filename = sapi_ipc_message_get(message, "filename");
      auto entry = sapi_ipc_message_get(message, "entry");
      auto file = fopen(filename, "r");
      auto program = mrb_load_file(state, file);
      auto object = Types::GetClassInstance(state, Types::Context, context);

      mrb_define_method(
        state,
        state->kernel_module,
        "sapi_ipc_router_map",
        Bindings::sapi_ipc_router_map,
        MRB_ARGS_REQ(2) | MRB_ARGS_BLOCK()
      );

      if (!entry) {
        entry = "main";
      }

      mrb_gc_arena_save(state);
      mrb_funcall_argv(
          state,
          program,
          mrb_intern_static(state, entry, strlen(entry)),
          2,
          &object
          );

      fclose(file);
      sapi_ipc_reply(result);
      sapi_log(0, "END onopen");
    }
  }
}
