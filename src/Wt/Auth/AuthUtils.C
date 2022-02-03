/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <vector>
#include <iterator>
#include <cstring>
#include <memory>

#include "web/WebUtils.h"

#include "Wt/Utils.h"

#include "AuthUtils.h"
#include "base64.h"

#include "Wt/WException.h"
#include "Wt/WRandom.h"
#include "Wt/WServer.h"

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

std::string encodeState(const std::string &secret, const std::string &url)
{
  std::string hash(Wt::Utils::base64Encode(Wt::Utils::hmac_sha1(url, secret)));

  std::string b = Wt::Utils::base64Encode(hash + "|" + url, false);

  /* Variant of base64 encoding which is resistant to broken OAuth2 peers
   * that do not properly re-encode the state */
  b = Wt::Utils::replace(b, "+", "-");
  b = Wt::Utils::replace(b, "/", "_");
  b = Wt::Utils::replace(b, "=", ".");

  return b;
}

std::string decodeState(const std::string &secret, const std::string &state)
{
  std::string s = state;
  s = Wt::Utils::replace(s, "-", "+");
  s = Wt::Utils::replace(s, "_", "/");
  s = Wt::Utils::replace(s, ".", "=");

#ifndef WT_TARGET_JAVA
  s = Wt::Utils::base64Decode(s);
#else
  s = Wt::Utils::base64DecodeS(s);
#endif

  std::size_t i = s.find('|');
  if (i != std::string::npos) {
    std::string url = s.substr(i + 1);

    std::string check = encodeState(secret, url);
    if (check == state)
      return url;
    else
      return std::string();
  } else
    return std::string();
}

std::string configurationProperty(const std::string &prefix,
                                  const std::string &property)
{
  WServer *instance = WServer::instance();

  if (instance) {
    std::string result;

    bool error;
#ifndef WT_TARGET_JAVA
    error = !instance->readConfigurationProperty(property, result);
#else
    std::string* v = instance->readConfigurationProperty(property, result);
      if (v != &result) {
        error = false;
        result = *v;
      } else {
        error = true;
      }
#endif

    if (error)
      throw WException(prefix + ": no '" + property + "' property configured");

    return result;
  } else
    throw WException(prefix + ": could not find a WServer instance");
}

    }
  }
}
