// Copyright (c) 2013, Per Eckerdal. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <type_traits>
#include <unordered_set>

#include <gtest/gtest.h>

#include "handle.h"

namespace {

struct DummyStruct {
  explicit DummyStruct(int v_) : v(v_) {}
  int v;
};

static constexpr auto VAL = arw::HandleType::VALUE;
static constexpr auto REF = arw::HandleType::REFERENCE;
static constexpr auto WEAK = arw::HandleType::WEAK;

template<typename T, arw::HandleType Type>
class TestHandleHooks {
  typedef arw::Handle<T, Type, TestHandleHooks> H;

 public:
  TestHandleHooks() = delete;

  static void reset() {
    _created.clear();
    _destroyed.clear();
    _read.clear();
    _written.clear();
  }

  static bool hasBeenCreated(const H* h) {
    return 0 != _created.count(h);
  }

  static bool hasBeenDestroyed(const H* h) {
    return 0 != _destroyed.count(h);
  }

  static bool isAlive(const H* h) {
    return hasBeenCreated(h) && !hasBeenDestroyed(h);
  }

  static size_t timesRead(const H* h) {
    return _read.count(h);
  }

  static size_t timesWritten(const H* h) {
    return _written.count(h);
  }

  inline static void
  created(const H* handle) {
    EXPECT_EQ(0, _created.count(handle));
    _created.insert(handle);
  }

  inline static void
  destroyed(const H* handle) {
    EXPECT_EQ(0, _destroyed.count(handle));
    _destroyed.insert(handle);
  }

  inline static T* read(T** ptr) {
    _read.insert(reinterpret_cast<const H*>(ptr));
    return *ptr;
  }

  inline static void write(T** ptr, T* value) {
    _written.insert(reinterpret_cast<const H*>(ptr));
    *ptr = value;
  }

 private:
  static std::unordered_set<const H*> _created;
  static std::unordered_set<const H*> _destroyed;
  static std::unordered_multiset<const H*> _read;
  static std::unordered_multiset<const H*> _written;
};

template<typename T, arw::HandleType Type>
std::unordered_set<const arw::Handle<T, Type, TestHandleHooks<T, Type> >* >
TestHandleHooks<T, Type>::_created;

template<typename T, arw::HandleType Type>
std::unordered_set<const arw::Handle<T, Type, TestHandleHooks<T, Type> >* >
TestHandleHooks<T, Type>::_destroyed;

template<typename T, arw::HandleType Type>
std::unordered_multiset<const arw::Handle<T, Type, TestHandleHooks<T, Type> >* >
TestHandleHooks<T, Type>::_read;

template<typename T, arw::HandleType Type>
std::unordered_multiset<const arw::Handle<T, Type, TestHandleHooks<T, Type> >* >
TestHandleHooks<T, Type>::_written;

template<typename T>
using ValHooks = TestHandleHooks<T, VAL>;

template<typename T>
using RefHooks = TestHandleHooks<T, REF>;

template<typename T>
using WeakHooks = TestHandleHooks<T, WEAK>;

template<typename T, arw::HandleType Type>
using TestHandle = arw::Handle<T, Type, TestHandleHooks<T, Type> >;

template<typename T>
using Val = TestHandle<T, VAL>;

template<typename T>
using Ref = TestHandle<T, REF>;

template<typename T>
using Weak = TestHandle<T, WEAK>;

}  // namespace

TEST(Handle, Harness) {
  ValHooks<int>::reset();

  Val<int>* vPtr;
  {
    Val<int> v(1);
    vPtr = &v;
    EXPECT_TRUE(ValHooks<int>::hasBeenCreated(&v));
    EXPECT_TRUE(!ValHooks<int>::hasBeenDestroyed(&v));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v));
  }
  EXPECT_TRUE(ValHooks<int>::hasBeenCreated(vPtr));
  EXPECT_TRUE(ValHooks<int>::hasBeenDestroyed(vPtr));
  EXPECT_TRUE(!ValHooks<int>::isAlive(vPtr));

  ValHooks<int>::reset();
  EXPECT_TRUE(!ValHooks<int>::hasBeenCreated(vPtr));
  EXPECT_TRUE(!ValHooks<int>::hasBeenDestroyed(vPtr));
  EXPECT_TRUE(!ValHooks<int>::isAlive(vPtr));
}

