#include <socket/ruby.h>
#include <uv.h>

namespace Socket::Ruby::IPC {
  namespace Routes {
    void Open (
      sapi_context_t* context,
      sapi_ipc_message_t* message,
      const sapi_ipc_router_t* router
    ) {
      Lock lock(Socket::Ruby::mutex);

      auto result = sapi_ipc_result_create(context, message);
      auto filename = sapi_ipc_message_get(message, "filename");
      auto entry = sapi_ipc_message_get(message, "entry");

      if (filename == nullptr) {
        auto error = sapi_json_object_create(context);
        auto message = sapi_json_string_create(context, "Missing 'filename'");
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      uv_fs_t req;
      int status = 0;

      if ((status = uv_fs_stat(uv_default_loop(), &req, filename, nullptr))) {
        auto error = sapi_json_object_create(context);
        auto syscall = sapi_json_string_create(context, "stat");
        auto message = sapi_json_string_create(context, uv_strerror(status));
        sapi_json_object_set(error, "syscall", sapi_json_any(syscall));
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      auto stats = req.statbuf;
      memset(&req, 0, sizeof(uv_fs_t));

      if ((status = uv_fs_open(uv_default_loop(), &req, filename, UV_FS_O_RDONLY, 0x1B6, nullptr)) < 0) {
        auto error = sapi_json_object_create(context);
        auto syscall = sapi_json_string_create(context, "open");
        auto message = sapi_json_string_create(context, uv_strerror(status));
        sapi_json_object_set(error, "syscall", sapi_json_any(syscall));
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      auto fd = req.result;
      memset(&req, 0, sizeof(uv_fs_t));

      if (stats.st_size == 0) {
        if ((status = uv_fs_close(uv_default_loop(), &req, fd, nullptr)) < 0) {
          auto error = sapi_json_object_create(context);
          auto syscall = sapi_json_string_create(context, "close");
          auto message = sapi_json_string_create(context, uv_strerror(status));
          sapi_json_object_set(error, "syscall", sapi_json_any(syscall));
          sapi_json_object_set(error, "message", sapi_json_any(message));
          sapi_ipc_result_set_json_error(result, sapi_json_any(error));
          sapi_ipc_reply(result);
          return;
        }

        auto error = sapi_json_object_create(context);
        auto message = sapi_json_string_create(context, "Empty file");
        sapi_json_object_set(error, "syscall", sapi_json_any(syscall));
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      auto buffer = uv_buf_t { new char[stats.st_size + 1]{0}, stats.st_size };
      memset(&req, 0, sizeof(uv_fs_t));

      if ((status = uv_fs_read(uv_default_loop(), &req, fd, &buffer, 1, 0, nullptr)) < 0) {
        delete buffer.base;

        auto error = sapi_json_object_create(context);
        auto syscall = sapi_json_string_create(context, "read");
        auto message = sapi_json_string_create(context, uv_strerror(status));
        sapi_json_object_set(error, "syscall", sapi_json_any(message));
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      auto state = State::GetRoot();

      if (state == nullptr) {
        auto error = sapi_json_object_create(context);
        auto message = sapi_json_string_create(context, "Failed to clone root ruby state.");
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      DATA_PTR(mrb_obj_value(state->top_self)) = context;

      auto ctx = mrbc_context_new(state);
      auto arena = mrb_gc_arena_save(state);

      mrbc_filename(state, ctx, filename);
      auto wrappedSource = std::string(
        std::string("begin ") +
        std::string(buffer.base, buffer.len) + "\n" +
        "rescue => e\n" +
        "  puts \"Error: #{e.message}\"\n" +
        "  puts '  at ' + e.backtrace.join(\"\\n  at \")\n" +
        "end"
      );

      auto program = mrb_load_nstring_cxt(
        state,
        wrappedSource.data(),
        wrappedSource.size(),
        ctx
      );

      delete buffer.base;

      if (entry) {
        auto programContext = Types::GetClassInstance(
          state,
          Types::Context,
          context
        );

        mrb_funcall_argv(
          state,
          program,
          mrb_intern_static(state, entry, strlen(entry)),
          1,
          &programContext
        );
      }

      mrb_gc_arena_restore(state, arena);
      memset(&req, 0, sizeof(uv_fs_t));

      mrbc_context_free(state, ctx);

      if ((status = uv_fs_close(uv_default_loop(), &req, fd, nullptr)) < 0) {
        auto error = sapi_json_object_create(context);
        auto syscall = sapi_json_string_create(context, "close");
        auto message = sapi_json_string_create(context, uv_strerror(status));
        sapi_json_object_set(error, "syscall", sapi_json_any(syscall));
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      sapi_ipc_reply(result);
    }

    void Evaluate (
      sapi_context_t* context,
      sapi_ipc_message_t* message,
      const sapi_ipc_router_t* router
    ) {
      Lock lock(Socket::Ruby::mutex);

      auto filename = sapi_ipc_message_get(message, "filename");
      auto source = sapi_ipc_message_get(message, "source");
      auto result = sapi_ipc_result_create(context, message);

      if (source == nullptr) {
        auto error = sapi_json_object_create(context);
        auto message = sapi_json_string_create(context, "Missing 'source'");
        sapi_json_object_set(error, "message", sapi_json_any(message));
        sapi_ipc_result_set_json_error(result, sapi_json_any(error));
        sapi_ipc_reply(result);
        return;
      }

      auto state = State::GetRoot();
      DATA_PTR(mrb_obj_value(state->top_self)) = context;
      auto ctx = mrbc_context_new(state);

      mrbc_filename(state, ctx, filename);

      auto wrappedSource = std::string(
        std::string("begin ") +
        source + "\n" +
        "rescue => e\n" +
        "  puts \"Error: #{e.message}\"\n" +
        "  puts '  at ' + e.backtrace.join(\"\\n  at \")\n" +
        "end"
      );

      auto arena = mrb_gc_arena_save(state);
      auto value = mrb_load_nstring_cxt(
        state,
        wrappedSource.data(),
        wrappedSource.size(),
        ctx
      );

      if (!mrb_nil_p(value)) {
        auto string = RSTRING_PTR(mrb_funcall(state, value, "to_s", 0, nullptr));
        if (string != nullptr) {
          sapi_ipc_result_set_json_data(result, sapi_json_any(
            sapi_json_string_create(context, string)
          ));
        }
      }

      mrb_gc_arena_restore(state, arena);
      mrbc_context_free(state, ctx);
      sapi_ipc_reply(result);
    }
  }

  void Start (sapi_context_t* context) {
    sapi_ipc_router_map(context, "ruby.open", Routes::Open, nullptr);
    sapi_ipc_router_map(context, "ruby.evaluate", Routes::Evaluate, nullptr);
  }

  void Stop (sapi_context_t* context) {
  }
}
