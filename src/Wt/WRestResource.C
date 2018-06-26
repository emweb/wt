/*
 * Copyright (C) 2018 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WRestResource.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <algorithm>

namespace Wt {
  
namespace {
  static const std::array<std::string, 5> METHOD_STRINGS = {std::string("GET"),
                                                            std::string("POST"),
                                                            std::string("PUT"),
                                                            std::string("DELETE"),
                                                            std::string("PATCH")};
}

WRestResource::Exception::Exception(int status)
  : WException("REST exception"), // TODO(Roel): improve what()?
    status_(status)
{ }

WRestResource::WRestResource()
{ }

WRestResource::~WRestResource()
{
  beingDeleted();
}

void WRestResource::handleRequest(const Http::Request &request,
                                  Http::Response &response)
{
  try {
    auto it = std::find(METHOD_STRINGS.cbegin(), METHOD_STRINGS.cend(), request.method());
    auto idx = static_cast<std::size_t>(std::distance(METHOD_STRINGS.cbegin(), it));
    if (it == METHOD_STRINGS.cend() || !handlers_[idx])
      response.setStatus(405);
  } catch (Exception e) {
    response.setStatus(e.status());
  }
}

void WRestResource::setHandler(Http::Method method, std::function<void(const Http::Request &, Http::Response &)> handler)
{
  auto idx = static_cast<std::size_t>(method);
  handlers_[idx] = std::move(handler);
}

void WRestResource::setMimeType(Http::Response &res, const std::string &mimeType)
{
  res.setMimeType(mimeType);
}

std::ostream &WRestResource::ostream(Http::Response &res)
{
  return res.out();
}

const std::string &WRestResource::param(const Http::Request &req, std::size_t n)
{
  return req.urlParams()[n].second;
}

const char *WRestResource::RestTraits::Result<std::string>::mimeType()
{
  return "text/plain";
}
void WRestResource::RestTraits::Result<std::string>::output(std::ostream &os, std::string &&ret)
{
  os << ret;
}
const char *WRestResource::RestTraits::Result<Wt::WString>::mimeType()
{
  return "text/plain";
}
void WRestResource::RestTraits::Result<Wt::WString>::output(std::ostream &os, Wt::WString &&ret)
{
  os << ret.toUTF8();
}
std::string WRestResource::RestTraits::Param<std::string>::parse(const std::string &str)
{
  return str;
}
Wt::WString WRestResource::RestTraits::Param<Wt::WString>::parse(const std::string &str)
{
  return Wt::utf8(str);
}
int WRestResource::RestTraits::Param<int>::parse(const std::string &str)
{
  return 0; // TODO(Roel)
}
unsigned WRestResource::RestTraits::Param<unsigned>::parse(const std::string &str)
{
  return 0U; // TODO(Roel)
}
long WRestResource::RestTraits::Param<long>::parse(const std::string &str)
{
  return 0L; // TODO(Roel)
}
unsigned long WRestResource::RestTraits::Param<unsigned long>::parse(const std::string &str)
{
  return 0UL; // TODO(Roel)
}
long long WRestResource::RestTraits::Param<long long>::parse(const std::string &str)
{
  return 0LL; // TODO(Roel)
}
unsigned long long WRestResource::RestTraits::Param<unsigned long long>::parse(const std::string &str)
{
  return 0ULL; // TODO(Roel)
}

}
