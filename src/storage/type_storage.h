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

#include <cstdint>
#include <new>
#include <vector>
#include <memory>

#include "base/checks.h"
#include "storage/handle.h"
#include "storage/storage_descriptor.h"

/**
 * This file defines the arw::internal::TypeStorage class, which (as the
 * namespace implies) should not usually be accessed directly. The functionality
 * that it provides is nonetheless important to understand: It exposes
 * StorageDescriptor objects for C++ types via its static storageDescriptor<T>()
 * method.
 *
 * Each C++ type that expects to be garbage collected should specialize this
 * method. This is done with helper macros also defined in this file.
 *
 * This information is then used by Local and Persistent handles to provide
 * StorageDescriptor information along with the actual objects to the garbage
 * collector.
 *
 * ---
 *
 * The macros defined in this file should be used when defining C++ classes
 * that should be in garbage collected memory. They are helpers that define
 * StorageDescriptor objects that contain the information that the garbage
 * collector needs in order to be able to traverse the object graph through
 * your C++ objects.
 *
 * On one hand this is rather tricky and weird, on the other hand this is really
 * powerful and cool; this is a well-defined and simple interface that makes it
 * possible to create truly first-class Arrow objects!
 *
 * When working with Arrow GC managed C++ objects, there are a bunch of rules
 * and limitations that must adhered to:
 *
 * 1) Arrow GC managed C++ objects must be declared as such. Here's an example:
 *
 * class AClass {
 *   ARW_DECLARE_STORAGE
 * };
 *
 * ARW_DEFINE_STORAGE_0(
 *   AClass
 * )
 *
 * The key points to consider here is that the class should have
 * ARW_DECLARE_STORAGE in it, and after the declaration, there should be an
 * ARW_DEFINE_STORAGE_{n}, where n is the number of member handles.
 *
 * 2) Every Arrow GC managed object must be wrapped inside the correct type of
 * handle. There are three main handle types: Member, Local and Persistent.
 * Member handles must be used in Arrow GC managed object instance variables.
 * Local handles should be used when storing values or references to managed
 * objects on the stack. Persistent handles must be used for values or
 * references with dynamic extent.
 *
 * The different handle types have different performance characteristics: Member
 * handles have the lowest overhead, bust can only be used for instance
 * variables of managed objects. Local handles have a relatively low overhead,
 * but they do maintain a shadow stack in order to be able to get the GC root
 * set. Persistent handles are the slowest handle type; it maintains a separate
 * root set.
 *
 * 3) The Arrow GC relies on all mutator thread invoking GC safe-points at
 * regular intervals. When Arrow code is executed, safe-points are automatically
 * invoked at proper times, but this is not the case for C++ code. C++ that
 * deals directly with managed objects must explicitly invoke GC safe points
 * and make sure to do it regularly. Failing to do so might cause GC stalls.
 *
 * 4) A fundamental difference between C++'s explicit memory management and
 * Arrow's GC is that C++ code, with its deterministic destruction behavior, can
 * rely on code being executed when an object is destructed. Since Arrow doesn't
 * guarantee object destruction at any given time (or on any given thread),
 * Arrow doesn't have the concept of destructors. For managed C++ objects, this
 * means that destructors will never be invoked.
 *
 * This seriously limits what managed C++ objects can contain: They can't
 * contain member variables of C++ objects that have non-trivial destructors.
 * This means for instance that managed C++ objects can't use the STL, for
 * instance.
 *
 * It is still possible to have pointers to manually managed resources, but
 * destruction of them must be managed explicitly; the user of the object is
 * responsible for invoking the cleanup method before the object becomes
 * garbage.
 */

/**
 * This macro should not be used directly. Use ARW_DEFINE_STORAGE[_ARR]_[N]
 * instead.
 *
 * For more information on how this stuff works, see the comments on
 * internal::TypeStorage.
 */
