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
	  if (argNumber < nargs) {
	    return validate<Args>(env, args[argNumber++]).value();
	  } else {
	    return Args(); // This is a little sketchy, but we only
			   // get here at runtime when the argument
			   // type is optional<T> and the argument has
			   // not been passed by the elisp caller.  A
			   // default-constructed optional to
			   // represent an unset argument is what we
			   // require.  TODO: There are potential
			   // extra copies here to look into remove.
	  }
        }
      }())) ...
    };
  }
};

template <auto F>
struct EmacsCallable : EmacsCallableBase<decltype(F)> {
  using function_traits = FunctionTraits<decltype(F)>;

  void defineInEmacs(struct emacs_runtime *runtime, const char* lisp_function_name, const char* documentation, void* data,
  		     emacs_value (*fn)(emacs_env*, ptrdiff_t, emacs_value*, void*) noexcept) {
    emacs_env* env = runtime->get_environment(runtime);
    size_t requiredParameterCount = function_traits::ParameterTraits::parameterCount - function_traits::ParameterTraits::optionalParameterCount;
    emacs_value func = env->make_function(env,
  					  requiredParameterCount,
  					  function_traits::ParameterTraits::parameterCount,
  					  fn,
  					  documentation,
  					  data);
    emacs_value symbol = env->intern(env, lisp_function_name);
    emacs_value args[] = { symbol, func };
    emacs_value defalias = env->intern(env, "defalias");
    env->funcall(env, defalias, 2, args);
  }

  auto operator()(emacs_env *env, ptrdiff_t nargs, emacs_value* args, void* data) noexcept -> typename function_traits::RetType {
    this->unpackArguments(env, nargs, args, data);
    return std::apply(F, this->unpackedArgs);
  }
};

template <auto C>
auto elispCallableFunction(emacs_env *env, ptrdiff_t nargs, emacs_value* args, void* data) noexcept -> emacs_value {
  return (*C)(env, nargs, args, data);
}
