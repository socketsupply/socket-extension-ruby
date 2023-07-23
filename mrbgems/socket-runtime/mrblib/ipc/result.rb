module SocketRuntime::IPC
  class Result
    attr_reader :internal

    def initialize (context, message)
      @internal = sapi_ipc_result_create(context.internal, message.internal)
      @message = message
      @context = context

      @bytes = nil
      @data = nil
      @json = nil
      @error = nil
    end

    def self.reply (context, message)
      result = self.new(context, message)
      result.reply
      result
    end

    def reply
      sapi_ipc_reply(self.internal)
    end

    def source
      @message.name
    end

    def bytes
      @bytes
    end

    def bytes= (bytes)
      if bytes.respond_to?(:to_bytes)
        bytes = bytes.to_bytes
      elsif bytes.respond_to?(:bytes)
        bytes = bytes.bytes
      end

      @bytes = bytes
      sapi_ipc_result_set_bytes(self.internal, @bytes)
    end

    def json
      if @json.is_a? String
        JSON.parse(@json)
      else
        @json
      end
    end

    def json= (json)
      @json = json
      if not json.nil? and not json.empty?
        object = JSON.parse(json)
        if not object['data'].nil? && object['data'].empty?
          @data = object['data']
        end

        if not object['error'].nil? && object['error'].empty?
          @error = object['error']
        end

        object['source'] = self.source
        @json = JSON.stringify(object)
      end

      sapi_ipc_result_set_json(self.internal, @json)
    end

    def data
      if @data.is_a? String
        JSON.parse(@data)
      else
        @data
      end
    end

    def data= (data)
      if data.is_a?(Hash) or data.is_a?(Array)
        data = JSON.stringify(data)
      end

      @data = data
      sapi_ipc_result_set_json_data(self.internal, @data)
    end

    def error
      if @error.is_a? String
        JSON.parse(@error)
      else
        @error
      end
    end

    def error= (error)
      if error.is_a?(Hash) or error.is_a?(Array)
        error = JSON.stringify(error)
      end

      @error = error
      sapi_ipc_result_set_json_error(self.internal, @error)
    end

    def set_header (key, value)
      sapi_ipc_result_set_header(self.internal, key, value)
    end
  end

  class ErrorResult < Result
    def initialize (context, message, error)
      super(context, message)
      self.error = error
    end

    def self.reply (context, message, error)
      result = self.new(context, message, error)
      result.reply
      result
    end
  end

  class DataResult < Result
    def initialize (context, message, data)
      super(context, message)
      self.data = data
    end

    def self.reply (context, message, data)
      result = self.new(context, message, data)
      result.reply
    end
  end

  class JSONResult < Result
    def initialize (context, message, json)
      super(context, message)
      self.json = json
    end

    def self.reply (context, message, json)
      result = self.new(context, message, json)
      result.reply
      result
    end
  end
end
