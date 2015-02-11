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

#include <set>

#include "gtest/gtest.h"

#include "storage/newable.h"

namespace {

struct MockGCHooks {
 public:
  static void *allocate(size_t size) {
    return reinterpret_cast<void *>(0xdeadbeef);
  }
};

struct MockNewable : public arw::Newable<MockNewable, MockGCHooks> {
};

}  // namespace

TEST(Newable, New) {
  MockNewable *obj = new MockNewable;
  EXPECT_EQ(obj, reinterpret_cast<MockNewable *>(0xdeadbeef));
}

TEST(Newable, Delete) {
  MockNewable *obj = reinterpret_cast<MockNewable *>(0x1234);
  delete obj;  // should be ok, delete is a no-op
}
