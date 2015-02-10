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

#pragma once

#include <vector>
#include <utility>

#include "checks.h"

namespace arw {

/**
 * In order to be able to create local (i.e. stack-based scoped) handles, the
 * garbage collector needs some way to iterate through the references that are
 * on the stack. LocalStack is the data structure that contains the references.
 *
 * Local handles will add themselves to this stack, and the garbage collector
 * will inspect it.
 *
 * This class is intended to have one instance per thread, and be accessible
 * through thread-local storage. It is not thread safe.
 *
 * Local handles are not supposed to remove themselves from this stack, that's
 * the task of separate LocalScope objects. This is an optimization inspired by
 * V8's HandleScope.
 */
template<typename Descriptor, typename Data>
class LocalStack {
 public:
  typedef std::pair<Descriptor, Data> value_type;
  /**
   * This is supposed to be an opaque type. Don't do anything with it except to
   * pass it back to the LocalStack.
   */
  typedef size_t ref;

 private:
  typedef std::vector<value_type> Vec;

 public:
  typedef typename Vec::const_iterator const_iterator;

  /**
   * Construct an empty LocalStack.
   */
  LocalStack() {}

  LocalStack(const LocalStack&) = delete;
  LocalStack& operator=(const LocalStack&) = delete;

  /**
   * Get a reference to the top of the stack. This can be used later to pop all
   * references that have been added since.
   */
  ref top() const {
    return _vec.size();
  }

  /**
   * Returns true if the stack is empty.
   */
  bool empty() const {
    return _vec.empty();
  }

  /**
   * Erases all elements of the stack.
   */
  void clear() {
    _vec.clear();
  }

  /**
   * Pop all references that have been added since he ref was created.
   *
   * It is an error to pop to a ref that itself has been popped. For instance,
   * consider the following sequence of operations: add something, take a top
   * ref 1, add something, take a top ref 2, pop to ref 1. After this, it is
   * illegal to use ref 2.
   *
   * Note that in the current implementation, LocalStack does not verify this
   * in all cases.
   */
  void popTo(ref r) {
    ARW_CHECK(_vec.size() >= r);
    _vec.resize(r);
  }

  /**
   * Push a reference to the stack. This is shorthand for
   * push(std::make_pair(desc, data)).
   */
  void push(const Descriptor& desc, const Data& data) {
    push(std::make_pair(desc, data));
  }

  /**
   * Push a reference to the stack.
   */
  void push(const value_type& v) {
    _vec.push_back(v);
  }

  /**
   * Returns a const_iterator pointing to the beginning of the stack.
   */
  const_iterator begin() const {
    return _vec.begin();
  }

  /**
   * Returns a const_iterator pointing to the end of the stack.
   */
  const_iterator end() const {
    return _vec.end();
  }

 private:
  Vec _vec;
};

}  // namespace arw
