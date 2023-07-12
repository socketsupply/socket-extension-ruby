#include <mruby.h>
#include <socket/extension.h>

void mrb_socket_gem_init (mrb_state* state);
void mrb_socket_gem_final (mrb_state* state);

void mrb_socket_gem_init (mrb_state* state) {
  sapi_log(0, "HELLO FROM GEM");
}

void mrb_socket_gem_final (mrb_state* state) {
}
