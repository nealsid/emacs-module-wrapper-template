#include <iostream>
#include <optional>

using namespace std;

// template<int N, int J, typename... Args>
// struct ParameterCounterBase {
//   static constexpr int parameterCount = J;
// };

// template <int N, int J, typename FirstParam, typename... Args>
// struct ParameterCounterBase : ParameterCounterBase<N-1, J+1, Args...> {};

// template<typename F>
// struct ParameterCounterBase {};

// template<int ArgCount, int ArgCounter, typename FirstParam, typename... Args>
// struct ParameterCounterIter {};

// template<int ArgCounter>
// struct ParameterCounterIter<0> {};

// template<int ArgCount, int ArgCounter, typename FirstParam, typename...Args>
// struct ParameterCounterIter : ParameterCounterIter<ArgCount - 1, ArgCounter + 1, Args...> {};

// template <typename R, typename... Args>
// struct ParameterCounterBase<R(*)(Args...)> : ParameterCounterIter<sizeof...(Args), 0, Args...> {};

// template<auto F>
// struct ParameterCounter : ParameterCounterBase<decltype(F)> {};

/************** */

template<typename>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template<int CurrentArgCount, int ArgsLeft, typename FirstParam, typename... Args>
struct CountParameterIter : CountParameterIter<CurrentArgCount + 1, ArgsLeft - 1, Args...> {};

template<int CurrentArgCount, int ArgsLeft, typename FirstParam, typename... Args>
struct CountParameterIter<CurrentArgCount, ArgsLeft, std::optional<FirstParam>, Args...> : CountParameterIter<CurrentArgCount + 1, ArgsLeft - 1, Args...> {};

template<int CurrentArgCount, typename LastParam>
struct CountParameterIter<CurrentArgCount, 1, LastParam> {
  static constexpr size_t argCount = CurrentArgCount + 1;
};

template<int CurrentArgCount, typename LastParam>
struct CountParameterIter<CurrentArgCount, 1, std::optional<LastParam>> : CountParameterIter<CurrentArgCount, 1, LastParam> {
  static constexpr size_t optionalParamCount = 1;
};

// template<int CurrentArgCount>
// struct CountParameterIter<CurrentArgCount, 0> {
//   static constexpr size_t argCount = CurrentArgCount;
// };

template<typename F>
struct FunctionTraits {};

template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> {
  static constexpr size_t argSize = sizeof...(Args);
  static constexpr std::tuple optionalParams =
    make_tuple((([] () {
		   return (std::is_same<is_optional<Args>, std::true_type>::value);
		 }) ()) ...);
  static constexpr size_t optionalParameterCount = optionalParams ...
  static constexpr size_t argCount = CountParameterIter<0, sizeof...(Args), Args...>::argCount;
  static constexpr size_t optParamCount = CountParameterIter<0, sizeof...(Args), Args...>::optionalParamCount;
};


void func(int a, std::optional<int> b, std::optional<float> c);

int main(int argc, char* argv[]) {
  cout << FunctionTraits<decltype(&func)>::argCount << endl;
  cout << FunctionTraits<decltype(&func)>::argSize << endl;
  cout << FunctionTraits<decltype(&func)>::optParamCount << endl;
}
