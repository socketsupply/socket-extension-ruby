module SocketRuntime
  def SocketRuntime.__timer_loop (timer)
    SocketRuntime::Context.global.dispatch do
      if not timer.stopped? and not timer.cancelled?
        begin
          now = Time.now().to_f * 1000
          timer.state.timing.elapsed = now - timer.state.timing.started

          if timer.state.timing.invoked.nil?
            if timer.state.timing.elapsed >= timer.state.timeout
              timer.state.timing.invoked = Time.now().to_f * 1000
              timer.call()
            end
          else
            delta = now - timer.state.timing.invoked
            if delta >= timer.state.timeout
              timer.state.timing.invoked = Time.now().to_f * 1000
              timer.call()
            end
          end
        rescue => e
          puts "Timer: UncaughtException: #{e.message}"
          puts '  at ' + e.backtrace.join("\n  at ")
          return
        end

        SocketRuntime.__timer_loop(timer)
      end
    end
  end

  class TimerTiming
    attr_accessor :started, :stopped, :cancelled, :elapsed, :invoked

    def initialize
      @started = 0
      @stopped = 0
      @cancelled = 0
      @elapsed = 0
      @invoked = nil
    end
  end

  class TimerState
    attr_accessor :started, :stopped, :cancelled
    attr_reader :timeout, :repeat, :timing

    def initialize (timeout = 0, repeat = false)
      @cancelled = false
      @stopped = false
      @started = false

      @timeout = timeout
      @repeat = repeat

      @timing = TimerTiming.new()
    end
  end

  class Timer
    attr_reader :state

    def self.timeout (timeout, &block)
      Timer.new(timeout: timeout, repeat: false, autostart: true, &block)
    end

    def self.interval (timeout, &block)
      Timer.new(timeout: timeout, repeat: true, autostart: true, &block)
    end

    def initialize (timeout: 0, repeat: false, autostart: false, &block)
      @state = TimerState.new(timeout, repeat)

      if block_given?
        @proc = block.to_proc
      end

      if autostart
        start
      end
    end

    def cancelled?
      @state.cancelled
    end

    def started?
      @state.started
    end

    def stopped?
      @state.stopped
    end

    def call (*args)
      if not stopped? and not cancelled?
        if not @proc.nil?
          @proc.call(*args)
          if not @state.repeat
            stop
          end
          return true
        end
      end

      return false
    end

    def start
      if not cancelled? and not started?
        @state.started = true
        @state.timing.started = Time.now().to_f * 1000
        SocketRuntime.__timer_loop(self)
      end
    end

    def stop
      if not cancelled? and not stopped?
        @state.stopped = true
        @state.started = false
        @state.timing.stopped = Time.now().to_f * 1000
      end
    end

    def cancel
      if not cancelled?
        @state.cancelled = true
        @state.stopped = true
        @state.started = false
        @state.timing.cancelled = Time.now().to_f * 1000
      end
    end
  end
end
