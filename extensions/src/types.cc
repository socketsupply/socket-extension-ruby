#include <socket/ruby.h>

namespace Socket::Ruby::Types {
  struct mrb_data_type Context = {
    "Socket::Context",
    nullptr
  };

  namespace IPC {
    struct mrb_data_type Router = {
      "Socket::IPC::Router",
      nullptr
    };

    struct mrb_data_type Result = {
      "Socket::IPC::Result",
      nullptr
    };

    struct mrb_data_type Message = {
      "Socket::IPC::Message",
      nullptr
    };
  }

  const char** GetNames () {
    static const char *names[] = {
      Context.struct_name,
      IPC::Router.struct_name,
      IPC::Result.struct_name,
      IPC::Message.struct_name,
      nullptr
    };

    return names;
  }

  struct RClass* GetClass (mrb_state *state, struct mrb_data_type &type) {
    auto Class = mrb_define_class(state, type.struct_name, state->object_class);
    MRB_SET_INSTANCE_TT(Class, MRB_TT_DATA);
    return Class;
  }

  mrb_value GetClassInstance (mrb_state *state, struct mrb_data_type &type, void *data) {
    auto Class = GetClass(state, type);
    auto object = mrb_obj_new(state, Class, 0, NULL);
    DATA_PTR(object) = data;
    DATA_TYPE(object) = &type;
    return object;
  }
};
