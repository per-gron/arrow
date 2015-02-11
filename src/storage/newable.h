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

namespace arw {

/**
 * Newable is a little helper class that makes it easy to write classes that can
 * be easily allocated with the garbage collector. It defines a custom new
 * operator that allocates memory with the garbage collector (which GC to use
 * is specified as a template parameter).
 *
 * Please note that this class can only be used for fixed-size objects; arrays
 * need to declare their own new operators.
 */
template<typename ChildClass, typename GCHooks>
class Newable {
 protected:
  Newable() {}
  // Intentionally don't have a virtual destructor; this class is not supposed
  // to be use as a base class in ad hoc polymorphism.

 public:
  inline static void* operator new(std::size_t sz) {
    return GCHooks::allocate(sizeof(ChildClass) * sz);
  }
  inline static void operator delete(void* ptr, std::size_t sz) {
    // The garbage collector is a garbage collector so it will collect the
    // garbage for us later.
  }
};

}  // namespace arw
