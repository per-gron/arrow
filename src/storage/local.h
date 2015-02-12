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

#include "storage/handle.h"

namespace arw {

namespace internal {

template<typename T, HandleType Type, typename GCHooks>
class LocalHandleHooks : public GCHooks {
 public:
  LocalHandleHooks() = delete;

  inline static void
  created(const Handle<T, Type, LocalHandleHooks>* handle) {}

  inline static void
  destroyed(const Handle<T, Type, LocalHandleHooks>* handle) {}
};

}  // namespace internal

/**
 * Scoped handle, for stack use. Used both for references and values.
 *
 * Beware that with local values, the user is responsible for making sure that
 * the value doesn't outlive its scope!
 */
template<typename T, HandleType Type, typename GCHooks>
using Local = Handle<T, Type, internal::LocalHandleHooks<T, Type, GCHooks>>;

/**
 * LocalVal<T> is shorthand for Local<T, HandleType::VALUE>
 */
template<typename T, typename GCHooks>
using LocalVal = Local<T, HandleType::VALUE, GCHooks>;

/**
 * LocalRef<T> is shorthand for Local<T, HandleType::REFERENCE>
 */
template<typename T, typename GCHooks>
using LocalRef = Local<T, HandleType::REFERENCE, GCHooks>;

/**
 * LocalWeak<T> is shorthand for Local<T, HandleType::WEAK>
 */
template<typename T, typename GCHooks>
using LocalWeak = Local<T, HandleType::WEAK, GCHooks>;

template<typename T, HandleType Type, typename GCHooks, typename... Args>
inline Local<T, Type, GCHooks> local(Args&&... args) {
  return Local<T, Type, GCHooks>::make(std::forward<Args>(args)...);
}

template<typename T, typename GCHooks, typename... Args>
inline LocalVal<T, GCHooks> localVal(Args&&... args) {
  return LocalVal<T, GCHooks>::make(std::forward<Args>(args)...);
}

template<typename T, typename GCHooks, typename... Args>
inline LocalRef<T, GCHooks> localRef(Args&&... args) {
  return LocalRef<T, GCHooks>::make(std::forward<Args>(args)...);
}

// There is no variant for creating a weak handle because it doesn't make sense;
// there has to be at least one strong reference to the object from the
// beginning.

}  // namespace arw
