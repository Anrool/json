#ifndef JSON_HPP
#define JSON_HPP

#include <adl_serializer.hpp>
#include <deserialize_functions.hpp>
#include <input_adapter.hpp>
#include <iterator.hpp>
#include <json_type.hpp>
#include <macro.hpp>
#include <output_adapter.hpp>
#include <parser.hpp>
#include <serialize_functions.hpp>
#include <serializer.hpp>

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <cstdint>
#include <cstring>

namespace anrool {

ANROOL_JSON_TEMPLATE_DECLARATION
class basic_json {
public:
  template <json_type> friend struct detail::constructor;

  template <typename> friend class detail::iterator;

  template <typename, typename> friend class detail::serializer;

  template <typename, typename> friend class detail::parser;

  using value_type = basic_json;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = detail::iterator<value_type>;
  using const_iterator = detail::iterator<const value_type>;
  using pointer = value_type *;
  using const_pointer = const value_type *;

  using object_type = ObjectType<StringType, value_type>;
  using array_type = ArrayType<value_type>;
  using string_type = StringType;
  using integer_number_type = IntegerNumberType;
  using unsigned_number_type = UnsignedNumberType;
  using float_number_type = FloatNumberType;
  using boolean_type = BooleanType;

  using json_serializer = adl_serializer;

  template <typename T,
            detail::enable_if_t<detail::is_deserializable<basic_json, T>::value,
                                int> = 0>
  T get() const {
    static_assert(!std::is_reference<T>::value, "");
    static_assert(std::is_default_constructible<T>::value, "");

    T val;
    deserialize(*this, val);
    return val;
  }

  template <typename T,
            detail::enable_if_t<detail::is_deserializable<basic_json, T>::value,
                                int> = 0>
  void get_to(T &val) const {
    deserialize(*this, val);
  }

  template <typename T> operator T() const { return get<T>(); }

  basic_json(std::nullptr_t = nullptr) : type_{json_type::null} {
    value_.object_ = nullptr;

    assert_invariant();
  }

  basic_json(const basic_json &other) : type_{other.type_} {
    switch (type_) {
    case json_type::object:
      value_.object_ = create<object_type>(*other.value_.object_);
      break;
    case json_type::array:
      value_.array_ = create<array_type>(*other.value_.array_);
      break;
    case json_type::string:
      value_.string_ = create<string_type>(*other.value_.string_);
      break;
    case json_type::integer_number:
      value_.integer_number_ = other.value_.integer_number_;
      break;
    case json_type::unsigned_number:
      value_.unsigned_number_ = other.value_.unsigned_number_;
      break;
    case json_type::float_number:
      value_.float_number_ = other.value_.float_number_;
      break;
    case json_type::boolean:
      value_.boolean_ = other.value_.boolean_;
      break;
    default:
      break;
    }

    assert_invariant();
  }

  basic_json(basic_json &&other)
      : type_{std::move(other.type_)}, value_{std::move(other.value_)} {
    other.type_ = json_type::null;
    other.value_ = json_value{};

    assert_invariant();
    other.assert_invariant();
  }

  template <typename T> T *get_ptr() {
    static_assert(!std::is_pointer<T>::value, "");

    return get_ptr_impl(static_cast<T *>(nullptr));
  }

  template <typename T> const T *get_ptr() const {
    return const_cast<basic_json *>(this)->get_ptr<T>();
  }

  template <typename T> T &get_ref() {
    static_assert(!std::is_reference<T>::value, "");

    T *ptr = get_ptr<T>();

    if (ptr != nullptr) {
      return *ptr;
    }

    throw std::invalid_argument{std::string{"actual type: "} +
                                to_string(type())};
  }

  template <typename T> const T &get_ref() const {
    static_assert(!std::is_reference<T>::value, "");

    return const_cast<basic_json *>(this)->get_ref<T>();
  }

  template <typename T, typename U = detail::remove_cvref_t<T>,
            detail::enable_if_t<detail::is_serializable<basic_json, U>::value,
                                int> = 0>
  basic_json(T &&val) {
    serialize(*this, std::forward<T>(val));

    assert_invariant();
  }

  basic_json(size_type count, const value_type &value)
      : type_{json_type::array} {
    value_.array_ = create<array_type>(count, value);

    assert_invariant();
  }

