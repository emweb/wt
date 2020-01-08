/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Utils.h>
#include <iostream>

const int WrapSize = 76;

std::string loremIpsumText()
{
  return 
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.\n";
}

std::string loremIpsumBase64(bool wrap)
{
  std::string base64 =  
    "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2ljaW5nIGVsaX"
    "QsIHNlZCBkbyBlaXVzbW9kIHRlbXBvciBpbmNpZGlkdW50IHV0IGxhYm9yZSBldCBkb2xvcmUg"
    "bWFnbmEgYWxpcXVhLiBVdCBlbmltIGFkIG1pbmltIHZlbmlhbSwgcXVpcyBub3N0cnVkIGV4ZX"
    "JjaXRhdGlvbiB1bGxhbWNvIGxhYm9yaXMgbmlzaSB1dCBhbGlxdWlwIGV4IGVhIGNvbW1vZG8g"
    "Y29uc2VxdWF0LiBEdWlzIGF1dGUgaXJ1cmUgZG9sb3IgaW4gcmVwcmVoZW5kZXJpdCBpbiB2b2"
    "x1cHRhdGUgdmVsaXQgZXNzZSBjaWxsdW0gZG9sb3JlIGV1IGZ1Z2lhdCBudWxsYSBwYXJpYXR1"
    "ci4gRXhjZXB0ZXVyIHNpbnQgb2NjYWVjYXQgY3VwaWRhdGF0IG5vbiBwcm9pZGVudCwgc3VudC"
    "BpbiBjdWxwYSBxdWkgb2ZmaWNpYSBkZXNlcnVudCBtb2xsaXQgYW5pbSBpZCBlc3QgbGFib3J1"
    "bS4K";

  if (wrap) {
    std::stringstream ss;
    for (unsigned i = 0; i < base64.size(); ++i) {
      ss << base64[i];
      if ((i + 1 ) % WrapSize == 0)
	ss << (char)13 << (char)10;
    }
    return ss.str();
  } else {
    return base64;
  }
}

BOOST_AUTO_TEST_CASE( Base64_test1 )
{
  std::string original = loremIpsumText();
  std::string reference = loremIpsumBase64(true);

  std::string encoded = Wt::Utils::base64Encode(original);
  BOOST_REQUIRE(reference == encoded);

  std::string decoded = Wt::Utils::base64Decode(encoded);
  BOOST_REQUIRE(original == decoded);
}

BOOST_AUTO_TEST_CASE( Base64_test2 )
{
  std::string original = loremIpsumText();
  std::string reference = loremIpsumBase64(false);

  std::string encoded = Wt::Utils::base64Encode(original, false);
  BOOST_REQUIRE(reference == encoded);

  std::string decoded = Wt::Utils::base64Decode(encoded);
  BOOST_REQUIRE(original == decoded);
}
