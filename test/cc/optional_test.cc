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

#include "gtest/gtest.h"
#include "optional.h"

namespace {

class RefCounter {
 public:
  RefCounter() : _counter(NULL) {}

  explicit RefCounter(int* counter)
      : _counter(counter) {
    inc();
  }

  virtual ~RefCounter() { dec(); }

  RefCounter(const RefCounter& rhs)
      : _counter(rhs._counter) {
    inc();
  }

  RefCounter(RefCounter&& rhs) : _counter(NULL) {
    std::swap(_counter, rhs._counter);
  }

  RefCounter& operator=(const RefCounter& rhs) {
    dec();
    _counter = rhs._counter;
    inc();

    return *this;
  }

  RefCounter& operator=(RefCounter&& rhs) {
    if (this != &rhs) {
      _counter = rhs._counter;
      rhs._counter = NULL;
    }
    return *this;
  }

 private:
  int* _counter;

  void inc() {
    if (_counter) {
      (*_counter)++;
    }
  }

  void dec() {
    if (_counter) {
      (*_counter)--;
    }
  }
};

template<typename T>
class Holder {
 public:
  explicit Holder(const T& val) : _val(val) {}
  virtual ~Holder() {}

  Holder(const Holder<T>&) = delete;
  Holder<T>& operator=(const Holder<T>&) = delete;

  T get() { return _val; }
  const T get() const { return _val; }
  void set(const T& val) { _val = val; }

 private:
  T _val;
};

}  // namespace

TEST(Optional, Harness) {
  // Test RefCounter
  {
    int counter = 0;
    {
      RefCounter rc1(&counter);
      EXPECT_EQ(1, counter);
      RefCounter rc2(rc1);
      EXPECT_EQ(2, counter);
      RefCounter rc3;
      rc3 = rc2;
      EXPECT_EQ(3, counter);
      rc1 = rc3;
      EXPECT_EQ(3, counter);
      RefCounter rc4;
    }
    EXPECT_EQ(0, counter);
  }

  // Test Holder
  {
    Holder<int> h(0);
    EXPECT_EQ(0, h.get());
    h.set(1);
    EXPECT_EQ(1, h.get());
  }
}

TEST(Optional, IsSet) {
  {
    arw::Optional<int> m;
    EXPECT_TRUE(!m);
    EXPECT_TRUE(!m.isSet());
  }

  {
    arw::Optional<int> m(0);
    EXPECT_TRUE(!!m);
    EXPECT_TRUE(m.isSet());
  }
}

TEST(Optional, Assignment) {
  {
    // Test optional assignment operator
    int counter = 0;
    {
      RefCounter rc(&counter);
      arw::Optional<RefCounter> m1;
      EXPECT_EQ(1, counter);
      m1 = rc;
      EXPECT_EQ(2, counter);
      arw::Optional<RefCounter> m2;
      m2 = m1;
      EXPECT_EQ(3, counter);
      m1 = m2;
      EXPECT_EQ(3, counter);
    }
    EXPECT_EQ(0, counter);
  }

  {
    // Test optional move assignment operator
    int counter = 0;
    {
      RefCounter rc(&counter);
      arw::Optional<RefCounter> m1;
      EXPECT_EQ(1, counter);
      m1 = std::move(rc);
      EXPECT_EQ(1, counter);
      arw::Optional<RefCounter> m2;
      m2 = std::move(m1);
      EXPECT_EQ(1, counter);
    }
    EXPECT_EQ(0, counter);
  }
}

TEST(Optional, Constructor) {
  {
    // Test optional copy constructor
    int counter = 0;
    {
      RefCounter rc(&counter);
      arw::Optional<RefCounter> m1(rc);
      EXPECT_EQ(2, counter);
      arw::Optional<RefCounter> m2(m1);
      EXPECT_EQ(3, counter);
    }
    EXPECT_EQ(0, counter);
  }

  {
    // Test Optional move constructor
    int counter = 0;
    {
      RefCounter rc(&counter);
      arw::Optional<RefCounter> m1(std::move(rc));
      EXPECT_EQ(1, counter);
      arw::Optional<RefCounter> m2(std::move(m1));
      EXPECT_EQ(1, counter);
    }
    EXPECT_EQ(0, counter);
  }
}

