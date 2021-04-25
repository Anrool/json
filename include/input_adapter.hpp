#ifndef INPUT_ADAPTER_HPP
#define INPUT_ADAPTER_HPP

#include <macro.hpp>

#include <istream>
#include <string>
#include <utility>

namespace anrool {
namespace detail {

class input_stream_adapter {
public:
  using char_type = char;

  explicit input_stream_adapter(std::istream &stream) : buf_{stream.rdbuf()} {}

  std::char_traits<char>::int_type get_character() const {
    return buf_->sbumpc();
  }

private:
  std::streambuf *buf_;
};

template <typename StringType> class input_string_adapter {
public:
  using char_type = typename StringType::value_type;

  input_string_adapter(const input_string_adapter &other)
      : string_{other.string_}, it_{std::begin(string_)} {}

  input_string_adapter &operator=(const input_string_adapter &other) {
    if (this != &other) {
      string_ = other.string_;
      it_ = std::begin(string_);
    }

    return *this;
  }

  explicit input_string_adapter(StringType string)
      : string_{std::move(string)}, it_{std::begin(string_)} {}

  typename std::char_traits<char_type>::int_type get_character() {
    if (it_ != std::end(string_)) {
      return std::char_traits<char_type>::to_int_type(*it_++);
    }

    return std::char_traits<char_type>::eof();
  }

  JSON_PRIVATE_UNLESS_TESTED : StringType string_;
  typename StringType::iterator it_;
};

inline input_stream_adapter input_adapter(std::istream &stream) {
  return input_stream_adapter{stream};
}

template <typename CharType>
input_string_adapter<std::basic_string<CharType>>
input_adapter(const std::basic_string<CharType> &string) {
  return input_string_adapter<std::basic_string<CharType>>{string};
}

} // namespace detail
} // namespace anrool

#endif // INPUT_ADAPTER_HPP
