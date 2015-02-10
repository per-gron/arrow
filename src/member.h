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

#include "handle.h"
#include "member-handle-hooks.h"

namespace arw {

/**
 * Storage handle, for use in Storage enabled classes. Used both for references
 * values. The main purpose of this class is to be a hook for implementing
 * read/write barriers.
 */
template<typename T, HandleType Type, typename GCHooks>
using Member = Handle<T, Type, internal::MemberHandleHooks<T, Type, GCHooks> >;

/**
 * MemberVal<T, GCHooks> is shorthand for Member<T, HandleType::VALUE, GCHooks>
 */
template<typename T, typename GCHooks>
using MemberVal = Member<T, HandleType::VALUE, GCHooks>;

/**
 * MemberRef<T, GCHooks> is shorthand for
 * Member<T, HandleType::REFERENCE, GCHooks>
 */
template<typename T, typename GCHooks>
using MemberRef = Member<T, HandleType::REFERENCE, GCHooks>;

/**
 * MemberWeak<T, GCHooks> is shorthand for Member<T, HandleType::VALUE, GCHooks>
 */
template<typename T, typename GCHooks>
using MemberWeak = Member<T, HandleType::WEAK, GCHooks>;

}  // namespace arw