TEST(Optional, Clear) {
  {
    arw::Optional<int> m;
    EXPECT_TRUE(!m.isSet());
    m.clear();
    EXPECT_TRUE(!m.isSet());
  }

  {
    arw::Optional<int> m(0);
    EXPECT_TRUE(m.isSet());
    m.clear();
    EXPECT_TRUE(!m.isSet());
  }
}

TEST(Optional, Swap) {
  {
    arw::Optional<int> m1;
    arw::Optional<int> m2;
    std::swap(m1, m2);
    EXPECT_TRUE(!m1.isSet());
    EXPECT_TRUE(!m2.isSet());
  }

  {
    arw::Optional<int> m1(1);
    arw::Optional<int> m2;
    std::swap(m1, m2);
    EXPECT_TRUE(!m1.isSet());
    EXPECT_TRUE(1 == m2);  // NOLINT(readability/check)
  }

  {
    arw::Optional<int> m1;
    arw::Optional<int> m2(1);
    std::swap(m1, m2);
    EXPECT_TRUE(1 == m1);  // NOLINT(readability/check)
    EXPECT_TRUE(!m2.isSet());
  }

  {
    arw::Optional<int> m1(1);
    arw::Optional<int> m2(2);
    std::swap(m1, m2);
    EXPECT_TRUE(2 == m1);  // NOLINT(readability/check)
    EXPECT_TRUE(1 == m2);  // NOLINT(readability/check)
  }
}

TEST(Optional, Reference) {
  {
    arw::Optional<int&> m;
  }

  {
    int val = 1;
    int& ref = val;
    arw::Optional<int&> m(ref);
    EXPECT_EQ(1, *m);
  }

  {
    int val1 = 1;
    int& ref1 = val1;
    arw::Optional<int&> m1(ref1);

    int val2 = 2;
    int& ref2 = val2;
    arw::Optional<int&> m2(ref2);

    int val3 = 2;
    int& ref3 = val3;
    arw::Optional<int&> m3(ref3);

    EXPECT_TRUE(ref1 == m1);
    EXPECT_TRUE(m1 == ref1);
    EXPECT_TRUE(m2 == m3);
    EXPECT_TRUE(!(ref1 != m1));
    EXPECT_TRUE(!(m1 != ref1));
    EXPECT_TRUE(!(m2 != m3));

    EXPECT_TRUE(1 == m1);  // NOLINT(readability/check)
    EXPECT_TRUE(m1 == 1);  // NOLINT(readability/check)
    EXPECT_TRUE(!(1 != m1));  // NOLINT(readability/check)
    EXPECT_TRUE(!(m1 != 1));  // NOLINT(readability/check)

    EXPECT_TRUE(m1 < m2);  // NOLINT(readability/check)
    EXPECT_TRUE(m1 < ref2);  // NOLINT(readability/check)
    EXPECT_TRUE(ref1 < m2);  // NOLINT(readability/check)
    EXPECT_TRUE(!(m1 > m2));  // NOLINT(readability/check)
    EXPECT_TRUE(!(m1 > ref2));  // NOLINT(readability/check)
    EXPECT_TRUE(!(ref1 > m2));  // NOLINT(readability/check)

    EXPECT_TRUE(1 < m2);  // NOLINT(readability/check)
    EXPECT_TRUE(m1 < 2);  // NOLINT(readability/check)
    EXPECT_TRUE(!(1 > m2));  // NOLINT(readability/check)
    EXPECT_TRUE(!(m1 > 2));  // NOLINT(readability/check)

    EXPECT_TRUE(m1 <= m1);  // NOLINT(readability/check)
    EXPECT_TRUE(m1 <= ref1);  // NOLINT(readability/check)
    EXPECT_TRUE(ref1 <= m1);  // NOLINT(readability/check)
    EXPECT_TRUE(m1 >= m1);  // NOLINT(readability/check)
    EXPECT_TRUE(m1 >= ref1);  // NOLINT(readability/check)
    EXPECT_TRUE(ref1 >= m1);  // NOLINT(readability/check)
  }

  {
    Holder<int> val(1);
    auto& ref = val;
    arw::Optional<decltype(ref)> m(ref);

    EXPECT_EQ(1, m->get());
    m->set(2);
    EXPECT_EQ(2, m->get());
    val.set(3);
    EXPECT_EQ(3, m->get());
  }

  {
    Holder<int> val(1);
    auto& ref = val;
    arw::Optional<decltype(ref)> m(std::move(ref));

    EXPECT_EQ(1, m->get());
    m->set(2);
    EXPECT_EQ(2, m->get());
    val.set(3);
    EXPECT_EQ(3, m->get());
  }
}

