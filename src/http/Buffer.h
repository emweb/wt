// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_BUFFER_HPP
#define HTTP_BUFFER_HPP

#include <boost/asio.hpp>

#include <boost/array.hpp>

namespace http {
namespace server {

typedef boost::array<char, 8192> Buffer;

}
}

#endif // HTTP_BUFFER_HPP
