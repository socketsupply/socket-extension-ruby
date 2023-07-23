import extension from 'socket:extension'
import os from 'socket:os'

let ruby = null

export async function load () {
  if (!ruby) {
    ruby = await extension.load('ruby')
  }
}

export async function unload () {
  ruby = null
  await extension.unload('ruby')
}

export async function open (filename, options) {
  await load()
  // try loading file from assets or the network first
  if (/android/i.test(os.platform()) || /^(file|https?):/.test(filename)) {
    const response = await fetch(filename, options)
    if (!response.ok) {
      throw new Error(`Failed to open '${filename}': ${response.statusText}`)
    }

    const source = await response.text()
    await evaluate(source)
    return ruby.binding
  }

  const result = await ruby.binding.open({ filename })

  if (result.err) {
    throw new Error(`Failed to open '${filename}'`, {
      cause: result.err
    })
  }

  return ruby.binding
}

export async function evaluate (source, options) {
  await load()
  const result = await ruby.binding.evaluate({ source, ...options })

  if (result.err) {
    throw new Error('Failed to evaluate ruby source', {
      cause: result.err
    })
  }

  return result.data
}

export default {
  open,
  load,
  unload,
  evaluate
}
