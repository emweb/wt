#include "gason.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int parsed;
static int failed;

void parse(const char *csource, bool ok) {
    char *source = strdup(csource);
    char *endptr;
    JsonValue value;
    JsonAllocator allocator;
    int result = jsonParse(source, &endptr, &value, allocator);
    if (ok && result) {
        fprintf(stderr, "FAILED %d: %s\n%s\n%*s - \\x%02X\n", parsed, jsonStrError(result), csource, (int)(endptr - source + 1), "^", *endptr);
        ++failed;
    }
    if (!ok && !result) {
        fprintf(stderr, "PASSED %d:\n%s\n", parsed, csource);
        ++failed;
    }
    ++parsed;
    free(source);
}

#define pass(csource) parse(csource, true)
#define fail(csource) parse(csource, false)

int main() {
      pass(u8R"json(1234567890)json");
      pass(u8R"json(1e-21474836311)json");
      pass(u8R"json(1e-42147483631)json");
      pass(u8R"json("A JSON payload should be an object or array, not a string.")json");
      fail(u8R"json(["Unclosed array")json");
      fail(u8R"json({unquoted_key: "keys must be quoted"})json");
      pass(u8R"json(["extra comma",])json");
      fail(u8R"json(["double extra comma",,])json");
      fail(u8R"json([   , "<-- missing value"])json");
      fail(u8R"json([ 1 [   , "<-- missing inner value 1"]])json");
      fail(u8R"json({ "1" [   , "<-- missing inner value 2"]})json");
      fail(u8R"json([ "1" {   , "<-- missing inner value 3":"x"}])json");
      pass(u8R"json(["Comma after the close"],)json");
      pass(u8R"json({"Extra comma": true,})json");
      pass(u8R"json({"Extra value after close": true} "misplaced quoted value")json");
      fail(u8R"json({"Illegal expression": 1 + 2})json");
      fail(u8R"json({"Illegal invocation": alert()})json");
      pass(u8R"json({"Numbers cannot have leading zeroes": 013})json");
      fail(u8R"json({"Numbers cannot be hex": 0x14})json");
      fail(u8R"json(["Illegal backslash escape: \x15"])json");
      fail(u8R"json([\naked])json");
      fail(u8R"json(["Illegal backslash escape: \017"])json");
      fail(u8R"json([[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["Too deep"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]])json");
      pass(u8R"json({"Missing colon" null})json");
      fail(u8R"json({"Unfinished object"})json");
      fail(u8R"json({"Unfinished object 2" null "x"})json");
      fail(u8R"json({"Double colon":: null})json");
      fail(u8R"json({"Comma instead of colon", null})json");
      fail(u8R"json(["Colon instead of comma": false])json");
      fail(u8R"json(["Bad value", truth])json");
      fail(u8R"json(['single quote'])json");
      fail(u8R"json(["	tab	character	in	string	"])json");
      fail(u8R"json(["line
break"])json");
      fail(u8R"json(["line\
break"])json");
      pass(u8R"json([0e])json");
      pass(u8R"json([0e+])json");
      fail(u8R"json([0e+-1])json");
      fail(u8R"json({"Comma instead if closing brace": true,)json");
      fail(u8R"json(["mismatch"})json");
      pass(u8R"json([[[[[[[[[[[[[[[[[[["Not too deep"]]]]]]]]]]]]]]]]]]])json");
      pass(u8R"json([1, 2, "хУй", [[0.5], 7.11, 13.19e+1], "ba\u0020r", [ [ ] ], -0, -.666, [true, null], {"WAT?!": false}])json");
      pass(u8R"json({
    "JSON Test Pattern pass3": {
        "The outermost value": "must be an object or array.",
        "In this test": "It is an object."
    }
}
)json");
      pass(u8R"json([
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&json()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /json <!-- --",
        "# -- --> json/": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&json()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"])json");

    if (failed)
        fprintf(stderr, "%d/%d TESTS FAILED\n", failed, parsed);
    else
        fprintf(stderr, "ALL TESTS PASSED\n");

    return 0;
}
