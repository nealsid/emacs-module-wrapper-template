#ifndef __FUNCTION_TRAITS__
#define __FUNCTION_TRAITS__

#include <iostream>
#include <optional>

#include <emacs-module.h>

using namespace std;

template<typename F> struct FunctionTraits;

template<uint8_t NumberOfParameters,
         uint8_t NumberOfOptionalParameters,
         uint8_t NumberParametersRequireDeallocation,
         bool AllOptionalTrailing,
         typename FirstParam,
         typename... Args>
struct ParameterTraits;

template<typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> {
  using RetType = R;
  using ParameterTraits = ParameterTraits<0, 0, 0, true, Args...>;
};

static_assert((uint8_t)true == 1, "Boolean 'true' did not cast to 1");

template<typename T>
struct is_optional_type {
  static constexpr bool value = false;
};

template<typename T>
struct is_optional_type<optional<T>> {
  static constexpr bool value = true;
};

template<typename T>
inline constexpr bool is_optional_type_v = is_optional_type<T>::value;

// TODO: add
//   static assert when user instantiates with a function with std::optional<void*> and std::optional<emacs_env*>.
//   static assert when user instantiates with a function with more than 1 void* parameter.

template<typename T>
struct parameter_provided_by_elisp_caller {
  static constexpr bool value = !(is_same_v<T, emacs_env*> || is_same_v<T, void*>);
};

template<typename T>
inline constexpr bool parameter_provided_by_elisp_caller_v =
  parameter_provided_by_elisp_caller<T>::value;

template<typename T>
struct remove_optional {
  using type = T;
};

template<typename T>
using remove_optional_t = typename remove_optional<T>::type;

template<typename T>
struct remove_optional<optional<T>> {
  using type = remove_optional_t<T>;
};

template<typename T>
struct parameter_requires_deallocation {
  static constexpr bool value =
    is_same_v<remove_reference_t<remove_cv_t<remove_optional_t<T>>>, string_view>;
};

template<typename T>
inline constexpr bool parameter_requires_deallocation_v = parameter_requires_deallocation<T>::value;

// Template class definition for calculating parameter traits.
template<uint8_t NumberOfParameters,
         uint8_t NumberOfOptionalParameters,
         uint8_t NumberParametersRequireDeallocation,
         bool AllOptionalTrailing,
         typename FirstParam,
         typename... Args>
struct ParameterTraits
  : ParameterTraits<NumberOfParameters + ((uint8_t)(parameter_provided_by_elisp_caller_v<FirstParam>)), // Increment parameters by 1 for FirstParam
                    NumberOfOptionalParameters + ((uint8_t)is_optional_type_v<FirstParam>), // Increment number of optional parameters if necessary
                    NumberParametersRequireDeallocation + ((uint8_t)parameter_requires_deallocation_v<FirstParam>), // Increment number of parameters that requires deallocation.
                    AllOptionalTrailing &&
                    (NumberOfOptionalParameters > 0 && is_optional_type_v<FirstParam>) ||
                      NumberOfOptionalParameters == 0, // If NumberOfOptionalParameters > 0,
                                                       // and the current parameter is optional,
                                                       // set this value to true.  If there are no optional parameters, set it true.
                    Args...> {};

// Base case.
template<uint8_t NumberOfParameters,
         uint8_t NumberOfOptionalParameters,
         uint8_t NumParametersRequireDeallocation,
         bool AllOptionalTrailing,
         typename LastParameter>
struct ParameterTraits<NumberOfParameters, NumberOfOptionalParameters, NumParametersRequireDeallocation, AllOptionalTrailing, LastParameter> {
  static constexpr uint8_t parameterCount = NumberOfParameters + ((uint8_t)(parameter_provided_by_elisp_caller_v<LastParameter>));
  static constexpr uint8_t optionalParameterCount = NumberOfOptionalParameters + ((uint8_t)is_optional_type_v<LastParameter>);
  static constexpr uint8_t numDeallocatedParameters = NumParametersRequireDeallocation + ((uint8_t)parameter_requires_deallocation_v<LastParameter>);
  static constexpr bool allOptionalParametersTrailing = AllOptionalTrailing && ((NumberOfOptionalParameters > 0 && is_optional_type_v<LastParameter> ) || (NumberOfOptionalParameters == 0));
};

#endif  // __FUNCTION_TRAITS__
