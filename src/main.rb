class Hello
  def world
    "hello world"
  end
end

def main(ctx)
  puts ctx
  #sapi_ipc_router_map ctx, "foo.bar", &method(:onfoo)
  sapi_ipc_router_map ctx, "foo.bar" do |ctx, ctx2|
    puts "foo"
  end

  hello = Hello.new
  hello.world
  JSON.stringify({"foo"=>"bar"})
  Time.now.to_s
end