  basic_json(json_type type) : type_{type} {
    switch (type) {
    case json_type::object:
      value_.object_ = create<object_type>();
      break;
    case json_type::array:
      value_.array_ = create<array_type>();
      break;
    case json_type::string:
      value_.string_ = create<string_type>();
      break;
    case json_type::integer_number:
      value_.integer_number_ = 0;
      break;
    case json_type::unsigned_number:
      value_.unsigned_number_ = 0;
      break;
    case json_type::float_number:
      value_.float_number_ = 0.f;
      break;
    case json_type::boolean:
      value_.boolean_ = false;
      break;
    case json_type::null:
      value_.object_ = nullptr;
      break;
    default:
      ANROOL_JSON_ABORT;
    }

    assert_invariant();
  }

  basic_json &operator=(const basic_json &other) {
    if (this != &other) {
      other.assert_invariant();

      basic_json copy = other;

      std::swap(type_, copy.type_);
      std::swap(value_, copy.value_);

      assert_invariant();
      other.assert_invariant();
    }

    return *this;
  }

  basic_json &operator=(basic_json &&other) {
    if (this != &other) {
      type_ = std::move(other.type_);
      value_ = std::move(other.value_);

      other.type_ = json_type::null;
      other.value_ = json_value{};

      assert_invariant();
      other.assert_invariant();
    }

    return *this;
  }

  ~basic_json() {
    assert_invariant();

    switch (type_) {
    case json_type::object:
      delete value_.object_;
      break;
    case json_type::array:
      delete value_.array_;
      break;
    case json_type::string:
      delete value_.string_;
      break;
    default:
      break;
    }
  }

  json_type type() const { return type_; }

  reference at(size_type pos) {
    ANROOL_JSON_ASSERT(type() == json_type::array);

    return value_.array_->at(pos);
  }

  const_reference at(size_type pos) const {
    return const_cast<basic_json *>(this)->at(pos);
  }

  reference at(const typename object_type::key_type &key) {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    return value_.object_->at(key);
  }

  const_reference at(const typename object_type::key_type &key) const {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    return value_.object_->at(key);
  }

  reference operator[](size_type pos) {
    // ANROOL_JSON_ASSERT(type() != json_type::array);
    ANROOL_JSON_ASSERT(pos < value_.array_->size());

    return value_.array_->operator[](pos);
  }

  const_reference operator[](size_type pos) const {
    return const_cast<basic_json *>(this)->operator[](pos);
  }

  reference operator[](const typename object_type::key_type &key) {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    return value_.object_->operator[](key);
  }

  template <typename T> reference operator[](T *key) {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    return value_.object_->operator[](key);
  }

  reference front() {
    ANROOL_JSON_ASSERT(type() == json_type::array);

    return value_.array_->front();
  }

  const_reference front() const {
    return const_cast<basic_json *>(this)->front();
  }

  reference back() {
    ANROOL_JSON_ASSERT(type() == json_type::array);

    return value_.array_->back();
  }

  const_reference back() const {
    return const_cast<basic_json *>(this)->back();
  }

  size_type count(const typename object_type::key_type &key) const {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    return value_.object_->count(key);
  }

  iterator find(const typename object_type::key_type &key) {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    iterator it{this};
    it.value_.object_it_ = value_.object_->find(key);
    return it;
  }

  const_iterator find(const typename object_type::key_type &key) const {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    const_iterator it{this};
    it.value_.object_it_ = value_.object_->find(key);
    return it;
  }

  iterator begin() { return iterator::begin_iterator(this); }

  iterator end() { return iterator::end_iterator(this); }

  const_iterator begin() const { return const_iterator::begin_iterator(this); }

  const_iterator end() const { return const_iterator::end_iterator(this); }

  const_iterator cbegin() const { return begin(); }

  const_iterator cend() const { return end(); }

  // TODO: implement reverse iterator

