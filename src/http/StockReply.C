/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
//
// reply.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "StockReply.h"
#include "Configuration.h"

#include <string>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include "Request.h"
#include "../Wt/Utils.h"

namespace http {
namespace server {

namespace stock_replies {

const std::string ok = "";
const std::string ok_name = "200-ok.html";
const std::string created =
  "<html>"
  "<head><title>Created</title></head>"
  "<body><h1>201 Created</h1></body>"
  "</html>";
const std::string created_name = "201-created.html";
const std::string accepted =
  "<html>"
  "<head><title>Accepted</title></head>"
  "<body><h1>202 Accepted</h1></body>"
  "</html>";
const std::string accepted_name = "202-accepted.html";
const std::string no_content =
  "<html>"
  "<head><title>No Content</title></head>"
  "<body><h1>204 Content</h1></body>"
  "</html>";
const std::string no_content_name = "204-nocontent.html";
const std::string multiple_choices =
  "<html>"
  "<head><title>Multiple Choices</title></head>"
  "<body><h1>300 Multiple Choices</h1></body>"
  "</html>";
const std::string multiple_choices_name = "300-multiple-choices.html";
const std::string moved_permanently =
  "<html>"
  "<head><title>Moved Permanently</title></head>"
  "<body><h1>301 Moved Permanently</h1></body>"
  "</html>";
const std::string moved_permanently_name = "301-moved-permanently.html";
const std::string found =
  "<html>"
  "<head><title>Found</title></head>"
  "<body><h1>302 Found</h1></body>"
  "</html>";
const std::string found_name = "302-found.html";
const std::string see_other =
  "<html>"
  "<head><title>See Other</title></head>"
  "<body><h1>303 See Other</h1></body>"
  "</html>";
const std::string see_other_name = "303-see-other.html";
const std::string not_modified =
  "<html>"
  "<head><title>Not Modified</title></head>"
  "<body><h1>304 Not Modified</h1></body>"
  "</html>";
const std::string not_modified_name = "304-not-modified.html";
const std::string moved_temporarily =
  "<html>"
  "<head><title>Moved Temporarily</title></head>"
  "<body><h1>307 Moved Temporarily</h1></body>"
  "</html>";
const std::string moved_temporarily_name = "307-moved-temporarily.html";
const std::string bad_request =
  "<html>"
  "<head><title>Bad Request</title></head>"
  "<body><h1>400 Bad Request</h1></body>"
  "</html>";
const std::string bad_request_name = "400-bad-request.html";
const std::string unauthorized =
  "<html>"
  "<head><title>Unauthorized</title></head>"
  "<body><h1>401 Unauthorized</h1></body>"
  "</html>";
const std::string unauthorized_name = "401-unauthorized.html";
const std::string forbidden =
  "<html>"
  "<head><title>Forbidden</title></head>"
  "<body><h1>403 Forbidden</h1></body>"
  "</html>";
const std::string forbidden_name = "403-forbidden.html";
const std::string not_found =
  "<html>"
  "<head><title>Not Found</title></head>"
  "<body><h1>404 Not Found</h1></body>"
  "</html>";
const std::string not_found_name = "404-not-found.html";
const std::string request_entity_too_large =
  "<html>"
  "<head><title>Request Entity Too Large</title></head>"
  "<body><h1>413 Request Entity Too Large</h1></body>"
  "</html>";
const std::string request_entity_too_large_name = "413-request-entity-too-large.html";
const std::string requested_range_not_satisfiable =
  "<html>"
  "<head><title>Requested Range Not Satisfiable</title></head>"
  "<body><h1>416 Requested Range Not Satisfiable</h1></body>"
  "</html>";
const std::string requested_range_not_satisfiable_name =
  "416-requested-range-not-satisfiable.html";
const std::string internal_server_error =
  "<html>"
  "<head><title>Internal Server Error</title></head>"
  "<body><h1>500 Internal Server Error</h1></body>"
  "</html>";
const std::string internal_server_error_name = "500-internal-server-error.html";
const std::string not_implemented =
  "<html>"
  "<head><title>Not Implemented</title></head>"
  "<body><h1>501 Not Implemented</h1></body>"
  "</html>";
const std::string not_implemented_name = "501-not-implemented.html";
const std::string bad_gateway =
  "<html>"
  "<head><title>Bad Gateway</title></head>"
  "<body><h1>502 Bad Gateway</h1></body>"
  "</html>";
const std::string bad_gateway_name = "502-bad-gateway.html";
const std::string service_unavailable =
  "<html>"
  "<head><title>Service Unavailable</title></head>"
  "<body><h1>503 Service Unavailable</h1></body>"
  "</html>";
const std::string service_unavailable_name ="503-service-unavailable.html";
const std::string version_not_supported =
  "<html>"
  "<head><title>HTTP Version Not Supported</title></head>"
  "<body><h1>505 HTTP Version Not Supported</h1></body>"
  "</html>";
const std::string version_not_supported_name = "505-version-not-supported.html";

const std::string& toText(Reply::status_type status)
{
  switch (status)
  {
  case Reply::ok:
    return ok;
  case Reply::created:
    return created;
  case Reply::accepted:
    return accepted;
  case Reply::no_content:
    return no_content;
  case Reply::multiple_choices:
    return multiple_choices;
  case Reply::moved_permanently:
    return moved_permanently;
  case Reply::found:
    return found;
  case Reply::see_other:
    return see_other;
  case Reply::not_modified:
    return not_modified;
  case Reply::moved_temporarily:
    return moved_temporarily;
  case Reply::bad_request:
    return bad_request;
  case Reply::unauthorized:
    return unauthorized;
  case Reply::forbidden:
    return forbidden;
  case Reply::not_found:
    return not_found;
  case Reply::request_entity_too_large:
    return request_entity_too_large;
  case Reply::requested_range_not_satisfiable:
    return requested_range_not_satisfiable;
  case Reply::internal_server_error:
    return internal_server_error;
  case Reply::not_implemented:
    return not_implemented;
  case Reply::bad_gateway:
    return bad_gateway;
  case Reply::service_unavailable:
    return service_unavailable;
  case Reply::version_not_supported:
    return version_not_supported;
  default:
    return internal_server_error;
  }
}

const std::string& toName(Reply::status_type status)
{
  switch (status)
  {
  case Reply::ok:
    return ok_name;
  case Reply::created:
    return created_name;
  case Reply::accepted:
    return accepted_name;
  case Reply::no_content:
    return no_content_name;
  case Reply::multiple_choices:
    return multiple_choices_name;
  case Reply::moved_permanently:
    return moved_permanently_name;
  case Reply::found:
    return found_name;
  case Reply::see_other:
    return see_other_name;
  case Reply::not_modified:
    return not_modified_name;
  case Reply::moved_temporarily:
    return moved_temporarily_name;
  case Reply::bad_request:
    return bad_request_name;
  case Reply::unauthorized:
    return unauthorized_name;
  case Reply::forbidden:
    return forbidden_name;
  case Reply::not_found:
    return not_found_name;
  case Reply::request_entity_too_large:
    return request_entity_too_large_name;
  case Reply::requested_range_not_satisfiable:
    return requested_range_not_satisfiable;
  case Reply::internal_server_error:
    return internal_server_error_name;
  case Reply::not_implemented:
    return not_implemented_name;
  case Reply::bad_gateway:
    return bad_gateway_name;
  case Reply::service_unavailable:
    return service_unavailable_name;
  case Reply::version_not_supported:
    return version_not_supported_name;
  default:
    return internal_server_error_name;
  }
}

void buildOriginalURL(Request &req, std::string &url)
{
  if (url.empty()) {
    url = "http://";
    for (Request::HeaderList::const_iterator i = req.headers.begin();
	 i != req.headers.end(); ++i) {
      if (i->name == "Host") {
	url += i->value.str();
	break;
      }
    }
    url += req.uri.str();
  }
}

} // namespace stock_replies

StockReply::StockReply(Request& request,
		       status_type status,
		       const Configuration& configuration)
  : Reply(request, configuration),
    transmitted_(false)
{ 
  setStatus(status);
}

StockReply::StockReply(Request& request,
		       status_type status,
		       std::string extraContent,
		       const Configuration& configuration)
  : Reply(request, configuration),
    content_(extraContent),
    transmitted_(false)
{
  setStatus(status);
}

void StockReply::reset(const Wt::EntryPoint *ep)
{
  assert(false);
}

bool StockReply::consumeData(const char *begin,
			     const char *end,
			     Request::State state)
{
  if (state != Request::Partial)
    send();
  return true;
}

std::string StockReply::contentType()
{
  return "text/html";
}

::int64_t StockReply::contentLength()
{
  std::string full_path(configuration().errRoot()
			+ stock_replies::toName(status()));
  std::string original_url;
  std::string content = "";
  std::string line;
  size_t clen = content_.length();
  std::ifstream ifstr(full_path.c_str(), std::ios::in | std::ios::binary);

  while (ifstr.good() && !ifstr.eof()) {
    std::getline(ifstr, line);
    size_t index = 0;

    while ((index = line.find("<-- SPECIAL CONTENT -->", index)) != line.npos) {
      line.replace(index,sizeof("<-- SPECIAL CONTENT -->")-1, content_);
      index += clen;
    }

    index = line.find("<-- ORIGINAL URL -->");

    if (index != line.npos) {
      stock_replies::buildOriginalURL(request_, original_url);
      clen = original_url.length();

      do {
	line.replace(index,sizeof("<-- ORIGINAL URL -->")-1, original_url);
	index += clen;
      } while((index = line.find("<-- ORIGINAL URL -->", index) != line.npos));

    }

    index = line.find("<-- ORIGINAL URL ESCAPED -->");

    if (index != line.npos) {
      if (original_url.empty())
	stock_replies::buildOriginalURL(request_, original_url);

      std::string escapedUrl = Wt::Utils::urlEncode(original_url);
      clen = escapedUrl.length();

      do {
	line.replace(index,sizeof("<-- ORIGINAL URL ESCAPED -->") - 1,
		     escapedUrl);
	index += clen;
      } while((index = line.find("<-- ORIGINAL URL ESCAPED -->", index)
	       != line.npos));
    }
    content += line + "\r\n";
  }
  ifstr.close();

  if (content.empty())
    content_ = stock_replies::toText(status()) + content_;
  else
    content_ = content;

  return content_.length();
}

bool StockReply::nextContentBuffers(std::vector<asio::const_buffer>& result)
{
  if (!transmitted_) {
    transmitted_ = true;
    result.push_back(asio::buffer(content_));
  }
  return true;
}


} // namespace server
} // namespace http
