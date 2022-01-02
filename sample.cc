#include "emacs-module-wrapper-template.tcc"

emacs_value func1(emacs_env* env, emacs_env* env1, emacs_env* env2, emacs_env* env3) {
  cout << "Func1" << endl;
  if (!env) {
    cout << "emacs env is null" << endl;
    return nullptr;
  } else {
    return env->intern(env, "nil");
  }
}

int main(int argc, char* argv[]) {
  createFunctionWrapperForEmacs(std::function<emacs_value(emacs_env*, emacs_env*, emacs_env*, emacs_env*)>(func1), 0)(nullptr, 3, nullptr, nullptr);
}
