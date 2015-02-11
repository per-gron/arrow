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

#include "storage/persistent_pool.h"

typedef arw::PersistentPool<int, int> Pool;

TEST(PersistentPool, ConstructEmpty) {
  Pool p;
  EXPECT_TRUE(p.empty());
}

TEST(PersistentPool, AddConflict) {
  Pool p;

  EXPECT_TRUE(p.empty());
  p.add(0, 0);
  EXPECT_TRUE(!p.empty());

  // Add reference with conflicting descriptor
  EXPECT_DEATH(p.add(0, 1), "");
}

TEST(PersistentPool, AddAndRemove) {
  Pool p;

  p.add(0, 0);
  EXPECT_TRUE(!p.empty());
  p.remove(0);
  EXPECT_TRUE(p.empty());
}

TEST(PersistentPool, OverRemove) {
  Pool p;

  p.add(0, 0);
  p.remove(0);
  EXPECT_DEATH(p.remove(0), "");
}

TEST(PersistentPool, OneRefcount) {
  Pool p;

  p.add(0, 0);
  EXPECT_TRUE(!p.empty());
  p.add(0, 0);
  EXPECT_TRUE(!p.empty());
  p.remove(0);
  EXPECT_TRUE(!p.empty());
  p.remove(0);
  EXPECT_TRUE(p.empty());
}

TEST(PersistentPool, TwoRefcounts) {
  Pool p;

  p.add(1, 0);
  EXPECT_TRUE(!p.empty());

  p.add(0, 0);
  EXPECT_TRUE(!p.empty());
  p.add(0, 0);
  EXPECT_TRUE(!p.empty());
  p.remove(0);
  EXPECT_TRUE(!p.empty());
  p.remove(0);
  EXPECT_TRUE(!p.empty());

  p.remove(1);
  EXPECT_TRUE(p.empty());
}

TEST(PersistentPool, IteratorEquality) {
  Pool p;

  p.add(1, 0);

  auto begin = p.begin();
  auto end = p.end();
  auto end2 = p.end();

  EXPECT_TRUE(!(begin == end));
  EXPECT_TRUE(end == end2);
}

TEST(PersistentPool, IteratorInequality) {
  Pool p;

  p.add(1, 0);

  auto begin = p.begin();
  auto end = p.end();
  auto end2 = p.end();

  EXPECT_TRUE(begin != end);
  EXPECT_TRUE(!(end != end2));
}

TEST(PersistentPool, IteratorDereference) {
  Pool p;

  p.add(1, 0);

  auto begin = p.begin();

  auto entry = *begin;
  EXPECT_EQ(entry.first, 0);
  EXPECT_EQ(entry.second, 1);
}

TEST(PersistentPool, IteratorPointerDereference) {
  Pool p;

  p.add(1, 0);

  auto begin = p.begin();

  EXPECT_EQ(begin->first, 0);
  EXPECT_EQ(begin->second, 1);
}


TEST(PersistentPool, IteratorPreIncrement) {
  Pool p;

  p.add(1, 0);

  auto begin = p.begin();
  auto end = p.end();

  ++begin;
  EXPECT_TRUE(begin == end);
}

TEST(PersistentPool, IteratorPostIncrement) {
  Pool p;

  p.add(1, 0);

  auto begin = p.begin();
  auto end = p.end();

  begin++;
  EXPECT_TRUE(begin == end);
}

TEST(PersistentPool, BasicIteration) {
  std::set<int> refs;
  refs.insert(0);
  refs.insert(1);
  refs.insert(2);

  Pool p;
  for (auto val : refs) {
    p.add(val, 0);
  }

  for (auto val : p) {
    EXPECT_EQ(0, val.first);
    EXPECT_EQ(1, refs.erase(val.second));
  }
  EXPECT_TRUE(refs.empty());
}

TEST(PersistentPool, DuplicateIteration) {
  std::set<int> refs;
  refs.insert(0);
  refs.insert(1);
  refs.insert(2);

  Pool p;
  for (auto val : refs) {
    p.add(val, 0);
    p.add(val, 0);
  }

  for (auto val : p) {
    EXPECT_EQ(0, val.first);
    EXPECT_EQ(1, refs.erase(val.second));
  }
  EXPECT_TRUE(refs.empty());
}
