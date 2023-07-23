#include <socket/ruby.h>

mrb_value socket_runtime_ruby_kernel_randid (
  mrb_state *state,
  mrb_value self
) {
  uint64_t value = sapi_rand64();
  char string[21];
  size_t len = sprintf(string, "%" PRIu64, value);
  return mrb_str_new(state, string, len);
}
