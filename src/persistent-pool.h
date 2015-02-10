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

#include <map>
#include <iterator>
#include <utility>

#include "checks.h"

namespace arw {

/**
 * In order to be able to implement persistent (i.e. long-lived) handles, the
 * garbage collector needs to be able to access a root set containing all the
 * references (along with their Storage descriptor) that are currently pointed
 * to. PersistentPool is that root set.
 *
 * Persistent handles will add and remove themselves to this pool, and the
 * garbage collector will inspect it.
 *
 * This class is intended to have one instance per thread, and be accessible
 * through thread-local storage. It is not thread safe.
 *
 * Since there can be multiple references to the same object, PersistentPool
 * keeps track of a reference count for each object. Be aware that with this
 * comes all the usual circular reference hazards that come with reference
 * counting. One way to avoid that is to use the storage system and use member
 * handles instead of persistent handles.
 */
template<typename Descriptor, typename Data>
class PersistentPool {
  /**
   * The size_t is a refcount.
   */
  typedef std::map<Data, std::pair<Descriptor, size_t> > Map;

 public:
  typedef std::pair<Descriptor, Data> value_type;

  /**
   * Input iterator class for the persistent pool.
   */
  class const_iterator : public std::iterator<std::input_iterator_tag,
                                              value_type> {
    friend class PersistentPool;

   public:
    const_iterator operator++(int) {
      const_iterator mycopy(*this);
      ++_i;
      update();
      return mycopy;
    }

    const_iterator& operator++() {
      ++_i;
      update();
      return *this;
    }

    const value_type& operator*() const {
      return _v;
    }

    const value_type* operator->() const {
      return &_v;
    }

    bool operator==(const const_iterator& other) {
      return _i == other._i;
    }

    bool operator!=(const const_iterator& other) {
      return _i != other._i;
    }

   private:
    explicit const_iterator(typename Map::const_iterator i) :
      _i(i) {
      update();
    }

    void update() {
      _v = std::make_pair((*_i).second.first, (*_i).first);
    }

    typename Map::const_iterator _i;
    value_type _v;
  };

  /**
   * Construct an empty persistent pool.
   */
  PersistentPool() {}

  PersistentPool(const PersistentPool&) = delete;
  PersistentPool& operator=(const PersistentPool&) = delete;

  /**
   * Adds a reference with a particular Storage descriptor to the pool.
   *
   * It is an error to add the same reference twice with a different Storage
   * descriptor.
   */
  void add(const Data& data, const Descriptor& desc) {
    // Refcount of 1
    auto i = _map.insert(std::make_pair(data, std::make_pair(desc, 1)));
    if (!i.second) {
      // We already have this data in the map. Increment the refcount.
      ARW_CHECK((*i.first).second.first == desc);
      (*i.first).second.second++;
    }
  }

  /**
   * Removes a reference from the pool.
   *
   * It is an error to remove a reference that hasn't already been added.
   */
  void remove(const Data& data) {
    auto i = _map.find(data);
    ARW_CHECK(i != _map.end());

    auto& descAndRefcount = (*i).second;
    descAndRefcount.second--;  // Decrement refcount
    if (0 == descAndRefcount.second) {
      // Refcount is now zero. Remove from _map
      _map.erase(i);
    }
  }

  /**
   * Returns a const_iterator pointing to the beginning of the pool.
   */
  const_iterator begin() const {
    return const_iterator(_map.begin());
  }

  /**
   * Returns a const_iterator pointing to the end of the pool.
   */
  const_iterator end() const {
    return const_iterator(_map.end());
  }

  /**
   * Returns true if the pool is empty.
   */
  bool empty() const {
    return _map.empty();
  }

 private:
  Map _map;
};

}  // namespace arw
