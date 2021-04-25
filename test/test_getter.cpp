#include <json.hpp>

#include <gtest/gtest.h>

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <valarray>
#include <vector>

using namespace anrool;

template <typename T> void getExplicit(const json &j) {
  T value = j.get<T>();
  EXPECT_EQ(json(value), j);
}

template <typename T> void getImplicit(const json &j) {
  T value = j;
  EXPECT_EQ(json(value), j);
}

TEST(ExplicitGetTest, CheckObjectGetter) {
  json j;

  j = json::object_type{{"ww", true},
                        {"jj", 664},
                        {"tt", nullptr},
                        {"ss", "qwerty"},
                        {"oo", json::object_type{{"eee", 3.3}, {"nn", false}}}};
  ASSERT_EQ(j.type(), json_type::object);

  getExplicit<json::object_type>(j);
  getExplicit<std::map<std::string, json>>(j);
  getExplicit<std::multimap<std::string, json>>(j);
  getExplicit<std::unordered_map<std::string, json>>(j);
  getExplicit<std::unordered_multimap<std::string, json>>(j);

  j = std::map<std::string, int>{{"ggg", 1}, {"rrr", 2}, {"aa", 3}};
  ASSERT_EQ(j.type(), json_type::object);

  getExplicit<std::map<std::string, int>>(j);
  getExplicit<std::multimap<std::string, int>>(j);
  getExplicit<std::unordered_map<std::string, int>>(j);
  getExplicit<std::unordered_multimap<std::string, int>>(j);
}

TEST(ImplicitGetTest, CheckObjectGetter) {
  json j;

  j = json::object_type{{"ww", true},
                        {"jj", 664},
                        {"tt", nullptr},
                        {"ss", "qwerty"},
                        {"oo", json::object_type{{"eee", 3.3}, {"nn", false}}}};
  ASSERT_EQ(j.type(), json_type::object);

  getImplicit<json::object_type>(j);
  getImplicit<std::map<std::string, json>>(j);
  getImplicit<std::multimap<std::string, json>>(j);
  getImplicit<std::unordered_map<std::string, json>>(j);
  getImplicit<std::unordered_multimap<std::string, json>>(j);

  j = std::map<std::string, int>{{"ggg", 1}, {"rrr", 2}, {"aa", 3}};
  ASSERT_EQ(j.type(), json_type::object);

  getImplicit<std::map<std::string, int>>(j);
  getImplicit<std::multimap<std::string, int>>(j);
  getImplicit<std::unordered_map<std::string, int>>(j);
  getImplicit<std::unordered_multimap<std::string, int>>(j);
}

TEST(ExplicitGetTest, CheckArrayGetter) {
  json j;

  j = json::array_type{"wwww", 4, true, nullptr};
  ASSERT_EQ(j.type(), json_type::array);

  getExplicit<json::array_type>(j);
  getExplicit<std::vector<json>>(j);
  getExplicit<std::list<json>>(j);
  getExplicit<std::forward_list<json>>(j);
  getExplicit<std::array<json, 4>>(j);
  getExplicit<std::deque<json>>(j);
  getExplicit<std::valarray<json>>(j);
  getExplicit<std::tuple<std::string, int, bool, std::nullptr_t>>(j);
  getExplicit<std::set<json>>(j);

  std::map<int, double> m = {{4, 5.7}, {1, 0.5}, {2, 8.3}};

  j = std::map<int, double>{{4, 5.7}, {1, 0.5}, {2, 8.3}};
  ASSERT_EQ(j.type(), json_type::array);

  getExplicit<std::map<int, double>>(j);
  getExplicit<std::multimap<int, double>>(j);

  j = std::pair<float, std::string>(3.7F, "jjj");
  ASSERT_EQ(j.type(), json_type::array);

  getExplicit<std::pair<float, std::string>>(j);

  j = std::make_tuple("ggg", false, nullptr);
  ASSERT_EQ(j.type(), json_type::array);

  getExplicit<std::tuple<std::string, bool, std::nullptr_t>>(j);

  int arr[] = {0, 2, 4, 9};
  j = arr;
  ASSERT_EQ(j.type(), json_type::array);

  getExplicit<std::vector<int>>(j);
  getExplicit<std::vector<unsigned>>(j);
  getExplicit<std::vector<double>>(j);
  getExplicit<std::list<int>>(j);
  getExplicit<std::forward_list<int>>(j);
  getExplicit<std::array<int, 4>>(j);
  getExplicit<std::deque<int>>(j);
  getExplicit<std::valarray<int>>(j);
  getExplicit<std::tuple<int, int, int, int>>(j);
  getExplicit<std::set<int>>(j);
}

