#include "emacs-module-wrapper-template.tcc"
#include "function-traits.tcc"
#include <emacs-module.h>
#include <optional>

using namespace std;

emacs_value lispfunction1(emacs_env* e, int a, optional<int> b, optional<string_view> s);

using lf1 = EmacsCallable<&lispfunction1>::parameter_traits;
static_assert(lf1::parameterCount == 3);
static_assert(lf1::optionalParameterCount == 2);
static_assert(lf1::numDeallocatedParameters == 1);
