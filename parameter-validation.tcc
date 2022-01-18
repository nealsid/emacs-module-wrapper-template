// Template header file for parameter validation functions.

#include <emacs-module.h>
#include <iostream>
#include <optional>
#include <string_view>

using namespace std;

template<typename Param>
struct ValidateParameterFromElisp {
  auto operator()(emacs_env*, emacs_value) -> Param;
};

template<>
struct ValidateParameterFromElisp<emacs_env*> {
  auto operator()(emacs_env* env, emacs_value arg) -> emacs_env* {
    cout << "Validating emacs_env" << endl;
    if (env != nullptr) {
      return env;
    } else {
      return nullptr;
    }
  }
};

template<>
struct ValidateParameterFromElisp<string_view> {
  auto operator()(emacs_env* env, emacs_value arg) -> string_view {
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
    // This will construct a string_view over the char* array Emacs
    // gives us. TODO: Look into embedded NULLs in Elisp string
    // parameters.
    return argument;
  }
};

template<>
struct ValidateParameterFromElisp<int> {
  auto operator()(emacs_env* env, emacs_value arg) -> int {
    return env->extract_integer(env, arg);
  }
};

template<typename T>
struct ValidateParameterFromElisp<optional<T>> {
  auto operator()(emacs_env* env, emacs_value arg) -> optional<T> {
    return ValidateParameterFromElisp<T>{}(env, arg);
  }
};
