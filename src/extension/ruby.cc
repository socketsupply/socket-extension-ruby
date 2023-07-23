#include <socket/ruby.h>

namespace Socket::Ruby {
  Mutex mutex;
}

static bool Start (sapi_context_t* context, const void *data) {
  auto state = Socket::Ruby::State::Open();
  Socket::Ruby::IPC::Start(context);

  mrb_define_global_const(
    state,
    "SOCKET_RUNTIME_GLOBAL_CONTEXT",
    Socket::Ruby::Types::GetClassInstance(
      state,
      Socket::Ruby::Types::Context,
      sapi_context_create(context, true)
    )
  );

  mrb_define_global_const(
    state,
    "SOCKET_RUNTIME_EXTENSION_ABI_VERSION_MAJOR",
    mrb_fixnum_value(SOCKET_RUNTIME_EXTENSION_ABI_VERSION_MAJOR)
  );

  mrb_define_global_const(
    state,
    "SOCKET_RUNTIME_EXTENSION_ABI_VERSION_MINOR",
    mrb_fixnum_value(SOCKET_RUNTIME_EXTENSION_ABI_VERSION_MINOR)
  );

  mrb_define_global_const(
    state,
    "SOCKET_RUNTIME_EXTENSION_ABI_VERSION_PATCH",
    mrb_fixnum_value(SOCKET_RUNTIME_EXTENSION_ABI_VERSION_PATCH)
  );

  mrb_define_global_const(
    state,
    "SOCKET_RUNTIME_EXTENSION_ABI_VERSION",
    mrb_fixnum_value(SOCKET_RUNTIME_EXTENSION_ABI_VERSION)
  );

  return true;
}

static bool Stop (sapi_context_t* context, const void *data) {
  Socket::Ruby::IPC::Stop(context);
  Socket::Ruby::State::Close();
  return true;
}

SOCKET_RUNTIME_REGISTER_EXTENSION(
  "ruby",
  Start,
  Stop,
  "Socket Ruby native extension powered by mruby.",
  "mruby-" MRUBY_RUBY_VERSION
);
