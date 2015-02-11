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

#include <gtest/gtest.h>

#include "type-storage.h"

#include "member.h"

namespace {

class MockGCHooks {
 public:
  MockGCHooks() = delete;

  inline static void* read(void** ptr) {
    return *ptr;
  }

  inline static void write(void** ptr, void* value) {
    *ptr = value;
  }
};

class Members0 {
  ARW_DECLARE_STORAGE;
};

class Members1 {
  ARW_DECLARE_STORAGE;

  arw::MemberRef<Members0, MockGCHooks> member1;
};

class Members2 {
  ARW_DECLARE_STORAGE;

  arw::MemberRef<Members0, MockGCHooks> member1;
  arw::MemberRef<Members0, MockGCHooks> member2;
};

class Circular {
  ARW_DECLARE_STORAGE;
 public:
  typedef MockGCHooks gc_hooks;

 private:
  arw::MemberRef<Circular, MockGCHooks> self;
};

}  // namespace

ARW_DEFINE_STORAGE_1(
  Circular,
  self
);

ARW_DEFINE_STORAGE_0(
  Members0
);

ARW_DEFINE_STORAGE_1(
  Members1,
  member1
);

ARW_DEFINE_STORAGE_2(
  Members2,
  member1,
  member2
);

TEST(Circular, TypeStorage) {
  const auto storage = arw::storageDescriptor<Circular>();
  EXPECT_TRUE(storage != nullptr);
  EXPECT_FALSE(storage->hasArray());
  EXPECT_FALSE(storage->isBoxed());
}

TEST(StorageDescriptorGetterWithGCHooks, TypeStorage) {
  const auto storage = arw::storageDescriptor<MockGCHooks, Circular>();
  EXPECT_TRUE(storage != nullptr);
}

// TODO(peck): Write tests that verify the actual memory layout.
