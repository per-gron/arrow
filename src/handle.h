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

#ifndef ARW_HANDLE_H_
#define ARW_HANDLE_H_

#include <algorithm>

namespace arw {

enum class HandleType : char {
  /**
   * This is a value, stored in place.
   */
  VALUE,
  /**
   * This is a strong reference.
   */
  REFERENCE,
  /**
   * This is a weak reference.
   */
  WEAK
};

namespace internal {

template<typename T, HandleType Type>
struct HandleTypes {
};

template<typename T>
struct HandleTypes<T, HandleType::VALUE> {
  typedef T          value_type;
  typedef value_type storage_type;
};

template<typename T>
struct HandleTypes<T, HandleType::REFERENCE> {
  typedef T           value_type;
  typedef value_type* storage_type;
};

template<typename T>
struct HandleTypes<T, HandleType::WEAK> {
  typedef T           value_type;
  typedef value_type* storage_type;
};

}  // namespace internal

/**
 * This class implements the common handle logic: construction, copying, moving,
 * assignment, operator*, operator-> etc. It is not intended to be used
 * directly. Instead it provides a set of hooks for when the handle is created,
 * destroyed, read and written.
 *
 * Hooks is a class that is expected to have four static methods, like this:
 *
 * template<typename T, HandleType Type>
 * struct HandleHooks {
 *   inline static void
 *   created(const Handle<T, Type, HandleHooks>* handle) {}
 *
 *   inline static void
 *   destroyed(const Handle<T, Type, HandleHooks>* handle) {}
 *
 *   inline static T* read(T** ptr) {
 *     return *ptr;
 *   }
 *
 *   inline static void write(T** ptr, T* value) {
 *     *ptr = value;
 *   }
 * };
 *
 * created and destroyed are only called on construction and destruction of
 * handles, not on assignment or move. They are used to be able to implement
 * tracking of a root set for the garbage collector.
 *
 * read is invoked on reference and weak reference handles whenever the handle
 * is read. write is invoked on reference handles and weak reference  handles
 * whenever the handle is set. Neither read nor write are invoked on value
 * handles. The read and write methods are used to implement various garbage
 * collection barrier techniques.
 *
 * read takes a pointer to the reference that the handle holds and should return
 * a reference. It is allowed to update the reference and can return anything.
 *
 * write takes a pointer to the reference that the handle holds and a reference
 * that the handle should be set to. It is allowed to set the handle to
 * something else than the reference it was given.
 *
 * Please note that write is not invoked on handle construction.
 */
template<typename T, HandleType Type, typename Hooks>
class Handle {
 private:
  typedef internal::HandleTypes<T, Type> types;

 public:
  static constexpr HandleType handleType = Type;
  typedef typename types::value_type value_type;
  typedef typename types::storage_type storage_type;
  typedef storage_type const& reference_const_type;
  typedef storage_type&       reference_type;
  typedef value_type const*   pointer_const_type;
  typedef value_type*         pointer_type;
  typedef storage_type const& argument_type;

  Handle() {
    wasCreated();
  }

  Handle(const Handle<T, Type, Hooks>& other)
      : _data(other._data) {
    wasCreated();
  }

  Handle(Handle<T, Type, Hooks>&& other)
      : _data(std::move(other._data)) {
    wasCreated();
  }

  explicit Handle(argument_type other)
      : _data(other) {
    wasCreated();
  }

  explicit Handle(storage_type&& other)
      : _data(std::move(other)) {
    wasCreated();
  }

  ~Handle() {
    wasDestroyed();
  }

  Handle<T, Type, Hooks>& operator=(const Handle<T, Type, Hooks>& other) {
    write(other._data);
    return *this;
  }

  Handle<T, Type, Hooks>& operator=(Handle<T, Type, Hooks>&& other) {
    write(std::move(other._data));
    return *this;
  }

  Handle<T, Type, Hooks>& operator=(argument_type other) {
    write(other);
    return *this;
  }

  Handle<T, Type, Hooks>& operator=(storage_type&& other) {
    write(std::move(other));
    return *this;
  }

  /**
   * Returns true if this is a dead weak pointer.
   */
  explicit operator bool() const {
    static_assert(HandleType::WEAK == Type,
                  "bool conversion only available for weak refs");
    return !!get();
  }

  /**
   * Returns a pointer to the stored object.
   */
  template<typename U = T>
  typename std::enable_if<HandleType::VALUE == Type,
                          typename internal::HandleTypes<U, Type>::
                            value_type*>::type
  get() { return &_data; }

  template<typename U = T>
  typename std::enable_if<HandleType::VALUE == Type,
                          typename internal::HandleTypes<U, Type>::
                            value_type const*>::type
  get() const { return &_data; }

  template<typename U = T>
  typename std::enable_if<HandleType::VALUE != Type,
                          typename internal::HandleTypes<U, Type>::
                            value_type*>::type
  get() {
    return Hooks::read(&_data);
  }

  template<typename U = T>
  typename std::enable_if<HandleType::VALUE != Type,
                          typename internal::HandleTypes<U, Type>::
                            value_type const*>::type
  get() const {
    return Hooks::read(&const_cast<storage_type&>(_data));
  }

  /**
   * This is a convenience operator in order to be able to use the
   * object as you would a smart pointer. Returns nullptr if not set.
   */
  pointer_type operator->() { return get(); }

  pointer_const_type operator->() const { return get(); }

  /**
   * This is a convenience operator in order to be able to use the
   * object as you would a smart pointer. Return value is undefined
   * if not set.
   */
  value_type& operator*() { return *get(); }

  value_type const& operator*() const { return *get(); }

 private:
  inline void wasCreated() const {
    static_assert(sizeof(Handle<T, Type, HandleType>) == sizeof(storage_type),
                  "Handle must be the same size as its storage type");

    Hooks::created(this);
  }

  inline void wasDestroyed() const {
    Hooks::destroyed(this);
  }

  template<typename U = HandleType>
  inline
  typename std::enable_if<U::VALUE == Type, void>::type
  write(argument_type val) {
    _data = val;
  }

  template<typename U = HandleType>
  inline
  typename std::enable_if<U::VALUE == Type, void>::type
  write(storage_type&& val) {
    _data = std::move(val);
  }

  template<typename U = HandleType>
  inline
  typename std::enable_if<U::VALUE != Type, void>::type
  write(T* val) {
    Hooks::write(&_data, val);
  }

  storage_type _data;
};

}  // namespace arw

namespace std {

template<typename T, ::arw::HandleType Type, typename Hooks>
void swap(arw::Handle<T, Type, Hooks>& lhs,
          arw::Handle<T, Type, Hooks>& rhs) {
  std::swap(*lhs, *rhs);
}

}  // namespace std

#endif  // ARW_HANDLE_H_
