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

// Alias for function signature that Emacs knows how to call into.
using emacs_function = emacs_value (*)(emacs_env*, ptrdiff_t, emacs_value*, void*) noexcept;

template<typename... Args>
using lisp_callable_type = emacs_value (*)(Args...);

template<typename Param>
auto validate(emacs_env*, emacs_value arg) -> std::optional<Param>;

template<>
auto validate<emacs_env*>(emacs_env* env, emacs_value arg) -> std::optional<emacs_env*> {
  cout << "Validating emacs_env" << endl;
  if (env != nullptr) {
    return env;
  } else {
    return nullopt;
  }
}

template<>
auto validate<string>(emacs_env* env, emacs_value arg) -> std::optional<std::string> {
  cout << "Validating string" << endl;
  ptrdiff_t string_length;
  char* argument = NULL;
  bool ret = env->copy_string_contents(env, arg, NULL, &string_length);
  if (!ret) {
    cout << "Could not retrieve string length." << endl;
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

// // Overload for reference parameter types which removes the reference.
// template<typename FirstParam, typename... Args>
// auto createFunctionWrapperForEmacs(user_function<FirstParam&, Args...> func,
//                                    int argNumber) -> std::optional<std::function<emacs_function>> {
//   return createFunctionWrapperForEmacs((user_function<FirstParam, Args...>) func,
//                                        argNumber);
// }

template<typename F> struct FunctionTraits;


template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)>
{
  using Pointer = R(*)(Args...);
  using RetType = R;
  using ArgTypes = std::tuple<Args...>;
  static constexpr std::size_t ArgCount = sizeof...(Args);
  template<std::size_t N>
  using NthArg = std::tuple_element_t<N, ArgTypes>;
  using FirstArg = NthArg<0>;
  using LastArg = NthArg<ArgCount - 1>;
};

template <typename F>
struct EmacsCallableBase;

template <typename R, typename... Args>
struct EmacsCallableBase<R(*)(Args...)> {
  std::tuple<Args...> unpackedArgs;

  auto unpackArguments(emacs_env *env, ptrdiff_t nargs, emacs_value* args, void* data) noexcept -> void {
    int argNumber = 0;
    // When we generate code to unpack arguments, most of the user
    // function arguments come from the args array that Emacs gives
    // us.  However, two don't: the emacs_env pointer, and the void*
    // user data pointer.  So we have to make sure that we don't
    // increment argNumber or look into the args array when generating
    // code to pass those parameters, and, instead, just pass along
    // whatever Emacs gives us. (the void* pointer is not implemented
    // yet, but will follow the same approach).
    unpackedArgs = {
      (([&] () {
        if constexpr (std::is_same<Args, emacs_env*>::value) {
          return env;
        } else {
          return validate<Args>(env, args[argNumber++]).value();
        }
      }())) ...
    };
  }
};

template <auto F>
struct EmacsCallable : EmacsCallableBase<decltype(F)> {
  using function_traits = FunctionTraits<decltype(F)>;

  auto operator()(emacs_env *env, ptrdiff_t nargs, emacs_value* args, void* data) noexcept -> typename function_traits::RetType {
    this->unpackArguments(env, nargs, args, data);
    return std::apply(F, this->unpackedArgs);
  }
};

template <auto C>
auto elispCallableFunction(emacs_env *env, ptrdiff_t nargs, emacs_value* args, void* data) noexcept -> emacs_value {
  return (*C)(env, nargs, args, data);
}
