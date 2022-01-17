#include "emacs-module-wrapper-template.tcc"
#include <emacs-module.h>
#include <iostream>
#include <optional>

using namespace std;

int plugin_is_GPL_compatible;

int register_elisp_functions();

emacs_value lisp_callable(emacs_env* env, const string s, std::optional<int> i, std::optional<int> i2) {
  cout << s << endl;
  if (i) {
    cout << "i set: " << i.value() << endl;
  } else {
    cout << "i not provided" << endl;
  }

  if (i2) {
    cout << "i2 set: " << i2.value() << endl;
  } else {
    cout << "i2 not provided" << endl;
  }
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
