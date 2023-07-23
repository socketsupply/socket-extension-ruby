module SocketRuntime::UDP
  SOCKET_STATE_STATUS_ERROR = -1
  SOCKET_STATE_STATUS_NONE = 0
  SOCKET_STATE_STATUS_INITLIZATING = 1
  SOCKET_STATE_STATUS_INITIALIZED = 2
  SOCKET_STATE_STATUS_CREATING = 3
  SOCKET_STATE_STATUS_CREATED = 4
  SOCKET_STATE_STATUS_BINDING = 5
  SOCKET_STATE_STATUS_BOUND = 6
  SOCKET_STATE_STATUS_CONNECTING = 7
  SOCKET_STATE_STATUS_CONNECTED = 8

  class SocketState
    attr_accessor :address, :port, :status

    def initialize
      @address = nil
      @port = nil
      @status = SOCKET_STATE_STATUS_NONE
    end
  end

  class Socket < SocketRuntime::Events::EventTarget
    attr_reader :id, :state, :router

    def initialize
      super()

      this = self

      @router = SocketRuntime::IPC::Router.new()
      @state = SocketState.new()
      @id = randid()

      @state.status = SOCKET_STATE_STATUS_INITLIZATING

      # create socket in runtime on next loop
      SocketRuntime::Context.global.dispatch do
        this.state.status = SOCKET_STATE_STATUS_INITIALIZED
        this.create()
      end

      @router.map("ruby.internal.udp.event-#{id}") do |context, message|
        id = message.get('id')
        type = message.get('type')
        detail = { id: id }
        bytes = nil

        if id == this.id and not type.nil?
          if message.has('address')
            detail[:address] = message.get('address')
          end

          if message.has('port')
            detail[:port] = message.get('port').to_i
          end

          case type
          when 'create'
            this.state.status = SOCKET_STATE_STATUS_CREATED
          when 'bind'
            this.state.status = SOCKET_STATE_STATUS_BOUND
            this.state.address = detail[:address]
            this.state.port = detail[:port]
          when 'connect'
            this.state.status = SOCKET_STATE_STATUS_CONNECTED
            this.state.address = detail[:address]
            this.state.port = detail[:port]
          when 'error'
            this.state.status = SOCKET_STATE_STATUS_ERROR
          when 'message'
            detail[:bytes] = message.bytes
          when 'send'
            type = "send-#{message.get('nonce')}"
          when 'close'
            SocketRuntime::Context.global.dispatch do
              this.router.unmap("ruby.internal.udp.event-#{id}")
            end
          end

          SocketRuntime::Context.global.dispatch do
            this.emit(type, detail)
          end
        end

        SocketRuntime::IPC::Result.reply(context, message)
      end
    end

    def address
      {
        address: @state.address,
        port: @state.port
      }
    end

    def create
      if @state.status == SOCKET_STATE_STATUS_INITIALIZED
        @state.status = SOCKET_STATE_STATUS_CREATING
        SocketRuntime::JavaScript::evaluate "
          await import('socket:ruby/internal/udp/socket?id=#{self.id}')
        "
      end
    end

    def bind (port = 0, address = '0.0.0.0', &block)
      this = self

      # return early if already bound, just dispatch callback
      if @state.status == SOCKET_STATE_STATUS_BOUND
        if block_given?
          SocketRuntime::Context.global.dispatch(&block)
        end
        return
      end

      if block_given?
        self.once('bind', &block)
      end

      # retur early if already binding
      if @state.status == SOCKET_STATE_STATUS_BINDING
        return
      end

      javascript = "
        await import('socket:ruby/internal/udp/bind?id=#{self.id}&address=#{address}&port=#{port}')
      "

      if @state.status < SOCKET_STATE_STATUS_BINDING
        self.once('create') do
          this.state.status = SOCKET_STATE_STATUS_BINDING
          SocketRuntime::JavaScript::evaluate(javascript)
        end
      elsif @state.status == SOCKET_STATE_STATUS_CREATED
        SocketRuntime::JavaScript::evaluate(javascript)
      end
    end

    def connect (port, address = '0.0.0.0', &block)
      this = self
      if @state.status == SOCKET_STATE_STATUS_BOUND or @state.status == SOCKET_STATE_STATUS_BINDING
        raise 'Socket is already bound'
      end

      if block_given?
        self.once('connect', &block)
      end

      # retur early if already connectiing
      if @state.status == SOCKET_STATE_STATUS_CONNECTING
        return
      end

      javascript = "
        await import('socket:ruby/internal/udp/connect?id=#{self.id}&address=#{address}&port=#{port}')
      "

      if @state.status < SOCKET_STATE_STATUS_CONNECTED
        self.once('create') do
          this.state.status = SOCKET_STATE_STATUS_CONNECTING
          SocketRuntime::JavaScript::evaluate(javascript)
        end
      elsif @state.status == SOCKET_STATE_STATUS_CREATED
        SocketRuntime::JavaScript::evaluate(javascript)
      end
    end

    def send (bytes, port = nil, address = nil, &block)
      this = self
      nonce = randid()

      if block_given?
        self.once("send-#{nonce}", &block)
      end

      if @state.status == SOCKET_STATE_STATUS_CONNECTED
        if port.nil? or port == 0
          port = @state.port
        end

        if address.nil?
          address = @state.address
        end
      else
        if port.nil?
          raise 'Port should be > 0 and < 65536. Received nil'
        end

        if address.nil?
          address = '0.0.0.0'
        end
      end
      SocketRuntime::Context.global.dispatch do
        this.router.send(bytes, "
          x-ruby-socket-id: #{this.id}
          x-ruby-socket-action: send
          x-ruby-socket-nonce: #{nonce}
          x-ruby-socket-port: #{port}
          x-ruby-socket-address: #{address}
        ")
      end
    end

    def close
      if block_given?
        self.once('close', &block)
      end

      javascript = "
        await import('socket:ruby/internal/udp/close?id=#{self.id}')
      "

      SocketRuntime::JavaScript::evaluate(javascript)
    end
  end
end