#define ARW_DEFINE_STORAGE_BEGIN(n, klass, hasArray, array)                  \
  template<typename GCHooks>                                                 \
  struct ArwInternalGetStorageDescriptor<GCHooks, klass> {                   \
    static ::arw::StorageDescriptor<GCHooks>* get() {                        \
      typedef ::arw::StorageDescriptor<GCHooks> SD;                          \
      static std::unique_ptr<SD> _arw_macro_storage(nullptr);                \
      if (_arw_macro_storage) {                                              \
        return _arw_macro_storage.get();                                     \
      }                                                                      \
      constexpr size_t _arw_macro_numValues = (n);                           \
      constexpr size_t _arw_macro_size =                                     \
          SD::objectSize(_arw_macro_numValues);                              \
      /* It is important that _arw_macro_storage is allocated before it's */ \
      /* initialized. See the comments on TypeStorage::storage for more   */ \
      /* info.                                                            */ \
      _arw_macro_storage = std::unique_ptr<SD>(                              \
          reinterpret_cast<SD*>(                                             \
              new char[_arw_macro_size]));                                   \
      SD::init(_arw_macro_storage.get(),                                     \
          sizeof(klass),                                                     \
          hasArray,                                                          \
          array,

/**
 * This macro should not be used directly. Use ARW_DEFINE_STORAGE[_ARR]_[N]
 * instead.
 *
 * For more information on how this stuff works, see the comments on
 * internal::TypeStorage.
 */
#define ARW_DEFINE_STORAGE_END                                               \
      ); /* NOLINT(whitespace/parens) */                                     \
      return _arw_macro_storage.get();                                       \
    }                                                                        \
  }

/**
 * This macro should be placed in the class definition of classes that use the
 * storage system. It should be put in the very beginning of the class, and
 * should not be followed by a semicolon.
 */
#define ARW_DECLARE_STORAGE                                                  \
  template<typename ARW_DECLARE_STORAGE_1, typename ARW_DECLARE_STORAGE_2>   \
  friend class ::ArwInternalGetStorageDescriptor

/**
 * This macro should be placed (with no following semicolon) after a class
 * definition for classes that use storage. The first argument is the name of
 * the class, and the following arguments (this is the zero member version, but
 * there are versions that take more parameters as well) are the names of the
 * members that are handles that should be recognized by Storage.
 */
#define ARW_DEFINE_STORAGE_0(klass)                                          \
  ARW_DEFINE_STORAGE_BEGIN(1,                                                \
                           klass,                                            \
                           false,                                            \
                           ::arw::StorageDescriptor<GCHooks>::Slot::empty)   \
    { }                                                                      \
  ARW_DEFINE_STORAGE_END

/**
 * This macro is like ARW_DEFINE_STORAGE_0, but it supports classes with an
 * array. You probably don't want to use this unless you know exactly what
 * you're doing. It's not usually necessary to use this directly, you should be
 * able to use the predefined array class for that.
 */
#define ARW_DEFINE_STORAGE_ARR_0(klass, array)                               \
  ARW_DEFINE_STORAGE_BEGIN(1,                                                \
                           klass,                                            \
                           true,                                             \
                           ::arw::internal::TypeStorage<GCHooks>::arraySlot( \
                             &klass::array))                                 \
    { }                                                                      \
  ARW_DEFINE_STORAGE_END


#define ARW_DEFINE_STORAGE_1(klass, member1)                                 \
  ARW_DEFINE_STORAGE_BEGIN(1,                                                \
                           klass,                                            \
                           false,                                            \
                           ::arw::StorageDescriptor<GCHooks>::Slot::empty)   \
    { ::arw::internal::TypeStorage<GCHooks>::slot(                           \
          &::arw::internal::Quote<klass>::member1) }                         \
  ARW_DEFINE_STORAGE_END

#define ARW_DEFINE_STORAGE_ARR_1(klass, array, member1)                      \
  ARW_DEFINE_STORAGE_BEGIN(1,                                                \
                           klass,                                            \
                           true,                                             \
                           ::arw::internal::TypeStorage<GCHooks>::arraySlot( \
                             &klass::array))                                 \
    { ::arw::internal::TypeStorage<GCHooks>::slot(                           \
          &::arw::internal::Quote<klass>::member1) }                         \
  ARW_DEFINE_STORAGE_END

/**
 * 2 argument version of ARW_DEFINE_STORAGE_1.
 */
#define ARW_DEFINE_STORAGE_2(klass, member1, member2)                        \
  ARW_DEFINE_STORAGE_BEGIN(2,                                                \
                           klass,                                            \
                           false,                                            \
                           ::arw::StorageDescriptor<GCHooks>::Slot::empty)   \
    {                                                                        \
      ::arw::internal::TypeStorage<GCHooks>::slot(                           \
          &::arw::internal::Quote<klass>::member1),                          \
      ::arw::internal::TypeStorage<GCHooks>::slot(                           \
          &::arw::internal::Quote<klass>::member2),                          \
    }                                                                        \
  ARW_DEFINE_STORAGE_END

