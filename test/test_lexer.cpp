#include <json.hpp>

#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace anrool;

using token_type = detail::token_type;
template <typename BasicJsonType, typename InputAdapter>

using lexer = detail::lexer<BasicJsonType, InputAdapter>;

detail::token_type scan(const std::string &input) {
  auto adapter = detail::input_adapter(input);
  lexer<json, decltype(adapter)> lexer(detail::input_adapter(input));

  return lexer.scan();
}

std::string scanString(const std::string &input) {
  auto adapter = detail::input_adapter(input);
  lexer<json, decltype(adapter)> lexer(std::move(adapter));

  EXPECT_EQ(lexer.scan(), token_type::string);

  const std::string res = lexer.get_string();

  EXPECT_EQ(lexer.scan(), token_type::end);

  return res;
}

class LexerScanTest
    : public testing::TestWithParam<std::tuple<std::string, token_type>> {};

TEST_P(LexerScanTest, CheckScan) {
  const std::string &input = std::get<0>(GetParam());
  const token_type &token = std::get<1>(GetParam());

  ASSERT_EQ(scan(input), token);
}

INSTANTIATE_TEST_SUITE_P(
    StructuralTokenInstantiation, LexerScanTest,
    testing::Values(std::make_tuple("{", token_type::begin_object),
                    std::make_tuple("}", token_type::end_object),
                    std::make_tuple(":", token_type::name_separator),
                    std::make_tuple(",", token_type::value_separator),
                    std::make_tuple("[", token_type::begin_array),
                    std::make_tuple("]", token_type::end_array)));

INSTANTIATE_TEST_SUITE_P(
    LiteralTokenInstantiation, LexerScanTest,
    testing::Values(std::make_tuple("true", token_type::true_literal),
                    std::make_tuple("false", token_type::false_literal),
                    std::make_tuple("null", token_type::null_literal)));

INSTANTIATE_TEST_SUITE_P(
    UnsignedNumberTokenInstantiation, LexerScanTest,
    testing::Values(std::make_tuple("0", token_type::unsigned_number),
                    std::make_tuple("1", token_type::unsigned_number),
                    std::make_tuple("2", token_type::unsigned_number),
                    std::make_tuple("3", token_type::unsigned_number),
                    std::make_tuple("4", token_type::unsigned_number),
                    std::make_tuple("5", token_type::unsigned_number),
                    std::make_tuple("6", token_type::unsigned_number),
                    std::make_tuple("7", token_type::unsigned_number),
                    std::make_tuple("8", token_type::unsigned_number),
                    std::make_tuple("9", token_type::unsigned_number)));

INSTANTIATE_TEST_SUITE_P(
    IntegerNumberTokenInstantiation, LexerScanTest,
    testing::Values(std::make_tuple("-0", token_type::integer_number),
                    std::make_tuple("-1", token_type::integer_number),
                    std::make_tuple("-2", token_type::integer_number),
                    std::make_tuple("-3", token_type::integer_number),
                    std::make_tuple("-4", token_type::integer_number),
                    std::make_tuple("-5", token_type::integer_number),
                    std::make_tuple("-6", token_type::integer_number),
                    std::make_tuple("-7", token_type::integer_number),
                    std::make_tuple("-8", token_type::integer_number),
                    std::make_tuple("-9", token_type::integer_number)));

INSTANTIATE_TEST_SUITE_P(
    FloatNumberTokenInstantiation, LexerScanTest,
    testing::Values(std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("-0E123", token_type::float_number),
                    std::make_tuple("-0e+123", token_type::float_number),
                    std::make_tuple("-0e-123", token_type::float_number),
                    std::make_tuple("-0.345E123", token_type::float_number),
                    std::make_tuple("-0.345e+123", token_type::float_number),
                    std::make_tuple("-0.345E-123", token_type::float_number),
                    std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("0e123", token_type::float_number),
                    std::make_tuple("654.1123", token_type::float_number),
                    std::make_tuple("-654.1123", token_type::float_number),
                    std::make_tuple("654e123", token_type::float_number),
                    std::make_tuple("-654e123", token_type::float_number),
                    std::make_tuple("-654e+123", token_type::float_number),
                    std::make_tuple("-654e-123", token_type::float_number),
                    std::make_tuple("-654.345e123", token_type::float_number),
                    std::make_tuple("-654.345e+123", token_type::float_number),
                    std::make_tuple("-654.345e-123",
                                    token_type::float_number)));

