#include <socket/ruby.h>

namespace Socket::Ruby::Kernel {
  static Mutex mutex;

  static mrb_value puts (
    mrb_state *state,
    mrb_value self
  ) {
    mrb_value value;
    char *string = nullptr;

    mrb_get_args(state, "o|", &value);

    if (mrb_type(value) == MRB_TT_ARRAY) {
      for (int i = 0; i < ARY_LEN(mrb_ary_ptr(value)); ++i) {
        string = RSTRING_PTR(mrb_funcall(state, mrb_ary_entry(value, i), "to_s", 0, nullptr));
        if (string != nullptr) {
          sapi_printf(nullptr, "%s", string);
        } else {
          sapi_printf(nullptr, "");
        }
      }
    } else {
      string = RSTRING_PTR(mrb_funcall(state, value, "to_s", 0, nullptr));
      if (string != nullptr) {
        sapi_printf(nullptr, "%s", string);
      } else {
        sapi_printf(nullptr, "");
      }
    }

    return mrb_nil_value();
  }

  void Init (mrb_state* state) {
    Lock lock(mutex);

    mrb_define_method(
      state,
      state->kernel_module,
      "puts",
      Socket::Ruby::Kernel::puts,
      MRB_ARGS_REQ(1)
    );
  }
}
