#include "emacs-module-wrapper-template.tcc"
#include "function-traits.tcc"
#include <emacs-module.h>
#include <optional>

using namespace std;

/****/
emacs_value lispfunction1(emacs_env* e, int a, optional<int> b, optional<string_view> s);
using lf1 = EmacsCallable<&lispfunction1>::parameter_traits;

static_assert(lf1::parameterCount == 3);
static_assert(lf1::optionalParameterCount == 2);
static_assert(lf1::numDeallocatedParameters == 1);
static_assert(lf1::allOptionalParametersTrailing);
/****/

/****/
emacs_value lispfunction2(emacs_env* e, int a, int b, optional<string_view> s);
using lf2 = EmacsCallable<&lispfunction2>::parameter_traits;

static_assert(lf2::parameterCount == 3);
static_assert(lf2::optionalParameterCount == 1);
static_assert(lf2::numDeallocatedParameters == 1);
static_assert(lf2::allOptionalParametersTrailing);
/****/

/****/
emacs_value lispfunction3(emacs_env* e, int a, int b, string_view s);
using lf3 = EmacsCallable<&lispfunction3>::parameter_traits;

static_assert(lf3::parameterCount == 3);
static_assert(lf3::optionalParameterCount == 0);
static_assert(lf3::numDeallocatedParameters == 1);
static_assert(lf3::allOptionalParametersTrailing);
/****/

/****/
template<typename A, typename B>
struct assert_same_types {
  static_assert(is_same_v<A, B>);
};

assert_same_types<remove_optional_t<int>, int> a;
/****/

/****/
static_assert(is_type_any_of_v<int, int, char*, bool>);
static_assert(is_type_any_of_v<bool, int, char*, bool>);
static_assert(!is_type_any_of_v<is_type_any_of<int, char*>, int, char*, bool>);
static_assert(!is_type_any_of_v<is_type_any_of<int, char*>, is_type_any_of<int, char>>);
static_assert(is_type_any_of_v<is_type_any_of<int, char*>, is_type_any_of<int, char*>>);
static_assert(!is_type_any_of_v<int, char*, bool>);
/****/