INSTANTIATE_TEST_SUITE_P(WhitespaceInstantiation, LexerScanTest,
                         testing::Values(std::make_tuple("", token_type::end),
                                         std::make_tuple("\n", token_type::end),
                                         std::make_tuple("\r", token_type::end),
                                         std::make_tuple("\t", token_type::end),
                                         std::make_tuple(" \t\n\r\n\t ",
                                                         token_type::end)));

INSTANTIATE_TEST_SUITE_P(
    CommentInstantiation, LexerScanTest,
    testing::Values(std::make_tuple("//", token_type::end),
                    std::make_tuple("/**/", token_type::end),
                    std::make_tuple("//asdfg", token_type::end),
                    std::make_tuple("/*asdfg*/", token_type::end),
                    std::make_tuple("/***/", token_type::end),
                    std::make_tuple("/*/**/", token_type::end),
                    std::make_tuple("/*/* */", token_type::end),
                    std::make_tuple("/****************/", token_type::end),
                    std::make_tuple("//\n", token_type::end),
                    std::make_tuple("//\n//\n", token_type::end),
                    std::make_tuple("/**//**//**/", token_type::end),

                    std::make_tuple("/", token_type::error),
                    std::make_tuple("/!", token_type::error),
                    std::make_tuple("/*", token_type::error),
                    std::make_tuple("/**", token_type::error),
                    std::make_tuple("/** /", token_type::error)));

class LexerScanStringTest
    : public testing::TestWithParam<std::tuple<std::string, std::string>> {};

