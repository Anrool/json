#ifndef JSON_FWD_HPP
#define JSON_FWD_HPP

#include <macro.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace anrool {

template <template <typename, typename, typename...> class ObjectType =
              std::map,
          template <typename, typename...> class ArrayType = std::vector,
          typename StringType = std::string,
          typename IntegerNumberType = std::int64_t,
          typename UnsignedNumberType = std::uint64_t,
          typename FloatNumberType = double, typename BooleanType = bool>
class basic_json;

template <typename T> struct is_basic_json : std::false_type {};

ANROOL_JSON_TEMPLATE_DECLARATION
struct is_basic_json<ANROOL_JSON_TEMPLATE> : std::true_type {};

} // namespace anrool

#endif // JSON_FWD_HPP