TEST(Optional, Functional) {
  {
    arw::Optional<int> m1;
    arw::Optional<int> m2 = m1.map([] (int a) -> int { return a; });
    EXPECT_TRUE(!m2);
  }

  {
    arw::Optional<int> m1(1);
    arw::Optional<int> m2 = m1.map([] (int a) -> int { return a; });
    EXPECT_TRUE(1 == m1);  // NOLINT(readability/check)
  }

  {
    arw::Optional<int> m1;
    int m2 = m1.ifElse([] (int a) -> int {
      return 0;
    }, [] () -> int {
      return 1;
    });
    EXPECT_TRUE(1 == m2);  // NOLINT(readability/check)
  }

  {
    arw::Optional<int> m1(1);
    int m2 = m1.ifElse([] (int a) -> int {
      return 0;
    }, [] () -> int {
      return 1;
    });
    EXPECT_TRUE(0 == m2);  // NOLINT(readability/check)
  }

  {
    bool flag1 = false;
    bool flag2 = false;
    arw::Optional<int> m1(1);
    m1.ifElse([&] (int a) {
      flag1 = true;
    }, [&] () {
      flag2 = true;
    });
    EXPECT_TRUE(flag1 && !flag2);
  }

  {
    bool flag1 = false;
    bool flag2 = false;
    arw::Optional<int> m1;
    m1.ifElse([&] (int a) {
      flag1 = true;
    }, [&] () {
      flag2 = true;
    });
    EXPECT_TRUE(!flag1 && flag2);
  }

  {
    bool flag = false;
    arw::Optional<int> m1;
    m1.each([&] (int a) {
      flag = true;
    });
    EXPECT_TRUE(!flag);
  }

  {
    bool flag = false;
    arw::Optional<int> m1(1);
    m1.each([&] (int a) {
      flag = true;
    });
    EXPECT_TRUE(flag);
  }
}

TEST(Optional, Equal) {
  arw::Optional<int> empty;
  arw::Optional<int> one(1);
  arw::Optional<int> two(2);
  arw::Optional<int> three(3);

  EXPECT_TRUE(empty == empty);
  EXPECT_TRUE(one == one);
  EXPECT_TRUE(!(one == empty));
  EXPECT_TRUE(!(empty == one));
  EXPECT_TRUE(!(one == two));
  EXPECT_TRUE(!(two == one));

  EXPECT_TRUE(one == 1);  // NOLINT(readability/check)
  EXPECT_TRUE(1 == one);  // NOLINT(readability/check)
  EXPECT_TRUE(1 != empty);  // NOLINT(readability/check)
  EXPECT_TRUE(empty != 1);  // NOLINT(readability/check)

  EXPECT_TRUE(!(empty != empty));
  EXPECT_TRUE(!(one != one));
  EXPECT_TRUE(one != empty);
  EXPECT_TRUE(empty != one);
  EXPECT_TRUE(one != two);
  EXPECT_TRUE(two != one);
}

TEST(Optional, Compare) {
  arw::Optional<int> empty;
  arw::Optional<int> one(1);
  arw::Optional<int> two(2);

  EXPECT_TRUE(empty < one);
  EXPECT_TRUE(one > empty);

  EXPECT_TRUE(one < two);
  EXPECT_TRUE(two > one);
  EXPECT_TRUE(!(one > two));
  EXPECT_TRUE(!(two < one));

  EXPECT_TRUE(one <= one);
  EXPECT_TRUE(one >= one);
  EXPECT_TRUE(one <= two);
  EXPECT_TRUE(two >= one);
  EXPECT_TRUE(!(two <= one));
  EXPECT_TRUE(!(one >= two));

  EXPECT_TRUE(1 < two);  // NOLINT(readability/check)
  EXPECT_TRUE(two > 1);  // NOLINT(readability/check)
  EXPECT_TRUE(empty < 1);  // NOLINT(readability/check)
  EXPECT_TRUE(1 > empty);  // NOLINT(readability/check)
}
