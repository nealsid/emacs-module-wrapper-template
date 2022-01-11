#include "emacs-module-wrapper-template.tcc"
#include <emacs-module.h>
#include <iostream>

using namespace std;

int plugin_is_GPL_compatible;

int register_elisp_functions();

emacs_value lisp_callable(emacs_env* env, const string s, int i) {
  cout << s << endl;
  cout << i << endl;
  return env->intern(env, "nil");
}

template<typename... Args>
class TD;

EmacsCallable<lisp_callable> c;
int emacs_module_init(struct emacs_runtime *runtime) noexcept {
  emacs_env* env = runtime->get_environment(runtime);
  emacs_value func = env->make_function(env,
                                        2,
                                        2,
					elispCallableFunction<&c>,
                                        "Test function",
                                        nullptr);
  emacs_value symbol = env->intern(env, "emwt-lisp-callable");
  emacs_value args[] = { symbol, func };
  emacs_value defalias = env->intern(env, "defalias");
  env->funcall(env, defalias, 2, args);
  return 0;
}
