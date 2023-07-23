module SocketRuntime
  module JavaScript
    def self.evaluate(source)
      sapi_javascript_evaluate(
        SocketRuntime::Context::global.internal,
        'evaluate',
        "void await (async function () { ;#{source}; }())
          .catch((err) => {
            console.error(
              'SocketRuntime::JavaScript::EvaluateError:',
              err.stack || err
            );
        });"
      )
    end
  end
end