TEST(Handle, HandleTypes) {
  typedef int T;
  typedef arw::internal::HandleTypes<T, VAL>   ValVal;
  typedef arw::internal::HandleTypes<T, REF>   ValRef;
  typedef arw::internal::HandleTypes<T, WEAK>  ValWeak;

  static_assert(std::is_same<T, ValVal::value_type>::value,
                "Invalid value_type of HandleType");
  static_assert(std::is_same<T, ValRef::value_type>::value,
                "Invalid value_type of HandleType");
  static_assert(std::is_same<T, ValWeak::value_type>::value,
                "Invalid value_type of HandleType");

  static_assert(std::is_same<T, ValVal::storage_type>::value,
                "Invalid storage_type of HandleType");
  static_assert(std::is_same<T*, ValRef::storage_type>::value,
                "Invalid storage_type of HandleType");
  static_assert(std::is_same<T*, ValWeak::storage_type>::value,
                "Invalid storage_type of HandleType");
}

TEST(Handle, Destructor) {
  // Default constructor
  ValHooks<int>::reset();
  Val<int>* vPtr;
  {
    Val<int> v;
    vPtr = &v;
  }
  EXPECT_TRUE(ValHooks<int>::hasBeenDestroyed(vPtr));
}

TEST(Handle, Constructor) {
  // Default constructor
  ValHooks<int>::reset();
  {
    Val<int> v;
    EXPECT_TRUE(ValHooks<int>::isAlive(&v));
  }

  // Value copy constructor
  ValHooks<int>::reset();
  {
    Val<int> v(1);
    EXPECT_TRUE(ValHooks<int>::isAlive(&v));
  }

  // Value move constructor
  ValHooks<int>::reset();
  {
    int num = 1;
    Val<int> v(std::move(num));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v));
  }

  // Copy constructor
  ValHooks<int>::reset();
  {
    Val<int> v(1);
    Val<int> v2(v);
    EXPECT_TRUE(ValHooks<int>::isAlive(&v));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v2));
  }

  // Move constructor
  ValHooks<int>::reset();
  {
    Val<int> v(1);
    Val<int> v2(std::move(v));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v2));
  }
}

TEST(Handle, Assignment) {
  // Handle assignment operator (value)
  ValHooks<int>::reset();
  {
    Val<int> v1(1);
    Val<int> v2(2);

    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v2));

    v1 = v2;

    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v2));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v1));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v2));
    EXPECT_EQ(*v1, *v2);
  }

  // Handle assignment operator (value)
  ValHooks<int>::reset();
  {
    Val<int> v1(1);
    Val<int> v2(2);

    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v2));

    v1 = std::move(v2);

    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, ValHooks<int>::timesWritten(&v2));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v1));
    EXPECT_TRUE(ValHooks<int>::isAlive(&v2));
    EXPECT_EQ(*v1, *v2);
  }

  int one = 1;
  int two = 2;

  // Handle assignment operator (reference)
  RefHooks<int>::reset();
  {
    Ref<int> v1(&one);
    Ref<int> v2(&two);

    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v2));

    v1 = v2;

    EXPECT_EQ(1, RefHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v2));
    EXPECT_TRUE(RefHooks<int>::isAlive(&v1));
    EXPECT_TRUE(RefHooks<int>::isAlive(&v2));
    EXPECT_EQ(v1.get(), v2.get());
  }

  // Handle move assignment operator (reference)
  RefHooks<int>::reset();
  {
    Ref<int> v1(&one);
    Ref<int> v2(&two);

    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v2));

    v1 = std::move(v2);

    EXPECT_EQ(1, RefHooks<int>::timesWritten(&v1));
    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v2));
    EXPECT_TRUE(RefHooks<int>::isAlive(&v1));
    EXPECT_TRUE(RefHooks<int>::isAlive(&v2));
    EXPECT_EQ(v1.get(), v2.get());
  }

  // Handle value assignment operator (reference)
  RefHooks<int>::reset();
  {
    Ref<int> v1(&one);

    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v1));

    v1 = &two;

    EXPECT_EQ(1, RefHooks<int>::timesWritten(&v1));
    EXPECT_TRUE(RefHooks<int>::hasBeenCreated(&v1));
    EXPECT_EQ(v1.get(), &two);
  }

  // Handle move assignment operator (reference)
  RefHooks<int>::reset();
  {
    Ref<int> v1(&one);

    EXPECT_EQ(0, RefHooks<int>::timesWritten(&v1));

    v1 = std::move(&two);

    EXPECT_EQ(1, RefHooks<int>::timesWritten(&v1));
    EXPECT_TRUE(RefHooks<int>::hasBeenCreated(&v1));
    EXPECT_EQ(v1.get(), &two);
  }
}

