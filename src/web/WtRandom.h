// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 * $Id: WtRandom.h,v 1.7 2008/07/15 06:25:58 kdf Exp $
 */

#ifndef WTRANDOM_H_
#define WTRANDOM_H_

#include <Wt/WDllDefs.h>
#include <stdlib.h>

namespace Wt {

class WtRandom_Private;

class WT_API WtRandom
{
public:
  /* Create and seed a new random number generator. */
  WtRandom();

  ~WtRandom();

  /* Get a next random number */
  unsigned int rand();

  /* Get a next random number, static version*/
  static unsigned int getUnsigned();

  /* Get a next random number, static version*/
  static double getDouble();

private:
  class Private;
  WtRandom::Private *_p;
};

}

#endif // IFNDEF WTRANDOM_H_
