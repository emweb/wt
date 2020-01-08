/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <vector>
#include <iterator>
#include <cstring>
#include <memory>

#include "AuthUtils.h"
#include "base64.h"

#include "Wt/WRandom.h"

namespace Wt {
  namespace Auth {
    namespace Utils {

std::string createSalt(unsigned int length)
{
  auto saltBuf = std::unique_ptr<unsigned char[]>(new unsigned char[length]);
  for (unsigned i = 0; i < length; i += 3) {
    unsigned r = WRandom::get();
    std::memcpy(saltBuf.get() + i, &r, 3);
  }

  std::string s(saltBuf.get(), saltBuf.get() + length);

  return s;
}

/*
 * This is like base64 encoding except that we use [a-zA-Z0-9./]
 * instead of [a-zA-Z0-9+/]
 */
std::string encodeAscii(const std::string& a)
{
  std::vector<char> v;

  base64::encode(a.begin(), a.end(), std::back_inserter(v));

  std::string result(v.begin(), v.end());

  for (unsigned i = 0; i < result.length(); ++i)
    if (result[i] == '+')
      result[i] = '.';

  return result;
}

std::string decodeAscii(const std::string& a)
{
  std::string msg = a;

  for (unsigned i = 0; i < msg.length(); ++i)
    if (msg[i] == '.')
      msg[i] = '+';

  std::vector<char> v;
  base64::decode(msg.begin(), msg.end(), std::back_inserter(v));

  return std::string(v.begin(), v.end());
}

    }
  }
}