  bool operator==(const_reference other) const {
    const json_type type = type_;
    const json_type other_type = other.type();

    if (type == other_type) {
      switch (type) {
      case json_type::object:
        return *value_.object_ == *other.value_.object_;
      case json_type::array:
        return *value_.array_ == *other.value_.array_;
      case json_type::string:
        return *value_.string_ == *other.value_.string_;
      case json_type::integer_number:
        return value_.integer_number_ == other.value_.integer_number_;
      case json_type::unsigned_number:
        return value_.unsigned_number_ == other.value_.unsigned_number_;
      case json_type::float_number:
        return value_.float_number_ == other.value_.float_number_;
      case json_type::boolean:
        return value_.boolean_ == other.value_.boolean_;
      case json_type::null:
        return true;
      default:
        return false;
      }
    } else if (type == json_type::integer_number &&
               other_type == json_type::float_number) {
      return static_cast<float_number_type>(value_.integer_number_) ==
             other.value_.float_number_;
    } else if (type == json_type::float_number &&
               other_type == json_type::integer_number) {
      return value_.float_number_ ==
             static_cast<float_number_type>(other.value_.integer_number_);
    } else if (type == json_type::unsigned_number &&
               other_type == json_type::float_number) {
      return static_cast<float_number_type>(value_.unsigned_number_) ==
             other.value_.float_number_;
    } else if (type == json_type::float_number &&
               other_type == json_type::unsigned_number) {
      return value_.float_number_ ==
             static_cast<float_number_type>(other.value_.unsigned_number_);
    } else if (type == json_type::unsigned_number &&
               other_type == json_type::integer_number) {
      return static_cast<integer_number_type>(value_.unsigned_number_) ==
             other.value_.integer_number_;
    } else if (type == json_type::integer_number &&
               other_type == json_type::unsigned_number) {
      return value_.integer_number_ ==
             static_cast<integer_number_type>(other.value_.unsigned_number_);
    }

    return false;
  }

  bool operator!=(const_reference other) const { return !operator==(other); }

  bool operator<(const basic_json &other) const {
    const json_type type = type_;
    const json_type other_type = other.type();

    if (type == other_type) {
      switch (type) {
      case json_type::object:
        return *value_.object_ < *other.value_.object_;
      case json_type::array:
        return *value_.array_ < *other.value_.array_;
      case json_type::string:
        return *value_.string_ < *other.value_.string_;
      case json_type::integer_number:
        return value_.integer_number_ < other.value_.integer_number_;
      case json_type::unsigned_number:
        return value_.unsigned_number_ < other.value_.unsigned_number_;
      case json_type::float_number:
        return value_.float_number_ < other.value_.float_number_;
      case json_type::boolean:
        return value_.boolean_ < other.value_.boolean_;
      default:
        return false;
      }
    } else if (type == json_type::integer_number &&
               other_type == json_type::float_number) {
      return static_cast<float_number_type>(value_.integer_number_) <
             other.value_.float_number_;
    } else if (type == json_type::float_number &&
               other_type == json_type::integer_number) {
      return value_.float_number_ <
             static_cast<float_number_type>(other.value_.integer_number_);
    } else if (type == json_type::unsigned_number &&
               other_type == json_type::float_number) {
      return static_cast<float_number_type>(value_.unsigned_number_) <
             other.value_.float_number_;
    } else if (type == json_type::float_number &&
               other_type == json_type::unsigned_number) {
      return value_.float_number_ <
             static_cast<float_number_type>(other.value_.unsigned_number_);
    } else if (type == json_type::integer_number &&
               other_type == json_type::unsigned_number) {
      return value_.integer_number_ <
             static_cast<integer_number_type>(other.value_.unsigned_number_);
    } else if (type == json_type::integer_number &&
               other_type == json_type::unsigned_number) {
      return static_cast<integer_number_type>(value_.unsigned_number_) <
             other.value_.integer_number_;
    }

    return type < other_type;
  }

  DECLARE_COMPARATORS(basic_json);

  void swap(reference other) {
    std::swap(type_, other.type_);
    std::swap(value_, other.value_);

    assert_invariant();
    other.assert_invariant();
  }

  size_type size() const {
    switch (type_) {
    case json_type::object:
      return value_.object_->size();
    case json_type::array:
      return value_.array_->size();
    case json_type::null:
      return 0;
    default:
      return 1;
    }
  }

  size_type max_size() const {
    switch (type_) {
    case json_type::object:
      return value_.object_->max_size();
    case json_type::array:
      return value_.array_->max_size();
    case json_type::null:
      return 0;
    default:
      return 1;
    }
  }

  bool empty() const {
    switch (type_) {
    case json_type::object:
      return value_.object_->empty();
    case json_type::array:
      return value_.array_->empty();
    case json_type::null:
      return true;
    default:
      return false;
    }
  }

