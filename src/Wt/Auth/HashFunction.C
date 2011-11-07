/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "HashFunction"
#include "Utils.h"

#include "Wt/WConfig.h"

#ifdef WT_WITH_SSL
#include <openssl/evp.h>
#endif // WT_WITH_SSL

namespace Wt {
  namespace Auth {

HashFunction::~HashFunction()
{ }

#ifdef WT_WITH_SSL

std::string SHA1HashFunction::compute(const std::string& msg,
				      const std::string& salt) const
{
  std::string binSalt = Utils::decodeAscii(salt);

  EVP_MD_CTX mdctx;
  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, EVP_sha1(), 0);
  EVP_DigestUpdate(&mdctx, binSalt.c_str(), binSalt.length());
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

  }
}
