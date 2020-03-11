/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cstring>
#include <stdio.h>
#include <algorithm>

#include "StringStream.h"

namespace Wt {
  namespace Dbo {
    namespace Utils {

char *itoa(int value, char *result, int base = 10) {
  char* out = result;
  int quotient = value;

  if (quotient < 0)
    quotient = -quotient;

  do {
    *out =
      "0123456789abcdefghijklmnopqrstuvwxyz"[quotient % base];
    ++out;
    quotient /= base;
  } while (quotient);

  if (value < 0 && base == 10)
    *out++ = '-';

  std::reverse(result, out);
  *out = 0;

  return result;
}

char *lltoa(long long value, char *result, int base = 10) {
  char* out = result;
  long long quotient = value;

  if (quotient < 0)
    quotient = -quotient;

  do {
    *out =
      "0123456789abcdefghijklmnopqrstuvwxyz"[ quotient % base ];
    ++out;
    quotient /= base;
  } while (quotient);

  if (value < 0 && base == 10)
    *out++ = '-';
  std::reverse(result, out);
  *out = 0;

  return result;
}

    } // namespace Utils
  }
}

#define WT_DBO_STRINGSTREAM
#include "../WStringStream.C"
#undef WT_DBO_STRINGSTREAM
