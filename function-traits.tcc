#include <iostream>
#include <optional>

using namespace std;

template<typename F> struct FunctionTraits;

template<int NumberOfParameters,
         int NumberOfOptionalParameters,
         bool AllOptionalTrailing,
         typename FirstParam,
         typename... Args>
struct ParameterTraits;

template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> {
  using RetType = R;
  using ParameterTraits = ParameterTraits<0, 0, true, Args...>;
};

// TODO: add
//   static assert when user instantiates with a function with std::optional<void*> and std::optional<emacs_env*>.
//   static assert when user instantiates with a function with more than 1 void* parameter.
//   static assert when allOptionalParametersTrailing == false.

template<typename T>
struct parameter_provided_by_elisp_caller {
  static constexpr bool value = !(std::is_same<T, emacs_env*>::value || std::is_same<T, void*>::value);
};

// Template class definition for calculating parameter traits.
template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename FirstParam, typename... Args>
struct ParameterTraits
  : ParameterTraits<NumberOfParameters + ((int)(parameter_provided_by_elisp_caller<FirstParam>::value)), // Increment parameters by 1 for FirstParam
                    NumberOfOptionalParameters, // Pass thru number of optional parameters
                    !(NumberOfOptionalParameters > 0), // If NumberOfOptionalParameters > 0,
                                                       // set this value to false, since we're
                                                       // in the specialization that handles
                                                       // non-optional parameters.
                    Args...> {};

// Partial specialization when the parameter we are matching is std::optional<T>.
template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename FirstParameter, typename... Args>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, AllOptionalTrailing, optional<FirstParameter>, Args...>
  : ParameterTraits<NumberOfParameters + 1, NumberOfOptionalParameters + 1, AllOptionalTrailing, Args...> {};

// Base case when last parameter is not optional.
template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, AllOptionalTrailing, LastParameter> {
  static constexpr size_t parameterCount = NumberOfParameters + ((int)(parameter_provided_by_elisp_caller<LastParameter>::value));
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters;
  static constexpr bool allOptionalParametersTrailing = AllOptionalTrailing;
};

// Base case when last parameter is optional.
//
// I tried this without a partial specialization for an optional last
// parameter (instead, I made the test for optional part of the
// calculation of optionalParameterCount in the base case above), but
// then a function with an optional last parameter would match both of
// the previous two specializations (the first one with an empty
// parameter pack for remaining arguments).
template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, AllOptionalTrailing, optional<LastParameter>> {
  static constexpr size_t parameterCount = NumberOfParameters + 1;
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters + 1;
  static constexpr bool allOptionalParametersTrailing = AllOptionalTrailing;
};
