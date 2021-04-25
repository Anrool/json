#ifndef LEXER_HPP
#define LEXER_HPP

#include <macro.hpp>

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <utility>

namespace anrool {
namespace detail {

struct position_type {
  std::size_t total_;
  std::size_t column_;
  std::size_t line_;
};

enum class token_type {
  begin_object,
  end_object,
  name_separator,
  value_separator,
  begin_array,
  end_array,
  string,
  integer_number,
  unsigned_number,
  float_number,
  true_literal,
  false_literal,
  null_literal,
  end,
  error
};

inline const char *to_string(token_type type) {
  switch (type) {
  case token_type::begin_object:
    return "begin_object";
  case token_type::end_object:
    return "end_object";
  case token_type::name_separator:
    return "name_separator";
  case token_type::value_separator:
    return "value_separator";
  case token_type::begin_array:
    return "begin_array";
  case token_type::end_array:
    return "end_array";
  case token_type::string:
    return "string";
  case token_type::integer_number:
    return "integer_number";
  case token_type::unsigned_number:
    return "unsigned_number";
  case token_type::float_number:
    return "float_number";
  case token_type::true_literal:
    return "true_literal";
  case token_type::false_literal:
    return "false_literal";
  case token_type::null_literal:
    return "null_literal";
  case token_type::end:
    return "end";
  case token_type::error:
    return "error";
  default:
    return "unknown";
  }
}

template <typename BasicJsonType, typename InputAdapter> class lexer {
private:
  using object_type = typename BasicJsonType::object_type;
  using array_type = typename BasicJsonType::array_type;
  using string_type = typename BasicJsonType::string_type;
  using integer_number_type = typename BasicJsonType::integer_number_type;
  using unsigned_number_type = typename BasicJsonType::unsigned_number_type;
  using float_number_type = typename BasicJsonType::float_number_type;

  using char_type = typename InputAdapter::char_type;
  using int_type = typename std::char_traits<char_type>::int_type;

public:
  explicit lexer(InputAdapter adapter)
      : adapter_{std::move(adapter)}, next_unget{false}, position_{0, 0, 0},
        integer_number_{0}, unsigned_number_{0}, float_number_{0} {}

  lexer(const lexer &) = delete;
  lexer &operator=(const lexer &) = delete;

  token_type scan() {
    if (position_.total_ == 0 && !skip_bom()) {
      return token_type::error;
    }

    skip_whitespace();

    while (current_ == '/') {
      if (!scan_comment()) {
        return token_type::error;
      }

      skip_whitespace();
    }

    switch (current_) {
    case '{':
      return token_type::begin_object;
    case '}':
      return token_type::end_object;
    case ':':
      return token_type::name_separator;
    case ',':
      return token_type::value_separator;
    case '[':
      return token_type::begin_array;
    case ']':
      return token_type::end_array;
    case '\"':
      return scan_string();
    // add implicit fallthrough
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return scan_number();
    case 't':
      for (char c : {'r', 'u', 'e'}) {
        if (c != get()) {
          return token_type::error;
        }
      }

      return token_type::true_literal;
    case 'f':
      for (char c : {'a', 'l', 's', 'e'}) {
        if (c != get()) {
          return token_type::error;
        }
      }

      return token_type::false_literal;
    case 'n':
      for (char c : {'u', 'l', 'l'}) {
        if (c != get()) {
          return token_type::error;
        }
      }

      return token_type::null_literal;
    case std::char_traits<char_type>::eof():
      return token_type::end;
    default:
      return token_type::error;
    }
  }

  integer_number_type get_integer_number() const { return integer_number_; }

  unsigned_number_type get_unsigned_number() const { return unsigned_number_; }

  float_number_type get_float_number() const { return float_number_; }

  const string_type &get_string() const { return token_buffer_; }

  position_type get_position() const { return position_; }

private:
  void skip_whitespace() {
    do {
      get();
    } while (current_ == ' ' || current_ == '\n' || current_ == '\r' ||
             current_ == '\t');
  }

  bool scan_comment() {
    switch (get()) {
    case '/': {
      while (true) {
        switch (get()) {
        case '\n':
        case '\r':
        case std::char_traits<char_type>::eof():
          return true;
        default:
          break;
        }
      }
    }
    case '*': {
      while (true) {
        switch (get()) {
        case std::char_traits<char_type>::eof():
          return false;
        case '*': {
          switch (get()) {
          case '/':
            return true;
          default:
            unget();
            break;
          }
        }
        default:
          break;
        }
      }
    }
    default:
      return false;
    }
  }

  int get_codepoint() {
    ANROOL_JSON_ASSERT(current_ == 'u');

    int codepoint = 0;

    const unsigned offsets[] = {12u, 8u, 4u, 0u};
    const unsigned negated[3] = {0x30u, 0x37u, 0x57u};

    for (auto offset : offsets) {
      get();

      if ('0' <= current_ && current_ <= '9') {
        codepoint += (current_ - negated[0]) << offset;
      } else if ('A' <= current_ && current_ <= 'F') {
        codepoint += (current_ - negated[1]) << offset;
      } else if ('a' <= current_ && current_ <= 'f') {
        codepoint += (current_ - negated[2]) << offset;
      } else {
        return -1;
      }
    }

    ANROOL_JSON_ASSERT(codepoint >= 0x0 && codepoint <= 0xFFFF);

    return codepoint;
  }

  bool next_byte_in_range(std::initializer_list<int_type> ilist) {
    ANROOL_JSON_ASSERT(ilist.size() == 2 || ilist.size() == 4 ||
                       ilist.size() == 6);

    add(current_);

    for (auto it = std::begin(ilist); it != std::end(ilist); ++it) {
      get();

      if (*it <= current_ && current_ <= *++it) {
        add(current_);
      } else {
        return false;
      }
    }

    return true;
  }

  token_type scan_string() {
    ANROOL_JSON_ASSERT(current_ == '"');

    reset();

    while (true) {
      switch (get()) {
      case std::char_traits<char>::eof():
        return token_type::error;
      case '\"':
        return token_type::string;
      case '\\':
        switch (get()) {
        // quotation mark
        case '\"':
          add('\"');
          break;
        // reverse solidus
        case '\\':
          add('\\');
          break;
        // solidus
        case '/':
          add('/');
          break;
        // backspace
        case 'b':
          add('\b');
          break;
        // form feed
        case 'f':
          add('\f');
          break;
        // line feed
        case 'n':
          add('\n');
          break;
        // carriage return
        case 'r':
          add('\r');
          break;
        // tab
        case 't':
          add('\t');
          break;
        case 'u': {
          const int codepoint_1 = get_codepoint();

          if (codepoint_1 == -1) {
            return token_type::error;
          }

          int codepoint = codepoint_1;

          // high surrogate
          if (0xD800 <= codepoint_1 && codepoint_1 <= 0xDBFF) {
            if (get() == '\\' && get() == 'u') {
              const int codepoint_2 = get_codepoint();

              if (codepoint_2 == -1) {
                return token_type::error;
              }

              // low surrogate
              if (0xDC00 <= codepoint_2 && codepoint_2 <= 0xDFFF) {
                codepoint = ((codepoint_1 - 0xD800) << 10u) +
                            (codepoint_2 - 0xDC00) + 0x10000;
              } else {
                return token_type::error;
              }
            } else {
              return token_type::error;
            }
          } else if (0xDC00 <= codepoint_1 && codepoint_1 <= 0xDFFF) {
            return token_type::error;
          }

          ANROOL_JSON_ASSERT(0x0 <= codepoint && codepoint <= 0x10FFFF);

          if (codepoint < 0x80) {
            add(static_cast<int_type>(codepoint));
          } else if (codepoint < 0x800) {
            add(static_cast<int_type>(0xC0 | (codepoint >> 6)));
            add(static_cast<int_type>(0x80 | (codepoint & 0x3F)));
          } else if (codepoint < 0x10000) {
            add(static_cast<int_type>(0xE0 | (codepoint >> 12)));
            add(static_cast<int_type>(0x80 | ((codepoint >> 6) & 0x3F)));
            add(static_cast<int_type>(0x80 | (codepoint & 0x3F)));
          } else {
            add(static_cast<int_type>(0xF0 | (codepoint >> 18)));
            add(static_cast<int_type>(0x80 | ((codepoint >> 12) & 0x3F)));
            add(static_cast<int_type>(0x80 | ((codepoint >> 6) & 0x3F)));
            add(static_cast<int_type>(0x80 | (codepoint & 0x3F)));
          }
          break;
        }
        default:
          return token_type::error;
        }
        break;

      // control characters [0x0, 0x1F]
      case 0x0:
      case 0x1:
      case 0x2:
      case 0x3:
      case 0x4:
      case 0x5:
      case 0x6:
      case 0x7:
      case 0x8:
      case 0x9:
      case 0xA:
      case 0xB:
      case 0xC:
      case 0xD:
      case 0xE:
      case 0xF:
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
      case 0x18:
      case 0x19:
      case 0x1A:
      case 0x1B:
      case 0x1C:
      case 0x1D:
      case 0x1E:
      case 0x1F:
        return token_type::error;

      // 1-byte codepoints [0x20, 0x21] [0x23, 0x5B] [0x5D, 0x7F]
      case 0x20:
      case 0x21:
      // case 0x22: quotation mark
      case 0x23:
      case 0x24:
      case 0x25:
      case 0x26:
      case 0x27:
      case 0x28:
      case 0x29:
      case 0x2A:
      case 0x2B:
      case 0x2C:
      case 0x2D:
      case 0x2E:
      case 0x2F:
      case 0x30:
      case 0x31:
      case 0x32:
      case 0x33:
      case 0x34:
      case 0x35:
      case 0x36:
      case 0x37:
      case 0x38:
      case 0x39:
      case 0x3A:
      case 0x3B:
      case 0x3C:
      case 0x3D:
      case 0x3E:
      case 0x3F:
      case 0x40:
      case 0x41:
      case 0x42:
      case 0x43:
      case 0x44:
      case 0x45:
      case 0x46:
      case 0x47:
      case 0x48:
      case 0x49:
      case 0x4A:
      case 0x4B:
      case 0x4C:
      case 0x4D:
      case 0x4E:
      case 0x4F:
      case 0x50:
      case 0x51:
      case 0x52:
      case 0x53:
      case 0x54:
      case 0x55:
      case 0x56:
      case 0x57:
      case 0x58:
      case 0x59:
      case 0x5A:
      case 0x5B:
      // case 0x5C: reverse solidus
      case 0x5D:
      case 0x5E:
      case 0x5F:
      case 0x60:
      case 0x61:
      case 0x62:
      case 0x63:
      case 0x64:
      case 0x65:
      case 0x66:
      case 0x67:
      case 0x68:
      case 0x69:
      case 0x6A:
      case 0x6B:
      case 0x6C:
      case 0x6D:
      case 0x6E:
      case 0x6F:
      case 0x70:
      case 0x71:
      case 0x72:
      case 0x73:
      case 0x74:
      case 0x75:
      case 0x76:
      case 0x77:
      case 0x78:
      case 0x79:
      case 0x7A:
      case 0x7B:
      case 0x7C:
      case 0x7D:
      case 0x7E:
      case 0x7F:
        add(current_);
        break;

      // 2-byte codepoints [0xC2, 0xDF] [0x80, 0xBF]
      case 0xC2:
      case 0xC3:
      case 0xC4:
      case 0xC5:
      case 0xC6:
      case 0xC7:
      case 0xC8:
      case 0xC9:
      case 0xCA:
      case 0xCB:
      case 0xCC:
      case 0xCD:
      case 0xCE:
      case 0xCF:
      case 0xD0:
      case 0xD1:
      case 0xD2:
      case 0xD3:
      case 0xD4:
      case 0xD5:
      case 0xD6:
      case 0xD7:
      case 0xD8:
      case 0xD9:
      case 0xDA:
      case 0xDB:
      case 0xDC:
      case 0xDD:
      case 0xDE:
      case 0xDF:
        if (!next_byte_in_range({0x80, 0xBF})) {
          return token_type::error;
        }

        break;

      // 3-byte codepoints [0xE0, 0xEF] [0x80, 0xBF] [0x80, 0xBF]
      case 0xE0:
      case 0xE1:
      case 0xE2:
      case 0xE3:
      case 0xE4:
      case 0xE5:
      case 0xE6:
      case 0xE7:
      case 0xE8:
      case 0xE9:
      case 0xEA:
      case 0xEB:
      case 0xEC:
        if (!next_byte_in_range({0x80, 0xBF, 0x80, 0xBF})) {
          return token_type::error;
        }

        break;
      case 0xED:
        if (!next_byte_in_range({0x80, 0x9F, 0x80, 0xBF})) {
          return token_type::error;
        }

        break;
      case 0xEE:
      case 0xEF:
        if (!next_byte_in_range({0x80, 0xBF, 0x80, 0xBF})) {
          return token_type::error;
        }

        break;

      // 4-byte codepoints [0xF0, 0xF7] [0x80, 0xBF] [0x80, 0xBF] [0x80,
      // 0xBF]
      case 0xF0:
        if (!next_byte_in_range({0x90, 0xBF, 0x80, 0xBF, 0x80, 0xBF})) {
          return token_type::error;
        }

        break;
      case 0xF1:
      case 0xF2:
      case 0xF3:
        if (!next_byte_in_range({0x80, 0xBF, 0x80, 0xBF, 0x80, 0xBF})) {
          return token_type::error;
        }

        break;
      case 0xF4:
        if (!next_byte_in_range({0x80, 0x8F, 0x80, 0xBF, 0x80, 0xBF})) {
          return token_type::error;
        }

        break;
      default:
        return token_type::error;
      }
    }
  }

  token_type scan_number() {
    reset();

    token_type type = token_type::unsigned_number;

    switch (current_) {
    case '-':
      add(current_);
      goto label_minus;
    case '0':
      add(current_);
      goto label_zero;
    default:
      ANROOL_JSON_ASSERT(current_ >= '1' && current_ <= '9');
      add(current_);
      goto label_any_1;
    }

  label_minus:
    type = token_type::integer_number;

    switch (get()) {
    case '0':
      add(current_);
      goto label_zero;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      add(current_);
      goto label_any_1;
    default:
      return token_type::error;
    }

  label_zero:
    switch (get()) {
    case '.':
      add(current_);
      goto label_decimal_point;
    case 'e':
    case 'E':
      add(current_);
      goto label_exponent;
    default:
      goto label_number_done;
    }

  label_any_1:
    switch (get()) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      add(current_);
      goto label_any_1;
    case '.':
      add(current_);
      goto label_decimal_point;
    case 'e':
    case 'E':
      add(current_);
      goto label_exponent;
    default:
      goto label_number_done;
    }

  label_decimal_point:
    type = token_type::float_number;

    switch (get()) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      add(current_);
      goto label_decimal;
    default:
      return token_type::error;
    }

