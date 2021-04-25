#ifndef CPP_UTILS_HPP
#define CPP_UTILS_HPP

#include <cstddef>
#include <type_traits>

namespace anrool {
namespace detail {

template <typename...> using void_t = void;

template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T>
using remove_cvref_t =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template <std::size_t... Is> struct index_sequence {
  using type = index_sequence;
};

template <typename, typename> struct merge_sequences;

template <std::size_t... I1, std::size_t... I2>
struct merge_sequences<index_sequence<I1...>, index_sequence<I2...>>
    : index_sequence<I1..., (sizeof...(I1) + I2)...> {};

template <std::size_t N>
struct make_index_sequence
    : merge_sequences<typename make_index_sequence<N / 2>::type,
                      typename make_index_sequence<N - N / 2>::type> {};

template <> struct make_index_sequence<0> : index_sequence<> {};

template <> struct make_index_sequence<1> : index_sequence<0> {};

template <typename T> struct static_const { static constexpr T value{}; };

template <typename T> constexpr T static_const<T>::value;

} // namespace detail
} // namespace anrool

#endif // CPP_UTILS_HPP
