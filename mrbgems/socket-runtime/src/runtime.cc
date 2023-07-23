#include <map>
#include <string>

// These rely on project-specific, compile-time variables.
namespace SSC {
  using Map = std::map<std::string, std::string>;
  extern bool isDebugEnabled ();
  extern const Map getUserConfig ();
  extern const char* getDevHost ();
  extern int getDevPort ();
}
