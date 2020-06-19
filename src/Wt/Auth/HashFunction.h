// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_HASH_FUNCTION
#define WT_AUTH_HASH_FUNCTION

#include <string>
#include <Wt/WDllDefs.h>

namespace Wt {
  namespace Auth {

/*! \class HashFunction Wt/Auth/HashFunction.h
 *  \brief An abstract cryptographic hash function interface.
 *
 * A cryptographic hash function computes a hash value from a message,
 * for which it is hard to guess another message that generates the
 * same hash.
 *
 * These hash functions are intended for short messages, typically
 * passwords or random tokens, and thus not suitable for computing the
 * hash value of a large document.
 *
 * When used for passwords, to avoid dictionary attacks, the hash
 * functions accept also a random salt which is hashed together with
 * the password. Not all hash functions are adequate for passwords
 * hashes.
 *
 * \ingroup auth
 */
class WT_API HashFunction
{
public:
  /*! \brief Destructor.
   */
  virtual ~HashFunction();

  /*! \brief Returns the name for this hash function.
   *
   * This should return a (short) name that uniquely identifies this
   * hash function.
   */
  virtual std::string name() const = 0;

  /*! \brief Computes the hash of a message + salt.
   *
   * The message is usually an ASCII or UTF-8 string.
   *
   * The \p salt and the computed hash are encoded in printable
   * characters. This is usually ASCII-encoded \if cpp (as for the UNIX crypt()
   * functions) \endif or could be Base64-encoded.
   */
  virtual std::string compute(const std::string& msg, const std::string& salt)
    const = 0;

  /*! \brief Verifies a message with the salted hash
   *
   * The base implementation will recompute the hash of the message with
   * the given salt, and compare it to the \p hash.
   *
   * Some methods however store the salt and additional settings in
   * the \p hash, and this information is thus needed to verify the message
   * hash.
   */
  virtual bool verify(const std::string& msg,
		      const std::string& salt,
		      const std::string& hash) const;
};

/*! \class MD5HashFunction Wt/Auth/HashFunction.h
 *  \brief A cryptograhpic hash function implemented using MD5.
 *
 * This hash function is useful for creating token hashes, but
 * should not be used for password hashes.
 *
 * \ingroup auth
 */
class WT_API MD5HashFunction : public HashFunction
{
public:
  /*! \brief Returns the name for this hash function.
   *
   * Returns <tt>"MD5"</tt>.
   */
  virtual std::string name() const override;

  virtual std::string compute(const std::string& msg,
			      const std::string& salt) const override;
};

#ifndef WT_TARGET_JAVA
/*! \class SHA1HashFunction Wt/Auth/HashFunction.h
 *  \brief A cryptographic hash function implemented using SHA1.
 *
 * This hash function is only available if %Wt was compiled with
 * OpenSSL support.
 *
 * This hash function is useful for creating token hashes, but
 * should not be used for password hashes.
 *
 * \ingroup auth
 */
class WT_API SHA1HashFunction final : public HashFunction
{
public:
  /*! \brief Returns the name for this hash function.
   *
   * Returns <tt>"SHA1"</tt>.
   */
  virtual std::string name() const override;

  virtual std::string compute(const std::string& msg,
			      const std::string& salt) const override;
};
#endif

/*! \class BCryptHashFunction Wt/Auth/HashFunction.h
 *  \brief An cryptographic hash function that implements bcrypt.
 *
 * This hashing function is intended for password hashes. In addition
 * to be collision-resistant, the bcrypt algorithm has a parameter
 * which makes the computation more computationally intensive. In this
 * way, a dictionary-based attack on a compromised hash is also less
 * feasible.
 *
 * \ingroup auth
 */
class WT_API BCryptHashFunction final : public HashFunction
{
public:
  /*! \brief Constructor.
   *
   * The \p count parameter controls the number of iterations, and
   * thus the computational complexity. With a value of 0, a small
   * default is chosen. The computational complexity increases
   * exponentionally with increasing values of \p count. The parameter
   * only affects new hashes computed with compute(), and its value is
   * stored in the computed hash.
   *
   * The value of \p count needs to be 0, or in the range 4-31.
   */
  BCryptHashFunction(int count = 0);

  /*! \brief Returns the name for this hash function.
   *
   * Returns <tt>"bcrypt"</tt>.
   */
  virtual std::string name() const override;

  virtual std::string compute(const std::string& msg,
			      const std::string& salt) const override;

  virtual bool verify(const std::string& msg,
		      const std::string& salt,
		      const std::string& hash) const override;

private:
  int count_;
};

  } 
}

#endif // WT_AUTH_HASH_FUNCTION
