#ifndef MACRO_HPP
#define MACRO_HPP

#include <cassert>

#define ANROOL_JSON_ASSERT(cond) assert(cond)
#define ANROOL_JSON_ABORT std::abort()

#if defined(JSON_TESTS_PRIVATE)
#define JSON_PRIVATE_UNLESS_TESTED public
#else
#define JSON_PRIVATE_UNLESS_TESTED private
#endif

#define ANROOL_JSON_TEMPLATE_DECLARATION                                       \
  template <template <typename, typename, typename...> class ObjectType,       \
            template <typename, typename...> class ArrayType,                  \
            typename StringType, typename IntegerNumberType,                   \
            typename UnsignedNumberType, typename FloatNumberType,             \
            typename BooleanType>

#define ANROOL_JSON_TEMPLATE                                                   \
  basic_json<ObjectType, ArrayType, StringType, IntegerNumberType,             \
             UnsignedNumberType, FloatNumberType, BooleanType>

#define DECLARE_COMPARATORS(type)                                              \
  bool operator<=(const type &other) const { return !(other < *this); }        \
  bool operator>(const type &other) const { return other < *this; }            \
  bool operator>=(const type &other) const { return !(*this < other); }

#define ANROOL_JSON_SERIALIZE(arg) j[#arg] = obj.arg;
#define ANROOL_JSON_DESERIALIZE(arg) j.at(#arg).get_to(obj.arg);

#define ANROOL_JSON_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11,    \
                              _12, _13, _14, _15, _16, _17, NAME, ...)         \
  NAME

#define ANROOL_JSON_PASTE2(func, _1) func(_1)
#define ANROOL_JSON_PASTE3(func, _1, _2)                                       \
  ANROOL_JSON_PASTE2(func, _1) ANROOL_JSON_PASTE2(func, _2)
#define ANROOL_JSON_PASTE4(func, _1, _2, _3)                                   \
  ANROOL_JSON_PASTE2(func, _1) ANROOL_JSON_PASTE3(func, _2, _3)
#define ANROOL_JSON_PASTE5(func, _1, _2, _3, _4)                               \
  ANROOL_JSON_PASTE2(func, _1) ANROOL_JSON_PASTE4(func, _2, _3, _4)
#define ANROOL_JSON_PASTE6(func, _1, _2, _3, _4, _5)                           \
  ANROOL_JSON_PASTE2(func, _1) ANROOL_JSON_PASTE5(func, _2, _3, _4, _5)
#define ANROOL_JSON_PASTE7(func, _1, _2, _3, _4, _5, _6)                       \
  ANROOL_JSON_PASTE2(func, _1) ANROOL_JSON_PASTE6(func, _2, _3, _4, _5, _6)
#define ANROOL_JSON_PASTE8(func, _1, _2, _3, _4, _5, _6, _7)                   \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE7(func, _2, _3, _4, _5, _6, _7)
#define ANROOL_JSON_PASTE9(func, _1, _2, _3, _4, _5, _6, _7, _8)               \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE8(func, _2, _3, _4, _5, _6, _7, _8)
#define ANROOL_JSON_PASTE10(func, _1, _2, _3, _4, _5, _6, _7, _8, _9)          \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE9(func, _2, _3, _4, _5, _6, _7, _8, _9)
#define ANROOL_JSON_PASTE11(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10)     \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE10(func, _2, _3, _4, _5, _6, _7, _8, _9, _10)
#define ANROOL_JSON_PASTE12(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
                            _11)                                               \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE11(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)
#define ANROOL_JSON_PASTE13(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
                            _11, _12)                                          \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE12(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)
#define ANROOL_JSON_PASTE14(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
                            _11, _12, _13)                                     \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE13(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)
#define ANROOL_JSON_PASTE15(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
                            _11, _12, _13, _14)                                \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE14(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12,     \
                      _13, _14)
#define ANROOL_JSON_PASTE16(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
                            _11, _12, _13, _14, _15)                           \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE15(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12,     \
                      _13, _14, _15)
#define ANROOL_JSON_PASTE17(func, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
                            _11, _12, _13, _14, _15, _16)                      \
  ANROOL_JSON_PASTE2(func, _1)                                                 \
  ANROOL_JSON_PASTE16(func, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12,     \
                      _13, _14, _15, _16)

#define ANROOL_JSON_PASTE(...)                                                 \
  ANROOL_JSON_GET_MACRO(                                                       \
      __VA_ARGS__, ANROOL_JSON_PASTE17, ANROOL_JSON_PASTE16,                   \
      ANROOL_JSON_PASTE15, ANROOL_JSON_PASTE14, ANROOL_JSON_PASTE13,           \
      ANROOL_JSON_PASTE12, ANROOL_JSON_PASTE11, ANROOL_JSON_PASTE10,           \
      ANROOL_JSON_PASTE9, ANROOL_JSON_PASTE8, ANROOL_JSON_PASTE7,              \
      ANROOL_JSON_PASTE6, ANROOL_JSON_PASTE5, ANROOL_JSON_PASTE4,              \
      ANROOL_JSON_PASTE3, ANROOL_JSON_PASTE2, ANROOL_JSON_PASTE1)              \
  (__VA_ARGS__)

#define ANROOL_JSON_DEFINE_NON_INTRUSIVE(Type, ...)                            \
  void serialize(json &j, const Type &obj) {                                   \
    j = json_type::object;                                                     \
    ANROOL_JSON_PASTE(ANROOL_JSON_SERIALIZE, __VA_ARGS__)                      \
  }                                                                            \
  void deserialize(const json &j, Type &obj) {                                 \
    ANROOL_JSON_PASTE(ANROOL_JSON_DESERIALIZE, __VA_ARGS__)                    \
  }

#define ANROOL_JSON_DEFINE_INTRUSIVE(Type, ...)                                \
  friend void serialize(json &j, const Type &obj) {                            \
    j = json_type::object;                                                     \
    ANROOL_JSON_PASTE(ANROOL_JSON_SERIALIZE, __VA_ARGS__)                      \
  }                                                                            \
  friend void deserialize(const json &j, Type &obj) {                          \
    ANROOL_JSON_PASTE(ANROOL_JSON_DESERIALIZE, __VA_ARGS__)                    \
  }

#endif // MACRO_HPP