  void clear() {
    switch (type_) {
    case json_type::object:
      value_.object_->clear();
      break;
    case json_type::array:
      value_.array_->clear();
      break;
    case json_type::string:
      value_.string_->clear();
      break;
    case json_type::integer_number:
      value_.integer_number_ = 0;
      break;
    case json_type::unsigned_number:
      value_.unsigned_number_ = 0;
      break;
    case json_type::float_number:
      value_.float_number_ = 0.f;
      break;
    case json_type::boolean:
      value_.boolean_ = false;
      break;
    default:
      break;
    }
  }

  iterator insert(const_iterator pos, const value_type &value) {
    ANROOL_JSON_ASSERT(type() == json_type::array);
    ANROOL_JSON_ASSERT(pos.ptr_ == this);

    return insert_into_array(pos, value);
  }

  iterator insert(const_iterator pos, value_type &&value) {
    ANROOL_JSON_ASSERT(type() == json_type::array);
    ANROOL_JSON_ASSERT(pos.ptr_ == this);

    return insert_into_array(pos, std::move(value));
  }

  iterator insert(const_iterator pos, size_type count,
                  const value_type &value) {
    ANROOL_JSON_ASSERT(type() == json_type::array);
    ANROOL_JSON_ASSERT(pos.ptr_ == this);

    return insert_into_array(pos, count, value);
  }

  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    ANROOL_JSON_ASSERT(type() == json_type::array);
    ANROOL_JSON_ASSERT(pos.ptr_ == this);
    ANROOL_JSON_ASSERT(first.ptr_ == last.ptr_);
    ANROOL_JSON_ASSERT(first.ptr_ != this);

    return insert_into_array(pos, first, last);
  }

  iterator insert(const_iterator pos, std::initializer_list<value_type> ilist) {
    assert(type() == json_type::array);
    assert(pos.ptr_ == this);

    return insert_into_array(pos, std::begin(ilist), std::end(ilist));
  }

  template <typename InputIt> void insert(InputIt first, InputIt last) {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    value_.object_->insert(first, last);
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args &&...args) {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    iterator it{this};
    const auto object_it = value_.object_->emplace(std::forward<Args>(args)...);
    it.value_.object_it_ = object_it;

    return {it, object_it.second};
  }

  iterator erase(const_iterator pos) {
    ANROOL_JSON_ASSERT(pos.ptr_ == this);

    iterator it{this};

    switch (type_) {
    case json_type::object:
      ANROOL_JSON_ASSERT(pos.it_.object_it_ != std::end(*value_.object_));
      it.value_.object_it_ = value_.object_->erase(pos.it_.object_it_);
      break;
    case json_type::array:
      ANROOL_JSON_ASSERT(pos.it_.array_it_ != std::end(*value_.array_));
      it.value_.array_it_ = value_.array_->erase(pos.it_.array_it_);
      break;
    case json_type::string:
      delete value_.string_;
    case json_type::integer_number:
    case json_type::unsigned_number:
    case json_type::float_number:
    case json_type::boolean:
      ANROOL_JSON_ASSERT(pos.it_.value_it_.is_begin());

      type_ = json_type::null;
      it.set_end();

      assert_invariant();

      break;
    default:
      ANROOL_JSON_ABORT;
    }

    return it;
  }

  iterator erase(const_iterator first, const_iterator last) {
    ANROOL_JSON_ASSERT(first.ptr_ == last.ptr_);
    ANROOL_JSON_ASSERT(first.ptr_ == this);

    iterator it{this};

    switch (type_) {
    case json_type::object:
      it.value_.object_it_ =
          value_.object_->erase(first.it_.object_it_, last.it_.object_it_);
      break;
    case json_type::array:
      it.value_.array_it_ =
          value_.array_->erase(first.it_.array_it_, last.it_.array_it_);
      break;
    case json_type::string:
      delete value_.string_;
    case json_type::integer_number:
    case json_type::unsigned_number:
    case json_type::float_number:
    case json_type::boolean:
      ANROOL_JSON_ASSERT(first.it_.value_it_.is_begin());
      ANROOL_JSON_ASSERT(!last.it_.value_it_.is_begin());

      type_ = json_type::null;
      it.set_end();

      assert_invariant();

      break;
    default:
      ANROOL_JSON_ABORT;
    }

    return it;
  }

  size_type erase(const typename object_type::key_type &key) {
    ANROOL_JSON_ASSERT(type() == json_type::object);

    return value_.object_->erase(key);
  }

