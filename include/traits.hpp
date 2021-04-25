#ifndef TRAITS_HPP
#define TRAITS_HPP

#include <detected.hpp>
#include <json_fwd.hpp>

#include <tuple>
#include <type_traits>
#include <valarray>

namespace anrool {
namespace detail {

template <typename T> using key_type_t = typename T::key_type;

template <typename T> using mapped_type_t = typename T::mapped_type;

template <typename T> using value_type_t = typename T::value_type;

template <typename T> using iterator_t = typename T::iterator;

template <typename T>
using begin_function = decltype(std::begin(std::declval<T>()));

template <typename T>
using end_function = decltype(std::end(std::declval<T>()));

template <typename T>
using reserve_function =
    decltype(std::declval<T>().reserve(std::declval<typename T::size_type>()));

template <typename T, typename... Args>
using serialize_function = decltype(T::serialize(std::declval<Args>()...));

template <typename T, typename... Args>
using deserialize_function = decltype(T::deserialize(std::declval<Args>()...));

template <typename T, typename = void>
struct is_complete_type : std::false_type {};

template <typename T>
struct is_complete_type<T, decltype(void(sizeof(T)))> : std::true_type {};

template <typename...> struct is_constructible_from;

template <typename T> struct is_constructible_from<T> : std::true_type {};

template <typename T, typename U, typename... Us>
struct is_constructible_from<T, U, Us...>
    : std::conditional<std::is_constructible<T, U>::value,
                       is_constructible_from<T, Us...>, std::false_type>::type {
};

template <typename BasicJsonType, typename T, typename = void>
struct is_serializable_impl : std::false_type {};

template <typename BasicJsonType, typename T>
struct is_serializable_impl<BasicJsonType, T,
                            enable_if_t<!is_basic_json<T>::value>> {
  using serializer = typename BasicJsonType::json_serializer;

  static constexpr bool value =
      is_detected<serialize_function, serializer, BasicJsonType &, T>::value;
};

template <typename BasicJsonType, typename T>
struct is_serializable : is_serializable_impl<BasicJsonType, T> {};

template <typename T, typename U> struct is_compatible_integer_type {
  static constexpr bool value =
      std::is_integral<T>::value && std::is_integral<U>::value &&
      std::is_signed<T>::value == std::is_signed<U>::value;
};

template <typename T, typename U>
struct is_compatible_string_type : std::is_constructible<T, U> {};

template <typename T, typename U, typename = void>
struct is_compatible_object_type_impl : std::false_type {};

template <typename T, typename U>
struct is_compatible_object_type_impl<
    T, U,
    enable_if_t<
        is_detected<key_type_t, U>::value &&
        is_detected<mapped_type_t, U>::value &&
        is_compatible_string_type<key_type_t<T>, key_type_t<U>>::value &&
        std::is_constructible<mapped_type_t<T>, mapped_type_t<U>>::value>>
    : std::true_type {};

template <typename T, typename U>
struct is_compatible_object_type : is_compatible_object_type_impl<T, U> {};

template <typename T, typename U, typename = void>
struct is_compatible_array_type_impl : std::false_type {};

template <typename T, typename U>
struct is_compatible_array_type_impl<
    T, U,
    enable_if_t<is_detected<value_type_t, U>::value &&
                is_detected<begin_function, U>::value &&
                is_detected<end_function, U>::value>>
    : std::is_constructible<value_type_t<T>, value_type_t<U>> {};

template <typename T, typename U>
struct is_compatible_array_type : is_compatible_array_type_impl<T, U> {};

template <typename BasicJsonType, typename T> struct is_deserializable {
  using serializer = typename BasicJsonType::json_serializer;

  static constexpr bool value = is_detected<deserialize_function, serializer,
                                            const BasicJsonType &, T &>::value;
};

template <typename T, typename U>
struct is_constructible_string_type : std::is_constructible<U, T> {};

template <typename T, typename U, typename = void>
struct is_constructible_object_type_impl : std::false_type {};

template <typename T, typename U>
struct is_constructible_object_type_impl<
    T, U,
    enable_if_t<is_detected<key_type_t, U>::value &&
                is_detected<mapped_type_t, U>::value>> {
  static constexpr bool value =
      is_constructible_string_type<key_type_t<T>, key_type_t<U>>::value &&
      (std::is_same<mapped_type_t<T>, mapped_type_t<U>>::value ||
       is_deserializable<mapped_type_t<T>, mapped_type_t<U>>::value);
};

template <typename T, typename U>
struct is_constructible_object_type : is_constructible_object_type_impl<T, U> {
};

template <typename BasicJsonType, typename ConstructibleArrayType,
          typename = void>
struct is_constructible_array_type_impl : std::false_type {};

template <typename T, typename U>
struct is_constructible_array_type_impl<
    T, U,
    enable_if_t<!is_basic_json<U>::value &&
                is_detected<value_type_t, U>::value &&
                is_detected<begin_function, U>::value &&
                is_detected<end_function, U>::value>> {
  static constexpr bool value =
      is_deserializable<value_type_t<T>, value_type_t<U>>::value;
};

template <typename BasicJsonType, typename T>
struct is_constructible_array_type
    : is_constructible_array_type_impl<BasicJsonType, T> {};

} // namespace detail
} // namespace anrool

#endif // TRAITS_HPP
