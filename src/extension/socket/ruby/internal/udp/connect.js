import { emit } from 'socket:ruby/internal/udp/event'

export const url = new URL(import.meta.url)

export const id = url.searchParams.get('id')
export const port = parseInt(url.searchParams.get('port'))
export const address = url.searchParams.get('address')

export default connect()

export async function connect () {
  const { socket, ready } = await import(`socket:ruby/internal/udp/socket?id=${id}`)

  await ready

  socket.connect(port, address, async (err) => {
    if (err) {
      return await emit(id, 'error', {
        message: err.message,
        code: err.code ?? null,
        name: err.name,
      })
    }

    await emit(id, 'connect', socket.address())
  })

  socket.on('error', async (error) => {
    await emit(id, 'error', {
      message: err.message,
      code: err.code ?? null,
      name: err.name,
    })
  })

  socket.on('message', async (message, info) => {
    await emit(id, 'message', info, message)
  })

  socket.on('close', async () => {
    await emit(id, 'close')
  })
}
