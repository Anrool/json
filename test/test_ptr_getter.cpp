#include <json.hpp>

#include <gtest/gtest.h>

#include <type_traits>

using namespace anrool;

template <typename Type, json_type ExpType> struct GetPtrParam {
  using ArgType = Type;
  static constexpr json_type expected_type_ = ExpType;
};

template <typename T> class GetPtrTest : public testing::Test {
protected:
  template <bool ConstTest> void doTest() {
    using JSONType =
        typename std::conditional<ConstTest, const json, json>::type;
    using argType =
        typename std::conditional<ConstTest, const ArgType, ArgType>::type;

    JSONType j(argType{});
    ASSERT_EQ(j.type(), expectedType());

    argType *ptr = j.template get_ptr<ArgType>();
    ASSERT_NE(ptr, nullptr);
    ASSERT_EQ(JSONType(*ptr), j);

    checkGetPtr<json::object_type>(j);
    checkGetPtr<json::array_type>(j);
    checkGetPtr<json::string_type>(j);
    checkGetPtr<json::integer_number_type>(j);
    checkGetPtr<json::unsigned_number_type>(j);
    checkGetPtr<json::float_number_type>(j);
    checkGetPtr<json::boolean_type>(j);
  }

private:
  using ArgType = typename T::ArgType;

  static constexpr json_type expectedType() { return T::expected_type_; }

  template <typename U, bool SameType = std::is_same<ArgType, U>::value>
  static void checkGetPtr(const json &j) {
    EXPECT_EQ(SameType, (j.get_ptr<U>() != nullptr));
  }
};

using GetPtrTypeList = testing::Types<
    GetPtrParam<json::object_type, json_type::object>,
    GetPtrParam<json::array_type, json_type::array>,
    GetPtrParam<json::string_type, json_type::string>,
    GetPtrParam<json::integer_number_type, json_type::integer_number>,
    GetPtrParam<json::unsigned_number_type, json_type::unsigned_number>,
    GetPtrParam<json::float_number_type, json_type::float_number>,
    GetPtrParam<json::boolean_type, json_type::boolean>>;

TYPED_TEST_SUITE(GetPtrTest, GetPtrTypeList);

TYPED_TEST(GetPtrTest, CheckNonConstGetter) { this->template doTest<false>(); }

TYPED_TEST(GetPtrTest, CheckConstGetter) { this->template doTest<true>(); }
