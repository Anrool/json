#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <json_type.hpp>
#include <macro.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

namespace anrool {
namespace detail {

template <typename BasicJsonType, typename OutputAdapter> class serializer {
public:
  explicit serializer(OutputAdapter adapter)
      : adapter_{std::move(adapter)}, indent_string_(512, ' ') {}

  serializer(const serializer &) = delete;
  serializer &operator=(const serializer &) = delete;

  void dump(const BasicJsonType &j, std::size_t indent_step,
            std::size_t current_indent = 0) {
    switch (j.type()) {
    case json_type::object:
      dump_object(*j.value_.object_, indent_step, current_indent);
      break;
    case json_type::array:
      dump_array(*j.value_.array_, indent_step, current_indent);
      break;
    case json_type::string:
      dump_string(*j.value_.string_);
      break;
    case json_type::integer_number:
      dump_as_string(j.value_.integer_number_);
      break;
    case json_type::unsigned_number:
      dump_as_string(j.value_.unsigned_number_);
      break;
    case json_type::float_number:
      dump_as_string(j.value_.float_number_);
      break;
    case json_type::boolean:
      if (j.value_.boolean_) {
        adapter_.write("true", 4);
      } else {
        adapter_.write("false", 5);
      }
      break;
    case json_type::null:
      adapter_.write("null", 4);
      break;
    default:
      ANROOL_JSON_ASSERT(false);
    }
  }

  void dump(const BasicJsonType &j) {
    switch (j.type()) {
    case json_type::object:
      dump_object(*j.value_.object_);
      break;
    case json_type::array:
      dump_array(*j.value_.array_);
      break;
    case json_type::string:
      dump_string(*j.value_.string_);
      break;
    case json_type::integer_number:
      dump_as_string(j.value_.integer_number_);
      break;
    case json_type::unsigned_number:
      dump_as_string(j.value_.unsigned_number_);
      break;
    case json_type::float_number:
      dump_as_string(j.value_.float_number_);
      break;
    case json_type::boolean:
      if (j.value_.boolean_) {
        adapter_.write("true", 4);
      } else {
        adapter_.write("false", 5);
      }
      break;
    case json_type::null:
      adapter_.write("null", 4);
      break;
    default:
      ANROOL_JSON_ASSERT(false);
    }
  }

private:
  using object_type = typename BasicJsonType::object_type;
  using array_type = typename BasicJsonType::array_type;
  using string_type = typename BasicJsonType::string_type;
  using integer_number_type = typename BasicJsonType::integer_number_type;
  using unsigned_number_type = typename BasicJsonType::unsigned_number_type;
  using float_number_type = typename BasicJsonType::float_number_type;

  static constexpr std::uint8_t UTF8_ACCEPT = 0;
  static constexpr std::uint8_t UTF8_REJECT = 1;

  void dump_object(const object_type &object, std::size_t indent_step,
                   std::size_t current_indent) {
    if (object.empty()) {
      adapter_.write("{}", 2);
      return;
    }

    adapter_.write("{\n", 2);

    const std::size_t new_indent = current_indent + indent_step;

    if (new_indent > indent_string_.size()) {
      indent_string_.resize(new_indent * 2, ' ');
    }

    auto it = std::begin(object);

    for (auto last = --std::end(object); it != last; ++it) {
      adapter_.write(indent_string_.data(), new_indent);
      dump_string(it->first);
      adapter_.write(": ", 2);
      dump(it->second, indent_step, new_indent);
      adapter_.write(",\n", 2);
    }

    adapter_.write(indent_string_.data(), new_indent);
    dump_string(it->first);
    adapter_.write(": ", 2);
    dump(it->second, indent_step, new_indent);
    adapter_.write('\n');

    adapter_.write(indent_string_.data(), current_indent);
    adapter_.write('}');
  }

  void dump_object(const object_type &object) {
    if (object.empty()) {
      adapter_.write("{}", 2);
      return;
    }

    adapter_.write('{');

    auto it = std::begin(object);

    for (auto last = --std::end(object); it != last; ++it) {
      dump_string(it->first);
      adapter_.write(':');
      dump(it->second);
      adapter_.write(',');
    }

    dump_string(it->first);
    adapter_.write(':');
    dump(it->second);

    adapter_.write('}');
  }

  void dump_array(const array_type &array, std::size_t indent_step,
                  std::size_t current_indent) {
    if (array.empty()) {
      adapter_.write("[]", 2);
      return;
    }

    adapter_.write("[\n", 2);

    const std::size_t new_indent = current_indent + indent_step;

    if (new_indent > indent_string_.size()) {
      indent_string_.resize(new_indent * 2, ' ');
    }

    auto it = std::begin(array);

    for (auto last = std::end(array) - 1; it != last; ++it) {
      adapter_.write(indent_string_.data(), new_indent);
      dump(*it, indent_step, new_indent);
      adapter_.write(",\n", 2);
    }

    adapter_.write(indent_string_.data(), new_indent);
    dump(*it, indent_step, new_indent);
    adapter_.write('\n');

    adapter_.write(indent_string_.data(), current_indent);
    adapter_.write(']');
  }

  void dump_array(const array_type &array) {
    if (array.empty()) {
      adapter_.write("[]", 2);
      return;
    }

    adapter_.write('[');

    auto it = std::begin(array);

    for (auto last = std::end(array) - 1; it != last; ++it) {
      dump(*it);
      adapter_.write(',');
    }

    dump(*it);

    adapter_.write(']');
  }

  void dump_string(string_type value) {
    adapter_.write('\"');

    std::uint32_t codepoint;
    std::uint8_t state = UTF8_ACCEPT;

    for (std::size_t i = 0; i < value.size(); ++i) {
      const auto byte = static_cast<std::uint8_t>(value[i]);

      switch (decode(state, codepoint, byte)) {
      case UTF8_ACCEPT:
        switch (codepoint) {
        // quotation mark
        case '\"':
          adapter_.write("\\\"", 2);
          break;
        // reverse solidus
        case '\\':
          adapter_.write("\\\\", 2);
          break;
        // backspace
        case '\b':
          adapter_.write("\\b", 2);
          break;
        // formfeed
        case '\f':
          adapter_.write("\\f", 2);
          break;
        // linefeed
        case '\n':
          adapter_.write("\\n", 2);
          break;
        // carriage return
        case '\r':
          adapter_.write("\\r", 2);
          break;
        // horizontal tab
        case '\t':
          adapter_.write("\\t", 2);
          break;
        default:
          if (codepoint <= 0x1F) {
            char buf[7];
            (std::snprintf)(buf, 7, "\\u%04x",
                            static_cast<std::uint16_t>(codepoint));
            adapter_.write(buf, 6);
          } else {
            adapter_.write(value[i]);
          }
          break;
        }
        break;
      case UTF8_REJECT:
        break;
      default:
        adapter_.write(value[i]);
        break;
      }
    }

    adapter_.write('\"');

    if (state != UTF8_ACCEPT) {
      throw std::invalid_argument{"incomplete utf-8 string"};
    }
  }

  static std::uint8_t decode(std::uint8_t &state, std::uint32_t &codep,
                             const std::uint8_t byte) {
    static const std::array<std::uint8_t, 400> utf8d = {{
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 00..1F
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20..3F
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 40..5F
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60..7F
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9, // 80..9F
        7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
        7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
        7,   7,   7,   7,   7,   7,   7,   7,   7,   7, // A0..BF
        8,   8,   2,   2,   2,   2,   2,   2,   2,   2,   2,
        2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
        2,   2,   2,   2,   2,   2,   2,   2,   2,   2, // C0..DF
        0xA, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
        0x3, 0x3, 0x4, 0x3, 0x3, // E0..EF
        0xB, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
        0x8, 0x8, 0x8, 0x8, 0x8, // F0..FF
        0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4,
        0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   0,   1,   1,   1,   1,
        1,   0,   1,   0,   1,   1,   1,   1,   1,   1, // s1..s2
        1,   2,   1,   1,   1,   1,   1,   2,   1,   2,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   2,   1,   1,   1,   1,   1,   1,   1,   1, // s3..s4
        1,   2,   1,   1,   1,   1,   1,   1,   1,   2,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   3,   1,   3,   1,   1,   1,   1,   1,   1, // s5..s6
        1,   3,   1,   1,   1,   1,   1,   3,   1,   3,   1,
        1,   1,   1,   1,   1,   1,   3,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1 // s7..s8
    }};

    const std::uint8_t type = utf8d[byte];

    codep = (state != UTF8_ACCEPT) ? (byte & 0x3fu) | (codep << 6u)
                                   : (0xFFu >> type) & (byte);

    std::size_t index =
        256u + static_cast<size_t>(state) * 16u + static_cast<size_t>(type);
    ANROOL_JSON_ASSERT(index < 400);
    state = utf8d[index];
    return state;
  }

  template <typename T> void dump_as_string(T value) {
    const std::string s = std::to_string(value);
    adapter_.write(s.data(), s.size());
  }

  OutputAdapter adapter_;
  std::string indent_string_;
};

} // namespace detail
} // namespace anrool

#endif // SERIALIZER_HPP
