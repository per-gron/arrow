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

#include "storage/local.h"
#include "storage/type_storage.h"

namespace {

class EmptyObject {
  ARW_DECLARE_STORAGE;
};

class MockGCHooks {
 public:
  MockGCHooks() = delete;

  static void* allocate(size_t size) {
    return nullptr;
  }

  static void* read(void** ptr) {
    return *ptr;
  }

  static void write(void** ptr, void* value) {
    *ptr = value;
  }
};

}  // namespace

ARW_DEFINE_STORAGE_0(
  EmptyObject
);

TEST(MakeLocalValue, Local) {
  auto local = arw::local<
      EmptyObject, arw::HandleType::VALUE, MockGCHooks>();
}

TEST(MakeLocalReference, Local) {
  auto local = arw::local<
      EmptyObject, arw::HandleType::REFERENCE, MockGCHooks>();
}

TEST(MakeLocalVal, Local) {
  auto local = arw::localVal<EmptyObject, MockGCHooks>();
}

TEST(MakeLocalRef, Local) {
  auto local = arw::localRef<EmptyObject, MockGCHooks>();
}
