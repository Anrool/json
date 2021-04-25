#include <json.hpp>

#include <gtest/gtest.h>

#include <stdexcept>
#include <type_traits>

using namespace anrool;

template <typename Type, json_type ExpType> struct GetRefParam {
  using ArgType = Type;
  static constexpr json_type expected_type_ = ExpType;
};

template <typename T> class GetRefTest : public testing::Test {
protected:
  template <bool ConstTest> void doTest() {
    using JSONType =
        typename std::conditional<ConstTest, const json, json>::type;
    using argType =
        typename std::conditional<ConstTest, const ArgType, ArgType>::type;

    JSONType j(argType{});
    ASSERT_EQ(j.type(), expectedType());

    argType &ref = j.template get_ref<ArgType>();
    ASSERT_EQ(JSONType(ref), j);

    checkGetRef<json::object_type>(j);
    checkGetRef<json::array_type>(j);
    checkGetRef<json::string_type>(j);
    checkGetRef<json::integer_number_type>(j);
    checkGetRef<json::unsigned_number_type>(j);
    checkGetRef<json::float_number_type>(j);
    checkGetRef<json::boolean_type>(j);
  }

private:
  using ArgType = typename T::ArgType;

  static constexpr json_type expectedType() { return T::expected_type_; }

  template <typename U, bool SameType = std::is_same<ArgType, U>::value>
  static void checkGetRef(const json &j) {
    try {
      EXPECT_EQ(j.get_ref<U>(), *j.get_ptr<U>());
      EXPECT_TRUE(SameType);
    } catch (std::invalid_argument &e) {
      EXPECT_FALSE(SameType);
    } catch (...) {
      EXPECT_TRUE(false) << "Unexpected exception";
    }
  }
};

using GetRefTypeList = testing::Types<
    GetRefParam<json::object_type, json_type::object>,
    GetRefParam<json::array_type, json_type::array>,
    GetRefParam<json::string_type, json_type::string>,
    GetRefParam<json::integer_number_type, json_type::integer_number>,
    GetRefParam<json::unsigned_number_type, json_type::unsigned_number>,
    GetRefParam<json::float_number_type, json_type::float_number>,
    GetRefParam<json::boolean_type, json_type::boolean>>;

TYPED_TEST_SUITE(GetRefTest, GetRefTypeList);

TYPED_TEST(GetRefTest, CheckNonConstGetter) { this->template doTest<false>(); }

TYPED_TEST(GetRefTest, CheckConstGetter) { this->template doTest<true>(); }