/**
 * 2 argument version of ARW_DEFINE_STORAGE_ARR_1.
 */
#define ARW_DEFINE_STORAGE_ARR_2(klass, array, member1, member2)             \
  ARW_DEFINE_STORAGE_BEGIN(2,                                                \
                           klass,                                            \
                           true,                                             \
                           ::arw::internal::TypeStorage<GCHooks>::arraySlot( \
                             &klass::array))                                 \
    {                                                                        \
      ::arw::internal::TypeStorage<GCHooks>::slot(                           \
          &::arw::internal::Quote<klass>::member1),                          \
      ::arw::internal::TypeStorage<GCHooks>::slot(                           \
          &::arw::internal::Quote<klass>::member2),                          \
    }                                                                        \
  ARW_DEFINE_STORAGE_END

/**
 * This is a template function that is specialized on each C++ type that uses
 * the storage system.
 *
 * The specializations of this function should be generated by the
 * ARW_DEFINE_STORAGE[_ARR]_[N] family of macros.
 *
 * The main reason that this has to be a function, and can't simply be a
 * statically initialized value, is that we need to support circular type
 * definitions. For instance, arw::StorageDescriptor has arw::Storage::Slot
 * objects which in turn have arw::StorageDescriptor objects. In order to
 * support this scenario, this function initializes the StorageDescriptor
 * objects lazily. When it is invoked recursively, it might return a pointer
 * to uninitialized memory. However, it is guaranteed to return an initialized
 * StorageDescriptor* if it is called non-recursively. (Every time it returns
 * a pointer to uninitialized memory, it promises to initialize it before the
 * top-most (in the stack) invocation of the function returns.)
 */
template<typename GCHooks, typename T>
class ArwInternalGetStorageDescriptor {
  ArwInternalGetStorageDescriptor() = delete;

  typedef typename arw::StorageDescriptor<GCHooks>::Slot Slot;

 public:
  static arw::StorageDescriptor<GCHooks>* get();
};

namespace arw {

namespace internal {

/**
 * Little helper to not have to write typename for some types.
 */
template<typename T>
using Quote = T;

template<typename GCHooks>
class TypeStorage {
 public:
  typedef typename StorageDescriptor<GCHooks>::Slot Slot;

  TypeStorage() = delete;

  /**
   * This is a helper method, only to be used by offset. It is there because the
   * compiler complains about the null pointer that's used.
   */
  template<typename U, typename V>
  static constexpr const void* offsetHelper(U* val, V U::*member) {
    return static_cast<const void*>(&(static_cast<U*>(val)->*member));
  }

  /**
   * This is a helper method that returns the offset of a particular member of a
   * standard layout class/struct.
   */
  template<typename U, typename V>
  static constexpr const void* offset(V U::*member) {
    static_assert(std::is_standard_layout<U>::value,
                  "Type must have standard layout");
    return offsetHelper(static_cast<U*>(nullptr), member);
  }

  /**
   * This is a helper method that creates a Storage::Slot object for a
   * particular member of a standard layout class/struct.
   */
  template<typename U, typename V>
  static Slot slot(V U::*member) {
    return {
      ArwInternalGetStorageDescriptor<GCHooks, typename V::value_type>::get(),
      V::handleType,
      reinterpret_cast<size_t>(offset(member))
    };
  }

  /**
   * Like the slot method, but creates a Storage::Slot object for the array
   * slot of a class.
   */
  template<typename U, typename V>
  static Slot arraySlot(V U::*member) {
    return {
      ArwInternalGetStorageDescriptor<GCHooks, typename V::storage_type>::get(),
      U::handleType,
      reinterpret_cast<size_t>(offset(member))
    };
  }
};

}  // namespace internal

template<typename GCHooks, typename T>
arw::StorageDescriptor<GCHooks>* storageDescriptor() {
  return ArwInternalGetStorageDescriptor<GCHooks, T>::get();
}

template<typename T>
arw::StorageDescriptor<typename T::gc_hooks>* storageDescriptor() {
  return ArwInternalGetStorageDescriptor<typename T::gc_hooks, T>::get();
}

}  // namespace arw

ARW_DEFINE_STORAGE_1(
  typename arw::StorageDescriptor<GCHooks>::Slot,
  storageDescriptor
);

ARW_DEFINE_STORAGE_ARR_1(
  arw::StorageDescriptor<GCHooks>,
  /*array member:*/_values,
  _array
);
