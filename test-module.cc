#include "emacs-module-wrapper-template.tcc"
#include <emacs-module.h>
#include <iostream>
#include <optional>

using namespace std;

int plugin_is_GPL_compatible;

int register_elisp_functions();

emacs_value lisp_callable(emacs_env* env, const string s, std::optional<int> i) {
  cout << s << endl;
  cout << i.value() << endl;
  return env->intern(env, "nil");
}

template<typename... Args>
class TD;

EmacsCallable<lisp_callable> c;
int emacs_module_init(struct emacs_runtime *runtime) noexcept {
  c.defineInEmacs(runtime, "emwt-lisp-callable", "Test function", nullptr,
  		  elispCallableFunction<&c>);
  return 0;
}
