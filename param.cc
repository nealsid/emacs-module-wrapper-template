#include <iostream>
#include <tuple>

using namespace std;

int my_func(int a, string s, double d) {
  cout << a << endl;
  return 5;
}

template<typename Ret>
Ret validate() {
  cout << ": " << Ret() << endl;
  return Ret();
}

template<typename... T>
class F;

template<typename R, typename... Args>
auto functionTaker(R (*Function)(Args...)) {
  cout << "Hello, world!" << endl;

  return [] (void *, int nargs, void*, void*) -> std::tuple<Args...> {
    return std::tuple<Args...>{
      (... ,(Args()))
    };
  };
    // cout << std::get<int>(foo) << endl;
    // cout << std::get<string>(foo) << endl;
    // cout << std::get<double>(foo) << endl;
    //    ((cout << std::get<Args>(foo)),...) << endl;

}

template<typename... Args>
void test(Args... args) {
  std::tuple<Args...> foo{
    ([] (auto& a) {
      cout << a << endl;
      return a;
    } (args), ...)
  };


  //  ((cout << std::get<Args, Args...>(foo) << " "),...);
  cout << endl;

}

int main(int argc, char* argv[]) {
  test(5, 7.0, 'a');
  auto t = functionTaker(my_func)(nullptr, 1, nullptr, nullptr);
  cout << std::get<int>(t) << endl;
  cout << std::get<string>(t) << endl;
  cout << std::get<double>(t) << endl;

  return 1;
}
