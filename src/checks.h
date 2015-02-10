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

#ifdef DEBUG

#define ARW_UNIMPLEMENTED()                                   \
  ::arw::Fatal(__FILE__, __LINE__, "Unimplemented")

#define ARW_UNREACHABLE()                                   \
  ::arw::Fatal(__FILE__, __LINE__, "Unreachable")

#define ARW_FATAL(msg)                                    \
  ::arw::Fatal(__FILE__, __LINE__, "%s", (msg))

#else

#define ARW_UNIMPLEMENTED()                       \
  ::arw::Fatal("", 0, "Unimplemented")

#define ARW_UNREACHABLE() ((void) 0)

#define ARW_FATAL(msg)                                    \
  ::arw::Fatal("", 0, "%s", (msg))

#endif

#define ARW_CHECK(condition)                         \
  do {                                               \
    if (!(condition)) {                              \
      ::arw::Check(__FILE__,                         \
                   __LINE__,                         \
                   "ARW_CHECK(%s) Failed",           \
                   #condition);                      \
    }                                                \
  } while (0)


#ifdef DEBUG

#define ARW_ASSERT(condition) ARW_CHECK(condition)

#else

#define ARW_ASSERT(condition) ((void) 0)

#endif

namespace arw {

void Check(const char* file, int line, const char* fmt, ...);

}  // namespace arw
