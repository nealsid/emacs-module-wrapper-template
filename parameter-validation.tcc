// Template header file for parameter validation functions.

#include <emacs-module.h>
#include <iostream>
#include <optional>
#include <string_view>

using namespace std;

template<typename Param>
struct ValidateParameterFromElisp {
  auto operator()(emacs_env*, emacs_value, Param&) -> void;
};

template<>
struct ValidateParameterFromElisp<string_view> {
  auto operator()(emacs_env* env, emacs_value arg, string_view& output) -> void {
    ptrdiff_t string_length;
    char* argument = nullptr;
    bool ret = env->copy_string_contents(env, arg, nullptr, &string_length);
    if (!ret) {
      cout << "Could not retrieve string length." << endl;
      return;
    }
    argument = new char[string_length]; // Length returned to us from
                                        // Emacs includes null
                                        // terminator.

    if (!argument) {
      return;
    }

    ret = env->copy_string_contents(env, arg, argument, &string_length);

    if (!ret) {
      delete [] argument;
      return;
    }
    // This will construct a string_view over the char* array Emacs
    // gives us. TODO: Look into embedded NULLs in Elisp string
    // parameters.
    string_view o(argument);
    output.swap(o);
  }
};

template<>
struct ValidateParameterFromElisp<int> {
  auto operator()(emacs_env* env, emacs_value arg, int& output) -> void {
    output = env->extract_integer(env, arg);
  }
};

template<typename T>
struct ValidateParameterFromElisp<optional<T>> {
  auto operator()(emacs_env* env, emacs_value arg, optional<T>& output) -> optional<T> {
    T o;
    ValidateParameterFromElisp<T>{}(env, arg, o);
    output.swap(optional<T>(o));
  }
};
