#include <socket/ruby.h>

static bool Start (sapi_context_t* context, const void *data) {
  auto state = Socket::Ruby::State::Open();
  Socket::Ruby::IPC::Start(context);
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
