// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_BUFFER_HPP
#define HTTP_BUFFER_HPP

#include <array>

namespace http {
namespace server {

typedef std::array<char, 8192> Buffer;

}
}

#endif // HTTP_BUFFER_HPP
