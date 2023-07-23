import ruby from 'socket:ruby'
import ipc from 'socket:ipc'

globalThis.ruby = ruby

try {
  const binding = await ruby.open('main.rb')

  /*
  const h1 = document.querySelector('h1')
  const result = await binding.hello.world()

  console.log({ result })

  await ruby.evaluate('puts "hello ruby from javascript"')
  await ruby.evaluate(`
    sapi_ipc_router_map "foo.bar" do |ctx, message, router|
      result = sapi_ipc_result_create(ctx, message)
      sapi_ipc_result_set_bytes result, "hello world from rubyssss".bytes
      sapi_ipc_reply result
    end
  `)


  console.log(await ipc.request('foo.bar'))

  if (h1) {
    h1.textContent = result.data
  }
  */
} catch (err) {
  console.log({err})
}
