#ifndef SERIALIZE_FUNCTIONS_HPP
#define SERIALIZE_FUNCTIONS_HPP

#include <cpp_utils.hpp>
#include <json_type.hpp>
#include <traits.hpp>

#include <type_traits>
#include <utility>

namespace anrool {
namespace detail {

template <json_type> struct constructor;

template <> struct constructor<json_type::object> {
  template <typename BasicJsonType>
  static void construct(BasicJsonType &j,
                        const typename BasicJsonType::object_type &val) {
    j.type_ = json_type::object;
    j.value_.object_ =
        j.template create<typename BasicJsonType::object_type>(val);
  }

  template <typename BasicJsonType>
  static void construct(BasicJsonType &j,
                        typename BasicJsonType::object_type &&val) {
    j.type_ = json_type::object;
    j.value_.object_ =
        j.template create<typename BasicJsonType::object_type>(std::move(val));
  }

  template <typename BasicJsonType, typename T,
            typename = enable_if_t<
                !std::is_same<T, typename BasicJsonType::object_type>::value>>
  static void construct(BasicJsonType &j, const T &val) {
    j.type_ = json_type::object;
    j.value_.object_ = j.template create<typename BasicJsonType::object_type>(
        std::begin(val), std::end(val));
  }
};

template <> struct constructor<json_type::array> {
  template <typename BasicJsonType>
  static void construct(BasicJsonType &j,
                        typename BasicJsonType::array_type &&val) {
    j.type_ = json_type::array;
    j.value_.array_ =
        j.template create<typename BasicJsonType::array_type>(std::move(val));
  }

  template <typename BasicJsonType, typename T>
  static void construct(BasicJsonType &j, const T &val) {
    j.type_ = json_type::array;
    j.value_.array_ = j.template create<typename BasicJsonType::array_type>(
        std::begin(val), std::end(val));
  }

  template <typename BasicJsonType, typename T>
  static void construct_from_tuple(BasicJsonType &j, const T &val) {
    construct_from_tuple_impl(j, val,
                              make_index_sequence<std::tuple_size<T>::value>{});
  }

private:
  template <typename BasicJsonType, typename T, std::size_t... Is>
  static void construct_from_tuple_impl(BasicJsonType &j, const T &val,
                                        index_sequence<Is...>) {
    j.type_ = json_type::array;
    j.value_.array_ = j.template create<typename BasicJsonType::array_type>();

    auto &array = *j.value_.array_;
    array.reserve(sizeof...(Is));

    using expander = int[];
    expander{(array.emplace_back(std::get<Is>(val)), 0)...};
  }
};

template <> struct constructor<json_type::string> {
  template <typename BasicJsonType, typename T>
  static void construct(BasicJsonType &j, T &&val) {
    j.type_ = json_type::string;
    j.value_.string_ = j.template create<typename BasicJsonType::string_type>(
        std::forward<T>(val));
  }
};

template <> struct constructor<json_type::integer_number> {
  template <typename BasicJsonType>
  static void construct(BasicJsonType &j,
                        typename BasicJsonType::integer_number_type val) {
    j.type_ = json_type::integer_number;
    j.value_.integer_number_ = val;
  }
};

template <> struct constructor<json_type::unsigned_number> {
  template <typename BasicJsonType>
  static void construct(BasicJsonType &j,
                        typename BasicJsonType::unsigned_number_type val) {
    j.type_ = json_type::unsigned_number;
    j.value_.unsigned_number_ = val;
  }
};

template <> struct constructor<json_type::float_number> {
  template <typename BasicJsonType>
  static void construct(BasicJsonType &j,
                        typename BasicJsonType::float_number_type val) {
    j.type_ = json_type::float_number;
    j.value_.float_number_ = val;
  }
};

template <> struct constructor<json_type::boolean> {
  template <typename BasicJsonType, typename T>
  static void construct(BasicJsonType &j, T val) {
    j.type_ = json_type::boolean;
    j.value_.boolean_ = val;
  }
};

template <
    typename BasicJsonType, typename T,
    enable_if_t<is_compatible_object_type<typename BasicJsonType::object_type,
                                          remove_cvref_t<T>>::value,
                int> = 0>
void serialize(BasicJsonType &j, T &&val) {
  constructor<json_type::object>::construct(j, std::forward<T>(val));
}

template <typename BasicJsonType, typename T, typename U = remove_cvref_t<T>,
          enable_if_t<is_compatible_array_type<
                          typename BasicJsonType::array_type, U>::value &&
                          !is_compatible_object_type<
                              typename BasicJsonType::object_type, U>::value &&
                          !is_compatible_string_type<
                              typename BasicJsonType::string_type, U>::value,
                      int> = 0>
void serialize(BasicJsonType &j, T &&val) {
  constructor<json_type::array>::construct(j, std::forward<T>(val));
}

template <
    typename BasicJsonType, typename... Ts,
    enable_if_t<is_constructible_from<BasicJsonType, Ts...>::value, int> = 0>
void serialize(BasicJsonType &j, const std::tuple<Ts...> &val) {
  constructor<json_type::array>::construct_from_tuple(j, val);
}

template <
    typename BasicJsonType, typename T, typename U,
    enable_if_t<is_constructible_from<BasicJsonType, T, U>::value, int> = 0>
void serialize(BasicJsonType &j, const std::pair<T, U> &val) {
  constructor<json_type::array>::construct_from_tuple(j, val);
}

template <
    typename BasicJsonType, typename T, std::size_t N,
    enable_if_t<!std::is_constructible<typename BasicJsonType::string_type,
                                       const T (&)[N]>::value,
                int> = 0>
void serialize(BasicJsonType &j, const T (&val)[N]) {
  constructor<json_type::array>::construct(j, val);
}

template <
    typename BasicJsonType, typename T,
    enable_if_t<is_compatible_string_type<typename BasicJsonType::string_type,
                                          remove_cvref_t<T>>::value,
                int> = 0>
void serialize(BasicJsonType &j, T &&val) {
  constructor<json_type::string>::construct(j, std::forward<T>(val));
}

template <
    typename BasicJsonType, typename T,
    enable_if_t<std::is_same<T, typename BasicJsonType::boolean_type>::value,
                int> = 0>
void serialize(BasicJsonType &j, T val) {
  constructor<json_type::boolean>::construct(j, val);
}

template <
    typename BasicJsonType, typename T,
    enable_if_t<is_compatible_integer_type<
                    typename BasicJsonType::integer_number_type, T>::value,
                int> = 0>
void serialize(BasicJsonType &j, T val) {
  constructor<json_type::integer_number>::construct(
      j, static_cast<typename BasicJsonType::integer_number_type>(val));
}

template <
    typename BasicJsonType, typename T,
    enable_if_t<!std::is_same<T, bool>::value &&
                    is_compatible_integer_type<
                        typename BasicJsonType::unsigned_number_type, T>::value,
                int> = 0>
void serialize(BasicJsonType &j, T val) {
  constructor<json_type::unsigned_number>::construct(
      j, static_cast<typename BasicJsonType::unsigned_number_type>(val));
}

template <typename BasicJsonType, typename T,
          enable_if_t<std::is_floating_point<T>::value, int> = 0>
void serialize(BasicJsonType &j, T val) {
  constructor<json_type::float_number>::construct(
      j, static_cast<typename BasicJsonType::float_number_type>(val));
}

struct serialize_fn {
  template <typename BasicJsonType, typename T>
  auto operator()(BasicJsonType &j, T &&val) const
      -> decltype(serialize(j, std::forward<T>(val))) {
    return serialize(j, std::forward<T>(val));
  }
};

} // namespace detail

namespace {
constexpr const auto &serialize =
    detail::static_const<detail::serialize_fn>::value;
} // unnamed namespace

} // namespace anrool

#endif // SERIALIZE_FUNCTIONS_HPP
