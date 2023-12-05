/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Utils.h>

namespace {
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

  std::string loremIpsumBase32(bool wrap)
  {
    std::string base32 =
      "JRXXEZLNEBUXA43VNUQGI33MN5ZCA43JOQQGC3LFOQWCAY3PNZZWKY3UMV2HK4RAMFSGS4DJONUW"
      "G2LOM4QGK3DJOQWCA43FMQQGI3ZAMVUXK43NN5SCA5DFNVYG64RANFXGG2LENFSHK3TUEB2XIIDM"
      "MFRG64TFEBSXIIDEN5WG64TFEBWWCZ3OMEQGC3DJOF2WCLRAKV2CAZLONFWSAYLEEBWWS3TJNUQH"
      "MZLONFQW2LBAOF2WS4ZANZXXG5DSOVSCAZLYMVZGG2LUMF2GS33OEB2WY3DBNVRW6IDMMFRG64TJ"
      "OMQG42LTNEQHK5BAMFWGS4LVNFYCAZLYEBSWCIDDN5WW233EN4QGG33OONSXC5LBOQXCARDVNFZS"
      "AYLVORSSA2LSOVZGKIDEN5WG64RANFXCA4TFOBZGK2DFNZSGK4TJOQQGS3RAOZXWY5LQORQXIZJA"
      "OZSWY2LUEBSXG43FEBRWS3DMOVWSAZDPNRXXEZJAMV2SAZTVM5UWC5BANZ2WY3DBEBYGC4TJMF2H"
      "K4ROEBCXQY3FOB2GK5LSEBZWS3TUEBXWGY3BMVRWC5BAMN2XA2LEMF2GC5BANZXW4IDQOJXWSZDF"
      "NZ2CYIDTOVXHIIDJNYQGG5LMOBQSA4LVNEQG6ZTGNFRWSYJAMRSXGZLSOVXHIIDNN5WGY2LUEBQW"
      "42LNEBUWIIDFON2CA3DBMJXXE5LNFYFA====";

    if (wrap) {
      std::stringstream ss;
      for (unsigned i = 0; i < base32.size(); ++i) {
        ss << base32[i];
        if ((i + 1 ) % WrapSize == 0)
          ss << (char)13 << (char)10;
      }
      return ss.str();
    } else {
      return base32;
    }
  }
}

BOOST_AUTO_TEST_CASE( Base32_test1 )
{
  std::string original = loremIpsumText();
  std::string reference = loremIpsumBase32(true);

  std::string encoded = Wt::Utils::base32Encode(original);
  BOOST_REQUIRE(reference == encoded);

  std::string decoded = Wt::Utils::base32Decode(encoded);
  BOOST_REQUIRE(original == decoded);
}

BOOST_AUTO_TEST_CASE( Base32_test2 )
{
  std::string original = loremIpsumText();
  std::string reference = loremIpsumBase32(false);

  std::string encoded = Wt::Utils::base32Encode(original, false);
  BOOST_REQUIRE(reference == encoded);

  std::string decoded = Wt::Utils::base32Decode(encoded);
  BOOST_REQUIRE(original == decoded);
}

BOOST_AUTO_TEST_CASE( Base32_rfc4648_encode )
{
  BOOST_REQUIRE(Wt::Utils::base32Encode("foo") == "MZXW6===");
  BOOST_REQUIRE(Wt::Utils::base32Encode("foob") == "MZXW6YQ=");
  BOOST_REQUIRE(Wt::Utils::base32Encode("fooba") == "MZXW6YTB");
  BOOST_REQUIRE(Wt::Utils::base32Encode("foobar") == "MZXW6YTBOI======");
}

BOOST_AUTO_TEST_CASE( Base32_rfc4648_decode )
{
  BOOST_REQUIRE(Wt::Utils::base32Decode("MZXW6===") == "foo");
  BOOST_REQUIRE(Wt::Utils::base32Decode("MZXW6YQ=") == "foob");
  BOOST_REQUIRE(Wt::Utils::base32Decode("MZXW6YTB") == "fooba");
  BOOST_REQUIRE(Wt::Utils::base32Decode("MZXW6YTBOI======") == "foobar");
}