TEST(ImplicitGetTest, CheckArrayGetter) {
  json j;

  j = json::array_type{"wwww", 4, true, nullptr};
  ASSERT_EQ(j.type(), json_type::array);

  getImplicit<json::array_type>(j);
  getImplicit<std::vector<json>>(j);
  getImplicit<std::list<json>>(j);
  getImplicit<std::forward_list<json>>(j);
  getImplicit<std::array<json, 4>>(j);
  getImplicit<std::deque<json>>(j);
  getImplicit<std::valarray<json>>(j);
  getImplicit<std::tuple<std::string, int, bool, std::nullptr_t>>(j);
  getImplicit<std::set<json>>(j);

  j = std::map<int, double>{{4, 5.7}, {1, 0.5}, {2, 8.3}};
  ASSERT_EQ(j.type(), json_type::array);

  getImplicit<std::map<int, double>>(j);
  getImplicit<std::multimap<int, double>>(j);

  j = std::pair<float, std::string>(3.7F, "jjj");
  ASSERT_EQ(j.type(), json_type::array);

  getImplicit<std::pair<float, std::string>>(j);

  j = std::make_tuple("ggg", false, nullptr);
  ASSERT_EQ(j.type(), json_type::array);

  getImplicit<std::tuple<std::string, bool, std::nullptr_t>>(j);

  int arr[] = {0, 2, 4, 9};
  j = arr;
  ASSERT_EQ(j.type(), json_type::array);

  getImplicit<std::vector<int>>(j);
  getImplicit<std::vector<unsigned>>(j);
  getImplicit<std::vector<double>>(j);
  getImplicit<std::list<int>>(j);
  getImplicit<std::forward_list<int>>(j);
  getImplicit<std::array<int, 4>>(j);
  getImplicit<std::deque<int>>(j);
  getImplicit<std::valarray<int>>(j);
  getImplicit<std::tuple<int, int, int, int>>(j);
  getImplicit<std::set<int>>(j);
}

TEST(ExplicitGetTest, CheckStringGetter) {
  json j;
  const std::string expectedValue("jggut");

  j = expectedValue;
  ASSERT_EQ(j.type(), json_type::string);

  getExplicit<json::string_type>(j);
  getExplicit<std::string>(j);
}

TEST(ImplicitGetTest, CheckStringGetter) {
  json j;
  const std::string expectedValue("jggut");

  j = expectedValue;
  ASSERT_EQ(j.type(), json_type::string);

  getImplicit<json::string_type>(j);
  getImplicit<std::string>(j);
}

template <typename Type, json_type ExpectedType> struct GetNumberParam {
  using ArgType = Type;
  static constexpr json_type expected_type_ = ExpectedType;
};

template <typename T> class GetNumberTest : public testing::Test {
public:
  void SetUp() final {
    J_ = json(typename T::ArgType());
    ASSERT_EQ(J_.type(), expectedType());
  }

  static constexpr json_type expectedType() { return T::expected_type_; }

protected:
  json J_;
};

using GetNumberTypeList = testing::Types<
    GetNumberParam<json::integer_number_type, json_type::integer_number>,
    GetNumberParam<json::unsigned_number_type, json_type::unsigned_number>,
    GetNumberParam<json::float_number_type, json_type::float_number>>;

TYPED_TEST_SUITE(GetNumberTest, GetNumberTypeList);

TYPED_TEST(GetNumberTest, CheckExplicitGetter) {
  getExplicit<json::integer_number_type>(this->J_);
  getExplicit<json::unsigned_number_type>(this->J_);
  getExplicit<json::float_number_type>(this->J_);
  getExplicit<signed char>(this->J_);
  getExplicit<unsigned char>(this->J_);
  getExplicit<signed short>(this->J_);
  getExplicit<unsigned short>(this->J_);
  getExplicit<int>(this->J_);
  getExplicit<unsigned>(this->J_);
  getExplicit<signed long>(this->J_);
  getExplicit<unsigned long>(this->J_);
  getExplicit<signed long long>(this->J_);
  getExplicit<unsigned long long>(this->J_);
}

TYPED_TEST(GetNumberTest, CheckImplicitGetter) {
  getImplicit<json::integer_number_type>(this->J_);
  getImplicit<json::unsigned_number_type>(this->J_);
  getImplicit<json::float_number_type>(this->J_);
  getImplicit<signed char>(this->J_);
  getImplicit<unsigned char>(this->J_);
  getImplicit<signed short>(this->J_);
  getImplicit<unsigned short>(this->J_);
  getImplicit<int>(this->J_);
  getImplicit<unsigned>(this->J_);
  getImplicit<signed long>(this->J_);
  getImplicit<unsigned long>(this->J_);
  getImplicit<signed long long>(this->J_);
  getImplicit<unsigned long long>(this->J_);
}

TEST(ExplicitGetTest, CheckBooleanGetter) {
  json j;
  bool expectedValue;
  bool value;

  expectedValue = false;
  j = json(expectedValue);
  value = j.get<decltype(value)>();
  EXPECT_EQ(value, expectedValue);
  EXPECT_EQ(json(value), j);

  expectedValue = true;
  j = json(expectedValue);
  value = j.get<decltype(value)>();
  EXPECT_EQ(value, expectedValue);
  EXPECT_EQ(json(value), j);
}

