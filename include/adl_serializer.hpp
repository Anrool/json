#ifndef ADL_SERIALIZER_HPP
#define ADL_SERIALIZER_HPP

#include <deserialize_functions.hpp>
#include <serialize_functions.hpp>

namespace anrool {

struct adl_serializer {
  template <typename BasicJsonType, typename T>
  static auto serialize(BasicJsonType &j, T &&val)
      -> decltype(anrool::serialize(j, std::forward<T>(val))) {
    return anrool::serialize(j, std::forward<T>(val));
  }

  template <typename BasicJsonType, typename T>
  static auto deserialize(const BasicJsonType &j, T &val)
      -> decltype(anrool::deserialize(j, val)) {
    return anrool::deserialize(j, val);
  }
};

} // namespace anrool

#endif // ADL_SERIALIZER_HPP
