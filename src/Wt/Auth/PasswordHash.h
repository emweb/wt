// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_PASSWORD_HASH_H_
#define WT_AUTH_PASSWORD_HASH_H_

#include <string>
#include <Wt/WDllDefs.h>

namespace Wt {
  namespace Auth {

/*! \class PasswordHash Wt/Auth/PasswordHash.h
 *  \brief A password hash.
 *
 * This combines the information for interpreting a hashed password:
 * - the hash value
 * - the salt used
 * - the hashing function used
 *
 * \sa HashFunction::compute()
 *
 * \ingroup auth
 */
class WT_API PasswordHash
{
public:
  /*! \brief Default constructor.
   *
   * Creates an empty password hash, i.e. with empty function, salt and value.
   */
  PasswordHash();

  /*! \brief Constructor.
   */
  PasswordHash(const std::string& function, const std::string& salt,
	       const std::string& value);

  /*! \brief Returns whether the password is empty.
   */
  bool empty() const { return value_.empty(); }

  /*! \brief Returns the function identifier.
   */
  std::string function() const { return function_; }

  /*! \brief Returns the salt.
   */
  std::string salt() const { return salt_; }

  /*! \brief Returns the hash value.
   */
  std::string value() const { return value_; }

private:
  std::string function_, salt_, value_;
};

  }
}

#endif // WT_AUTH_PASSWORD_HASH_H_
