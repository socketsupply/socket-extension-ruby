#include <socket/ruby.h>
static inline const std::vector<std::string> split (
  const std::string& string,
  const char character
) {
  std::string buffer;
  std::vector<std::string> items;

  for (auto item : string) {
    if (item != character) {
      buffer += item;
    } else if (item == character && buffer != "") {
      items.push_back(buffer);
      buffer = "";
    }
  }

  if (!buffer.empty()) {
    items.push_back(buffer);
  }

  return items;
}

namespace Socket::Ruby::Types {
  struct mrb_data_type Context = {
    "SocketRuntime::Native::Context",
    nullptr
  };

  struct mrb_data_type Timeout = {
    "SocketRuntime::Native::Timer",
    nullptr
  };

  namespace IPC {
    struct mrb_data_type Router = {
      "SocketRuntime::Native::IPC::Router",
      nullptr
    };

    struct mrb_data_type Result = {
      "SocketRuntime::Native::IPC::Result",
      nullptr
    };

    struct mrb_data_type Message = {
      "SocketRuntime::Native::IPC::Message",
      nullptr
    };
  }

  const std::vector<std::string> GetNames () {
    static std::vector<std::string> names = {
      Context.struct_name,
      IPC::Router.struct_name,
      IPC::Result.struct_name,
      IPC::Message.struct_name
    };

    return names;
  }

  struct RClass* GetClass (mrb_state *state, struct mrb_data_type &type) {
    auto Class = mrb_define_class(state, type.struct_name, state->object_class);
    MRB_SET_INSTANCE_TT(Class, MRB_TT_DATA);

    if (std::string(type.struct_name) == "SocketRuntime::Native::IPC::Message") {
      mrb_define_method(
        state,
        Class,
        "name",
        [](auto state, auto self) {
          auto message = (sapi_ipc_message_t*) DATA_PTR(self);
          auto name = sapi_ipc_message_get_name(message);

          if (name == nullptr) {
            return mrb_nil_value();
          }

          return mrb_str_new(state, name, std::string(name).size());
        },
        MRB_ARGS_NONE()
      );

      mrb_define_method(
        state,
        Class,
        "value",
        [](auto state, auto self) {
          auto message = (sapi_ipc_message_t*) DATA_PTR(self);
          auto value = sapi_ipc_message_get_value(message);

          if (value == nullptr) {
            return mrb_nil_value();
          }

          return mrb_str_new(state, value, std::string(value).size());
        },
        MRB_ARGS_NONE()
      );

      mrb_define_method(
        state,
        Class,
        "get",
        [](auto state, auto self) {
          char *key = nullptr;
          mrb_get_args(state, "z", &key);
          auto message = (sapi_ipc_message_t*) DATA_PTR(self);
          auto value = sapi_ipc_message_get(message, key);

          if (value == nullptr) {
            return mrb_nil_value();
          }

          return mrb_str_new(state, value, std::string(value).size());
        },
        MRB_ARGS_REQ(1)
      );

      mrb_define_method(
        state,
        Class,
        "has",
        [](auto state, auto self) {
          char *key = nullptr;
          mrb_get_args(state, "z", &key);
          auto message = (sapi_ipc_message_t*) DATA_PTR(self);
          auto value = sapi_ipc_message_get(message, key);

          if (value == nullptr) {
            return mrb_bool_value(false);
          }

          return mrb_bool_value(true);
        },
        MRB_ARGS_REQ(1)
      );

      mrb_define_method(
        state,
        Class,
        "bytes",
        [](auto state, auto self) {
          auto message = (sapi_ipc_message_t*) DATA_PTR(self);
          auto bytes = sapi_ipc_message_get_bytes(message);
          auto size = sapi_ipc_message_get_bytes_size(message);

          if (bytes == nullptr || size == 0) {
            return mrb_nil_value();
          }

          mrb_value array = mrb_ary_new_capa(state, size);

          for (int i = 0; i < size; ++i) {
            mrb_ary_push(state, array, mrb_fixnum_value(bytes[i]));
          }

          return array;
        },
        MRB_ARGS_REQ(1)
      );
    }

    return Class;
  }

  mrb_value GetClassInstance (mrb_state *state, struct mrb_data_type &type, void *data) {
    auto Class = GetClass(state, type);
    auto object = mrb_obj_new(state, Class, 0, NULL);
    mrb_data_init(object, data, &type);
    DATA_PTR(object) = data;
    DATA_TYPE(object) = &type;
    return object;
  }
};
