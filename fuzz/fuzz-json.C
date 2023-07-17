/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <string>

#include <Wt/Json/Parser.h>
#include <Wt/Json/Object.h>

#define kMinInputLength 10
#define kMaxInputLength 5120

using namespace Wt;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {

    if (Size < kMinInputLength || Size > kMaxInputLength) {
        return 1;
    }

    std::string data(Data, Data + Size);

    try {
        Json::Object initial;
        Json::parse(data, initial);
    }catch( ... ){ /* ... */}

    try {
        Json::Value initial;
        Json::parse(data, initial);
    }catch( ... ){ /* ... */}

    return 0;
}
