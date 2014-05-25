#pragma once

#ifndef __ASYNC_FOREVER_ITERATOR_HPP__
#define __ASYNC_FOREVER_ITERATOR__

#include <cmath>
#include <iterator>

namespace async {

/**
   This is an iterator that never terminates.
*/
class ForeverIterator : public std::iterator<std::forward_iterator_tag, float> {
public:
  ForeverIterator() {};
  ForeverIterator(const ForeverIterator& iter) {};
  ForeverIterator& operator++() { return *this; }
  ForeverIterator operator++(int) { return *this; }
  float operator+(const ForeverIterator &c1) { return NAN; }
  float operator-(const ForeverIterator &c1) { return NAN; }
  bool operator<(const ForeverIterator &c1) { return false; }
  bool operator>(const ForeverIterator &c1) { return false; }

  bool operator==(const ForeverIterator& rhs) { return false; }
  bool operator!=(const ForeverIterator& rhs) { return true; }
};



}

#endif
