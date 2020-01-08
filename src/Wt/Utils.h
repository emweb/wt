// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_UTILS_H_
#define WT_UTILS_H_

#include <Wt/WGlobal.h>
#include <string>
#include <vector>

/*! \file Utils
 */

namespace Wt {

  /*! \brief Namespace with utility functions.
   *
   * This namespace contains functions for computing message digests
   * with cryptographic hash functions (md5, sha1), and functions that
   * implement encoding and decoding for common encodings.
   *
   * These utility functions can be accessed by including the
   * \c Wt/Utils.h header.
   */
  namespace Utils {

/*! \brief An enumeration for HTML encoding flags.
 */
enum class HtmlEncodingFlag
{
  //! \brief Encode new-lines as line breaks (&lt;br&gt;)
  EncodeNewLines = 0x1
};

/*! \brief Computes an MD5 hash.
 *
 * This utility function computes an MD5 hash, and returns the raw
 * (binary) hash value.
 *
 * \sa sha1()
 */
WT_API extern std::string md5(const std::string& data);

/*! \brief Computes a SHA-1 hash.
 *
 * This utility function computes a SHA-1 hash, and returns the raw
 * (binary) hash value.
 *
 * \sa md5()
 */
#ifndef WT_TARGET_JAVA
WT_API extern std::string sha1(const std::string& data);
#else
WT_API extern std::vector<unsigned char> sha1(const std::string& data);
#endif


/*! \brief Performs Base64-encoding of data.
 *
 * This utility function implements a Base64 encoding (RFC 2045) of
 * the \p data.
 *
 * When the crlf argument is true, a CRLF character will be added
 * after each sequence of 76 characters.
 *
 * \sa base64Decode()
 */
WT_API extern std::string base64Encode(const std::string& data, 
				       bool crlf = true);

/*! \brief Performs Base64-decoding of data.
 *
 * This utility function implements a Base64 decoding (RFC 2045) of
 * the \p data. Illegal characters are discarded and skipped.
 *
 * \sa base64Encode()
 */
#ifndef WT_TARGET_JAVA
WT_API extern std::string base64Decode(const std::string& data);
#else
WT_API extern std::vector<unsigned char> base64Decode(const std::string& data);
WT_API extern std::string base64DecodeS(const std::string& data);
#endif

/*! \brief Performs Hex-encoding of data.
 *
 * A hex-encoding outputs the value of every byte as as two-digit
 * hexadecimal number.
 *
 * \sa hexDecode()
 */
WT_API extern std::string hexEncode(const std::string& data);

/*! \brief Performs Hex-decoding of data.
 *
 * Illegal characters are discarded and skipped.
 *
 * \sa hexEncode()
 */
WT_API extern std::string hexDecode(const std::string& data);

/*! \brief Performs HTML encoding of text.
 *
 * This utility function escapes characters so that the \p text can
 * be embodied verbatim in a HTML text block.
 */
WT_API extern std::string htmlEncode(const std::string& text,
				     WFlags<HtmlEncodingFlag> flags = None);

/*! \brief Performs HTML encoding of text.
 *
 * This utility function escapes characters so that the \p text can
 * be embodied verbatim in a HTML text block.
 *
 * By default, newlines are ignored. By passing the HtmlEncodingFlag::EncodeNewLines
 * flag, these may be encoded as line breaks (&lt;br&gt;).
 */
WT_API extern WString htmlEncode(const WString& text,
				 WFlags<HtmlEncodingFlag> flags = None);

/*! \brief Performs Url encoding (aka percentage encoding).
 *
 * This utility function percent encodes a \p text so that it can be
 * embodied verbatim in a URL (e.g. as a fragment).
 *
 * \note To url encode a unicode string, the de-facto standard
 * practice is to encode a UTF-8 encoded string.
 *
 * \sa WString::toUTF8(), urlDecode()
 */
WT_API extern std::string urlEncode(const std::string& text);

/*! \brief Performs Url decoding.
 *
 * This utility function percent encodes a \p text so that it can be
 * embodied verbatim in a URL (e.g. as a fragment).
 *
 * \note To url decode a unicode string, the de-facto standard
 * practice is to interpret the string as a UTF-8 encoded string.
 *
 * \sa WString::fromUTF8(), urlEncode()
 */
WT_API extern std::string urlDecode(const std::string& text);

/*! \brief Remove tags/attributes from text that are not passive.
 *
 * This removes tags and attributes from XHTML-formatted text that do
 * not simply display something but may trigger scripting, and could
 * have been injected by a malicious user for Cross-Site Scripting
 * (XSS).
 *
 * This method is used by the library to sanitize XHTML-formatted text
 * set in WText, but it may also be useful outside the library to
 * sanitize user content when directly using JavaScript.
 *
 * Modifies the \p text if needed. When the text is not proper XML,
 * returns \c false.
 */
WT_API extern bool removeScript(WString& text);

/*! \brief Guess the image mime type from an image.
 *
 * This function examines the header of an image and tries to identify
 * the image type.
 *
 * At the moment, it recognizes and returns as mime type :
 * - image/png
 * - image/jpeg
 * - image/gif
 * - image/bmp
 *
 * The header should contain (at least) the 25 first bytes of the image data.
 *
 * If no mime-type could be derived, an empty string is returned.
 *
 * \sa guessImageMimeTypeData()
 */
WT_API extern std::string
guessImageMimeTypeData(const std::vector<unsigned char>& header);

/*! \brief Guess the image mime type from an image.
 *
 * This function opens the image \p file, reads the first 25 bytes and calls
 * guessImageMimeTypeData() to infer the mime type.
 */
WT_API extern std::string guessImageMimeType(const std::string& file);

WT_API extern std::string createDataUrl(std::vector<unsigned char>& data, std::string mimeType);

/*! \brief Computes a hash-based message authentication code.
 *
 * This utility function computes a HMAC, and returns the raw
 * (binary) hash value. Takes as arguments the text to be hashed, a
 * secret key, a function pointer to a hashfunction, the internal
 * block size of the hashfunction in bytes and the size of the
 * resulting hash value the function produces. A maximum blocksize of
 * 2048 bits (256 bytes) is supported.
 *
 * \sa hmac_sha1()
 * \sa hmac_md5()
 */
WT_API extern std::string hmac(const std::string& text,
                               const std::string& key,
                               std::string (*hashfunction)(const std::string&),
                               size_t blocksize,
                               size_t keysize);

/*! \brief Computes a hash-based message authentication code.
 *
 * Uses the md5 hashfunction, returns a raw (binary) hash value.
 *
 * \sa hmac()
 */
WT_API extern std::string hmac_md5(const std::string& text, const std::string& key);

/*! \brief Computes a hash-based message authentication code.
 *
 * Uses the sha1 hashfunction, returns a raw (binary) hash value.
 *
 * \sa hmac()
 */
WT_API extern std::string hmac_sha1(const std::string& text, const std::string& key);

}

}

W_DECLARE_OPERATORS_FOR_FLAGS(Wt::Utils::HtmlEncodingFlag)

#endif // WT_UTILS_H_