TEST(Handle, BoolConversion) {
  WeakHooks<bool>::reset();

  bool aBool = true;

  Weak<bool> nonNull(&aBool);
  Weak<bool> isNull(nullptr);

  EXPECT_EQ(0, WeakHooks<bool>::timesRead(&nonNull));
  EXPECT_EQ(0, WeakHooks<bool>::timesRead(&isNull));

  EXPECT_TRUE(static_cast<bool>(nonNull));
  EXPECT_TRUE(!static_cast<bool>(isNull));

  EXPECT_EQ(1, WeakHooks<bool>::timesRead(&nonNull));
  EXPECT_EQ(1, WeakHooks<bool>::timesRead(&isNull));
}

TEST(Handle, Get) {
  ValHooks<int>::reset();
  {
    Val<int> v1(1);
    EXPECT_EQ(0, ValHooks<int>::timesRead(&v1));
    EXPECT_EQ(1, *v1.get());
    EXPECT_EQ(0, ValHooks<int>::timesRead(&v1));
  }

  ValHooks<int>::reset();
  {
    const Val<int> v2(2);
    EXPECT_EQ(0, ValHooks<int>::timesRead(&v2));
    EXPECT_EQ(2, *v2.get());
    EXPECT_EQ(0, ValHooks<int>::timesRead(&v2));
  }

  RefHooks<int>::reset();
  {
    int refVal1 = 3;
    Ref<int> r1(&refVal1);
    EXPECT_EQ(0, RefHooks<int>::timesRead(&r1));
    EXPECT_EQ(&refVal1, r1.get());
    EXPECT_EQ(1, RefHooks<int>::timesRead(&r1));
  }

  RefHooks<int>::reset();
  {
    int refVal2 = 4;
    const Ref<int> r2(&refVal2);
    EXPECT_EQ(0, RefHooks<int>::timesRead(&r2));
    EXPECT_EQ(&refVal2, r2.get());
    EXPECT_EQ(1, RefHooks<int>::timesRead(&r2));
  }
}

TEST(Handle, Deref) {
  ValHooks<int>::reset();
  {
    Val<int> v(1);
    EXPECT_EQ(1, *v);
  }

  ValHooks<int>::reset();
  {
    const Val<int> v(2);
    EXPECT_EQ(2, *v);
  }

  int one = 1;

  RefHooks<int>::reset();
  {
    Ref<int> v(&one);
    EXPECT_EQ(&one, v.get());
  }

  RefHooks<int>::reset();
  {
    const Ref<int> v(&one);
    EXPECT_EQ(&one, v.get());
  }
}

TEST(Handle, Member) {
  ValHooks<int>::reset();
  {
    DummyStruct d(1);
    Ref<DummyStruct> v(&d);
    EXPECT_TRUE(v->v == d.v);

    d.v = 2;
    EXPECT_TRUE(v->v == d.v);
  }

  ValHooks<int>::reset();
  {
    DummyStruct d(1);
    const Ref<DummyStruct> v(&d);
    EXPECT_TRUE(v->v == d.v);
  }
}

TEST(Handle, Swap) {
  ValHooks<int>::reset();
  Val<int> v1(1);
  Val<int> v2(2);
  std::swap(v1, v2);
  EXPECT_EQ(*v1, 2);
  EXPECT_EQ(*v2, 1);
}

TEST(Handle, Size) {
  ValHooks<int>::reset();
  Val<int> m1(0);
  static_assert(sizeof(m1) == sizeof(int),  // NOLINT(runtime/sizeof)
                "Handle size should be equal to its data");

  WeakHooks<int>::reset();
  Weak<int> m2(nullptr);
  static_assert(sizeof(m2) == sizeof(int*),  // NOLINT(runtime/sizeof)
                "Handle size should be equal to its data");

  RefHooks<int>::reset();
  Ref<int> m3(nullptr);
  static_assert(sizeof(m3) == sizeof(int*),  // NOLINT(runtime/sizeof)
                "Handle size should be equal to its data");
}
