#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include <json_type.hpp>
#include <macro.hpp>

#include <iterator>

namespace anrool {
namespace detail {

class value_iterator {
private:
  using difference_type = std::ptrdiff_t;

  explicit value_iterator(difference_type value) : value_{value} {}

  difference_type value_;

public:
  static value_iterator begin_iterator() { return value_iterator{0}; }

  static value_iterator end_iterator() { return value_iterator{1}; }

  bool is_begin() const { return *this == begin_iterator(); }

  bool is_end() const { return *this == end_iterator(); }

  value_iterator &operator++() {
    ++value_;
    return *this;
  }

  value_iterator &operator--() {
    --value_;
    return *this;
  }

  value_iterator &operator+=(difference_type n) {
    value_ += n;
    return *this;
  }

  difference_type operator-(const value_iterator &other) const {
    return value_ - other.value_;
  }

  bool operator==(const value_iterator &other) const {
    return value_ == other.value_;
  }

  bool operator<(const value_iterator &other) const {
    return value_ < other.value_;
  }

  difference_type get_value() const { return value_; }
};

template <typename BasicJsonType> class iterator {
public:
  using value_type = typename BasicJsonType::value_type;
  using difference_type = typename BasicJsonType::difference_type;
  using reference =
      typename std::conditional<std::is_const<BasicJsonType>::value,
                                typename BasicJsonType::const_reference,
                                typename BasicJsonType::reference>::type;
  using pointer =
      typename std::conditional<std::is_const<BasicJsonType>::value,
                                typename BasicJsonType::const_pointer,
                                typename BasicJsonType::pointer>::type;
  using iterator_category = std::bidirectional_iterator_tag;

  iterator() : ptr_{nullptr} {}

  static iterator begin_iterator(pointer ptr) {
    ANROOL_JSON_ASSERT(ptr != nullptr);

    iterator it;
    it.ptr_ = ptr;

    switch (ptr->type()) {
    case json_type::object:
      it.value_.object_iterator_ = std::begin(*ptr->value_.object_);
      break;
    case json_type::array:
      it.value_.array_iterator_ = std::begin(*ptr->value_.array_);
      break;
    case json_type::null:
      it.value_.value_iterator_ = value_iterator::end_iterator();
      break;
    default:
      it.value_.value_iterator_ = value_iterator::begin_iterator();
      break;
    }

    return it;
  }

  static iterator end_iterator(pointer ptr) {
    ANROOL_JSON_ASSERT(ptr != nullptr);

    iterator it;
    it.ptr_ = ptr;

    switch (ptr->type()) {
    case json_type::object:
      it.value_.object_iterator_ = std::end(*ptr->value_.object_);
      break;
    case json_type::array:
      it.value_.array_iterator_ = std::end(*ptr->value_.array_);
      break;
    default:
      it.value_.value_iterator_ = value_iterator::end_iterator();
      break;
    }

    return it;
  }

  reference operator*() const { return *operator->(); }

  pointer operator->() const {
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      ANROOL_JSON_ASSERT(value_.object_iterator_ !=
                         std::end(*ptr_->value_.object_));
      return &value_.object_iterator_->second;
    case json_type::array:
      ANROOL_JSON_ASSERT(value_.array_iterator_ !=
                         std::end(*ptr_->value_.array_));
      return value_.array_iterator_.operator->();
    default:
      ANROOL_JSON_ASSERT(value_.value_iterator_.is_begin());
      return ptr_;
    }
  }

  iterator &operator++() {
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      std::advance(value_.object_iterator_, 1);
      break;
    case json_type::array:
      std::advance(value_.array_iterator_, 1);
      break;
    default:
      ++value_.value_iterator_;
      break;
    }

    return *this;
  }

  iterator operator++(int) {
    iterator it = *this;
    ++*this;
    return it;
  }

  iterator &operator--() {
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      std::advance(value_.object_iterator_, -1);
      break;
    case json_type::array:
      std::advance(value_.array_iterator_, -1);
      break;
    default:
      --value_.value_iterator_;
      break;
    }

    return *this;
  }

  iterator operator--(int) {
    iterator it = *this;
    --*this;
    return it;
  }

  bool operator==(const iterator &other) const {
    ANROOL_JSON_ASSERT(ptr_ == other.ptr_);
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      return value_.object_iterator_ == other.value_.object_iterator_;
    case json_type::array:
      return value_.array_iterator_ == other.value_.array_iterator_;
    default:
      return value_.value_iterator_ == other.value_.value_iterator_;
    }
  }

  bool operator!=(const iterator &other) const { return !(*this == other); }

  // RAI
  iterator operator+(difference_type n) const {
    iterator it = *this;
    it += n;
    return it;
  }

  iterator operator-(difference_type n) const {
    iterator it = *this;
    it -= n;
    return it;
  }

  friend iterator operator+(difference_type n, const iterator &it) {
    return it + n;
  }

  difference_type operator-(const iterator &it) const {
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      ANROOL_JSON_ABORT;
    case json_type::array:
      return value_.array_iterator_ - it.value_.array_iterator_;
    default:
      return value_.value_iterator_ - it.value_.value_iterator_;
    }
  }

  iterator &operator+=(difference_type n) {
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      ANROOL_JSON_ABORT;
    case json_type::array:
      std::advance(value_.array_iterator_, n);
      break;
    default:
      value_.value_iterator_ += n;
      break;
    }

    return *this;
  }

  iterator &operator-=(difference_type n) { return *this += -n; }

  reference operator[](difference_type n) const {
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      ANROOL_JSON_ABORT;
    case json_type::array:
      return *std::next(value_.array_iterator_, n);
    case json_type::null:
      ANROOL_JSON_ABORT;
    default:
      ANROOL_JSON_ASSERT(value_.value_iterator_.get_value() == -n);
      return *ptr_;
    }
  }

  bool operator<(const iterator &other) const {
    ANROOL_JSON_ASSERT(ptr_ == other.ptr_);
    ANROOL_JSON_ASSERT(ptr_ != nullptr);

    switch (ptr_->type()) {
    case json_type::object:
      ANROOL_JSON_ABORT;
    case json_type::array:
      return value_.array_iterator_ < other.value_.array_iterator_;
    default:
      return value_.value_iterator_ < other.value_.value_iterator_;
    }
  }

  DECLARE_COMPARATORS(iterator);

  const typename BasicJsonType::object_type::key_type &key() const {
    ANROOL_JSON_ASSERT(ptr_ != nullptr);
    ANROOL_JSON_ASSERT(ptr_->type() == json_type::object);

    return value_.object_iterator_.first;
  }

private:
  using object_type = typename BasicJsonType::object_type;
  using array_type = typename BasicJsonType::array_type;

  union iterator_value {
    typename object_type::iterator object_iterator_;
    typename array_type::iterator array_iterator_;
    value_iterator value_iterator_;

    iterator_value() {}
  };

  pointer ptr_;
  iterator_value value_;
};

} // namespace detail
} // namespace anrool

#endif // ITERATOR_HPP
