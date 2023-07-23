module SocketRuntime
  ##
  # A {SocketRuntime::Context} represents a high level object for an internal
  # Socket Rutime context object {SocketRuntime::Native::Context} backed by the
  # `sapi_context_t` C type.
  ##
  class Context
    attr_reader :internal

    ##
    # A {SocketRuntime::Context} that is a reference to the global
    # Socket Runtime context object. This method returns a new instance of
    # {SocketRuntime::Context} but ultimately is backed by the same underlying
    # {SocketRuntime::Native::Context} object.
    # @return [SocketRuntime::Context]
    ##
    def self.global
      SocketRuntime::Context.new(SOCKET_RUNTIME_GLOBAL_CONTEXT)
    end

    ##
    # Initiali
    def initialize (internal = nil)
      @internal = internal unless internal.nil?
    end

    def dispatch (&block)
      if block_given?
        sapi_context_dispatch(@internal) do
          begin
            block.call()
          rescue => e
            puts "SocketRuntime::Context.global.dispatch: UncaughtException: #{e.message}"
            puts '  at ' + e.backtrace.join("\n  at ")
          end
        end
      end
    end
  end

  def SocketRuntime.global_context
    SocketRuntime::Context.global
  end

end
