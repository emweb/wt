/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 * In addition to these terms, permission is also granted to use and
 * modify these two files (CgiParser.C and CgiParser.h) so long as the
 * copyright above is maintained, modifications are documented, and
 * credit is given for any use of the library.
 *
 * CGI parser modelled after the PERL implementation cgi-lib.pl 2.18 by
 * Steven E. Brenner with the following original copyright:

# Perl Routines to Manipulate CGI input
# cgi-lib@pobox.com
#
# Copyright (c) 1993-1999 Steven E. Brenner
# Unpublished work.
# Permission granted to use and modify this library so long as the
# copyright above is maintained, modifications are documented, and
# credit is given for any use of the library.
#
# Thanks are due to many people for reporting bugs and suggestions

# For more information, see:
#     http://cgi-lib.stanford.edu/cgi-lib/

 */

#include <fstream>
#include <stdlib.h>

#include <regex>

#include "CgiParser.h"
#include "WebRequest.h"
#include "WebUtils.h"
#include "FileUtils.h"

#include "Wt/WException.h"
#include "Wt/WLogger.h"
#include "Wt/Http/Request.h"

using std::memmove;
using std::strcpy;
using std::strtol;

namespace Wt {

WT_MAYBE_UNUSED LOGGER("CgiParser");

namespace {

  constexpr int MAX_BOUNDARY_LENGTH = 70;
  constexpr int MAX_NAME_LENGTH = 255;
  constexpr int MAX_FILENAME_LENGTH = 1024;
  constexpr int MAX_HEADER_FIELD_LENGTH = 1024 * 8;
  constexpr int MAX_MULTIPART_NON_FILE_VALUE_SIZE = 1024 * 32;

  bool isHeader(const std::string& name, const std::string& text)
  {
    std::string lower_line = Wt::Utils::lowerCase(text);
    size_t pos = lower_line.find_first_not_of(" \t");
    return pos != std::string::npos && lower_line.compare(pos, name.length(), name) == 0;
  }

  size_t findParam(const std::string& name,
                  const std::string& text,
                  size_t pos = 0)
  {
    if (name.empty()) {
      return pos;
    }

    std::string stopChars {name[0], '"'};

    while (pos != std::string::npos) {
      pos = text.find_first_of(stopChars, pos);

      if (pos == std::string::npos) {
        break;
      }

      if (text[pos] == '"') {
        pos++;
        //ignore everything until the next quote
        pos = text.find('"', pos);

        if (pos == std::string::npos) {
          break; // Malformed quotes
        }

      } else if (text.compare(pos, name.length(), name) == 0 &&
                 (pos == 0 ||
                  (!::isalnum(static_cast<unsigned char>(text[pos - 1])) &&
                   text[pos - 1] != '_'))) {
        // parameter name found
        return pos + name.length();
      }

      pos++;
    }

    return std::string::npos;
  }

  bool extractParameterValue(const std::string& text, size_t pos,
                            std::string& result, size_t maxLength = 0,
                            const std::string& terminators = " \t:;")
  {
    size_t end_pos = pos;
    if (text[pos] == '"') {
      // Handle Quoted Values
      pos++;
      end_pos = text.find('"', pos);
      if (end_pos == std::string::npos) {
        return false; // Malformed quotes
      }
    } else {
      // Handle Unquoted Values
      end_pos = text.find_first_of(terminators, pos);
      if (end_pos == std::string::npos) {
        end_pos = text.length();
      }
    }

    size_t length = end_pos - pos;

    if (length == 0 || (maxLength != 0 && length > maxLength)) {
      return false;
    }
    result = text.substr(pos, end_pos - pos);
    return true;
  }


  bool fishContentTypeValue(const std::string& text, std::string& result)
  {
    std::string lower_line = Wt::Utils::lowerCase(text);

    size_t pos = text.find_first_not_of(" \t");
    if (pos == std::string::npos) {
      return false;
    }

    if (pos != lower_line.find("content-type:", pos)) {
      return false;
    }

    // Move past "content-type:"
    pos += 13;

    // Skip whitespace
    pos = text.find_first_not_of(" \t", pos);
    if (pos == std::string::npos) {
      return false;
    }

    return extractParameterValue(text, pos, result);
  }

  bool fishBoundaryValue(const std::string& text, std::string& result)
  {
    std::string lower_line = Wt::Utils::lowerCase(text);

    size_t pos = 0;

    pos = findParam("boundary=", lower_line, pos);
    if (pos == std::string::npos) {
      return false;
    }

    return extractParameterValue(text, pos, result, MAX_BOUNDARY_LENGTH, " \t");
  }

  bool fishNameValue(const std::string& text, std::string& result)
  {
    std::string lower_line = Wt::Utils::lowerCase(text);

    size_t pos = 0;

    pos = findParam("name=", lower_line, pos);
    if (pos == std::string::npos) {
      return false;
    }

    return extractParameterValue(text, pos, result, MAX_NAME_LENGTH);
  }

