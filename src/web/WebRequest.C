/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebRequest.h"

#include <cstdlib>

using std::atoi;

namespace Wt {

Http::ParameterValues WebRequest::emptyValues_;

WebRequest::WebRequest()
  : id_(-1)
{ }

void WebRequest::setId(int id)
{
  id_ = id;
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

const std::string *WebRequest::getParameter(const std::string& name) const
{
  const Http::ParameterValues& values = getParameterValues(name);

  return !values.empty() ? &values[0] : 0;
}

const Http::ParameterValues&
WebRequest::getParameterValues(const std::string& name) const
{
  Http::ParameterMap::const_iterator i = parameters_.find(name);
  if (i != parameters_.end())
    return i->second;
  else
    return emptyValues_;
}


}
