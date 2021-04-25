#ifndef PARSER_HPP
#define PARSER_HPP

#include <json_type.hpp>
#include <lexer.hpp>
#include <macro.hpp>

#include <string>
#include <utility>
#include <vector>

namespace anrool {
namespace detail {

template <typename BasicJsonType, typename InputAdapter> class parser {
public:
  using lexer_type = lexer<BasicJsonType, InputAdapter>;

  explicit parser(InputAdapter adapter) : lexer_{std::move(adapter)} {}

  void parse(BasicJsonType &j) {
    sax_parse_internal(j);

    if (get_token() != token_type::end) {
      notify_error();
    }
  }

private:
  BasicJsonType handle_value() const {
    switch (token_) {
    case token_type::string:
      return BasicJsonType(lexer_.get_string());
    case token_type::integer_number:
      return BasicJsonType(lexer_.get_integer_number());
    case token_type::unsigned_number:
      return BasicJsonType(lexer_.get_unsigned_number());
    case token_type::float_number:
      return BasicJsonType(lexer_.get_float_number());
    case token_type::true_literal:
      return BasicJsonType(true);
    case token_type::false_literal:
      return BasicJsonType(false);
    case token_type::null_literal:
      return BasicJsonType(nullptr);
    default:
      notify_error();
      return BasicJsonType();
    }
  }

  template <typename T> BasicJsonType *add_value(T &&val) {
    ANROOL_JSON_ASSERT(json_stack_.back()->type() == json_type::object ||
                       json_stack_.back()->type() == json_type::array);

    BasicJsonType *curr = json_stack_.back();

    if (curr->type() == json_type::object) {
      ANROOL_JSON_ASSERT(object_element != nullptr);

      *object_element = std::forward<T>(val);

      BasicJsonType *tmp = object_element;
      object_element = nullptr;

      return tmp;
    } else {
      curr->value_.array_->emplace_back(std::forward<T>(val));
      return &curr->value_.array_->back();
    }
  }

  void handle_key() {
    ANROOL_JSON_ASSERT(object_element == nullptr);
    ANROOL_JSON_ASSERT(json_stack_.back()->type() == json_type::object);

    object_element =
        &json_stack_.back()
             ->value_.object_->emplace(lexer_.get_string(), nullptr)
             .first->second;
  }

  bool handle_closing_tokens() {
    while (true) {
      ANROOL_JSON_ASSERT(json_stack_.back()->type() == json_type::object ||
                         json_stack_.back()->type() == json_type::array);

      if (json_stack_.back()->type() == json_type::array) {
        if (get_token() == token_type::value_separator) {
          return false;
        }

        if (token_ != token_type::end_array) {
          notify_error();
        }
      } else {
        if (get_token() == token_type::value_separator) {
          if (get_token() == token_type::string) {
            handle_key();

            if (get_token() == token_type::name_separator) {
              return false;
            }
          }

          notify_error();
        }

        if (token_ != token_type::end_object) {
          notify_error();
        }
      }

      ANROOL_JSON_ASSERT(!json_stack_.empty());

      json_stack_.pop_back();

      if (json_stack_.empty()) {
        return true;
      }
    }
  }

  void sax_parse_internal(BasicJsonType &j) {
    get_token();

    if (token_ == token_type::begin_object) {
      j = BasicJsonType(json_type::object);

      if (get_token() == token_type::end_object) {
        return;
      }

      json_stack_.push_back(&j);

      if (token_ != token_type::string) {
        notify_error();
      }

      handle_key();

      if (get_token() != token_type::name_separator) {
        notify_error();
      }

      get_token();
    } else if (token_ == token_type::begin_array) {
      j = BasicJsonType(json_type::array);

      if (get_token() == token_type::end_array) {
        return;
      }

      json_stack_.push_back(&j);
    } else {
      j = handle_value();
      return;
    }

    while (true) {
      switch (token_) {
      case token_type::begin_object:
        json_stack_.emplace_back(add_value(json_type::object));

        if (get_token() == token_type::end_object) {
          json_stack_.pop_back();
          break;
        }

        if (token_ != token_type::string) {
          notify_error();
        }

        handle_key();

        if (get_token() != token_type::name_separator) {
          notify_error();
        }

        get_token();
        continue;
      case token_type::begin_array:
        json_stack_.emplace_back(add_value(json_type::array));

        if (get_token() == token_type::end_array) {
          json_stack_.pop_back();
          break;
        }

        continue;
      default:
        add_value(handle_value());
        break;
      }

      if (handle_closing_tokens()) {
        return;
      }

      get_token();
    }
  }

  token_type get_token() { return token_ = lexer_.scan(); }

  void notify_error(const std::string &msg = "") const {
    const position_type pos = lexer_.get_position();

    std::string res = "parser error on line " + std::to_string(pos.line_ + 1) +
                      ", column " + std::to_string(pos.column_);

    if (token_ != token_type::error) {
      res += std::string(", unexpected token ") + to_string(token_);
    }

    if (!msg.empty()) {
      res += ", " + msg;
    }

    throw std::runtime_error(res);
  }

  std::vector<BasicJsonType *> json_stack_;
  BasicJsonType *object_element = nullptr;

  lexer_type lexer_;
  token_type token_;
};

} // namespace detail
} // namespace anrool

#endif // PARSER_HPP
