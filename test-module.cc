#include "emacs-module-wrapper-template.tcc"
#include <emacs-module.h>
#include <iostream>
#include <optional>
#include <string_view>

using namespace std;

int plugin_is_GPL_compatible;

int register_elisp_functions();
int counter = 0;

emacs_value lisp_callable(emacs_env* env, string_view s) {
  counter++;
  //  cout << s << endl;
  // if (i) {
  //   cout << "i set: " << i.value() << endl;
  // } else {
  //   cout << "i not provided" << endl;
  // }

  // if (i2) {
  //   cout << "i2 set: " << i2.value() << endl;
  // } else {
  //   cout << "i2 not provided" << endl;
  // }
  return env->intern(env, "nil");
}

template<typename... Args>
class TD;

EmacsCallable<lisp_callable> c;
int emacs_module_init(struct emacs_runtime *runtime) noexcept {
  emacs_funcall_exit result;

  result = c.defineInEmacs(runtime, "emwt-lisp-callable", "Test function", nullptr,
                           elispCallableFunction<&c>);

  if (result != emacs_funcall_exit_return) {
    return 1;
  }
  return 0;
}
