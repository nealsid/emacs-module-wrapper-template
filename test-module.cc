#include "emacs-module-wrapper-template.tcc"
#include <emacs-module.h>
#include <iostream>

using namespace std;

int plugin_is_GPL_compatible;

int register_elisp_functions();

emacs_value lisp_callable(emacs_env* env, const string s) {
  cout << s << endl;
  return env->intern(env, "nil");
}

template<typename... Args>
class TD;

emacs_env_27 e;

int emacs_module_init(struct emacs_runtime *runtime) noexcept {
  emacs_env* env = runtime->get_environment(runtime);
  // auto l =
  //   std::function<emacs_function>(createFunctionWrapperForEmacs(std::function<emacs_value(emacs_env*, const string)>(lisp_callable), 0));
  // cout << l.target_type().name() << endl;

  // if (!l.target<emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*)>()) {
  //   cout << "Could not get function pointer for user function" << endl;
  //   emacs_value ev[10];
  //   l(&e, 10, ev, nullptr);
  //   abort();
  // }
  EmacsCallable<lisp_callable> c;
  emacs_value func = env->make_function(env,
                                        1,
                                        1,
                                        nullptr,
                                        //                                        elispCallableFunction<FunctionTraits<decltype(lisp_callable)>::RetType, FunctionTraits<decltype(lisp_callable)>::ArgTypes, lisp_callable>,
                                        "Test function",
                                        nullptr);
  emacs_value symbol = env->intern(env, "emwt-lisp-callable");
  emacs_value args[] = { symbol, func };
  emacs_value defalias = env->intern(env, "defalias");
  env->funcall(env, defalias, 2, args);
  return 0;
}
