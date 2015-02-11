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

#include <type_traits>

#include "storage/handle.h"

namespace arw {

namespace internal {

template<typename T, HandleType Type, typename GCHooks>
class MemberHandleHooks {
 public:
  MemberHandleHooks() = delete;

  inline static void
  created(const Handle<T, Type, MemberHandleHooks>* handle) {}

  inline static void
  destroyed(const Handle<T, Type, MemberHandleHooks>* handle) {}

  inline static T* read(T** ptr) {
    typedef typename std::remove_const<T>::type NonConstT;
    return static_cast<T*>(GCHooks::read(
      reinterpret_cast<void**>(
        const_cast<NonConstT**>(ptr))));
  }

  inline static void write(T** ptr, T* value) {
    GCHooks::write(reinterpret_cast<void**>(ptr), static_cast<void*>(value));
  }
};

}  // namespace internal

}  // namespace arw
