// This is a library to ease building Emacs modules.  Generally, when
// you register functions with Emacs, you have to write logic to
// unpack & validate the parameters from an array of emacs_value
// types. This can lead to a lot of boiler plate if you are
// registering several different functions, so this library uses
// templates to generate the wrapper functions that call into the
// function you provide.

#include <iostream>
#include <emacs-module.h>

#include "varargs_bind.tcc"

using namespace std;
using namespace std::placeholders;

// Aliases for the user-defined function meant to be called into from
// emacs.  The function is curried parameter-by-parameter, with
// corresponding validation code generated for the parameter, and the
// corresponding wrapper template function is generated based on
// matching the first parameter type.

// Alias for functions in which the first parameter is emacs_env*.
template<typename... Args>
using first_parameter_emacs_env = std::function<emacs_value(emacs_env*, Args...)>;

// Alias for functions in which the first parameter is const std::string&.
template<typename... Args>
using first_parameter_string = std::function<emacs_value(const std::string&, Args...)>;

// Alias for functions in which the first parameter is int.
template<typename... Args>
using first_parameter_int = std::function<emacs_value(int, Args...)>;

// Alias for type of function that Emacs can call into from Elisp.
using emacs_function_type = emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*);

template<typename... Args>
std::function<emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*)>
createFunctionWrapperForEmacs(first_parameter_emacs_env<Args...> func) {
  cout << "Currying emacs_env" << endl;
  std::function<emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*)> l = [func] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void* data) {
    if (env == nullptr) {
      cout << "Emacs_env was invalid" << endl;
    }
    std::function<emacs_value(Args...)> f = varargs_bind(func, env, make_int_sequence<sizeof...(Args)>{});
    return createFunctionWrapperForEmacs(f)(env, nargs, args, data);
  };
  return l;
}

template<typename... Args>
std::function<emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*)>
createFunctionWrapperForEmacs(first_parameter_string<Args...> func,
                              int argNumber) {
  cout << "hello, emacs 1st param string!" << endl;
  std::function<emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*)> f = [argNumber, func] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void* data) {
    ptrdiff_t string_length;
    char* argument = NULL;
    bool ret = env->copy_string_contents(env, args[argNumber], NULL, &string_length);
    if (!ret) {
      abort();
    }
    argument = (char *)malloc(string_length);

    if (!argument) {
      abort();
    }

    ret = env->copy_string_contents(env, args[argNumber], argument, &string_length);

    if (!ret) {
      free(argument);
      abort();
    }
    std::function<emacs_value(Args...)> curried = std::bind(func, argument, _1);
    return createFunctionWrapperForEmacs(env, curried, argNumber + 1)(env, nargs, args, data);
  };

  return f;
}

std::function<emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*)>
createFunctionWrapperForEmacs(std::function<emacs_value()> func) {
  cout << "Final function generation" << endl;
  auto l = [func] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void*) {
    return func();
  };
  return l;
}
