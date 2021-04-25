#ifndef DESERIALIZE_FUNCTIONS_HPP
#define DESERIALIZE_FUNCTIONS_HPP

#include <cpp_utils.hpp>
#include <detected.hpp>
#include <json_type.hpp>
#include <traits.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <forward_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace anrool {
namespace detail {

template <typename BasicJsonType,
          enable_if_t<is_basic_json<BasicJsonType>::value, int> = 0>
void deserialize(const BasicJsonType &j, BasicJsonType &other) {
  other = j;
}

template <typename BasicJsonType>
void deserialize(const BasicJsonType &j, std::nullptr_t &) {
  if (j.type() != json_type::null) {
    throw std::invalid_argument{std::string{"expected null, but got "} +
                                to_string(j.type())};
  }
}

template <typename BasicJsonType>
void deserialize(const BasicJsonType &j,
                 typename BasicJsonType::boolean_type &val) {
  if (j.type() != json_type::boolean) {
    throw std::invalid_argument{std::string{"expected boolean, but got "} +
                                to_string(j.type())};
  }

  val = *j.template get_ptr<typename BasicJsonType::boolean_type>();
}

template <typename BasicJsonType, typename T,
          enable_if_t<
              std::is_arithmetic<T>::value &&
                  !std::is_same<T, typename BasicJsonType::boolean_type>::value,
              int> = 0>
void deserialize(const BasicJsonType &j, T &val) {
  switch (j.type()) {
  case json_type::integer_number:
    val = static_cast<T>(
        *j.template get_ptr<typename BasicJsonType::integer_number_type>());
    break;
  case json_type::unsigned_number:
    val = static_cast<T>(
        *j.template get_ptr<typename BasicJsonType::unsigned_number_type>());
    break;
  case json_type::float_number:
    val = static_cast<T>(
        *j.template get_ptr<typename BasicJsonType::float_number_type>());
    break;
  default:
    throw std::invalid_argument{std::string{"expected number, but got "} +
                                to_string(j.type())};
  }
}

template <typename BasicJsonType, typename T,
          enable_if_t<is_constructible_string_type<
                          typename BasicJsonType::string_type, T>::value,
                      int> = 0>
void deserialize(const BasicJsonType &j, T &val) {
  if (j.type() != json_type::string) {
    throw std::invalid_argument{std::string{"expected string, but got "} +
                                to_string(j.type())};
  }

  val = *j.template get_ptr<typename BasicJsonType::string_type>();
}

template <typename BasicJsonType, typename T,
          enable_if_t<is_constructible_object_type<
                          typename BasicJsonType::object_type, T>::value,
                      int> = 0>
void deserialize(const BasicJsonType &j, T &val) {
  if (j.type() != json_type::object) {
    throw std::invalid_argument{std::string{"expected object, but got "} +
                                to_string(j.type())};
  }

  const auto &inner_object =
      *j.template get_ptr<typename BasicJsonType::object_type>();

  val.clear();

  std::transform(
      std::begin(inner_object), std::end(inner_object),
      std::inserter<T>(val, std::begin(val)),
      [](const typename BasicJsonType::object_type::value_type &val) {
        return value_type_t<T>{val.first,
                               val.second.template get<mapped_type_t<T>>()};
      });
}

template <typename T, typename U,
          enable_if_t<is_detected<reserve_function, U>::value, int> = 0>
void deserialize_array_impl(const T &lhs, U &rhs) {
  rhs.clear();
  rhs.reserve(lhs.size());

  std::transform(std::begin(lhs), std::end(lhs),
                 std::inserter(rhs, std::end(rhs)),
                 [](const value_type_t<T> &val) {
                   return val.template get<value_type_t<U>>();
                 });
}

template <typename T, typename U,
          enable_if_t<!is_detected<reserve_function, U>::value, int> = 0>
void deserialize_array_impl(const T &lhs, U &rhs) {
  rhs.clear();

  std::transform(std::begin(lhs), std::end(lhs),
                 std::inserter(rhs, std::end(rhs)),
                 [](const value_type_t<T> &val) {
                   return val.template get<value_type_t<U>>();
                 });
}

template <typename T, typename U>
void deserialize_array_impl(const T &lhs, std::valarray<U> &rhs) {
  rhs.resize(lhs.size());

  std::transform(
      std::begin(lhs), std::end(lhs), std::begin(rhs),
      [](const value_type_t<T> &val) { return val.template get<U>(); });
}

template <typename BasicJsonType, typename T,
          enable_if_t<is_constructible_array_type<
                          typename BasicJsonType::array_type, T>::value &&
                          !is_constructible_string_type<
                              typename BasicJsonType::string_type, T>::value &&
                          !is_constructible_object_type<
                              typename BasicJsonType::object_type, T>::value,
                      int> = 0>
void deserialize(const BasicJsonType &j, T &val) {
  if (j.type() != json_type::array) {
    throw std::invalid_argument{std::string{"expected array, but got "} +
                                to_string(j.type())};
  }

  const auto &inner_array =
      *j.template get_ptr<typename BasicJsonType::array_type>();

  deserialize_array_impl(inner_array, val);
}

template <typename BasicJsonType, typename T,
          enable_if_t<is_deserializable<BasicJsonType, T>::value, int> = 0>
void deserialize(const BasicJsonType &j, std::forward_list<T> &val) {
  if (j.type() != json_type::array) {
    throw std::invalid_argument{std::string{"expected array, but got "} +
                                to_string(j.type())};
  }

  const auto &inner_array =
      *j.template get_ptr<typename BasicJsonType::array_type>();

  val.clear();
  auto it = val.before_begin();

  for (const auto &v : inner_array) {
    it = val.emplace_after(it, v);
  }
}

template <typename BasicJsonType, typename T, std::size_t N>
void deserialize(const BasicJsonType &j, std::array<T, N> &val) {
  if (j.type() != json_type::array) {
    throw std::invalid_argument{std::string{"expected array, but got "} +
                                to_string(j.type())};
  }

  if (j.size() != N) {
    throw std::invalid_argument{std::string{"bad argument size. expected "} +
                                std::to_string(N) + " , but got" +
                                std::to_string(j.size())};
  }

  const auto &inner_array =
      *j.template get_ptr<typename BasicJsonType::array_type>();

  for (std::size_t i = 0; i < N; i++) {
    val[i] = inner_array[i];
  }
}

template <typename BasicJsonType, typename T, typename U,
          enable_if_t<is_deserializable<BasicJsonType, T>::value &&
                          is_deserializable<BasicJsonType, U>::value,
                      int> = 0>
void deserialize(const BasicJsonType &j, std::pair<T, U> &val) {
  if (j.type() != json_type::array) {
    throw std::invalid_argument{std::string{"expected array, but got "} +
                                to_string(j.type())};
  }

  if (j.size() != 2) {
    throw std::invalid_argument{std::string{"bad argument, json has size "} +
                                std::to_string(j.size())};
  }

  deserialize(j[0], val.first);
  deserialize(j[1], val.second);
}

template <typename BasicJsonType, typename... Ts, std::size_t... Is>
void deserialize_from_tuple(BasicJsonType &j, std::tuple<Ts...> &val,
                            index_sequence<Is...>) {
  val = std::make_tuple(j[Is].template get<Ts>()...);
}

template <
    typename BasicJsonType, typename... Ts,
    enable_if_t<is_constructible_from<BasicJsonType, Ts...>::value, int> = 0>
void deserialize(const BasicJsonType &j, std::tuple<Ts...> &val) {
  if (j.type() != json_type::array) {
    throw std::invalid_argument{std::string{"expected array, but got "} +
                                to_string(j.type())};
  }

  constexpr std::size_t tuple_size = std::tuple_size<std::tuple<Ts...>>::value;

  if (j.size() != tuple_size) {
    throw std::invalid_argument{std::string{"bad argument, json has size "} +
                                std::to_string(tuple_size) +
                                " , but got tuple with size " +
                                std::to_string(j.size())};
  }

  deserialize_from_tuple(j, val, make_index_sequence<tuple_size>{});
}

template <typename BasicJsonType, typename Key, typename Value,
          enable_if_t<!is_constructible_string_type<
                          typename BasicJsonType::string_type, Key>::value,
                      int> = 0>
void deserialize(const BasicJsonType &j, std::map<Key, Value> &val) {
  if (j.type() != json_type::array) {
    throw std::invalid_argument{std::string{"expected array, but got "} +
                                to_string(j.type())};
  }

  val.clear();

  std::transform(std::begin(j), std::end(j), std::inserter(val, std::end(val)),
                 [](const BasicJsonType &val) {
                   return val.template get<std::pair<Key, Value>>();
                 });
}

template <typename BasicJsonType, typename Key, typename Value,
          enable_if_t<!is_constructible_string_type<
                          typename BasicJsonType::string_type, Key>::value,
                      int> = 0>
void deserialize(const BasicJsonType &j, std::multimap<Key, Value> &val) {
  if (j.type() != json_type::array) {
    throw std::invalid_argument{std::string{"expected array, but got "} +
                                to_string(j.type())};
  }

  val.clear();

  std::transform(std::begin(j), std::end(j), std::inserter(val, std::end(val)),
                 [](const BasicJsonType &val) {
                   return val.template get<std::pair<Key, Value>>();
                 });
}

struct deserialize_fn {
  template <typename BasicJsonType, typename T>
  auto operator()(const BasicJsonType &j, T &val) const
      -> decltype(deserialize(j, val)) {
    return deserialize(j, val);
  }
};

} // namespace detail

namespace {
constexpr const auto &deserialize =
    detail::static_const<detail::deserialize_fn>::value;
} // unnamed namespace

} // namespace anrool

#endif // DESERIALIZE_FUNCTIONS_HPP
