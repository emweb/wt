/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRandom.h"

#include <Wt/WGlobal.h>

#include <random>

namespace {

std::random_device &rnd() {
#ifdef WT_THREADED
  thread_local
#endif // WT_THREADED
      static std::random_device device;
  return device;
}

template<class Int1, class Int2>
constexpr Int1 int_pow(Int1 n, Int2 exp)
{
  return exp == 0 ? 1 : n * int_pow(n, exp - 1);
}

}

namespace Wt {
  
unsigned int WRandom::get()
{
  return rnd()();
}

std::string WRandom::generateId(int length)
{
  static const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  constexpr const std::int32_t n_chars = sizeof(chars) / sizeof(char) - 1;
  constexpr const int MAX_CHARS_AT_A_TIME = 5;

  static_assert(n_chars == 10 + 26 + 26, "There are 62 alphanumeric characters (case sensitive)");

  static_assert(int_pow(static_cast<std::int64_t>(n_chars), MAX_CHARS_AT_A_TIME) <= std::numeric_limits<std::int32_t>::max(),
                "You should be able to get 5 characters out of one random 32 bit integer");

  std::uniform_int_distribution<std::int32_t> dis(0, int_pow(n_chars, MAX_CHARS_AT_A_TIME));

  std::string result;
  result.reserve(static_cast<std::size_t>(length));

  int i = 0;
  while (i < length) {
    int n = dis(rnd());
    for (int j = 0; i < length && j < MAX_CHARS_AT_A_TIME; ++i, ++j) {
      result.push_back(chars[n % n_chars]);
      n /= n_chars;
    }
  }

  return result;
}

}
