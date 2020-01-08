// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_ASIO_IO_SERVICE_H_
#define WT_ASIO_IO_SERVICE_H_

#include "Wt/WConfig.h"

#ifdef WT_ASIO_IS_BOOST_ASIO

#include <boost/asio/io_service.hpp>

#else // WT_ASIO_IS_STANDALONE_ASIO

#include <asio/io_service.hpp>

#endif // WT_ASIO_IS_BOOST_ASIO

#include "namespace.hpp"

#endif // WT_ASIO_IO_SERVICE_H_