  void push_back(const value_type &value) {
    ANROOL_JSON_ASSERT(type() == json_type::array);

    value_.array_->push_back(value);
  }

  void push_back(value_type &&value) {
    ANROOL_JSON_ASSERT(type() == json_type::array);

    value_.array_->push_back(std::move(value));
  }

  template <typename... Args> void emplace_back(Args &&...args) {
    ANROOL_JSON_ASSERT(type() == json_type::array);

    value_.array_->emplace_back(std::forward<Args>(args)...);
  }

  void pop_back() {
    ANROOL_JSON_ASSERT(type() == json_type::array);

    value_.array_->pop_back();
  }

  std::string dump() const {
    std::string str;
    auto adapter = detail::output_adapter(str);
    detail::serializer<basic_json, decltype(adapter)> s{std::move(adapter)};
    s.dump(*this);

    return str;
  }

  std::string dump(std::size_t indent) const {
    std::string str;
    auto adapter = detail::output_adapter(str);
    detail::serializer<basic_json, decltype(adapter)> s{std::move(adapter)};
    s.dump(*this, indent);

    return str;
  }

  template <typename InputType> static basic_json parse(InputType &&i) {
    basic_json j;
    auto adapter = detail::input_adapter(i);
    detail::parser<basic_json, decltype(adapter)> parser{std::move(adapter)};

    parser.parse(j);

    return j;
  }

  friend std::ostream &operator<<(std::ostream &os, const basic_json &j) {
    auto adapter = detail::output_adapter(os);
    detail::serializer<basic_json, decltype(adapter)> s{std::move(adapter)};
    const auto indent = os.width();

    if (indent > 0) {
      os.width(0);
      s.dump(j, indent);
    } else {
      s.dump(j);
    }

    return os;
  }

  friend std::istream &operator>>(std::istream &is, basic_json &j) {
    auto adapter = detail::input_adapter(is);
    detail::parser<basic_json, decltype(adapter)> parser{std::move(adapter)};
    parser.parse(j);

    return is;
  }

private:
  union json_value {
    object_type *object_;
    array_type *array_;
    string_type *string_;
    integer_number_type integer_number_;
    unsigned_number_type unsigned_number_;
    float_number_type float_number_;
    boolean_type boolean_;
  };

  void assert_invariant() const {
    ANROOL_JSON_ASSERT(type_ != json_type::object || value_.object_ != nullptr);
    ANROOL_JSON_ASSERT(type_ != json_type::array || value_.array_ != nullptr);
    ANROOL_JSON_ASSERT(type_ != json_type::string || value_.string_ != nullptr);
  }

  template <typename T, typename... Args,
            typename = typename std::enable_if<!std::is_array<T>::value>::type>
  static T *create(Args &&...args) {
    return new T(std::forward<Args>(args)...);
  }

  object_type *get_ptr_impl(object_type *) {
    return type() == json_type::object ? value_.object_ : nullptr;
  }

  array_type *get_ptr_impl(array_type *) {
    return type() == json_type::array ? value_.array_ : nullptr;
  }

  string_type *get_ptr_impl(string_type *) {
    return type() == json_type::string ? value_.string_ : nullptr;
  }

  integer_number_type *get_ptr_impl(integer_number_type *) {
    return type() == json_type::integer_number ? &value_.integer_number_
                                               : nullptr;
  }

  unsigned_number_type *get_ptr_impl(unsigned_number_type *) {
    return type() == json_type::unsigned_number ? &value_.unsigned_number_
                                                : nullptr;
  }

  float_number_type *get_ptr_impl(float_number_type *) {
    return type() == json_type::float_number ? &value_.float_number_ : nullptr;
  }

  boolean_type *get_ptr_impl(boolean_type *) {
    return type() == json_type::boolean ? &value_.boolean_ : nullptr;
  }

  template <typename... Args>
  iterator insert_into_array(const const_iterator &pos, Args &&...args) {
    iterator it{this};
    it.value_.array_it_ = value_.array_->insert(pos.value_.array_it_,
                                                std::forward<Args>(args)...);
    return it;
  }

  json_type type_;
  json_value value_;
};

using json = basic_json<>;

} // namespace anrool

namespace std {

template <> struct hash<anrool::json> {
  std::size_t operator()(const anrool::json &) const {
    // WARNING: no implementation yet
    return 0;
  }
};

} // namespace std

#endif // JSON_HPP
