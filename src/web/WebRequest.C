/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebRequest.h"

#include <cstdlib>

using std::atoi;

namespace Wt {

WebRequest::WebRequest()
  : keepConnectionOpen_(false),
    id_(-1)
{ }

void WebRequest::setId(int id)
{
  id_ = id;
}

void WebRequest::flush()
{
  if (!keepConnectionOpen_) {
    delete this;
  }
}

WebRequest::~WebRequest()
{ }

std::string WebRequest::userAgent() const
{
  return headerValue("User-Agent");
}

std::string WebRequest::referer() const
{
  return headerValue("Referer");
}

std::string WebRequest::contentType() const
{
  return envValue("CONTENT_TYPE");
}

int WebRequest::contentLength() const
{
  std::string lenstr = envValue("CONTENT_LENGTH");

  if (lenstr.empty())
    return 0;
  else
    return atoi(lenstr.c_str());
}

void WebRequest::setKeepConnectionOpen(bool how)
{
  keepConnectionOpen_ = how;
}

}
