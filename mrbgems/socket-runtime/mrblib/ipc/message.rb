module SocketRuntime::IPC
  class Message
    attr_reader :internal

    def initialize (internal = nil)
      @internal = internal unless internal.nil?
    end

    def get (key)
      self.internal.get(key)
    end

    def has (key)
      self.internal.has(key)
    end

    def bytes
      self.internal.bytes
    end
  end
end
