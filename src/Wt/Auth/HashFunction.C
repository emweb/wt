/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "HashFunction"
#include "Utils.h"

#include "Wt/WConfig.h"

#include <string.h> 
#include <stdio.h>
#include <iostream>
#include <stdexcept>

#ifdef WT_WITH_SSL
#include <openssl/evp.h>
#endif // WT_WITH_SSL

extern "C" {
#include "bcrypt/ow-crypt.h"
}

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

std::string MD5HashFunction::compute(const std::string& msg,
				     const std::string& salt) const
{
  return Utils::encodeAscii(Utils::md5(salt + msg));
}

std::string MD5HashFunction::name() const
{
  return "MD5"; 
}

#ifdef WT_WITH_SSL

std::string SHA1HashFunction::compute(const std::string& msg,
				      const std::string& salt) const
{
  EVP_MD_CTX mdctx;
  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, EVP_sha1(), 0);
  EVP_DigestUpdate(&mdctx, salt.c_str(), salt.length());
  EVP_DigestUpdate(&mdctx, msg.c_str(), msg.length());

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned length = EVP_MAX_MD_SIZE;
  EVP_DigestFinal_ex(&mdctx, hash, &length);
  EVP_MD_CTX_cleanup(&mdctx);

  return Utils::encodeAscii(std::string(hash, hash + length));
}

std::string SHA1HashFunction::name() const
{
  return "SHA1"; 
}

#endif // WT_WITH_SSL

BCryptHashFunction::BCryptHashFunction(int count)
  : count_(count)
{ }

std::string BCryptHashFunction::compute(const std::string& msg,
					const std::string& salt) const
{
  char setting[32];

  char c_salt[16];
  strncpy(c_salt, salt.c_str(), 16);
  if (salt.length() < 16)
    memset(c_salt + salt.length(), 'A', 16 - salt.length());

  if (!crypt_gensalt_rn("$2y$", count_, c_salt, 16, setting, 32)) {
    perror("crypt_gen_salt_rn");
    throw std::runtime_error("bcrypt() gensalt internal error");
  } else {
    char result[64];
    if (!crypt_rn(msg.c_str(), setting, result, 64)) {
      perror("crypt_rn");
      throw std::runtime_error("bcrypt() internal error");
    }
    return result;
  }
}

bool BCryptHashFunction::verify(const std::string& msg,
				const std::string& salt,
				const std::string& hash) const
{
  char result[64];

  if (!crypt_rn(msg.c_str(), hash.c_str(), result, 64)) {
    perror("crypt_rn");
    throw std::runtime_error("bcrypt() internal error");
  }

  return result == hash;
}

std::string BCryptHashFunction::name() const
{
  return "bcrypt"; 
}

  }
}
