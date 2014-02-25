/*
 * Copyright (C) 2012 Emweb bvba, Heverlee, Belgium.
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

#include "Wt/WLogger"
#include "Wt/Utils"
#include "DomElement.h"
#include "md5.h"
#include "base64.h"
#include "ImageUtils.h"

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
  WWebWidget::escapeText(result, flags & EncodeNewLines ? true : false);
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
      char *e = 0;
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
  
  }
}
