#include <json.hpp>

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

using namespace anrool;

template <typename T, json_type ExpectedType>
class SerializerTest
    : public testing::TestWithParam<std::tuple<T, std::string>> {
public:
  void SetUp() final {
    json j;
    std::string expectedResult;

    std::tie(j, expectedResult) = this->GetParam();

    EXPECT_EQ(j.type(), ExpectedType);
    EXPECT_EQ(j.dump(), expectedResult);
  }
};

using ObjectSerilizerTest =
    SerializerTest<json::object_type, json_type::object>;
using ArraySerilizerTest = SerializerTest<json::array_type, json_type::array>;
using IntegerNumberSerilizerTest =
    SerializerTest<json::integer_number_type, json_type::integer_number>;
using UnsignedNumberSerilizerTest =
    SerializerTest<json::unsigned_number_type, json_type::unsigned_number>;
using FloatNumberSerilizerTest =
    SerializerTest<json::float_number_type, json_type::float_number>;
using BooleanSerilizerTest =
    SerializerTest<json::boolean_type, json_type::boolean>;
using NullSerilizerTest = SerializerTest<std::nullptr_t, json_type::null>;

class StringSerializerSuccessTest
    : public testing::TestWithParam<std::tuple<std::string, std::string>> {
public:
  void SetUp() final { std::tie(J_, ExpectedResult_) = GetParam(); }

protected:
  json J_;
  std::string ExpectedResult_;
};

class StringSerializerFailureTest : public testing::TestWithParam<std::string> {
public:
  void SetUp() final { J_ = GetParam(); }

protected:
  json J_;
};

TEST_P(ObjectSerilizerTest, Test) {}

TEST_P(ArraySerilizerTest, Test) {}

TEST_P(StringSerializerSuccessTest, Test) {
  try {
    EXPECT_EQ(J_.dump(), "\"" + ExpectedResult_ + "\"");
  } catch (std::invalid_argument &e) {
    EXPECT_TRUE(false) << "Unexpected error";
  } catch (...) {
    EXPECT_TRUE(false) << "Unexpected exception";
  }
}

TEST_P(StringSerializerFailureTest, Test) {
  try {
    static_cast<void>(J_.dump());
    EXPECT_TRUE(false) << "Expected an error";
  } catch (std::invalid_argument &e) {
  } catch (...) {
    EXPECT_TRUE(false) << "Unexpected exception";
  }
}

TEST_P(IntegerNumberSerilizerTest, Test) {}

TEST_P(UnsignedNumberSerilizerTest, Test) {}

TEST_P(FloatNumberSerilizerTest, Test) {}

TEST_P(BooleanSerilizerTest, Test) {}

TEST_P(NullSerilizerTest, Test) {}

INSTANTIATE_TEST_SUITE_P(
    ObjectInstantiation, ObjectSerilizerTest,
    testing::Values(
        std::make_tuple(json::object_type{}, "{}"),
        std::make_tuple(json::object_type{{"ggg", 6}}, "{\"ggg\":6}"),
        std::make_tuple(json::object_type{{"ggg", 6}, {"ww", false}},
                        "{\"ggg\":6,\"ww\":false}"),
        std::make_tuple(
            json::object_type{
                {"aa", 6},
                {"bbbbb", json::array_type{true, nullptr, "dd"}},
                {"cc",
                 json::object_type{
                     {"aa", nullptr},
                     {"bb", json::object_type{{"aa", false},
                                              {"bbbb", json::object_type{}}}},
                     {"ccc", 55555}}},
                {"rrr", false}},
            "{\"aa\":6,\"bbbbb\":[true,null,\"dd\"],\"cc\":{\"aa\":null,\"bb\":"
            "{\"aa\":false,\"bbbb\":{}},\"ccc\":55555},\"rrr\":false}")));

INSTANTIATE_TEST_SUITE_P(
    ArrayInstantiation, ArraySerilizerTest,
    testing::Values(
        std::make_tuple(json::array_type{}, "[]"),
        std::make_tuple(json::array_type{-7777}, "[-7777]"),
        std::make_tuple(json::array_type{4, true, "ssss"}, "[4,true,\"ssss\"]"),
        std::make_tuple(
            json::array_type{
                4, true,
                json::array_type{nullptr,
                                 json::object_type{{"aa", 88}, {"bb", false}},
                                 "ww", json::array_type{false, "yy"}},
                "ssss"},
            "[4,true,[null,{\"aa\":88,\"bb\":false},\"ww\",[false,\"yy\"]],"
            "\"ssss\"]")));

INSTANTIATE_TEST_SUITE_P(
    StringSerializerSuccessInstantiation, StringSerializerSuccessTest,
    testing::Values(
        std::make_tuple("{\"x\":5,\"y\":6}", "{\\\"x\\\":5,\\\"y\\\":6}"),
        std::make_tuple("{\"x\":[10,null,null,null]}",
                        "{\\\"x\\\":[10,null,null,null]}"),
        std::make_tuple("test", "test"),
        std::make_tuple("[3,\"false\",false]", "[3,\\\"false\\\",false]"),
        std::make_tuple("/", "/")));

INSTANTIATE_TEST_SUITE_P(StringSerializerFailureInstantiation,
                         StringSerializerFailureTest,
                         testing::Values("ä\xA9ü", "123\xC2",
                                         "123\xF1\xB0\x34\x35\x36"));

INSTANTIATE_TEST_SUITE_P(
    IntegerNumberInstantiation, IntegerNumberSerilizerTest,
    testing::Values(
        std::make_tuple(0, "0"), std::make_tuple(45, "45"),
        std::make_tuple(-45, "-45"),
        std::make_tuple(
            std::numeric_limits<json::integer_number_type>::min(),
            std::to_string(
                std::numeric_limits<json::integer_number_type>::min())),
        std::make_tuple(
            std::numeric_limits<json::integer_number_type>::max(),
            std::to_string(
                std::numeric_limits<json::integer_number_type>::max()))));

INSTANTIATE_TEST_SUITE_P(
    UnsignedNumberInstantiation, UnsignedNumberSerilizerTest,
    testing::Values(
        std::make_tuple(0, "0"), std::make_tuple(45, "45"),
        std::make_tuple(
            std::numeric_limits<json::unsigned_number_type>::min(),
            std::to_string(
                std::numeric_limits<json::unsigned_number_type>::min())),
        std::make_tuple(
            std::numeric_limits<json::unsigned_number_type>::max(),
            std::to_string(
                std::numeric_limits<json::unsigned_number_type>::max()))));

INSTANTIATE_TEST_SUITE_P(
    FloatNumberInstantiation, FloatNumberSerilizerTest,
    testing::Values(
        std::make_tuple(0.0, std::to_string(0.0)),
        std::make_tuple(-45.927, std::to_string(-45.927)),
        std::make_tuple(
            std::numeric_limits<json::float_number_type>::min(),
            std::to_string(
                std::numeric_limits<json::float_number_type>::min())),
        std::make_tuple(
            std::numeric_limits<json::float_number_type>::max(),
            std::to_string(
                std::numeric_limits<json::float_number_type>::max()))));

INSTANTIATE_TEST_SUITE_P(BooleanInstantiation, BooleanSerilizerTest,
                         testing::Values(std::make_tuple(true, "true"),
                                         std::make_tuple(false, "false")));

INSTANTIATE_TEST_SUITE_P(NullInstantiation, NullSerilizerTest,
                         testing::Values(std::make_tuple(nullptr, "null")));
