#pragma once

#ifndef ASYNC_FOREVER_ITERATOR_HPP
#define ASYNC_FOREVER_ITERATOR

#include <cmath>
#include <iterator>

namespace async {

static int forever_iterator_dummy = 0;

/**
   This is an iterator that never terminates.
*/
class ForeverIterator : public std::iterator<std::forward_iterator_tag, int> {
public:
  ForeverIterator() {};
  ForeverIterator(const ForeverIterator& iter) {};
  ForeverIterator& operator++() { return *this; }
  ForeverIterator operator++(int) { return *this; }
  int& operator*() { return forever_iterator_dummy; }
  bool operator==(const ForeverIterator& rhs) { return false; }
  bool operator!=(const ForeverIterator& rhs) { return true; }
};

// Forever iterators have no real state, so we can use the same instance everywhere.
static ForeverIterator ForeverIteratorInstance;


}

#endif
