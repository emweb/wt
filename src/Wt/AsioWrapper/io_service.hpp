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

#include <boost/asio/version.hpp>
#if BOOST_ASIO_VERSION >= 103300
#include <boost/asio/io_context.hpp>
namespace boost {
  namespace asio {
    using io_service = boost::asio::io_context;
  }
}
#else
#include <boost/asio/io_service.hpp>
#endif

#else // WT_ASIO_IS_STANDALONE_ASIO

#include <asio/version.hpp>
#if ASIO_VERSION >= 103300
#include <asio/io_context.hpp>
namespace asio {
  using io_service = asio::io_context;
}
#else
#include <asio/io_service.hpp>
#endif

#endif // WT_ASIO_IS_BOOST_ASIO

#include "namespace.hpp"

#endif // WT_ASIO_IO_SERVICE_H_
