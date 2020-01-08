// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WRANDOM_H_
#define WRANDOM_H_

#include <Wt/WDllDefs.h>
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
};

}

#endif // IFNDEF WTRANDOM_H_
