/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <string>

#include "Wt/Exception/WInvalidFormatException.h"
#include "Wt/Exception/WInvalidOperationException.h"

#include <Wt/WMessageResources.h>

#define kMinInputLength 10
#define kMaxInputLength 5120

namespace {
  int eval(std::string expression, ::uint64_t n)
  {
    try {
      return Wt::WMessageResources::evalPluralCase(expression, n);
    } catch (Wt::WInvalidFormatException& ife) {
    } catch (Wt::WInvalidOperationException& ioe) {
    }
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {

    if (Size < kMinInputLength || Size > kMaxInputLength) {
        return 1;
    }

    std::string input(Data, Data + Size);
    eval(input, 0);

    return 0;
}