  label_decimal:
    switch (get()) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      add(current_);
      goto label_decimal;
    case 'e':
    case 'E':
      add(current_);
      goto label_exponent;
    default:
      goto label_number_done;
    }

  label_exponent:
    type = token_type::float_number;

    switch (get()) {
    case '-':
    case '+':
      add(current_);
      goto label_sign;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      add(current_);
      goto label_any_2;
    default:
      return token_type::error;
    }

  label_sign:
    switch (get()) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      add(current_);
      goto label_any_2;
    default:
      return token_type::error;
    }

  label_any_2:
    switch (get()) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      add(current_);
      goto label_any_2;
    default:
      goto label_number_done;
    }

  label_number_done:
    unget();

    char *endptr = nullptr;
    errno = 0;

    if (type == token_type::unsigned_number) {
      const unsigned long long number =
          std::strtoull(token_buffer_.data(), &endptr, 10);

      ANROOL_JSON_ASSERT(endptr == token_buffer_.data() + token_buffer_.size());

      if (errno == 0) {
        unsigned_number_ = static_cast<unsigned_number_type>(number);
        return type;
      }
      // fallback to float_number
    } else if (type == token_type::integer_number) {
      const long long number = std::strtoll(token_buffer_.data(), &endptr, 10);

      ANROOL_JSON_ASSERT(endptr == token_buffer_.data() + token_buffer_.size());

      if (errno == 0) {
        integer_number_ = static_cast<integer_number_type>(number);
        return type;
      }
      // fallback to float_number
    }

    const long double number = std::strtold(token_buffer_.data(), &endptr);

    ANROOL_JSON_ASSERT(endptr == token_buffer_.data() + token_buffer_.size());

    if (errno == 0) {
      float_number_ = static_cast<float_number_type>(number);
      return token_type::float_number;
    }

    errno = 0;

    return token_type::error;
  }

  int_type get() {
    if (next_unget) {
      next_unget = false;
    } else {
      current_ = adapter_.get_character();
    }

    ++position_.total_;

    if (current_ != '\n') {
      ++position_.column_;
    } else {
      position_.column_ = 0;
      ++position_.line_;
    }

    return current_;
  }

  void unget() {
    next_unget = true;

    --position_.total_;

    if (position_.column_ != 0) {
      --position_.column_;
    } else {
      if (position_.line_ != 0) {
        --position_.line_;
      }
    }
  }

  void add(int_type c) {
    token_buffer_.push_back(static_cast<typename string_type::value_type>(c));
  }

  void reset() { token_buffer_.clear(); }

  bool skip_bom() {
    if (get() == 0xEF) {
      return get() == 0xBB && get() == 0xBF;
    }

    unget();
    return true;
  }

private:
  InputAdapter adapter_;

  int_type current_;
  bool next_unget;

  position_type position_;

  integer_number_type integer_number_;
  unsigned_number_type unsigned_number_;
  float_number_type float_number_;

  string_type token_buffer_;
};

} // namespace detail
} // namespace anrool

#endif // LEXER_HPP
