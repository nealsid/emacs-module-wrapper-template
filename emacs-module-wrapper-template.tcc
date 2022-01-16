// This is a library to ease building Emacs modules.  Generally, when
// you register module functions with Emacs, you have to write logic
// to unpack & validate the parameters from an array of emacs_value
// types. This can lead to a lot of boiler plate if you are
// registering several different functions, so this library uses
// templates to generate the wrapper functions that call into the
// function you provide.

#include <iostream>
#include <emacs-module.h>
#include "parameter-traits.tcc"
#include "parameter-validation.tcc"

using namespace std;

// Alias for functions written in modules that are wrapped by this library.
template<typename... Args>
using lisp_callable_type = emacs_value (*)(Args...);

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
  using RetType = R;
  using ParameterTraits = ParameterTraits<0, 0, true, Args...>;

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
    // increment argNumber or look into the args array in the generated
    // code that passes those parameters, and, instead, just pass along
    // whatever Emacs gives us.
    unpackedArgs = {
      (([&] () {
        if constexpr (std::is_same<Args, emacs_env*>::value) {
          return env;
        } else if constexpr (std::is_same<Args, void*>::value) {
          return data;
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
