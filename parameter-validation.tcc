// Template header file for parameter validation functions.

#include <emacs-module.h>
#include <iostream>
#include <optional>
#include <string_view>

using namespace std;

template<typename A, typename First, typename... Rest>
struct is_type_any_of {
  static constexpr bool value = is_same_v<A, First> || is_type_any_of<A, Rest...>::value;
};

template<typename A, typename B>
struct is_type_any_of<A, B> {
  static constexpr bool value = is_same_v<A, B>;
};

template<typename A, typename... Rest>
inline constexpr bool is_type_any_of_v = is_type_any_of<A, Rest...>::value;

template<typename Param>
struct ValidateParameterFromElisp {
  auto operator()(emacs_env*, emacs_value, void*) -> Param;
};

template<>
struct ValidateParameterFromElisp<emacs_env*> {
  auto operator()(emacs_env* env, emacs_value arg, void* data) -> emacs_env* {
    return env;
  }
};

template<>
struct ValidateParameterFromElisp<void*> {
  auto operator()(emacs_env* env, emacs_value arg, void* data) -> void* {
    return data;
  }
};

template<>
struct ValidateParameterFromElisp<string_view> {
  auto operator()(emacs_env* env, emacs_value arg, void* data) -> string_view {
    ptrdiff_t string_length;
    char* argument = nullptr;
    bool ret = env->copy_string_contents(env, arg, nullptr, &string_length);
    if (!ret) {
      cout << "Could not retrieve string length." << endl;
      return nullptr;
    }
    argument = new char[string_length]; // Length returned to us from
                                        // Emacs includes null
                                        // terminator.

    if (!argument) {
      return nullptr;
    }

    ret = env->copy_string_contents(env, arg, argument, &string_length);

    if (!ret) {
      delete [] argument;
      return nullptr;
    }
    // This will construct a string_view over the char* array Emacs
    // gives us. TODO: Look into embedded NULLs in Elisp string
    // parameters.
    return argument;
  }
};

template<>
struct ValidateParameterFromElisp<int> {
  auto operator()(emacs_env* env, emacs_value arg, void* data) -> int {
    return env->extract_integer(env, arg);
  }
};

template<typename T>
struct ValidateParameterFromElisp<optional<T>> {
  auto operator()(emacs_env* env, emacs_value arg, void* data) -> optional<T> {
    return ValidateParameterFromElisp<T>{}(env, arg);
  }
};
