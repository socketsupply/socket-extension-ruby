import { createSocket } from 'socket:dgram'
import hooks from 'socket:hooks'
import ipc from 'socket:ipc'

import { emit } from 'socket:ruby/internal/udp/event'

export const socket = createSocket({ type: 'udp4', reuseAddr: true })
export const url = new URL(import.meta.url)
export const id = url.searchParams.get('id')

// called on first load
export const ready = emit(id, 'create')

export const stopDataListener = hooks.onData((event) => {
  const result = ipc.Result.from(
    event.detail.data,
    event.detail.err,
    event.detail.params.source,
    Array.isArray(event.detail.headers)
    ? event.detail.headers.join('\n')
    : event.detail.headers
  )

  result.id = event.detail.params.id

  if (id === result.headers.get('x-ruby-socket-id')) {
    if (result.headers.get('x-ruby-socket-action') === 'send') {
      const address = result.headers.get('x-ruby-socket-address')
      const nonce = result.headers.get('x-ruby-socket-nonce')
      const port = parseInt(result.headers.get('x-ruby-socket-port'))

      return socket.send(result.data, 0, result.data.length, port, address, async (err) => {
        if (err) {
          return await emit(id, 'error', {
            message: err.message,
            code: err.code ?? null,
            name: err.name,
          })
        }

        await emit(id, 'send', { nonce })
      })
    }
  }
})

export default socket
