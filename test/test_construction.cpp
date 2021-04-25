#include <json.hpp>

#include <gtest/gtest.h>

#include <cassert>
#include <cstdio>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <new>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <vector>

using namespace anrool;

namespace {

bool TrackAllocations;
std::size_t TotalAllocations_;
std::size_t ActiveAllocations_;
std::size_t TotalAllocatedMemory_;
std::size_t ActiveUsedMemory;
std::unordered_map<void *, std::size_t> AllocationMap_;

void track() { TrackAllocations = true; }

void untrack() { TrackAllocations = false; }

void clearTrackingInfo() {
  TotalAllocations_ = 0;
  ActiveAllocations_ = 0;
  TotalAllocatedMemory_ = 0;
  ActiveUsedMemory = 0;
  AllocationMap_.clear();
}

} // unnamed namespace

void *operator new(std::size_t count) {
  void *ptr = std::malloc(count);

  if (TrackAllocations) {
    assert(ptr != nullptr);
    assert(count > 0);

    ++TotalAllocations_;
    ++ActiveAllocations_;
    TotalAllocatedMemory_ += count;
    ActiveUsedMemory += count;

    untrack();
    AllocationMap_[ptr] = count;
    track();
  }

  return ptr;
}

void operator delete(void *ptr) noexcept {
  if (TrackAllocations) {
    assert(ptr != nullptr);
    const auto allocationIt = AllocationMap_.find(ptr);

    assert(allocationIt != std::end(AllocationMap_));

    --ActiveAllocations_;
    ActiveUsedMemory -= allocationIt->second;

    untrack();
    AllocationMap_.erase(allocationIt);
    track();
  }

  std::free(ptr);
}

class AllocationTest
    : public testing::TestWithParam<std::tuple<json_type, bool>> {
public:
  AllocationTest()
      : allocationCount_{0}, capacity_{0}, type_{std::get<0>(GetParam())},
        primitive_{std::get<1>(GetParam())} {}

  void SetUp() final {
    clearTrackingInfo();
    checkAllocations();
  }

  void TearDown() final {
    ASSERT_EQ(TotalAllocations_, allocationCount_);
    ASSERT_EQ(ActiveAllocations_, 0);
    ASSERT_EQ(TotalAllocatedMemory_, capacity_ * allocationCount_);
    ASSERT_EQ(ActiveUsedMemory, 0);

    clearTrackingInfo();
  }

  void incrementAllocationCountIfNeeded() {
    if (!primitive_) {
      ++allocationCount_;
    }
  }

  void checkAllocations() const {
    ASSERT_EQ(TotalAllocations_, allocationCount_);
    ASSERT_EQ(ActiveAllocations_, allocationCount_);
    ASSERT_EQ(TotalAllocatedMemory_, capacity_ * allocationCount_);
    ASSERT_EQ(ActiveUsedMemory, capacity_ * allocationCount_);
  }

protected:
  std::size_t allocationCount_;
  std::size_t capacity_;
  const json_type type_;
  const bool primitive_;
};

TEST_P(AllocationTest, CheckAllocations) {
  {
    track();
    json j(type_);
    untrack();
    capacity_ = TotalAllocatedMemory_;
    ASSERT_TRUE(primitive_ ? capacity_ == 0 : capacity_ != 0);
    incrementAllocationCountIfNeeded();
    checkAllocations();

    track();
    json constructedCopy = j;
    untrack();
    incrementAllocationCountIfNeeded();
    checkAllocations();

    track();
    json assignedCopy;
    assignedCopy = j;
    untrack();
    incrementAllocationCountIfNeeded();
    checkAllocations();

    track();
    json moveConstructedCopy = std::move(constructedCopy);
    untrack();
    checkAllocations();

    track();
    json moveAssignedCopy;
    moveAssignedCopy = std::move(assignedCopy);
    untrack();
    checkAllocations();

    track();
  }

  untrack();
}

INSTANTIATE_TEST_SUITE_P(
    AllocationInstantiation, AllocationTest,
    testing::Values(std::make_tuple(json_type::object, false),
                    std::make_tuple(json_type::array, false),
                    std::make_tuple(json_type::string, false),
                    std::make_tuple(json_type::integer_number, true),
                    std::make_tuple(json_type::unsigned_number, true),
                    std::make_tuple(json_type::float_number, true),
                    std::make_tuple(json_type::boolean, true),
                    std::make_tuple(json_type::null, true)));

