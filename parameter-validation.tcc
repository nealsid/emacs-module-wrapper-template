// Template header file for parameter validation functions.

#include <emacs-module.h>
#include <iostream>
#include <optional>

using namespace std;

template<typename Param>
auto validate(emacs_env*, emacs_value arg) -> optional<Param>;

template<>
auto validate<emacs_env*>(emacs_env* env, emacs_value arg) -> optional<emacs_env*> {
  cout << "Validating emacs_env" << endl;
  if (env != nullptr) {
    return env;
  } else {
    return nullopt;
  }
}

template<>
auto validate<string>(emacs_env* env, emacs_value arg) -> optional<string> {
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

template<>
auto validate<int>(emacs_env* env, emacs_value arg) -> optional<int> {
  return env->extract_integer(env, arg);
}

template<>
auto validate<std::optional<int>>(emacs_env* env, emacs_value arg) -> optional<optional<int>> {
  return env->extract_integer(env, arg);
}
