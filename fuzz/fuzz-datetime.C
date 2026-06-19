/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <string>

#include "Wt/WString.h"
#include "Wt/WDate.h"
#include "Wt/WTime.h"
#include "Wt/WDateTime.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if (size < 1)
    return 0;

  // First byte splits the remaining bytes into a format string and a value
  // string, so both the format parser and the value parser get fuzzed.
  std::size_t formatLen = data[0] % size;
  std::string format(reinterpret_cast<const char*>(data + 1), formatLen);
  std::string value(reinterpret_cast<const char*>(data + 1 + formatLen),
                    size - 1 - formatLen);

  Wt::WString f = Wt::WString::fromUTF8(format);
  Wt::WString v = Wt::WString::fromUTF8(value);

  try { Wt::WDateTime::fromString(v, f); } catch (...) {}
  try { Wt::WDate::fromString(v, f); } catch (...) {}
  try { Wt::WTime::fromString(v, f); } catch (...) {}
  try { Wt::WDateTime::fromString(v); } catch (...) {}

  return 0;
}
