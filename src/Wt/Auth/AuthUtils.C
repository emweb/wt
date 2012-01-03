/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <vector>
#include <iterator>

#include "AuthUtils.h"
#include "base64.h"

namespace Wt {
  namespace Auth {
    namespace Utils {

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
