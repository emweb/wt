/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/SslUtils.h"

#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

namespace {
  // Private key generated with:
  // openssl genrsa -out private.pem 1024
  const std::string pkeyStr = R"=(-----BEGIN PRIVATE KEY-----
MIICeAIBADANBgkqhkiG9w0BAQEFAASCAmIwggJeAgEAAoGBAOx2sTL2sLMmEwwK
YYcSGknp0qYd66/3R2S7k7N7REj/1BH8VlwrBBM9cW6IKQU/w7WE61sM0WwjkZ4N
fhe54rZVZT+O/IAhePJyXzWzJrhuVja3bvZSQUcG7RMri4Tl/KLoukWEYHHI5xLf
SFQ0XUKTqZi8VqgMnaAvzPj4vMXHAgMBAAECgYEA6lpYFGJZjbPWfMBtGCjg1RBg
LTLKO4Ofxj4BjIhGjPK/g3PLQ11+lHL40MsxnHotxOEPQRXbGInH8BT/OxSHkrXv
vNf2iqGSD/VxUEZUXEeg7mEE4DkqqSCq3GUYnwauDLM2CJYM0husoRLq3DeS8zzP
cLmSJ0nRzcQW161wXMECQQD9/CrdUW93GBw6EeQRtAngsm9TDUtxDo9mURwOj32P
j7bma1pxQL8X4gYUvTAXaz6TaihenGirKZCIH0L9/VAlAkEA7lbwh3doOgPQe7XS
T1rFZcOZ+tWiL1IMz5swR2/bcwEKhqofVkN0Gb00JMUJDMX0rYgQggo07aLCmgHq
TdT0ewJBAKWt3ExY9gxLNwaOayc9OYBRBZu4vXC3ncWRvWqmIbMHfbkbaHkeUkmD
EJJwwzFTrRM+maz1/LVCvNx/ABVtK90CQDFsSdhthGlzXQoqPABEnGZr10RShJ03
cHyke7B0m5cPgjVGldT3i93ChEuTqDrD2ecaLgIpR6x3cc8p0oJtRH0CQQC87FBL
xPNLHqvzNWNWo9W76S5MNbhVkpg2EjKRBtD7Rg/c93Ftt/C3vwWmCXoepECHUi26
7oslewtZMbuNwjEv
-----END PRIVATE KEY-----)=";

  // Signature generated with:
  // echo "Hello, world!" | openssl dgst -sha256 -sign private.pem -hex
  const std::string signature =
          "DDFA8796AACA1275594E3B2B15987CE55A2A2B520C31D5B8C7C6C6E6D4BD911C"
          "BB12233D23ADAA29E95F578A8273AF7C92F98C2A80A9B4D5062E60814B8D9318"
          "E2C8D387C7109440237B1F0192819F314320BB21535DF9970DE10049B178CDF4"
          "6F50AA6B619937A543C7E2D07650D0EB21C8F9FCDA2410F1472E89B53CC1A00C";
}

BOOST_AUTO_TEST_CASE( SslUtils_readPrivateKey )
{
  boost::filesystem::path tmpFile = boost::filesystem::unique_path();
  {
    std::ofstream ofs(tmpFile.string(), std::ios::out | std::ios::binary);
    ofs << pkeyStr;
  }
  EVP_PKEY* pkey = Wt::Ssl::readPrivateKeyFromFile(tmpFile.string());
  BOOST_TEST(pkey != nullptr);
  EVP_PKEY_free(pkey);
  boost::filesystem::remove(tmpFile);
}

BOOST_AUTO_TEST_CASE( SslUtils_rs256 )
{
  boost::filesystem::path tmpFile = boost::filesystem::unique_path();
  {
    std::ofstream ofs(tmpFile.string(), std::ios::out | std::ios::binary);
    ofs << pkeyStr;
  }
  EVP_PKEY* pkey = Wt::Ssl::readPrivateKeyFromFile(tmpFile.string());

  const auto result = Wt::Ssl::rs256(pkey, "Hello, world!\n");
  BOOST_TEST(result.size() == 128);
  BOOST_TEST(boost::algorithm::hex(result) == signature);

  EVP_PKEY_free(pkey);
  boost::filesystem::remove(tmpFile);
}
