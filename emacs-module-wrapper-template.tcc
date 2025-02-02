// This is a library to ease building Emacs modules.  Generally, when
// you register module functions with Emacs, you have to write logic
// to unpack & validate the parameters from an array of emacs_value
// types. This can lead to a lot of boiler plate if you are
// registering several different functions, so this library uses
// templates to generate the wrapper functions that call into the
// function you provide.

#ifndef __EMACS_MODULE_WRAPPER_TEMPLATE__
#define __EMACS_MODULE_WRAPPER_TEMPLATE__

#include <array>
#include <iostream>
#include <vector>

#include <emacs-module.h>
#include <os/log.h>
#include <os/signpost.h>

#include "function-traits.tcc"
#include "parameter-validation.tcc"

using namespace std;

os_log_t logger = os_log_create("com.nealsid.emacs.emwt", OS_LOG_CATEGORY_POINTS_OF_INTEREST);

template<typename T>
struct TD;

template <typename... Args>
struct generate_argument_indices {
  static int parameterIndex;
  static constexpr initializer_list<int> values{
      ([] () {
        if constexpr (is_same_v<Args, emacs_env*>) {
          return -1;
        } else {
          return 0;
        }
      }()) ...
    };
};

template<typename... Args>
int generate_argument_indices<Args...>::parameterIndex = 0;

template<typename... Args>
inline constexpr auto generate_argument_indices_vs = generate_argument_indices<Args...>::values;

template <typename F>
struct EmacsCallableBase;

template <typename R, typename... Args>
struct EmacsCallableBase<R(*)(Args...)> {
  tuple<Args...> unpackedArgs;
  vector<char *> pointersToDelete;
  vector<int> argument_indices{generate_argument_indices_vs<Args...>};

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
        if (argNumber < nargs) {
          auto ret = ValidateParameterFromElisp<Args>{}(env, args[argNumber], data);
          if (env->non_local_exit_check(env) != emacs_funcall_exit_return) {
            return Args();
          }
          cout << argument_indices[argNumber] << "\t";
          argNumber += (int)(!is_type_any_of_v<Args, void*, emacs_env*>);

          // If argNumber < nargs and this parameter is optional,
          // it has to be specified.
          if constexpr (is_optional_type_v<Args>) {
            assert(ret);
          }

          if constexpr (is_same_v<Args, string_view>) {
            if (ret.data() != nullptr) {
              pointersToDelete.push_back(const_cast<char*>(ret.data()));
            }
          }

          if constexpr (is_same_v<Args, optional<string_view>>) {
            pointersToDelete.push_back(const_cast<char*>(ret.value().data()));
          }
          return ret;
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

      } ())) ...
    };
    cout << "Hello" <<  endl;
  }

  auto cleanup() -> void {
    for (auto ptr : pointersToDelete) {
      delete [] ptr;
    }
    pointersToDelete.clear();
  }
};

template <auto F>
struct EmacsCallable : EmacsCallableBase<decltype(F)> {
  using function_traits = FunctionTraits<decltype(F)>;
  using parameter_traits = typename function_traits::ParameterTraits;

  static constexpr size_t requiredParameterCount =
    parameter_traits::parameterCount - parameter_traits::optionalParameterCount;
  static_assert(parameter_traits::allOptionalParametersTrailing, "Optional parameters must be trailing");

  emacs_funcall_exit defineInEmacs(struct emacs_runtime *runtime, const char* lisp_function_name,
                                   const char* documentation, void* data,
                                   emacs_value (*fn)(emacs_env*, ptrdiff_t, emacs_value*, void*) noexcept) {
    emacs_env* env = runtime->get_environment(runtime);
    emacs_value func = env->make_function(env,
                                          requiredParameterCount,
                                          parameter_traits::parameterCount,
                                          fn,
                                          documentation,
                                          data);
    emacs_value symbol = env->intern(env, lisp_function_name);
    emacs_value args[] = { symbol, func };
    emacs_value defalias = env->intern(env, "defalias");
    env->funcall(env, defalias, 2, args);

    return env->non_local_exit_check(env);
  }

  auto operator()(emacs_env *env, ptrdiff_t nargs, emacs_value* args, void* data) noexcept -> typename function_traits::RetType {
    os_signpost_interval_begin(logger, OS_SIGNPOST_ID_EXCLUSIVE, "Function call");
    this->unpackArguments(env, nargs, args, data);
    if (env->non_local_exit_check(env) != emacs_funcall_exit_return) {
      return nullptr;
    }
    auto x = std::apply(F, this->unpackedArgs);
    this->cleanup();
    os_signpost_interval_end(logger, OS_SIGNPOST_ID_EXCLUSIVE, "Function call");
    return x;
  }
};

template <auto C>
auto elispCallableFunction(emacs_env *env, ptrdiff_t nargs, emacs_value* args, void* data) noexcept -> emacs_value {
  return (*C)(env, nargs, args, data);
}

#endif  // __EMACS_MODULE_WRAPPER_TEMPLATE__
