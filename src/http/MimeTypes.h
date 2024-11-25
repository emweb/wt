// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
//
// mime_types.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_MIME_TYPES_HPP
#define HTTP_MIME_TYPES_HPP

#include <string>

namespace http {
namespace server {
namespace mime_types {

/// Override file extension to MIME type mapping.
void setMapping(const std::string& csvFile);

/// Update file extension to MIME type mapping.
void updateMapping(const std::string& csvFile);

/// Convert a file extension into a MIME type.
std::string extensionToType(const std::string& extension);

} // namespace mime_types
} // namespace server
} // namespace http

#endif // HTTP_MIME_TYPES_HPP
