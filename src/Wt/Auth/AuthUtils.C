/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <vector>
#include <iterator>

#include <string.h>

// for htonl():
#ifndef WIN32
#include <arpa/inet.h>
#else
#include <Winsock2.h>
#endif

#include "Wt/WLogger"
#include "AuthUtils.h"
#include "md5.h"
#include "base64.h"

extern "C" {
  #include "sha1.h"
}

namespace Wt {

LOGGER("Auth::Utils");

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

std::string md5(const std::string& a)
{
  md5_state_t c;
  md5_init(&c);

  md5_append(&c, (const md5_byte_t *)a.c_str(), a.length());

  unsigned char buf[16];
  md5_finish(&c, buf);

  return std::string((const char *)buf, 16);
}

std::string sha1(const std::string& a)
{
  SHA1Context sha;

  SHA1Reset(&sha);
  SHA1Input(&sha, (unsigned char *)a.c_str(), a.length());

  if (!SHA1Result(&sha)) {
    LOG_ERROR("Error computing sha1 hash");
    return std::string();
  } else {
    const unsigned SHA1_LENGTH = 20;
    unsigned char hash[SHA1_LENGTH];

    for (unsigned i = 0; i < 5; ++i) {
      unsigned v = htonl(sha.Message_Digest[i]);
      memcpy(hash + (i*4), &v, 4);
    }

    return std::string(hash, hash + SHA1_LENGTH);
  }
}

    }
  }
}
