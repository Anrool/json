#ifndef DETECTED_HPP
#define DETECTED_HPP

#include <cpp_utils.hpp>

#include <type_traits>

namespace anrool {
namespace detail {

struct nonesuch;

template <typename Default, typename AlwaysVoid,
          template <typename...> class Op, typename... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template <typename Default, template <typename...> class Op, typename... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};

template <template <typename...> class Op, typename... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

template <template <typename...> class Op, typename... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

template <typename Default, template <typename...> class Op, typename... Args>
using detected_or = detector<Default, void, Op, Args...>;

template <typename Default, template <typename...> class Op, typename... Args>
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

template <typename Expected, template <typename...> class Op, typename... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template <typename To, template <typename...> class Op, typename... Args>
using is_detected_convertible =
    std::is_convertible<detected_t<Op, Args...>, To>;

} // namespace detail
} // namespace anrool

#endif // DETECTED_HPP
