#ifndef OUTPUT_ADAPTER_HPP
#define OUTPUT_ADAPTER_HPP

#include <cstddef>
#include <ostream>
#include <string>

namespace anrool {
namespace detail {

template <typename CharType> class output_stream_adapter {
public:
  explicit output_stream_adapter(std::basic_ostream<CharType> &stream)
      : stream_{stream} {}

  void write(CharType c) { stream_.put(c); }

  void write(const CharType *c, std::size_t length) {
    stream_.write(c, length);
  }

private:
  std::basic_ostream<CharType> &stream_;
};

template <typename CharType> class output_string_adapter {
public:
  explicit output_string_adapter(std::basic_string<CharType> &string)
      : string_{string} {}

  void write(CharType c) { string_ += c; }

  void write(const CharType *c, std::size_t length) {
    string_.append(c, length);
  }

private:
  std::basic_string<CharType> &string_;
};

template <typename CharType>
output_stream_adapter<CharType>
output_adapter(std::basic_ostream<CharType> &stream) {
  return output_stream_adapter<CharType>{stream};
}

template <typename CharType>
output_string_adapter<CharType>
output_adapter(std::basic_string<CharType> &string) {
  return output_string_adapter<CharType>{string};
}

} // namespace detail
} // namespace anrool

#endif // OUTPUT_ADAPTER_HPP
