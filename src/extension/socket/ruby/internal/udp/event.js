import ipc from 'socket:ipc'

export default { emit }

/**
 * Emits an internal UDP event.
 * @param {string} id
 * @param {string} type
 * @param {object} values
 * @param {object?|Uint8Array|ArrayBuffer|import('socket:buffer').Buffer} [data]
 * @return {Promise<import('socket:ipc').Result}
 */
export async function emit (id, type, values, data) {
  const options = { ...values, type, id }
  const method = data ? 'write' : 'request'
  const route = `ruby.internal.udp.event-${id}`
  return await ipc[method](route, options, data)
}
