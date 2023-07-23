#include <socket/ruby.h>

mrb_value socket_runtime_ruby_kernel_puts (
  mrb_state *state,
  mrb_value self
) {
  mrb_value value;
  char *string = NULL;

  mrb_get_args(state, "o|", &value);

  if (mrb_type(value) == MRB_TT_ARRAY) {
    for (int i = 0; i < ARY_LEN(mrb_ary_ptr(value)); ++i) {
      string = RSTRING_PTR(mrb_funcall(state, mrb_ary_entry(value, i), "to_s", 0, NULL));
      if (string != NULL) {
        sapi_printf(NULL, "%s", string);
      } else {
        sapi_printf(NULL, "");
      }
    }
  } else {
    string = RSTRING_PTR(mrb_funcall(state, value, "to_s", 0, NULL));
    if (string != NULL) {
      sapi_printf(NULL, "%s", string);
    } else {
      sapi_printf(NULL, "");
    }
  }

  return mrb_nil_value();
}
