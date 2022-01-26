#include <iostream>
#include <optional>

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

// TODO: add
//   static assert when user instantiates with a function with std::optional<void*> and std::optional<emacs_env*>.
//   static assert when user instantiates with a function with more than 1 void* parameter.

template<typename T>
struct parameter_provided_by_elisp_caller {
  static constexpr bool value = !(is_same<T, emacs_env*>::value || is_same<T, void*>::value);
};

template<typename T>
struct remove_optional {
  using type = T;
};

template<typename T>
struct remove_optional<optional<T>> {
  using type = T;
};

template<typename T>
struct parameter_requires_deallocation {
  static constexpr bool value =
    is_same<remove_reference<remove_cv<typename remove_optional<T>::type>>, string_view>::value;
};

// Template class definition for calculating parameter traits.
template<uint8_t NumberOfParameters,
         uint8_t NumberOfOptionalParameters,
         uint8_t NumberParametersRequireDeallocation,
         bool AllOptionalTrailing,
         typename FirstParam,
         typename... Args>
struct ParameterTraits
  : ParameterTraits<NumberOfParameters + ((uint8_t)(parameter_provided_by_elisp_caller<FirstParam>::value)), // Increment parameters by 1 for FirstParam
                    NumberOfOptionalParameters + ((uint8_t)is_optional_type<FirstParam>::value), // Increment number of optional parameters if necessary
                    NumberParametersRequireDeallocation + ((uint8_t)parameter_requires_deallocation<FirstParam>::value), // Increment number of parameters that requires deallocation.
                    AllOptionalTrailing &&
                    (NumberOfOptionalParameters > 0 && is_optional_type<FirstParam>::value) ||
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
  static constexpr size_t parameterCount = NumberOfParameters + ((uint8_t)(parameter_provided_by_elisp_caller<LastParameter>::value));
  static constexpr size_t optionalParameterCount = NumberOfOptionalParameters + ((uint8_t)is_optional_type<LastParameter>::value);
  static constexpr uint8_t numDeallocatedParaameters = NumParametersRequireDeallocation + ((uint8_t)parameter_requires_deallocation<LastParameter>::value);
  static constexpr bool allOptionalParametersTrailing = AllOptionalTrailing && ((NumberOfOptionalParameters > 0 && is_optional_type<LastParameter>::value ) || (NumberOfOptionalParameters == 0));
};
