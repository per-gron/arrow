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

#include <gtest/gtest.h>
#include "local_stack.h"

typedef arw::LocalStack<int, int> Stack;

TEST(LocalStack, ConstructEmpty) {
  Stack s;
  EXPECT_TRUE(s.empty());
}

TEST(LocalStack, TrivialIterator) {
  Stack s;

  auto begin = s.begin();
  auto end = s.end();
  EXPECT_EQ(begin, end);
}

TEST(LocalStack, Push) {
  Stack s;
  s.push(0, 1);

  EXPECT_TRUE(!s.empty());

  auto begin = s.begin();
  auto end = s.end();

  EXPECT_EQ((*begin).first, 0);
  EXPECT_EQ((*begin).second, 1);

  EXPECT_TRUE(begin != end);
  begin++;
  EXPECT_EQ(begin, end);
}

TEST(LocalStack, PushConvenience) {
  Stack s;
  s.push(0, 1);
  EXPECT_TRUE(!s.empty());
  EXPECT_EQ((*s.begin()).first, 0);
  EXPECT_EQ((*s.begin()).second, 1);
}

TEST(LocalStack, Clear) {
  Stack s;
  s.push(0, 1);
  s.clear();
  EXPECT_TRUE(s.empty());

  s.push(2, 3);
  s.push(4, 5);
  s.clear();
  EXPECT_TRUE(s.empty());
}

TEST(LocalStack, TrivialTopEquality) {
  Stack s;

  Stack::ref r1 = s.top();
  Stack::ref r2 = s.top();

  EXPECT_EQ(r1, r2);
}

TEST(LocalStack, TrivialTopInequality) {
  Stack s;

  Stack::ref r1 = s.top();

  s.push(0, 1);

  Stack::ref r2 = s.top();

  EXPECT_TRUE(r1 != r2);
}

TEST(LocalStack, Pop) {
  Stack s;

  Stack::ref r1 = s.top();
  s.popTo(r1);
  EXPECT_TRUE(s.empty());

  s.push(0, 1);
  s.popTo(r1);
  EXPECT_TRUE(s.empty());

  s.push(2, 3);
  Stack::ref r2 = s.top();
  s.push(4, 5);

  for (int i = 0; i < 2; i++) {
    s.popTo(r2);
    EXPECT_TRUE(!s.empty());

    auto iter = s.begin();
    auto end = s.end();
    ++iter;
    EXPECT_EQ(iter, end);
  }

  s.popTo(r1);
  EXPECT_TRUE(s.empty());
  EXPECT_DEATH(s.popTo(r2), "");
}
