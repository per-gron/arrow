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

#include "handle.h"
#include "member.h"
#include "checks.h"

namespace arw {

namespace internal {
template<typename GCHooks>
class TypeStorage;
}  // namespace internal

/**
 * This is a low level API. It is used to implement the GC visitor and to
 * implement the FFI managed memory management interface. Unless you are working
 * on the GC visitor, the FFI system or memory management parts of the compiler
 * itself, this is not the class you want to interface to.
 *
 * The purpose of this class is to describe objects so that the garbage
 * collector can traverse them. Since Arrow is a statically typed language, that
 * information is stored separately, and not (usually) as part of the objects
 * themselves.
 *
 * This class should not be inherited; each garbage collected object type has
 * one storage object.
 *
 * Because the storage type graph is an arbitrary directed graph, and for many
 * types contain cycles, StorageDescriptor objects can't be reference counted.
 * Because of JIT, and because I want to be able to easily manipulate
 * StorageDescriptor objects in C++, I don't want to manually (or statically)
 * manage the memory of StorageDescriptor objects.
 *
 * I have decided to make the StorageDescriptor objects themselves garbage
 * collected. This gives us automatic memory management, but also has the
 * unfortunate drawback of a somewhat weird circular definition of the
 * StorageDescriptor class: StorageDescriptor objects have an associated
 * StorageDescriptor object that describes its memory layout for the GC.
 *
 * Arrow's memory model is different from C++'s when it comes to arrays: Arrow
 * implements arrays through variable-size objects. Since StorageDescriptor
 * contains an array, it has variable size. I need to use a bit of magic in
 * order to make the class both C++ and Arrow-GC compatible.
 *
 * It is important to recognize the difference between concrete
 * StorageDescriptor objects and the types that are present in the surface
 * Arrow language. In some cases, there is a 1-1 mapping between the types, but
 * in other cases, the relationship is more complex. Polymorphic and existential
 * types are examples of when that's the case.
 *
 * I have tried to design the StorageDescriptor system to be as simple as
 * possible, yet powerful enough to express the types in Arrow. A consequence of
 * this is that information is lost when converting an Arrow type into a
 * StorageDescriptor object: only the information that is necessary for the GC
 * to do its work is left.
 *
 * It is up to the compiler to decide how it maps Arrow types to the concrete
 * StorageDescriptor types that the GC can understand. This should be
 * transparent to the Arrow user. The FFI will have to expose the
 * StorageDescriptor system, but the StorageDescriptor objects created by the
 * compiler will not be exposed.
 *
 * ---
 *
 * A GCed object can be an arbitrary blob of binary data. What the GC needs to
 * know about it is how large a particular object is and where the references
 * are and their types. This information is encoded in StorageDescriptor
 * objects.
 *
 * In order to support arrays, Arrow objects may optionally contain one (not
 * more than one) array. Arrays in Arrow are always, without exception, preceded
 * by a machine word that tells how many objects the array contains. C++ is more
 * flexible in this regard, since it allows arrays with implicit size,
 * null-terminated arrays etc. I came up with this more limited way of doing
 * things as a compromise between storage flexibility and GC simplicity.
 */
template<typename GCHooks>
class StorageDescriptor {
  typedef StorageDescriptor<GCHooks> SD;

  friend class ::arw::internal::TypeStorage<GCHooks>;

  /**
   * This is a special sentinel value. See the comments on
   * StorageDescriptor::Slot::storage for more information.
   */
  static const SD boxed;

 public:
  /**
   * The Slot class describes one value or one reference in an object; it
   * describes the type of the value or reference, whether it is a value or a
   * reference, and where in the object this value/reference is located.
   *
   * Once created, objects of this class should not be mutated.
   */
  struct Slot {
    friend class ::arw::internal::TypeStorage<GCHooks>;

    /**
     * Constructs a Slot.
     *
     * @param storage_ The storage type. NULL means boxed storage.
     */
    Slot(const SD* storageDescriptor_, HandleType type_, size_t offset_)
        : storageDescriptor(storageDescriptor_ ?: &boxed),
          type(type_),
          offset(offset_) {
      // Boxed slots can't be by value.
      ARW_CHECK(storageDescriptor.get() != &boxed || HandleType::VALUE != type);

      // Value slots can't be variable size.
      ARW_CHECK(HandleType::VALUE != type || !storageDescriptor->hasArray());
    }

    /**
     * If this points to StorageDescriptor::boxed, it means that this is a
     * boxed type: This describes a pointer to a piece of memory where the first
     * machine word is a StorageDescriptor* and the following data is the actual
     * value. This is the mechanism that makes it possible to implement for
     * example existential types.
     *
     * Boxed types must not be by value, but it can be either weak or a strong
     * reference.
     */
    const MemberRef<const SD, GCHooks> storageDescriptor;

