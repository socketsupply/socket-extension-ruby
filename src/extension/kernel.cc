#include <socket/ruby.h>

namespace Socket::Ruby::Kernel {
  static Mutex mutex;

  void Init (mrb_state* state) {
    Lock lock(mutex);

    mrb_define_method(
      state,
      state->kernel_module,
      "puts",
      socket_runtime_ruby_kernel_puts,
      MRB_ARGS_REQ(1)
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "rand64",
      socket_runtime_ruby_kernel_rand64,
      MRB_ARGS_NONE()
    );

    mrb_define_method(
      state,
      state->kernel_module,
      "randid",
      socket_runtime_ruby_kernel_randid,
      MRB_ARGS_NONE()
    );
  }
}
