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
    auto block = mrb_load_proc(state, proc);

    mrb_value argv[] = {
      Types::GetClassInstance(state, Types::Context, context)
    };

    mrb_yield_with_class(
      state,
      mrb_obj_value(proc),
      1,
      argv,
      mrb_nil_value(),
      state->object_class
    );
  }

  mrb_value sapi_ipc_router_map (mrb_state *state, mrb_value self) {
    mrb_value context;
    mrb_value block;
    mrb_value* argv;
    mrb_int argc;
    char* route = NULL;

    mrb_get_args(state, "o|z|&*", &context, &route, &block, &argv, &argc);

    if (mrb_block_given_p(state)) {
      if (mrb_obj_respond_to(state, mrb_class(state, block), mrb_intern_lit(state, "call"))) {
        sapi_context_t* ctx = (sapi_context_t*) DATA_PTR(context);
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

        for (int i = 0; i < argc; i++) {
          Migrate::MigrateValue(state, argv[i], destination_state);
        }

        ::sapi_ipc_router_map(ctx, route, sapi_ipc_router_map_block, proc_context);
      }
    }

    return self;
  }
}
