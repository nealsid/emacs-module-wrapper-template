#include <iostream>
#include <optional>

using namespace std;

template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename FirstParam, typename... Args>
struct ParameterTraits
  : ParameterTraits<NumberOfParameters + 1, // Increment parameters by 1 for FirstParam
                    NumberOfOptionalParameters, // Pass thru number of optional parameters
                    !(NumberOfOptionalParameters > 0), // If NumberOfOptionalParameters > 0,
                                                       // set this value to false, since we're
                                                       // in the specialization that handles
                                                       // non-optional parameters.
                    Args...> {};

template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename FirstParameter, typename... Args>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, AllOptionalTrailing, optional<FirstParameter>, Args...>
  : ParameterTraits<NumberOfParameters + 1, NumberOfOptionalParameters + 1, AllOptionalTrailing, Args...> {};

template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, AllOptionalTrailing, LastParameter> {
  static constexpr size_t parameterCount = NumberOfParameters + 1;
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters;
  static constexpr bool allOptionalParametersTrailing = AllOptionalTrailing;
};

// I tried this without a partial specialization for an optional last
// parameter (instead, I made the test for optional part of the
// calculation of optionalParameterCount), but then a function would
// match both of the previous two specializations (the first one with
// an empty parameter pack for remaining arguments).
template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, AllOptionalTrailing, optional<LastParameter>> {
  static constexpr size_t parameterCount = NumberOfParameters + 1;
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters + 1;
  static constexpr bool allOptionalParametersTrailing = AllOptionalTrailing;
};

template<typename F>
struct FunctionTraits {};

template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> {
  static constexpr ParameterTraits<0, 0, true, Args...> parameterTraits;
};

void func(int a, int b, optional<int> c, optional<int> d);

int main(int argc, char* argv[]) {
  cout << FunctionTraits<decltype(&func)>::parameterTraits.parameterCount << endl;
  cout << FunctionTraits<decltype(&func)>::parameterTraits.optionalParameterCount << endl;
  cout << FunctionTraits<decltype(&func)>::parameterTraits.allOptionalParametersTrailing << endl;
}
