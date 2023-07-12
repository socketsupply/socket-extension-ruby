#ifndef SOCKET_EXTENSION_RUBY
#define SOCKET_EXTENSION_RUBY

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifndef _MSC_VER
#include <strings.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#define _TIMESPEC_DEFINED
#endif

#include <socket/extension.h>

extern "C" {
  #include <mruby.h>
  #include <mruby/array.h>
  #include <mruby/class.h>
  #include <mruby/compile.h>
  #include <mruby/data.h>
  #include <mruby/dump.h>
  #include <mruby/error.h>
  #include <mruby/irep.h>
  #include <mruby/gc.h>
  #include <mruby/hash.h>
  #include <mruby/proc.h>
  #include <mruby/range.h>
  #include <mruby/string.h>
  #include <mruby/throw.h>
  #include <mruby/value.h>
  #include <mruby/variable.h>

  // included last
  #if MRUBY_RELEASE_NO > 30100
  #include <mruby/internal.h>
  #endif


  enum mrb_timezone {
    TZ_NONE = 0
  };

  struct mrb_time {
    time_t sec;
    time_t usec;
    enum mrb_timezone timezone;
    struct tm datetime;
  };
};

#include <mutex>
#include <vector>

#ifdef mrb_range_ptr
#define MRB_RANGE_PTR(state, value) mrb_range_ptr(value)
#else
#define MRB_RANGE_PTR(state, value) mrb_range_ptr(state, value)
#endif

#ifdef MRB_PROC_ENV
# define _MRB_PROC_ENV(p) (p)->e.env
#else
# define _MRB_PROC_ENV(p) (p)->env
#endif

#ifndef MRB_PROC_SET_TARGET_CLASS
#define MRB_PROC_SET_TARGET_CLASS(p,tc)                                        \
  p->target_class = tc
#endif

#define MRUBY_EXC_PROTECT_START(_state)                                        \
  struct mrb_jmpbuf *prev_jmp = _state->jmp;                                   \
  struct mrb_jmpbuf c_jmp;                                                     \
  mrb_value result = mrb_nil_value();                                          \
  MRB_TRY(&c_jmp) {                                                            \
    _state->jmp = &c_jmp;

#define MRUBY_EXC_PROTECT_END(_state)                                          \
    _state->jmp = prev_jmp;                                                    \
  } MRB_CATCH(&c_jmp) {                                                        \
    _state->jmp = prev_jmp;                                                    \
    result = mrb_nil_value();                                                  \
  } MRB_END_EXC(&c_jmp);                                                       \
  mrb_gc_protect(_state, result);                                              \

namespace Socket::Ruby {
  using Mutex = std::recursive_mutex;
  using Lock = std::lock_guard<Mutex>;

  extern Mutex mutex;

  namespace Types {
    extern struct mrb_data_type Context;
    namespace IPC {
      extern struct mrb_data_type Router;
      extern struct mrb_data_type Result;
      extern struct mrb_data_type Message;
    }

    const char** GetNames ();
    struct RClass* GetClass (mrb_state *state, struct mrb_data_type &type);
    mrb_value GetClassInstance (
      mrb_state *state,
      struct mrb_data_type &type,
      void *data
    );
  }

  namespace Proc {
    struct Context {
      struct RProc* proc;
      mrb_state* state;
    };
  }

  namespace State {
    void Close ();
    void Close (mrb_state* state);
    mrb_state* Open ();
    mrb_state* GetRoot ();
    mrb_state* Clone ();
    mrb_state* Clone (mrb_value self);
    mrb_state* Clone (mrb_state* source_state, mrb_value self);
    void Migrate (mrb_state* source_state);
    void Migrate (mrb_state* source_state, mrb_value self);
    void Migrate (mrb_state* source_state, mrb_state* destination_state, mrb_value self);
  }

  namespace Bindings {
    void Init (mrb_state *state);
  }

  namespace Kernel {
    void Init (mrb_state* state);
  }

  namespace IPC {
    void Start (sapi_context_t* context);
    void Stop (sapi_context_t* context);
  }

  namespace Migrate {
    mrb_value MigrateValue (
      mrb_state *source_state,
      mrb_value value,
      mrb_state *destination_state
    );

    mrb_sym MigrateSymbol (
      mrb_state *source_state,
      mrb_sym symbol,
      mrb_state *destination_state
    );

    void MigrateSymbols (
      mrb_state *mrb,
      mrb_state *destination_state
    );

    void MigrateInstanceVariables (
      mrb_state *source_state,
      mrb_value source_value,
      mrb_state *destination_state,
      mrb_value destination_value
    );

    mrb_bool IsSafeMigratableDataType (
      const mrb_data_type *type
    );

    mrb_bool IsSafeMigratableValue (
      mrb_state *source_state,
      mrb_value value,
      mrb_state *destination_state
    );

    void MigrateInternalRepresentationChild (
      mrb_state *source_state,
      mrb_irep *representation,
      mrb_state *destination_state
    );

    mrb_irep* MigrateInternalRepresentation (
      mrb_state *source_state,
      mrb_irep *source_representation,
      mrb_state *destination_state
    );

    struct RProc* MigrateProc (
      mrb_state *source_state,
      struct RProc *source_proc,
      mrb_state *destination_state
    );

    struct RClass* GetClass (
      mrb_state *source_state,
      char const *path_begin,
      mrb_int size
    );
  }
}

#endif
