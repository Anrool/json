#ifndef JSON_TYPE_HPP
#define JSON_TYPE_HPP

#include <cstdint>
#include <ostream>

namespace anrool {

enum class json_type : std::uint8_t {
  object,
  array,
  string,
  integer_number,
  unsigned_number,
  float_number,
  boolean,
  null,
  count // should be the last entry
};

inline const char *to_string(json_type type) {
  switch (type) {
  case json_type::object:
    return "object";
  case json_type::array:
    return "array";
  case json_type::string:
    return "string";
  case json_type::integer_number:
    return "integer_number";
  case json_type::unsigned_number:
    return "unsigned_number";
  case json_type::float_number:
    return "float_number";
  case json_type::boolean:
    return "boolean";
  case json_type::null:
    return "null";
  default:
    return "unknown";
  }
}

inline std::ostream &operator<<(std::ostream &os, json_type type) {
  return os << to_string(type);
}

} // namespace anrool

#endif // JSON_TYPE_HPP
