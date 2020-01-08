/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/AsioWrapper/asio.hpp"
#include "Wt/AsioWrapper/system_error.hpp"
#include "Wt/Http/WtClient.h"
#include "Wt/WException.h"

#include <sstream>
#include <regex>

using Wt::AsioWrapper::asio::ip::tcp;

namespace Wt {

namespace asio = AsioWrapper::asio;

  namespace Http {

namespace {
  unsigned int doGet(const std::string& host, const std::string& port,
		     const std::string& path, std::string *result)
  {

//
// based on:
//
// sync_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

    asio::io_service io_service;

    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(io_service);

    tcp::resolver::query query(host, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket(io_service);
    Wt::AsioWrapper::error_code error = asio::error::host_not_found;
    while (error && endpoint_iterator != end) {
      socket.close();
      socket.connect(*endpoint_iterator++, error);
    }

    if (error)
      throw WException("Could not connect to: " + host + ":" + port);

    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.

    asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Send the request.
    asio::write(socket, request);

    // Read the response status line.
    asio::streambuf response;
    asio::read_until(socket, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
      throw WException("Invalid response");

    if (status_code == 200) { 
      std::stringstream content;

      // Read the response headers, which are terminated by a blank line.
      asio::read_until(socket, response, "\r\n\r\n");

      // Write whatever content we already have to output.
      if (result && response.size() > 0)
	content << &response;

      // Read until EOF, writing data to output as we go.
      while (asio::read(socket, response,
                               asio::transfer_at_least(1), error))
	if (result)
	  content << &response;

      if (error != asio::error::eof)
	throw Wt::AsioWrapper::system_error(error);

      if (result)
	*result = content.str();
    }

    return status_code;
  }
}

void WtClient::startWtSession(const std::string& host,
			      const std::string& port,
			      const std::string& path,
			      const std::string& query,
			      WFlags<ClientOption> flags)
{
  std::string url = path;
  if (!query.empty())
    url += "?" + query;

  std::string result;
  int status = doGet(host, port, url, &result);

  if (status != 200)
    throw WException("Http status != 200: " + std::to_string(status));    

  static const std::regex session_e(".*\\?wtd=([a-zA-Z0-9]+)&amp;.*");

  std::string sessionId;
  std::smatch what;
  if (std::regex_match(result, what, session_e))
    sessionId = what[1];
  else
    throw WException("Unexpected response");

  url = path + "?wtd=" + sessionId;
  if (flags.test(ClientOption::SupportsAjax))
    url += "&request=script";
  else
    url += "&js=no";

  status = doGet(host, port, url, nullptr);

  if (status != 200)
    throw WException("Http status != 200: " + std::to_string(status));
}

  }
}
