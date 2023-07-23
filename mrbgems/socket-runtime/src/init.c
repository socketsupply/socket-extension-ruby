#include <socket/ruby.h>

void mrb_socket_runtime_gem_init (mrb_state* state);
void mrb_socket_runtime_gem_final (mrb_state* state);

void mrb_socket_runtime_gem_init (mrb_state* state) {
  mrb_define_method(
    state,
    state->kernel_module,
    "puts",
    socket_runtime_ruby_kernel_puts,
    MRB_ARGS_REQ(1)
  );
}

void mrb_socket_runtime_gem_final (mrb_state* state) {
}
