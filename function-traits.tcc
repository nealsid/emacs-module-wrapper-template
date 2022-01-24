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

static_assert((int)true == 1, "Boolean 'true' did not cast to 1");

template<typename Param>
struct is_optional_type {
  static constexpr bool value = false;
};

template<typename T>
struct is_optional_type<optional<T>> {
  static constexpr bool value = true;
};

// TODO: add
//   static assert when user instantiates with a function with std::optional<void*> and std::optional<emacs_env*>.
//   static assert when user instantiates with a function with more than 1 void* parameter.

template<typename T>
struct parameter_provided_by_elisp_caller {
  static constexpr bool value = !(std::is_same<T, emacs_env*>::value || std::is_same<T, void*>::value);
};

// Template class definition for calculating parameter traits.
template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename FirstParam, typename... Args>
struct ParameterTraits
  : ParameterTraits<NumberOfParameters + ((int)(parameter_provided_by_elisp_caller<FirstParam>::value)), // Increment parameters by 1 for FirstParam
                    NumberOfOptionalParameters + ((int)is_optional_type<FirstParam>::value), // Pass thru number of optional parameters
                    AllOptionalTrailing &&
                    (NumberOfOptionalParameters > 0 && is_optional_type<FirstParam>::value) ||
                      NumberOfOptionalParameters == 0, // If NumberOfOptionalParameters > 0,
                                                       // and the current parameter is optional,
                                                       // set this value to true.  If there are no optional parameters, set it true.
                    Args...> {};

// Base case.
template<int NumberOfParameters, int NumberOfOptionalParameters, bool AllOptionalTrailing, typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, AllOptionalTrailing, LastParameter> {
  static constexpr size_t parameterCount = NumberOfParameters + ((int)(parameter_provided_by_elisp_caller<LastParameter>::value));
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters + ((int)is_optional_type<LastParameter>::value);
  static constexpr bool allOptionalParametersTrailing = AllOptionalTrailing && ((NumberOfOptionalParameters > 0 && is_optional_type<LastParameter>::value ) || (NumberOfOptionalParameters == 0));
};
