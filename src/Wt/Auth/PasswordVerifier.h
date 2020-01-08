// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_PASSWORD_VERIFIER
#define WT_AUTH_PASSWORD_VERIFIER

#include <vector>

#include <Wt/WString.h>
#include <Wt/Auth/PasswordService.h>

namespace Wt {
  namespace Auth {

class HashFunction;
class PasswordHash;

/*! \class PasswordVerifier Wt/Auth/PasswordVerifier.h Wt/Auth/PasswordVerifier.h
 *  \brief Password hash computation and verification class
 *
 * This class implements the logic for comparing passwords against
 * password hashes, or computing a new password hash for a password.
 *
 * One or more hash functions can be added, which allow you to
 * introduce a new "preferred" hash function while maintaining support
 * for verifying existing passwords hashes.
 *
 * \ingroup auth
 */
class WT_API PasswordVerifier : public PasswordService::AbstractVerifier
{
public:
  /*! \brief Constructor.
   */
  PasswordVerifier();

  PasswordVerifier(const PasswordVerifier &) = delete;
  PasswordVerifier &operator=(const PasswordVerifier &) = delete;

  /*! \brief Destructor.
   */
  virtual ~PasswordVerifier();

  /*! \brief Sets the salt length.
   *
   * The salt length is used to create new salt when a new password is
   * being hashed.
   *
   * The salt length is specified in bytes, but should be a multiple
   * of 3 (so that Base64 encoding yields an integral number of bytes).
   *
   * The default length is 12.
   *
   * \sa hashPassword()
   */
  void setSaltLength(int words);

  /*! \brief Returns the salt length.
   */
  int saltLength() const;

  /*! \brief Adds a hash function.
   *
   * The first hash function added is the one that will be used for
   * creating new password hashes, i.e. the "preferred" hash
   * function. The other hash functions are used only for verifying
   * existing hash passwords. This allows you to move to new hash
   * functions as other ones are no longer deemed secure.
   *
   * Each hash function has a unique name, which is annotated in the
   * generated hash to identify the appropriate hash funtion to
   * evaluate it.
   *
   * Ownership of the hash functions is transferred.
   *
   * \sa hashFunctions()
   */
  void addHashFunction(std::unique_ptr<HashFunction> function);

  /*! \brief Returns the list of hash functions.
   *
   * This returns a list with references to hashfunctions that have
   * been added with addHashFunction().
   */
  const std::vector<HashFunction *> hashFunctions() const;

  virtual bool needsUpdate(const PasswordHash& hash) const override;

  /*! \brief Computes the password hash for a clear text password.
   *
   * This creates new salt and applies the "preferred" hash function to
   * the salt and clear text password to compute the hash.
   *
   * \sa verify()
   */
  virtual PasswordHash hashPassword(const WString& password) const override;

  /*! \brief Verifies a password against a hash.
   *
   * This verifies whether the password matches the hash.
   *
   * \sa hashPassword()
   */
  virtual bool verify(const WString& password, const PasswordHash& hash) const override;

private:
  std::vector<std::unique_ptr<HashFunction> > hashFunctions_;
  int saltLength_;
};

  } 
}

#endif // WT_AUTH_PASSWORD_VERIFIER
