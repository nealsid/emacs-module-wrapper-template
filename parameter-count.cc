#include <iostream>
#include <optional>

using namespace std;

template<int NumberOfParameters, int NumberOfOptionalParameters, typename FirstParam, typename... Args>
struct ParameterTraits : ParameterTraits<NumberOfParameters + 1, NumberOfOptionalParameters, Args...> {};

template<int NumberOfParameters, int NumberOfOptionalParameters, typename FirstParameter, typename... Args>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, std::optional<FirstParameter>, Args...>  : ParameterTraits<NumberOfParameters + 1, NumberOfOptionalParameters + 1, Args...> {};

template<int NumberOfParameters, int NumberOfOptionalParameters, typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, LastParameter> {
  static constexpr size_t parameterCount = NumberOfParameters + 1;
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters;
};

// I tried this without a partial specialization for an optional last
// parameter, but the function would match both of the previous two
// specializations (the first one with an empty parameter pack for
// remaining arguments).
template<int NumberOfParameters, int NumberOfOptionalParameters, typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, std::optional<LastParameter>> {
  static constexpr size_t parameterCount = NumberOfParameters + 1;
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters + 1;
};

template<typename F>
struct FunctionTraits {};

template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> {
  static constexpr ParameterTraits<0, 0, Args...> parameterTraits;
};


void func(int a, int b, std::optional<int> c);

int main(int argc, char* argv[]) {
  cout << FunctionTraits<decltype(&func)>::parameterTraits.parameterCount << endl;
  cout << FunctionTraits<decltype(&func)>::parameterTraits.optionalParameterCount << endl;
}
