module SocketRuntime::IPC
  class Router
    attr_reader :internal

    def initialize
      @internal = nil
    end

    def map (route, &block)
      sapi_ipc_router_map(route) do |native_context, native_message|
        context = SocketRuntime::Context.new(native_context)
        message = SocketRuntime::IPC::Message.new(native_message)

        begin
          block.call(context, message)
        rescue Exception => block_error
          begin
            SocketRuntime::IPC::ErrorResult.reply(context, message, {
              message: block_error.message,
              stack: block_error.backtrace.join('\n')
            })
          rescue Exception => rescue_error
            puts "sapi_ipc_router_map: InternalException: #{rescue_error.message}"
            puts '  at ' + rescue_error.backtrace.join("\n  at ")
            result = sapi_ipc_result_create(context, message)
            sapi_ipc_result_set_json_error(result.message)
            sapi_ipc_reply(result)
          end
        end
      end
    end

    def unmap (route)
      sapi_ipc_router_unmap(route)
    end

    def send (value, headers = '')
      if value.is_a?(Array)
        sapi_ipc_send_bytes(
          SocketRuntime::Context.global.internal,
          nil,
          value,
          headers
        )
      else
        sapi_ipc_send_json(
          SocketRuntime::Context.global.internal,
          nil,
          value
        )
      end
    end
  end
end