    /**
     * Specifies whether the slot is by value, strong reference or weak
     * reference.
     *
     * If VALUE==type, storage->_hasArray must be false.
     *
     * If this is a boxed value (see StorageDescriptor::Slot::storage for more
     * information) this value is ignored.
     */
    HandleType type;

    /**
     * Specifies where in the object this slot is located.
     */
    size_t offset;

    /**
     * empty is an invalid dummy Slot.
     */
    static Slot empty;
  };

  /**
   * Takes an uninitialized block of memory (that must be of the size indicated
   * by the objectSize method) and initializes a StorageDescriptor object there.
   * In this case, we can't use C++'s normal construction mechanism, because
   * StorageDescriptor is a variable-sized object.
   *
   * @param obj Pointer to the unititialized memory.
   * @param hasArray Whether the object should describe a variable size object.
   * @param array Indicates the type of the array part of the object being
   * described. If hasArray is false, this is ignored. In that case, use of
   * Slot::empty is recommended.
   * @param valuesBegin Start input iterator for the list of slots that are in
   * the constant-sized part of the object being described.
   * @param valuesEnd End iterator. See valuesBegin.
   */
  template<typename Iter>
  static SD* init(SD* obj,
                  size_t sizeWithEmptyArray,
                  bool hasArray,
                  const Slot& array,
                  Iter valuesBegin,
                  Iter valuesEnd) {
    ::new(obj) SD(sizeWithEmptyArray,
                  hasArray,
                  array,
                  valuesBegin,
                  valuesEnd);
    return obj;
  }

  /**
   * Helper function that makes it possible to pass an array literal as a
   * parameter instead of a pair of iterators.
   */
  static SD* init(SD* obj,
                  size_t sizeWithEmptyArray,
                  bool hasArray,
                  const Slot& array,
                  const std::vector<Slot>& values) {
    return init(obj,
                sizeWithEmptyArray,
                hasArray,
                array,
                values.begin(),
                values.end());
  }

  StorageDescriptor(const StorageDescriptor&) = delete;
  StorageDescriptor& operator=(const StorageDescriptor&) = delete;

  /**
   * Non-virtual destructor: We don't want a vtable for this class, and it's not
   * supposed to be inherited.
   */
  ~StorageDescriptor() {}

  /**
   * Calculates the size of a StorageDescriptor object with a certain number of
   * value slots. Note that it does not return the size of the object being
   * described by a StorageDescriptor object; it describes the StorageDescriptor
   * itself.
   */
  constexpr static size_t objectSize(size_t numValues) {
    return sizeof(SD)+sizeof(Slot)*numValues;
  }

  /**
   * Returns true if this StorageDescriptor describes variable size objects.
   */
  inline bool hasArray() const {
    return _hasArray;
  }

  /**
   * Returns true if this StorageDescriptor describes box. See the comments on
   * StorageDescriptor::Slot::storage.
   */
  inline bool isBoxed() const {
    return this == &boxed;
  }

  inline Slot &array() {
    return *_array;
  }

 private:
  /**
   * Private constructor. To create StorageDescriptor objects, use init.
   */
  template<typename Iter>
  StorageDescriptor(size_t sizeWithEmptyArray,
                    bool hasArray,
                    const Slot& array,
                    Iter valuesBegin,
                    Iter valuesEnd)
      : _sizeWithEmptyArray(sizeWithEmptyArray),
        _hasArray(hasArray),
        _array(array) {
    size_t numValues = 0;

    for (; valuesBegin != valuesEnd; numValues++, valuesBegin++) {
      ::new(const_cast<Slot*>(_values.get()[numValues])) Slot(*valuesBegin);
    }

    *const_cast<size_t*>(&_numValues) = numValues;

    ARW_ASSERT(sizeWithEmptyArray != 0 ||
               (!hasArray && 0 == numValues));
  }

  /**
   * The size of the object, with an empty array but including the array length
   * word if it's an array object.
   */
  size_t _sizeWithEmptyArray;
  bool _hasArray;
  /**
   * If _hasArray is false, this value is ignored.
   */
  MemberVal<Slot, GCHooks> _array;
  size_t _numValues;
  /**
   * This is a dummy zero-size object. Its actual size depends on _numValues.
   */
  MemberVal<Slot[0], GCHooks> _values;
};

template<typename GCHooks>
const StorageDescriptor<GCHooks>
StorageDescriptor<GCHooks>::boxed(/*sizeWithEmptyArray:*/0,
                                  /*hasArray:*/false,
                                  /*array:*/StorageDescriptor<GCHooks>::Slot(
                                    NULL, HandleType::REFERENCE, 0),
                                  /*valuesBegin:*/reinterpret_cast<Slot*>(0),
                                  /*valuesEnd:*/reinterpret_cast<Slot*>(0));

template<typename GCHooks>
typename StorageDescriptor<GCHooks>::Slot
StorageDescriptor<GCHooks>::Slot::empty(NULL, HandleType::REFERENCE, 0);

}  // namespace arw
