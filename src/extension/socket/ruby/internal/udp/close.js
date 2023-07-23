import { emit } from 'socket:ruby/internal/udp/event'

export const url = new URL(import.meta.url)

export const id = url.searchParams.get('id')
export const port = parseInt(url.searchParams.get('port'))
export const address = url.searchParams.get('address')

export default close()

export async function close () {
  const { socket, ready, stopDataListener } = await import(`socket:ruby/internal/udp/socket?id=${id}`)

  await ready
  await stopDataListener()

  socket.close(port, address, async (err) => {
    if (err) {
      return await emit(id, 'error', {
        message: err.message,
        code: err.code ?? null,
        name: err.name,
      })
    }

    await emit(id, 'close')
  })
}
