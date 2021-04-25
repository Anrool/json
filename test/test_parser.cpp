#include <json.hpp>

#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <utility>

using namespace anrool;

json parse(const std::string &input, bool shouldFail = false) {
  auto adapter = detail::input_adapter(input);
  detail::parser<json, decltype(adapter)> parser(std::move(adapter));
  json j;

  try {
    parser.parse(j);
    EXPECT_FALSE(shouldFail) << "Expected an error";
  } catch (std::runtime_error &e) {
    EXPECT_TRUE(shouldFail) << "Unexpected error";
  } catch (...) {
    EXPECT_TRUE(false) << "Unexpected exception";
  }

  return j;
}

class ParseTest : public testing::TestWithParam<std::tuple<std::string, json>> {
};

TEST_P(ParseTest, CheckParse) {
  const std::string &input = std::get<0>(GetParam());
  const json &expectedResult = std::get<1>(GetParam());

  ASSERT_EQ(parse(input), expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    ObjectInstantiation, ParseTest,
    testing::Values(
        std::make_tuple("{}", json::object_type{}),
        std::make_tuple("{            }", json::object_type{}),
        std::make_tuple("{\"hhh\": 5}", json::object_type{{"hhh", 5}}),
        std::make_tuple("{\"ww\": false, \"hhh\": 5, \"eeee\": null}",
                        json::object_type{
                            {"ww", false}, {"hhh", 5}, {"eeee", nullptr}}),
        std::make_tuple(
            "{\n    \"aa\": 6,\n    \"bbbbb\": [\n        true,\n        "
            "null,\n        \"dd\"\n    ],\n    \"cc\": {\n        \"aa\": "
            "null,\n        \"bb\": {\n            \"aa\": false,\n            "
            "\"bbbb\": {}\n        },\n        \"ccc\": 55555\n    },\n    "
            "\"rrr\": false\n}",
            json::object_type{
                {"aa", 6},
                {"bbbbb", json::array_type{true, nullptr, "dd"}},
                {"cc",
                 json::object_type{
                     {"aa", nullptr},
                     {"bb", json::object_type{{"aa", false},
                                              {"bbbb", json::object_type{}}}},
                     {"ccc", 55555}}},
                {"rrr", false}})));

INSTANTIATE_TEST_SUITE_P(
    ArrayInstantiation, ParseTest,
    testing::Values(
        std::make_tuple("[]", json::array_type{}),
        std::make_tuple("[            ]", json::array_type{}),
        std::make_tuple("[false]", json::array_type{false}),
        std::make_tuple("[true, false, null]",
                        json::array_type{true, false, nullptr}),
        std::make_tuple(
            "[\n    4,\n    true,\n    [\n        null,\n        {\n           "
            " \"aa\": 88,\n            \"bb\": false\n        },\n        "
            "\"ww\",\n        [\n            false,\n            \"yy\"\n      "
            "  ]\n    ],\n    \"ssss\"\n]",
            json::array_type{
                4, true,
                json::array_type{nullptr,
                                 json::object_type{{"aa", 88}, {"bb", false}},
                                 "ww", json::array_type{false, "yy"}},
                "ssss"})));

INSTANTIATE_TEST_SUITE_P(
    StringInstantiation, ParseTest,
    testing::Values(
        std::make_tuple("\"\"", json::string_type{}),
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

INSTANTIATE_TEST_SUITE_P(
    NumberInstantiation, ParseTest,
    testing::Values(std::make_tuple("0", 0), std::make_tuple("1", 1),
                    std::make_tuple("2", 2), std::make_tuple("3", 3),
                    std::make_tuple("4", 4), std::make_tuple("5", 5),
                    std::make_tuple("6", 6), std::make_tuple("7", 7),
                    std::make_tuple("8", 8), std::make_tuple("9", 9),
                    std::make_tuple("0", 0), std::make_tuple("-1", -1),
                    std::make_tuple("-2", -2), std::make_tuple("-3", -3),
                    std::make_tuple("-4", -4), std::make_tuple("-5", -5),
                    std::make_tuple("-6", -6), std::make_tuple("-7", -7),
                    std::make_tuple("-8", -8), std::make_tuple("-9", -9),

                    std::make_tuple("0e123", 0e123),
                    std::make_tuple("-0E123", -0E123),
                    std::make_tuple("-0e+123", -0e+123),
                    std::make_tuple("-0e-123", -0e-123),
                    std::make_tuple("-0.345E123", -0.345E123),
                    std::make_tuple("-0.345e+123", -0.345e+123),
                    std::make_tuple("-0.345E-123", -0.345E-123),
                    std::make_tuple("654.1123", 654.1123),
                    std::make_tuple("-654.1123", -654.1123),
                    std::make_tuple("654e123", 654e123),
                    std::make_tuple("-654e123", -654e123),
                    std::make_tuple("-654e+123", -654e+123),
                    std::make_tuple("-654e-123", -654e-123),
                    std::make_tuple("-654.345e123", -654.345e123),
                    std::make_tuple("-654.345e+123", -654.345e+123),
                    std::make_tuple("-654.345e-123", -654.345e-123)));

INSTANTIATE_TEST_SUITE_P(LiteralInstantiation, ParseTest,
                         testing::Values(std::make_tuple("null", nullptr),
                                         std::make_tuple("true", true),
                                         std::make_tuple("false", false)));

class ParseErrorTest : public testing::TestWithParam<std::string> {};

TEST_P(ParseErrorTest, CheckError) {
  const std::string &input = GetParam();

  parse(input, true);
}

INSTANTIATE_TEST_SUITE_P(ObjectErrorInstantiation, ParseErrorTest,
                         testing::Values("{", "{\"foo\"", "{\"foo\":",
                                         "{\"foo\":}", "{\"foo\":1,}", "}"));

INSTANTIATE_TEST_SUITE_P(ArrayErrorInstantiation, ParseErrorTest,
                         testing::Values("[", "[1", "[1,", "[1,]", "]"));

INSTANTIATE_TEST_SUITE_P(StringErrorInstantiation, ParseErrorTest,
                         testing::Values("\"", "\"\\\"", "\"\\u\"", "\"\\u0\"",
                                         "\"\\u01\"", "\"\\u012\"", "\"\\u",
                                         "\"\\u0", "\"\\u01", "\"\\u012"));

INSTANTIATE_TEST_SUITE_P(NumberErrorInstantiation, ParseErrorTest,
                         testing::Values("01", "--1", "1.", "1E", "1E-", "1.E1",
                                         "-1E", "-0E#", "-0E-#", "-0#",
                                         "-0.0:", "-0.0Z", "-0E123:", "-0e0-:",
                                         "-0e-:", "-0f", "+1", "+0"));

INSTANTIATE_TEST_SUITE_P(UnexpectedEndOfNumberInstantiation, ParseErrorTest,
                         testing::Values("0.", "-", "--", "-0.", "-.",
                                         "-:", "0.:", "e.", "1e.", "1e/",
                                         "1e:", "1E.", "1E/", "1E:"));

INSTANTIATE_TEST_SUITE_P(UnexpectedEnfOfTrueInstantiation, ParseErrorTest,
                         testing::Values("t", "tr", "tru", "trud", "truf"));

INSTANTIATE_TEST_SUITE_P(UnexpectedEnfOfFalseInstantiation, ParseErrorTest,
                         testing::Values("f", "fa", "fal", "fals", "falsd",
                                         "falsf"));

INSTANTIATE_TEST_SUITE_P(UnexpectedEnfOfNullInstantiation, ParseErrorTest,
                         testing::Values("n", "nu", "nul", "nulk", "nulm"));

INSTANTIATE_TEST_SUITE_P(MissingSurrogateInstantiation, ParseErrorTest,
                         testing::Values("\"\\uD80C\"", "\"\\uD80C\\uD80C\"",
                                         "\"\\uD80C\\u0000\"",
                                         "\"\\uD80C\\uFFFF\""));

class ParseASCIITest : public testing::TestWithParam<int> {};

TEST_P(ParseASCIITest, CheckASCII) {
  const char c = static_cast<char>(GetParam());
  const std::string input =
      std::string("\"\\") + std::string(1, c) + std::string("\"");

  switch (c) {
  case '\"':
  case '\\':
  case '/':
  case 'b':
  case 'f':
  case 'n':
  case 'r':
  case 't':
    parse(input);
    break;
  default:
    parse(input, true);
    break;
  }
}

TEST_P(ParseASCIITest, CheckEscapedASCII) {
  const char c = static_cast<char>(GetParam());
  const std::string input =
      std::string("\"\\") + std::string(1, c) + std::string("\"");
  const std::string prefix = "\"\\u";
  const std::string s1 =
      prefix + "000" + std::string(1, static_cast<char>(c)) + "\"";
  const std::string s2 =
      prefix + "00" + std::string(1, static_cast<char>(c)) + "0\"";
  const std::string s3 =
      prefix + "0" + std::string(1, static_cast<char>(c)) + "00\"";
  const std::string s4 =
      prefix + std::string(1, static_cast<char>(c)) + "000\"";
  bool invalidNumber = true;

  switch (c) {
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
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    invalidNumber = false;
    break;
  default:
    break;
  }

  parse(s1, invalidNumber);
  parse(s2, invalidNumber);
  parse(s3, invalidNumber);
  parse(s4, invalidNumber);
}

INSTANTIATE_TEST_SUITE_P(ASCIIInstantiation, ParseASCIITest,
                         testing::Range(0x0, 0x80));