TEST_P(LexerScanStringTest, CheckScan) {
  const std::string &input = std::get<0>(GetParam());
  const std::string &expectedResult = std::get<1>(GetParam());

  ASSERT_EQ(scanString(input), expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    StringInstantiation, LexerScanStringTest,
    testing::Values(
        std::make_tuple("\"\\\"\"", "\""), std::make_tuple("\"\\\\\"", "\\"),
        std::make_tuple("\"\\/\"", "/"), std::make_tuple("\"\\b\"", "\b"),
        std::make_tuple("\"\\f\"", "\f"), std::make_tuple("\"\\n\"", "\n"),
        std::make_tuple("\"\\r\"", "\r"), std::make_tuple("\"\\t\"", "\t"),
        std::make_tuple("\"\\u0001\"", "\x01"),
        std::make_tuple("\"\\u000a\"", "\n"),
        std::make_tuple("\"\\u00b0\"", "°"),
        std::make_tuple("\"\\u0c00\"", "ఀ"),
        std::make_tuple("\"\\ud000\"", "퀀"),
        std::make_tuple("\"\\u000E\"", "\x0E"),
        std::make_tuple("\"\\u00F0\"", "ð"),
        std::make_tuple("\"\\u0100\"", "Ā"),
        std::make_tuple("\"\\u2000\"", " "),
        std::make_tuple("\"\\uFFFF\"", "￿"),
        std::make_tuple("\"\\u20AC\"", "€"), std::make_tuple("\"€\"", "€"),
        std::make_tuple("\"🎈\"", "🎈"),
        std::make_tuple("\"\\ud83c\\udf1e\"", "🌞")));

class LexerScanASCIITest : public testing::TestWithParam<int> {};

TEST_P(LexerScanASCIITest, CheckASCII) {
  const char c = static_cast<char>(GetParam());
  const std::string input(1, c);
  const token_type token = scan(input);

  switch (c) {
  case '{':
  case '}':
  case ':':
  case ',':
  case '[':
  case ']':
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
    ASSERT_NE(token, token_type::error);
    break;
  case ' ':
  case '\n':
  case '\r':
  case '\t':
    ASSERT_EQ(token, token_type::end);
    break;
  default:
    ASSERT_EQ(token, token_type::error);
    break;
  }
}

INSTANTIATE_TEST_SUITE_P(ASCIIInstantiation, LexerScanASCIITest,
                         testing::Range(0x0, 0x80));

class LexerScanAdvanceTest
    : public testing::TestWithParam<
          std::tuple<std::string, std::vector<token_type>>> {};

TEST_P(LexerScanAdvanceTest, CheckScan) {
  const std::string &input = std::get<0>(GetParam());
  const std::vector<token_type> &tokens = std::get<1>(GetParam());
  auto adapter = detail::input_adapter(input);
  lexer<json, decltype(adapter)> lexer(std::move(adapter));

  for (token_type token : tokens) {
    ASSERT_EQ(lexer.scan(), token);
  }

  ASSERT_EQ(lexer.scan(), token_type::end);
}

INSTANTIATE_TEST_SUITE_P(
    AdvanceInstantiation, LexerScanAdvanceTest,
    testing::Values(
        std::make_tuple("[]", std::vector<token_type>{token_type::begin_array,
                                                      token_type::end_array}),
        std::make_tuple("[\n    -7777\n]",
                        std::vector<token_type>{token_type::begin_array,
                                                token_type::integer_number,
                                                token_type::end_array}),
        std::make_tuple("[\n    4,\n    true,\n    \"ssss\"\n]",
                        std::vector<token_type>{token_type::begin_array,
                                                token_type::unsigned_number,
                                                token_type::value_separator,
                                                token_type::true_literal,
                                                token_type::value_separator,
                                                token_type::string,
                                                token_type::end_array}),
        std::make_tuple(
            "[\n    4,\n    true,\n    [\n        null,\n        {\n           "
            " \"aa\": 88,\n            \"bb\": false\n        },\n        "
            "\"ww\",\n        [\n            false,\n            \"yy\"\n      "
            "  ]\n    ],\n    \"ssss\"\n]",
            std::vector<token_type>{
                token_type::begin_array,     token_type::unsigned_number,
                token_type::value_separator, token_type::true_literal,
                token_type::value_separator, token_type::begin_array,
                token_type::null_literal,    token_type::value_separator,
                token_type::begin_object,    token_type::string,
                token_type::name_separator,  token_type::unsigned_number,
                token_type::value_separator, token_type::string,
                token_type::name_separator,  token_type::false_literal,
                token_type::end_object,      token_type::value_separator,
                token_type::string,          token_type::value_separator,
                token_type::begin_array,     token_type::false_literal,
                token_type::value_separator, token_type::string,
                token_type::end_array,       token_type::end_array,
                token_type::value_separator, token_type::string,
                token_type::end_array}),
        std::make_tuple("{}", std::vector<token_type>{token_type::begin_object,
                                                      token_type::end_object}),
        std::make_tuple("{\n    \"ggg\": 6\n}",
                        std::vector<token_type>{token_type::begin_object,
                                                token_type::string,
                                                token_type::name_separator,
                                                token_type::unsigned_number,
                                                token_type::end_object}),
        std::make_tuple("{\n    \"ggg\": 6,\n    \"ww\": false\n}",
                        std::vector<token_type>{
                            token_type::begin_object, token_type::string,
                            token_type::name_separator,
                            token_type::unsigned_number,
                            token_type::value_separator, token_type::string,
                            token_type::name_separator,
                            token_type::false_literal, token_type::end_object}),
        std::make_tuple(
            "{\n    \"aa\": 6,\n    \"bbbbb\": [\n        true,\n        "
            "null,\n        \"dd\"\n    ],\n    \"cc\": {\n        \"aa\": "
            "null,\n        \"bb\": {\n            \"aa\": false,\n            "
            "\"bbbb\": {}\n        },\n        \"ccc\": 55555\n    },\n    "
            "\"rrr\": false\n}",
            std::vector<token_type>{
                token_type::begin_object,    token_type::string,
                token_type::name_separator,  token_type::unsigned_number,
                token_type::value_separator, token_type::string,
                token_type::name_separator,  token_type::begin_array,
                token_type::true_literal,    token_type::value_separator,
                token_type::null_literal,    token_type::value_separator,
                token_type::string,          token_type::end_array,
                token_type::value_separator, token_type::string,
                token_type::name_separator,  token_type::begin_object,
                token_type::string,          token_type::name_separator,
                token_type::null_literal,    token_type::value_separator,
                token_type::string,          token_type::name_separator,
                token_type::begin_object,    token_type::string,
                token_type::name_separator,  token_type::false_literal,
                token_type::value_separator, token_type::string,
                token_type::name_separator,  token_type::begin_object,
                token_type::end_object,      token_type::end_object,
                token_type::value_separator, token_type::string,
                token_type::name_separator,  token_type::unsigned_number,
                token_type::end_object,      token_type::value_separator,
                token_type::string,          token_type::name_separator,
                token_type::false_literal,   token_type::end_object})));
