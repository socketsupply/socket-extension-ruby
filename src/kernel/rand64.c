
#include <socket/ruby.h>
mrb_value socket_runtime_ruby_kernel_rand64 (
  mrb_state *state,
  mrb_value self
) {
  return mrb_fixnum_value(sapi_rand64());
}
