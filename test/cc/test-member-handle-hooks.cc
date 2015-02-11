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

#include "member-handle-hooks.h"
#include "handle.h"

#include <gtest/gtest.h>

namespace {


class MockGCHooks {
 public:
  MockGCHooks() = delete;

  inline static void* read(void** ptr) {
    return _readNull ? nullptr : *ptr;
  }

  inline static void write(void** ptr, void* value) {
    *ptr = _writeNull ? nullptr : value;
  }

  /**
   * Set to false by default. When this is set to true, any read operation will
   * return nullptr instead of the actual object. This is used to verify that
   * the read method is actually invoked.
   */
  static void setReadNull(bool readNull) {
    _readNull = readNull;
  }

  /**
   * Set to false by default. When this is set to true, any write operation will
   * set the pointer to nullptr instead of the actual object. This is used to
   * verify that the write method is actually invoked.
   */
  static void setWriteNull(bool writeNull) {
    _writeNull = writeNull;
  }

 private:
  static bool _readNull;
  static bool _writeNull;
};

bool MockGCHooks::_readNull = false;
bool MockGCHooks::_writeNull = false;

}  // namespace

template<typename T, arw::HandleType Type>
using Member =
arw::Handle<T, Type, arw::internal::MemberHandleHooks<T, Type, MockGCHooks> >;


TEST(MemberHooks, Lifetime) {
  Member<int, arw::HandleType::VALUE> v;
}

TEST(MemberHooks, Read) {
  int val = 5;
  Member<int, arw::HandleType::WEAK> m(&val);
  EXPECT_EQ(m.get(), &val);
  MockGCHooks::setReadNull(true);
  EXPECT_EQ(m.get(), nullptr);
  MockGCHooks::setReadNull(false);
}

TEST(MemberHooks, Write) {
  int val1 = 5;
  int val2 = 6;
  Member<int, arw::HandleType::WEAK> m(&val1);
  m = &val2;
  EXPECT_EQ(m.get(), &val2);
  MockGCHooks::setWriteNull(true);
  m = &val1;
  EXPECT_EQ(m.get(), nullptr);
  MockGCHooks::setWriteNull(false);
}

TEST(MemberHooks, Const) {
  Member<const int, arw::HandleType::REFERENCE> m(nullptr);
  // Make sure the following line compiles
  const int* ref = m.get();
}
