/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <string>

#include <Wt/Render/CssParser.h>
#include <iostream>


#define kMinInputLength 10
#define kMaxInputLength 5120


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {

    if (Size < kMinInputLength || Size > kMaxInputLength) {
        return 1;
    }

    std::string input(Data, Data + Size);
    Wt::Render::CssParser parser;
    parser.parse(input);

    return 0;
}
