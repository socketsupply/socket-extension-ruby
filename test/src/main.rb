server = SocketRuntime::UDP::Socket.new
client = SocketRuntime::UDP::Socket.new

server.bind(3000) do |event|
  puts 'bound'
end

server.on('message') do |event|
  puts "server message from #{event.detail[:address]}:#{event.detail[:port]}"
  puts event.detail[:bytes].pack('c*')
  port = event.detail[:port]
  port = event.detail[:port]
  SocketRuntime::Timer.timeout 1000 do
    puts 'on timeout'
    server.send('hello client'.bytes, event.detail[:port], event.detail[:address])
  end
end

client.connect(3000) do |event|
  puts 'connected'
  client.send('hello server'.bytes, 3000)
end

client.on('message') do |event|
  puts "client message from #{event.detail[:address]}:#{event.detail[:port]}"
  puts event.detail[:bytes].pack('c*')
end
