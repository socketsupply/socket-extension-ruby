import extension from 'socket:extension'
import ipc from 'socket:ipc'

const ruby = await extension.load('ruby')

globalThis.ruby = ruby

console.log(await ruby.binding.open({ filename: 'main.rb' }))

console.log(await ipc.request('foo.bar'))
