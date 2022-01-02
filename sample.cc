#include "emacs-module-wrapper-template.tcc"

emacs_value func1(emacs_env* env, const std::string& s) {
  cout << "Func1" << endl;
  if (!env) {
    cout << "emacs env is null" << endl;
    return nullptr;
  } else {
    return env->intern(env, "nil");
  }
}

int main(int argc, char* argv[]) {
  createFunctionWrapperForEmacs(std::function<emacs_value(emacs_env*, const std::string&)>(func1), 0).value()(nullptr, 3, nullptr, nullptr);
}
