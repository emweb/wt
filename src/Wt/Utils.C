/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <vector>
#include <iterator>

#include <string.h>

#include "Wt/WConfig.h" // WT_WIN32
// for htonl():
#ifndef WT_WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include "Wt/WLogger.h"
#include "Wt/Utils.h"
#include "DomElement.h"
#include "md5.h"
#include "base64.h"
#include "ImageUtils.h"

#include <cstring>

extern "C" {
  #include "sha1.h"
}

namespace Wt {

LOGGER("Utils");

  namespace Utils {

    namespace {

unsigned char fromHex(char b)
{
  if (b <= '9')
    return b - '0';
  else if (b <= 'F')
    return (b - 'A') + 0x0A;
  else 
    return (b - 'a') + 0x0A;
}

unsigned char fromHex(char msb, char lsb)
{
  return (fromHex(msb) << 4) + fromHex(lsb);
}

char toHex(unsigned char b)
{
  if (b < 0xA)
    return '0' + b;
  else
    return 'a' + (b - 0xA);
}

void toHex(unsigned char b, char& msb, char& lsb)
{
  lsb = toHex(b & 0x0F);
  msb = toHex(b >> 4);
}

    }

std::string md5(const std::string& text)
{
  md5_state_t c;
  wt_md5_init(&c);

  wt_md5_append(&c, (const md5_byte_t *)text.c_str(), text.length());

  unsigned char buf[16];
  wt_md5_finish(&c, buf);

  return std::string((const char *)buf, 16);
}

std::string sha1(const std::string& text)
{
  SHA1Context sha;

  wt_SHA1Reset(&sha);
  wt_SHA1Input(&sha, (unsigned char *)text.c_str(), text.length());

  if (!wt_SHA1Result(&sha)) {
    LOG_ERROR("Error computing sha1 hash");
    return std::string();
  } else {
    const unsigned SHA1_LENGTH = 20;
    unsigned char hash[SHA1_LENGTH];

    for (unsigned i = 0; i < 5; ++i) {
      unsigned v = htonl(sha.Message_Digest[i]);
      memcpy(hash + (i*4), &v, 4);
    }

    return std::string(hash, hash + SHA1_LENGTH);
  }
}

std::string base64Encode(const std::string& data, bool crlf)
{
  std::vector<char> v;
  
  // base64 encoded value will be 4/3 larger than original value
  v.reserve((std::size_t)(data.size() * 1.35)); 

  base64::encode(data.begin(), data.end(), std::back_inserter(v), crlf);

  return std::string(v.begin(), v.end());
}

std::string base64Decode(const std::string& data)
{
  std::vector<char> v;
  
  // decoded value will be 3/4 smaller than encoded value
  v.reserve((std::size_t)(data.size() * 0.8));

  base64::decode(data.begin(), data.end(), std::back_inserter(v));

  return std::string(v.begin(), v.end());
}

std::string hexEncode(const std::string& data)
{
  std::string result(data.length() * 2, '-');

  for (unsigned i = 0; i < data.length(); ++i)
    toHex(data[i], result[2 * i], result[2 * i + 1]);

  return result;
}

std::string hexDecode(const std::string& data)
{
  std::string result(data.length() / 2, '-');

  for (unsigned i = 0; i < result.length(); ++i)
    result[i] = fromHex(data[2 * i], data[2 * i + 1]);

  return result;
}

std::string htmlEncode(const std::string& text, WFlags<HtmlEncodingFlag> flags)
{
  std::string result = text;
  WWebWidget::escapeText(result, 
			 (flags.test(HtmlEncodingFlag::EncodeNewLines)) ? 
			 true : false);
  return result;
}

WString htmlEncode(const WString& text, WFlags<HtmlEncodingFlag> flags)
{
  return WString::fromUTF8(htmlEncode(text.toUTF8(), flags));
}

std::string urlEncode(const std::string& text)
{
  return DomElement::urlEncodeS(text);
}

std::string urlDecode(const std::string &text)
{
  WStringStream result;

  for (unsigned i = 0; i < text.length(); ++i) {
    char c = text[i];

    if (c == '+') {
      result << ' ';
    } else if (c == '%' && i + 2 < text.length()) {
      std::string h = text.substr(i + 1, 2);
      char *e = nullptr;
      int hval = std::strtol(h.c_str(), &e, 16);

      if (*e == 0) {
	result << (char)hval;
	i += 2;
      } else
	// not a proper %XX with XX hexadecimal format
	result << c;
    } else
      result << c;
  }

  return result.str();
}

bool removeScript(WString& text)
{
  return WWebWidget::removeScript(text);
}

std::string guessImageMimeTypeData(const std::vector<unsigned char>& header)
{
  return Wt::ImageUtils::identifyMimeType(header);
}
std::string guessImageMimeType(const std::string& file)
{
  return Wt::ImageUtils::identifyMimeType(file);
}
  

std::string createDataUrl(std::vector<unsigned char>& data, std::string mimeType){
  std::string url = "data:"+mimeType+";"+"base64,";
  std::string datab64 = base64Encode(std::string(data.begin(), data.end()));
  return url+datab64;
}

std::string hmac(const std::string& text,
                 const std::string& key,
                 std::string (*hashfunction)(const std::string&),
                 size_t blocksize,
                 size_t keysize)
{
  unsigned char ipad[256];
  unsigned char opad[256];

  std::memset(ipad, 0, sizeof(unsigned char) * blocksize);

  if (key.size() > blocksize) {
    std::memcpy(ipad, (unsigned char*) hashfunction(key).c_str(), keysize);
  }
  else {
    keysize = key.size();
    std::memcpy(ipad, (unsigned char*) key.c_str(), keysize);
  }
  std::memcpy(opad, ipad, blocksize);

  for (size_t i=0; i<blocksize; ++i) {
    ipad[i] ^= 0x36;
    opad[i] ^= 0x5c;
  }

  return hashfunction(std::string((char*) opad,blocksize)
       + hashfunction(std::string((char*) ipad,blocksize) + text));
}

std::string hmac_md5(const std::string& text, const std::string& key)
{
  return hmac(text,
              key,
              &md5,
              64,
              16);
}

std::string hmac_sha1(const std::string& text, const std::string& key)
{
  return hmac(text,
              key,
              &sha1,
              64,
              20);
}

}

}
