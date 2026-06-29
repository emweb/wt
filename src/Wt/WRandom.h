// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WRANDOM_H_
#define WRANDOM_H_

#include <Wt/WDllDefs.h>
#include <functional>
#include <string>

namespace Wt {

/*! \class WRandom Wt/WRandom.h Wt/WRandom.h
 *  \brief Random number generator.
 *
 * If an implementation is available for your OS, this class generates
 * high-entropy random numbers, suitable for secret ids (e.g. this is
 * used to generate %Wt's session IDs).
 */
class WT_API WRandom
{
public:
  /*! \brief Returns a random number.
   *
   * This returns a high-entropy (non deterministic) random number on
   * platforms for which this is supported (currently only Linux,
   * Windows and MacOS X).
   */
  static unsigned int get();

  /*! \brief A utility method to generate a random id.
   *
   * The id is composed of small and capitalized roman characters and
   * numbers [a-zA-Z0-9].
   *
   * \sa get()
   */
  static std::string generateId(int length = 16);

#ifndef WT_TARGET_JAVA
  /*! \brief Sets the random number generator used by WRandom.
   *
   * Sets the random number generator used by WRandom::get().
   *
   * The generator should be thread-safe, as it may be called from
   * multiple threads.
   *
   * \note This function is not thread safe, and should not be called
   *       concurrently with any function of WRandom.
   *
   * \warning Several security features of %Wt depend on the quality of
   *          the random number generator. Replacing it with a
   *          low-entropy or deterministic generator will compromise
   *          security.
   */
  static void setGenerator(const std::function<unsigned int()>& generator) { generate_ = generator; }

  /*! \brief Sets the uniform random number generator used by WRandom.
   *
   * Sets the uniform random number generator used by
   * WRandom::generateId(). The generator should return a uniformly
   * distributed random number in the range [ \p min, \p max ] where
   * \p min and \p max are given as parameters to the generator
   * function.
   *
   * The generator should also be thread-safe, as it may be called from
   * multiple threads.
   *
   * \note This function is not thread safe, and should not be called
   *       concurrently with any function of WRandom.
   *
   * \warning Several security features of %Wt depend on the quality of
   *          the random number generator. Replacing it with a
   *          low-entropy, non-uniform or deterministic generator will
   *          compromise security.
   */
  static void setUniformGenerator(const std::function<int(::int32_t min, ::int32_t max)>& generator) { generateUniform_ = generator; }

private:
  static std::function<unsigned int()> generate_;
  static std::function<int(::int32_t min, ::int32_t max)> generateUniform_;
#endif // WT_TARGET_JAVA

};

}

#endif // IFNDEF WTRANDOM_H_
