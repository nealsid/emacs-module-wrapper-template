// This is a library to ease building Emacs modules.  Generally, when
// you register functions with Emacs, you have to write logic to
// unpack & validate the parameters from an array of emacs_value
// types. This can lead to a lot of boiler plate if you are
// registering several different functions, so this library uses
// templates to generate the wrapper functions that call into the
// function you provide.

#include <iostream>
#include <emacs-module.h>
#include <optional>

#include "varargs_bind.tcc"

using namespace std;

// Aliases for the user-defined function meant to be called into from
// emacs.  The function is curried parameter-by-parameter, with
// corresponding validation code generated for the parameter, and the
// corresponding wrapper template function is generated based on
// matching the first parameter type.

template<typename FirstParam, typename... Args>
using user_function = std::function<emacs_value(FirstParam, Args...)>;

// Alias for function signature that Emacs knows how to call into.
using emacs_function = std::function<emacs_value(emacs_env*, ptrdiff_t, emacs_value*, void*) noexcept>;

template<typename Param>
auto validate(emacs_env*, emacs_value arg) -> std::optional<Param>;

template<>
auto validate(emacs_env* env, emacs_value arg) -> std::optional<emacs_env*> {
  if (env != nullptr) {
    return env;
  } else {
    return nullopt;
  }
}

template<>
auto validate(emacs_env* env, emacs_value arg) -> std::optional<std::string> {
    ptrdiff_t string_length;
    char* argument = NULL;
    bool ret = env->copy_string_contents(env, arg, NULL, &string_length);
    if (!ret) {
      return nullptr;
    }
    argument = (char *)malloc(string_length);

    if (!argument) {
      return nullptr;
    }

    ret = env->copy_string_contents(env, arg, argument, &string_length);

    if (!ret) {
      free(argument);
      return nullptr;
    }
    return argument;
}

// Overload for reference parameter types which removes the reference.
template<typename FirstParam, typename... Args>
auto createFunctionWrapperForEmacs(user_function<FirstParam&, Args...> func,
                                   int argNumber) -> std::optional<std::function<emacs_function>> {
  return createFunctionWrapperForEmacs((user_function<FirstParam, Args...>) func,
                                       argNumber);
}

template<typename FirstParam, typename... Args>
auto createFunctionWrapperForEmacs(user_function<FirstParam, Args...> func,
                              int argNumber) -> std::optional<std::function<emacs_function>> {
  cout << "Currying parameter " << argNumber << endl;
  auto l = [func, argNumber] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void* data) -> emacs_value  {
    std::optional<FirstParam> parameterValue = validate<FirstParam>(env, args[argNumber]);
    if (parameterValue) {
      std::function<emacs_value(Args...)> f =
        varargs_bind<decltype(func), FirstParam, sizeof...(Args)>(func, parameterValue.value());
      return createFunctionWrapperForEmacs(f, argNumber + 1).value()(env, nargs, args, data);
    } else {
      cout << "Parameter " << argNumber << endl;
      return env->intern(env, "nil");
    }
  };
  return l;
}

// template<typename... Args>
// std::optional<std::function<emacs_function>>
// createFunctionWrapperForEmacs(first_parameter_emacs_env<Args...> func,
//                               int argNumber) {
//   cout << "Currying emacs_env" << endl;
//   auto l = [func, argNumber] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void* data) -> emacs_value  {
//     if (env == nullptr) {
//       cout << "Emacs_env was invalid" << endl;
//       //      return nullptr;
//     }
//     std::function<emacs_value(Args...)> f = varargs_bind<decltype(func), emacs_env*, sizeof...(Args)>(func, env);
//     return createFunctionWrapperForEmacs(f, argNumber + 1).value()(env, nargs, args, data);
//   };
//   return l;
// }

// template<typename... Args>
// std::optional<std::function<emacs_function>>
// createFunctionWrapperForEmacs(first_parameter_string<Args...> func,
//                               int argNumber) {
//   cout << "hello, emacs 1st param string!" << endl;
//   std::function<emacs_function> f = [argNumber, func] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void* data) -> emacs_value {
//     ptrdiff_t string_length;
//     char* argument = NULL;
//     bool ret = env->copy_string_contents(env, args[argNumber], NULL, &string_length);
//     if (!ret) {
//       return nullptr;
//     }
//     argument = (char *)malloc(string_length);

//     if (!argument) {
//       return nullptr;
//     }

//     ret = env->copy_string_contents(env, args[argNumber], argument, &string_length);

//     if (!ret) {
//       free(argument);
//       return nullptr;
//     }
//     std::function<emacs_value(Args...)> curried = varargs_bind<decltype(func), const std::string&, sizeof...(Args)>(func, argument);
//     return createFunctionWrapperForEmacs(curried, argNumber + 1).value()(env, nargs, args, data);
//   };

//   return f;
// }

std::optional<std::function<emacs_function>>
createFunctionWrapperForEmacs(std::function<emacs_value()> func, int argNumber) {
  cout << "Final function generation" << endl;
  auto l = [func] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void*) {
    return func();
  };
  return l;
}

typedef emacs_value (*ev)(emacs_env*, ptrdiff_t, emacs_value*, void*);
template<std::function<emacs_function>& f>
ev createCPointerTemplateFunction() {
  return [] (emacs_env* env, ptrdiff_t nargs, emacs_value* args, void* data) {
    return f(env, nargs, args, data);
  };
}
