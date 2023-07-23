#ifndef SOCKET_RUNTIME_RUBY_KERNEL
#define SOCKET_RUNTIME_RUBY_KERNEL

#include <mruby.h>

#ifdef __cplusplus
extern "C" {
#endif

mrb_value socket_runtime_ruby_kernel_puts (
  mrb_state *state,
  mrb_value self
);

mrb_value socket_runtime_ruby_kernel_rand64 (
  mrb_state *state,
  mrb_value self
);

mrb_value socket_runtime_ruby_kernel_randid (
  mrb_state *state,
  mrb_value self
);

#ifdef __cplusplus
}
#endif
#endif
