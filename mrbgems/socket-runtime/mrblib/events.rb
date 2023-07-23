module SocketRuntime::Events
  class EventListener
    attr_reader :id, :type, :once, :called
      @details = nil

    def initialize (type, once, &block)
      @id = block.object_id
      @type = type
      @once = once
      @proc = block.to_proc
      @called = false
    end

    def call (*args)
      if @once and @called
        return false
      end

      begin
        @proc.call(*args)
      rescue => e
        puts "EventListener: UncaughtException: #{e.message}"
        puts '  at ' + e.backtrace.join("\n  at ")
        return false
      ensure
        @called = true
      end

      return true
    end

    def cancel
      @called = true
    end
  end

  class Event
    attr_reader :type, :detail
    def initialize (type, detail)
      @type = type
      @detail = detail
    end
  end

  class EventTarget
    attr_reader :listeners

    def initialize
      @listeners = {}
    end

    def addEventListener (type, once, &block)
      if @listeners[type.to_sym].nil?
        @listeners[type] = []
      end

      @listeners[type].push(EventListener.new(type, once, &block))
    end

    def removeEventListener (type, &block)
      removed = false

      if not @listeners[type].nil?
        @listeners[type].each_with_index do |listener, index|
          if listener.id == block.object_id
            @listeners[type].delete_at(index)
            break
          end
        end
      end

      return removed
    end

    def dispatchEvent (event)
      dispatched = false
      if event.nil? or event.type.nil? or @listeners[event.type].nil?
        return false
      end

      listeners = @listeners[event.type]
      listeners_to_be_deleted = []

      listeners.each_with_index do |listener, index|
        dispatched = true
        listener.call(event)
        if listener.once and listener.called
          listeners_to_be_deleted.push(listener)
        end
      end

      @listeners[event.type] = listeners.delete_if do |listener|
        listeners_to_be_deleted.include?(listener)
      end

      return true
    end

    def on (type, &block)
      addEventListener(type, false, &block)
    end

    def once (type, &block)
      addEventListener(type, true, &block)
    end

    def emit (type, *args)
      dispatchEvent(Event.new(type, *args))
    end
  end
end
