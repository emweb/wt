/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <string>

#include "web/WebUtils.h"
#include "3rdparty/rapidxml/rapidxml.hpp"
#include "3rdparty/rapidxml/rapidxml_print.hpp"
#include <vector>

#define kMinInputLength 10
#define kMaxInputLength 5120

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {

    if (Size < kMinInputLength || Size > kMaxInputLength) {
        return 1;
    }

    std::string data(Data, Data + Size);
    std::vector<char> text(data.begin(), data.end());
    text.push_back('\0');

    try {
        Wt::rapidxml::xml_document<> doc;
        doc.parse<0>(&text[0]);
        Wt::Utils::fixSelfClosingTags(&doc);
    } catch( ... ) {/*...*/}

    return 0;
}
