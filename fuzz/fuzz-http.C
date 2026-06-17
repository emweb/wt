/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <vector>

#include "http/RequestParser.h"
#include "http/Request.h"

#define kMinInputLength 10
#define kMaxInputLength 5120

// Fuzzes the httpd request line/header parser; parse() and validate() do not
// use the Server, so it is driven with a null one.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {

    if (Size < kMinInputLength || Size > kMaxInputLength) {
        return 1;
    }

    std::vector<char> buf(Data, Data + Size);

    http::server::RequestParser parser(nullptr);
    http::server::Request req;

    char *begin = &buf[0];
    char *end = begin + buf.size();

    boost::tribool result;
    char *next;
    boost::tie(result, next) = parser.parse(req, begin, end);

    if (result) {
        parser.validate(req);
    }

    return 0;
}
