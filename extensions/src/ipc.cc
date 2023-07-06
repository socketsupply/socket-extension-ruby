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
      auto state = State::GetRoot();
      auto result = sapi_ipc_result_create(context, message);
      auto filename = sapi_ipc_message_get(message, "filename");
      auto entry = sapi_ipc_message_get(message, "entry");
      auto file = fopen(filename, "r");

      DATA_PTR(mrb_obj_value(state->top_self)) = context;

      auto program = mrb_load_file(state, file);
      auto object = Types::GetClassInstance(state, Types::Context, context);

      fclose(file);

      mrb_gc_arena_save(state);
      if (entry) {
        mrb_funcall_argv(
          state,
          program,
          mrb_intern_static(state, entry, strlen(entry)),
          1,
          &object
        );
      }

      sapi_ipc_reply(result);
    }
  }
}