template <typename Type, json_type ExpectedType>
struct ParametrizedConstructionParam {
  using ArgType = Type;
  static constexpr json_type expected_type_ = ExpectedType;
};

template <typename T>
class ParametrizedConstructionTest : public testing::Test {
public:
  void SetUp() final {
    J_ = json(typename T::ArgType());
    ASSERT_EQ(J_.type(), expectedType());
  }

  static constexpr json_type expectedType() { return T::expected_type_; }

private:
  json J_;
};

using ParametrizationTypeList = testing::Types<
    ParametrizedConstructionParam<json::object_type, json_type::object>,
    ParametrizedConstructionParam<std::map<std::string, json>,
                                  json_type::object>,
    ParametrizedConstructionParam<const std::map<std::string, json>,
                                  json_type::object>,
    ParametrizedConstructionParam<std::map<std::string, int>,
                                  json_type::object>,
    ParametrizedConstructionParam<std::map<const char *, int>,
                                  json_type::object>,
    ParametrizedConstructionParam<std::map<const char *, const char *>,
                                  json_type::object>,

    ParametrizedConstructionParam<json::array_type, json_type::array>,
    ParametrizedConstructionParam<std::vector<bool>, json_type::array>,
    ParametrizedConstructionParam<std::list<json>, json_type::array>,
    ParametrizedConstructionParam<std::forward_list<json>, json_type::array>,
    ParametrizedConstructionParam<std::set<json>, json_type::array>,
    ParametrizedConstructionParam<std::unordered_set<json>, json_type::array>,
    ParametrizedConstructionParam<std::array<int, 7>, json_type::array>,
    ParametrizedConstructionParam<std::valarray<int>, json_type::array>,
    ParametrizedConstructionParam<std::deque<json>, json_type::array>,
    ParametrizedConstructionParam<std::pair<float, std::string>,
                                  json_type::array>,
    ParametrizedConstructionParam<std::tuple<json, json, json>,
                                  json_type::array>,
    ParametrizedConstructionParam<std::map<int, double>, json_type::array>,
    ParametrizedConstructionParam<std::multimap<int, double>, json_type::array>,

    ParametrizedConstructionParam<json::string_type, json_type::string>,
    ParametrizedConstructionParam<std::string, json_type::string>,

    ParametrizedConstructionParam<signed char, json_type::integer_number>,
    ParametrizedConstructionParam<short, json_type::integer_number>,
    ParametrizedConstructionParam<int, json_type::integer_number>,
    ParametrizedConstructionParam<long int, json_type::integer_number>,
    ParametrizedConstructionParam<long long int, json_type::integer_number>,

    ParametrizedConstructionParam<unsigned char, json_type::unsigned_number>,
    ParametrizedConstructionParam<unsigned short, json_type::unsigned_number>,
    ParametrizedConstructionParam<unsigned, json_type::unsigned_number>,
    ParametrizedConstructionParam<unsigned long int,
                                  json_type::unsigned_number>,
    ParametrizedConstructionParam<unsigned long long int,
                                  json_type::unsigned_number>,

    ParametrizedConstructionParam<float, json_type::float_number>,
    ParametrizedConstructionParam<double, json_type::float_number>,
    ParametrizedConstructionParam<long double, json_type::float_number>,

    ParametrizedConstructionParam<bool, json_type::boolean>,

    ParametrizedConstructionParam<std::nullptr_t, json_type::null>>;

TYPED_TEST_SUITE(ParametrizedConstructionTest, ParametrizationTypeList);

TYPED_TEST(ParametrizedConstructionTest, CheckJSONType) {}

TEST(RawArrayConstuctionTest, CheckJSONType) {
  {
    const int intArray[] = {3, 8, 1, 9};
    json j(intArray);
    EXPECT_EQ(j.type(), json_type::array);
  }

  {
    const char charArray[] = "this is a char array";
    json j(charArray);
    EXPECT_EQ(j.type(), json_type::string);
  }

  {
    const char *CString = "this is a Cstring";
    json j(CString);
    EXPECT_EQ(j.type(), json_type::string);
  }
}