  bool fishFilenameValue(const std::string& text, std::string& result)
  {
    std::string lower_line = Wt::Utils::lowerCase(text);

    size_t pos = 0;

    pos = findParam("filename=", lower_line, pos);
    if (pos == std::string::npos) {
      return false;
    }

    bool success = extractParameterValue(text, pos, result);

    if (success && result.length() > MAX_FILENAME_LENGTH) {
      // We still want a valid filename so that the file is spooled.
      LOG_WARN("Received a multipart/form-data request with filename length " << result.length() << " exceeding maximum of " << MAX_FILENAME_LENGTH << ". Truncating.");
      result = result.substr(result.length() - MAX_FILENAME_LENGTH);
    }

    return success;
  }
}

CgiParser::CgiParser(::int64_t maxRequestSize, ::int64_t maxFormData)
  : maxFormData_(maxFormData),
    maxRequestSize_(maxRequestSize)
{ }

void CgiParser::parse(WebRequest& request, ReadOption readOption)
{
  /*
   * TODO: optimize this ...
   */
  request_ = &request;

  ::int64_t len = request.contentLength();
  const char *type = request.contentType();
  const char *meth = request.requestMethod();

  request.postDataExceeded_ = (len > maxRequestSize_ ? len : 0);

  std::string queryString = request.queryString();

  LOG_DEBUG("queryString (len=" << len << "): " << queryString);

  if (!queryString.empty() && request_->parameters_.empty()) {
    Http::Request::parseFormUrlEncoded(queryString, request_->parameters_);
  }

  // XDomainRequest cannot set a contentType header, we therefore pass it
  // as a request parameter
  if (readOption != ReadHeadersOnly &&
      strcmp(meth, "POST") == 0 &&
      ((type && strstr(type, "application/x-www-form-urlencoded") == type) ||
       (queryString.find("&contentType=x-www-form-urlencoded") !=
        std::string::npos))) {
    /*
     * TODO: parse this stream-based to avoid the malloc here. For now
     * we protect the maximum that can be POST'ed as form data.
     */
    if (len > maxFormData_)
      throw WException("Oversized application/x-www-form-urlencoded ("
                       + std::to_string(len) + ")");

    auto buf = std::unique_ptr<char[]>(new char[len + 1]);

    request.in().read(buf.get(), len);

    if (request.in().gcount() != (int)len) {
      throw WException("Unexpected short read.");
    }

    buf[len] = 0;

    // This is a special Wt feature, I do not think it standard.
    // For POST, parameters in url-encoded URL are still parsed.

    std::string formQueryString = buf.get();

    LOG_DEBUG("formQueryString (len=" << len << "): " << formQueryString);
    if (!formQueryString.empty()) {
      Http::Request::parseFormUrlEncoded(formQueryString, request_->parameters_);
    }
    Http::ParameterMap::const_iterator it = request_->parameters_.find("Wt-params");
    if (it != request_->parameters_.end() && it->second.size() == 1) {
      // We need a copy here in case of unexpected second "Wt-params"
      // see issue#14616.
      std::string wtParams = it->second[0];
      Http::Request::parseFormUrlEncoded(wtParams, request_->parameters_);
    }
  }

  if (readOption != ReadHeadersOnly &&
      type && strstr(type, "multipart/form-data") == type) {
    if (strcmp(meth, "POST") != 0) {
      throw WException("Invalid method for multipart/form-data: "
                       + std::string(meth));
    }

    if (!request.postDataExceeded_)
      readMultipartData(request, type, len);
    else if (readOption == ReadBodyAnyway) {
      for (;len > 0;) {
        ::int64_t toRead = std::min(::int64_t(BUFSIZE), len);
        request.in().read(buf_, toRead);
        if (request.in().gcount() != (::int64_t)toRead)
          throw WException("CgiParser: short read");
        len -= toRead;
      }
    }
  }
}

void CgiParser::readMultipartData(WebRequest& request,
                                  const std::string type, ::int64_t len)
{
  std::string boundary;

  if (!fishBoundaryValue(type, boundary))
    throw WException("Could not find a boundary for multipart data.");

  boundary = "--" + boundary;

  buflen_ = 0;
  left_ = len;
  spoolStream_.reset();
  currentKey_.clear();

  if (!parseBody(request, boundary))
    return;

  for (;;) {
    if (!parseHead(request))
      break;
    if (!parseBody(request,boundary))
      break;
  }
}

/*
 * Read until finding the boundary, saving to resultString or
 * resultFile. The boundary itself is not consumed.
 *
 * tossAtBoundary controls how many characters extra (<0)
 * or few (>0) are saved at the start of the boundary in the result.
 *
 * maxLength controls the maximum number of characters to save in the
 * result. -1 means no limit. If the limit is exceeded, an exception is
 * thrown.
 */
void CgiParser::readUntilBoundary(WebRequest& request,
                                  const std::string boundary,
                                  int tossAtBoundary,
                                  int maxLength,
                                  std::string *resultString,
                                  std::ostream *resultFile)
{
  int bpos;
  int totalSaved = 0;

  while ((bpos = index(boundary)) == -1) {
    /*
     * If we couldn't find it. We need to wind the buffer, but only save
     * not including the boundary length.
     */
    if (left_ == 0)
      throw WException("CgiParser: reached end of input while seeking end of "
                       "headers or content. Format of CGI input is wrong");

    /* save (up to) BUFSIZE from buffer to file or value string, but
     * mind the boundary length */
    int save = std::min((buflen_ - (int)boundary.length()), (int)BUFSIZE);
    totalSaved += save;
    if (maxLength >= 0 && totalSaved > maxLength) {
      throw WException("CgiParser: maximum length for headers or content exceeded.");
    }

    if (save > 0) {
      if (resultString)
        *resultString += std::string(buf_, save);
      if (resultFile)
        resultFile->write(buf_, save);

      /* wind buffer */
      windBuffer(save);
    }

    unsigned amt = static_cast<unsigned>
      (std::min(left_,
                static_cast< ::int64_t >(BUFSIZE + MAXBOUND - buflen_)));

    request.in().read(buf_ + buflen_, amt);
    if (request.in().gcount() != (int)amt)
      throw WException("CgiParser: short read");

    left_ -= amt;
    buflen_ += amt;
  }

  if (maxLength >= 0 && totalSaved + bpos - tossAtBoundary > maxLength) {
    throw WException("CgiParser: maximum length for headers or content exceeded.");
  }

  if (resultString)
    *resultString += std::string(buf_, bpos - tossAtBoundary);
  if (resultFile)
    resultFile->write(buf_, bpos - tossAtBoundary);

  /* wind buffer */
  windBuffer(bpos);
}

void CgiParser::windBuffer(int offset)
{
  if (offset < buflen_) {
    memmove(buf_, buf_ + offset, buflen_ - offset);
    buflen_ -= offset;
  } else
    buflen_ = 0;
}

int CgiParser::index(const std::string search)
{
  std::string bufS = std::string(buf_, buflen_);

  std::string::size_type i = bufS.find(search);

  if (i == std::string::npos)
    return -1;
  else
    return i;
}

bool CgiParser::parseHead(WebRequest& request)
{
  std::string head;
  readUntilBoundary(request, "\r\n\r\n", -2, MAX_HEADER_FIELD_LENGTH, &head, 0);

  std::string name;
  std::string fn;
  std::string ctype;

  for (unsigned current = 0; current < head.length();) {
    /* read line by line */
    std::string::size_type i = head.find("\r\n", current);
    const std::string text = head.substr(current, (i == std::string::npos
                                                   ? std::string::npos
                                                   : i - current));

    if (isHeader("content-disposition:", text)) {
      // If the name is not found or is too long, we ignore the header.
      if (fishNameValue(text, name)) {
        fishFilenameValue(text, fn);
      } else {
        LOG_WARN("Received a Content-Disposition header in a multipart/form-data request without a name parameter, or with a name parameter exceeding maximum of " << MAX_NAME_LENGTH << ". Ignoring header.");
      }
    }

    if (isHeader("content-type:", text)) {
      fishContentTypeValue(text, ctype);
    }

    current = i + 2;
  }

  LOG_DEBUG("name: " << name << " ct: " << ctype  << " fn: " << fn);

  currentKey_ = name;

  if (!fn.empty()) {
    if (!request.postDataExceeded_) {
      /*
       * It is not easy to create a std::ostream pointing to a
       * temporary file name.
       */
      std::string spool = FileUtils::createTempFileName();

      spoolStream_ = std::make_unique<std::ofstream>(spool.c_str(),
        std::ios::out | std::ios::binary);

      request_->files_.insert
        (std::make_pair(name, Http::UploadedFile(spool, fn, ctype)));

      LOG_DEBUG("spooling file to " << spool.c_str());

    } else {
      spoolStream_.reset();
      // Clear currentKey so that file we don't do harm by reading this
      // giant blob in memory
      currentKey_ = "";
    }
  }

  windBuffer(4);

  return true;
}

bool CgiParser::parseBody(WebRequest& request, const std::string boundary)
{
  std::string value;
  bool writeToMemory = !spoolStream_ && !currentKey_.empty();

  int maxLength = -1;
  if (writeToMemory) {
    maxLength = MAX_MULTIPART_NON_FILE_VALUE_SIZE;
  } else if (spoolStream_) {
    maxLength = maxRequestSize_;
  }

  readUntilBoundary(request, boundary, 2,
                    maxLength,
                    writeToMemory ? &value : nullptr,
                    spoolStream_.get());

  if (spoolStream_) {
    LOG_DEBUG("completed spooling");
    spoolStream_.reset();
  } else {
    if (!currentKey_.empty()) {
      LOG_DEBUG("value: \"" << value << "\"");
      request_->parameters_[currentKey_].push_back(value);
    }
  }

  currentKey_.clear();

  if (std::string(buf_ + boundary.length(), 2) == "--")
    return false;

  windBuffer(boundary.length() + 2);

  return true;
}

} // namespace Wt
