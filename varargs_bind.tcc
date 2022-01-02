#include <functional>
#include <type_traits>
#include <utility>

using namespace std;

// We need to be able to call std::bind on functions with parameter
// packs as a template parameter, but std::bind requires all
// parameters to be specified (either with a value or a placeholder).
// The following lets us generate the call to bind using a sequence of
// placeholder objects.

// The strategy here is heavily derived from:
// https://stackoverflow.com/questions/21192659/variadic-templates-and-stdbind#21193316

// We need a custom placeholder type because there are a
// statically-defined number of placeholders in STL implementations
// (it probably wouldn't have been a limitation in practice, but, oh,
// well), and we don't know how many args the functions being wrapped
// will take.  Also, we could just generate integral_constant<int, N> for
// each placeholder instead of having a custom type, but that (to me)
// seemed to be relying on an implementation detail of std::bind,
// which is that placeholders are of type integral_constant<int, N
// >=1>.
template<int>
struct bind_placeholder
{};

// Since we have our own type, we need to specialize is_placeholder
// for the new custom placeholder type.
namespace std {
    template<int N>
    struct is_placeholder< bind_placeholder<N> >
        : integral_constant<int, N> {};
}

// Code to generate an integer sequence; I could have used
// make_integer_sequence from C++14, but transforming it from 0...N-1
// to 1...N seemed just as complex.
template<int N, int... Sequence>
// Move N to the beginning of the sequence and recurse with N - 1
struct make_int_sequence : make_int_sequence<N-1, N, Sequence...> {};

template<int... Sequence>
// For the specialization with N = 0, Sequence represents all integers from 1 to N.
struct make_int_sequence<0, Sequence...> : integer_sequence<int, Sequence...> {};

// Template to generate a call to std::bind with a number of
// placeholders from the size of a parameter pack.
template<typename Callable, typename Param, int N>
auto varargs_bind(Callable c, Param p) {
  return varargs_bind(c, p, make_int_sequence<N>{});
}

template<typename Callable, typename Param, int... Is>
auto varargs_bind(Callable c, Param p, integer_sequence<int, Is...>) {
  return std::bind(c, p, bind_placeholder<Is>{}...);
}