TEST(ImplicitGetTest, CheckBooleanGetter) {
  json j;
  bool expectedValue;
  bool value;

  expectedValue = false;
  j = json(expectedValue);
  value = j;
  EXPECT_EQ(value, expectedValue);
  EXPECT_EQ(json(value), j);

  expectedValue = true;
  j = json(expectedValue);
  value = j;
  EXPECT_EQ(value, expectedValue);
  EXPECT_EQ(json(value), j);
}

TEST(ExplicitGetTest, CheckNullGetter) {
  json j;
  std::nullptr_t value = j.get<decltype(value)>();
}

TEST(ImplicitGetTest, CheckNullGetter) {
  json j;
  std::nullptr_t value = j;
  static_cast<void>(value);
}

struct public_entity {
  bool b;
  int i;
  std::string s;

  bool operator==(const public_entity &other) const {
    bool res = true;
    res = b == other.b;
    res = res && i == other.i;
    res = res && s == other.s;
    return res;
  }
};

ANROOL_JSON_DEFINE_NON_INTRUSIVE(public_entity, b, i, s)

class private_entity {
public:
  private_entity() = default;

  private_entity(const json &j, const std::vector<int> &iv,
                 const std::map<std::string, int> &im,
                 const std::vector<public_entity> &pev,
                 const std::map<std::string, public_entity> &pem)
      : j{j}, iv{iv}, im{im}, pev{pev}, pem{pem} {}

  bool operator==(const private_entity &other) const {
    bool res = true;
    res = j == other.j;
    res = res && iv == other.iv;
    res = res && im == other.im;
    res = res && pev == other.pev;
    res = res && pem == other.pem;
    return res;
  }

  ANROOL_JSON_DEFINE_INTRUSIVE(private_entity, j, iv, im, pev, pem)

private:
  json j;
  std::vector<int> iv;
  std::map<std::string, int> im;
  std::vector<public_entity> pev;
  std::map<std::string, public_entity> pem;
};

template <typename T>
void testUDTSerialization(const T &value, const std::string &expectedResult) {
  json j(value);
  json copyJ = value;
  ASSERT_EQ(j, copyJ);

  const T valueCopy = copyJ;
  ASSERT_EQ(value, valueCopy);

  const std::string actualResult = j.dump(4);
  ASSERT_EQ(actualResult, expectedResult);

  json roadtripJ = json::parse(actualResult);
  ASSERT_EQ(j, roadtripJ);

  T roadtripValue = roadtripJ.get<T>();
  ASSERT_EQ(roadtripValue, value);
}

TEST(GetUDTTest, CheckGetter) {
  const public_entity publicEntity{true, 77, std::string{"Vasya"}};
  const std::string publicEntityString = "{\n"
                                         "    \"b\": true,\n"
                                         "    \"i\": 77,\n"
                                         "    \"s\": \"Vasya\"\n"
                                         "}";

  testUDTSerialization(publicEntity, publicEntityString);

  const private_entity privateEntity{
      json(json::array_type{9, 8, 7}),
      {4, 5, 6},
      {{"cc", 1}, {"aaa", 2}},
      {publicEntity, publicEntity},
      {{"gg", publicEntity}, {"eee", publicEntity}}};
  const std::string privateEntityString = "{\n"
                                          "    \"im\": {\n"
                                          "        \"aaa\": 2,\n"
                                          "        \"cc\": 1\n"
                                          "    },\n"
                                          "    \"iv\": [\n"
                                          "        4,\n"
                                          "        5,\n"
                                          "        6\n"
                                          "    ],\n"
                                          "    \"j\": [\n"
                                          "        9,\n"
                                          "        8,\n"
                                          "        7\n"
                                          "    ],\n"
                                          "    \"pem\": {\n"
                                          "        \"eee\": {\n"
                                          "            \"b\": true,\n"
                                          "            \"i\": 77,\n"
                                          "            \"s\": \"Vasya\"\n"
                                          "        },\n"
                                          "        \"gg\": {\n"
                                          "            \"b\": true,\n"
                                          "            \"i\": 77,\n"
                                          "            \"s\": \"Vasya\"\n"
                                          "        }\n"
                                          "    },\n"
                                          "    \"pev\": [\n"
                                          "        {\n"
                                          "            \"b\": true,\n"
                                          "            \"i\": 77,\n"
                                          "            \"s\": \"Vasya\"\n"
                                          "        },\n"
                                          "        {\n"
                                          "            \"b\": true,\n"
                                          "            \"i\": 77,\n"
                                          "            \"s\": \"Vasya\"\n"
                                          "        }\n"
                                          "    ]\n"
                                          "}";

  testUDTSerialization(privateEntity, privateEntityString);
}
