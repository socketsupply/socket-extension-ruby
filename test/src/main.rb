puts 'hello world'
sapi_ipc_router_map "ruby.hello.world" do |ctx, message, router|
  puts 'in route'
  result = sapi_ipc_result_create(ctx, message)
  sapi_ipc_result_set_bytes result, "hello world from ruby".bytes
  sapi_ipc_reply result
end
