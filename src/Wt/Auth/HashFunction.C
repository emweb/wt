/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "HashFunction.h"
#include "AuthUtils.h"

#include "Wt/Utils.h"
#include "Wt/WException.h"

#ifndef WT_TARGET_JAVA
// for htonl():
#ifndef WT_WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif
#endif

#include <cstring>
#include <cstdio>
#include <stdexcept>

#ifndef WT_TARGET_JAVA
extern "C" {
#include "bcrypt/ow-crypt.h"
#include "sha1.h"
}
#endif

namespace Wt {
  namespace Auth {

HashFunction::~HashFunction()
{ }

bool HashFunction::verify(const std::string& msg,
			  const std::string& salt,
			  const std::string& hash) const
{
  return compute(msg, salt) == hash;
}

#ifndef WT_TARGET_JAVA
std::string MD5HashFunction::compute(const std::string& msg,
				     const std::string& salt) const
{
  return Utils::encodeAscii(Wt::Utils::md5(salt + msg));
}

std::string MD5HashFunction::name() const
{
  return "MD5"; 
}

std::string SHA1HashFunction::compute(const std::string& msg,
				      const std::string& salt) const
{
  SHA1Context sha;

  wt_SHA1Reset(&sha);
  wt_SHA1Input(&sha, (unsigned char *)salt.c_str(), salt.length());
  wt_SHA1Input(&sha, (unsigned char *)msg.c_str(), msg.length());

  if (!wt_SHA1Result(&sha)) {
    throw WException("Could not compute SHA1 hash");
  } else {
    const unsigned SHA1_LENGTH = 20;
    unsigned char hash[SHA1_LENGTH];

    for (unsigned i = 0; i < 5; ++i) {
      unsigned v = htonl(sha.Message_Digest[i]);
      std::memcpy(hash + (i*4), &v, 4);
    }

    return Utils::encodeAscii(std::string(hash, hash + SHA1_LENGTH));
  }
}

std::string SHA1HashFunction::name() const
{
  return "SHA1"; 
}

BCryptHashFunction::BCryptHashFunction(int count)
  : count_(count)
{ }

std::string BCryptHashFunction::compute(const std::string& msg,
					const std::string& salt) const
{
  char setting[32];

  char c_salt[16];
  std::strncpy(c_salt, salt.c_str(), 16);
  if (salt.length() < 16)
    std::memset(c_salt + salt.length(), 'A', 16 - salt.length());

  if (!wt_crypt_gensalt_rn("$2y$", count_, c_salt, 16, setting, 32)) {
    std::perror("crypt_gen_salt_rn");
    throw WException("bcrypt() gensalt internal error");
  } else {
    char result[64];
    if (!wt_crypt_rn(msg.c_str(), setting, result, 64)) {
      std::perror("crypt_rn");
      throw WException("bcrypt() internal error");
    }
    return result;
  }
}

bool BCryptHashFunction::verify(const std::string& msg,
				const std::string& salt,
				const std::string& hash) const
{
  char result[64];

  if (!wt_crypt_rn(msg.c_str(), hash.c_str(), result, 64)) {
    std::perror("crypt_rn");
    throw WException("bcrypt() internal error");
  }

  return result == hash;
}

std::string BCryptHashFunction::name() const
{
  return "bcrypt"; 
}
#endif
	}
}
