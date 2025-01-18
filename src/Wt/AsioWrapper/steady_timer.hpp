// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_ASIO_STEADY_TIMER_H_
#define WT_ASIO_STEADY_TIMER_H_

#include "Wt/WConfig.h"

#ifdef WT_ASIO_IS_BOOST_ASIO

#include <boost/asio/steady_timer.hpp>
#if BOOST_ASIO_VERSION >= 103300
#include <boost/asio/io_context.hpp>
namespace boost {
  namespace asio {
    using io_service = boost::asio::io_context;
  }
}
#endif

#else // WT_ASIO_IS_STANDALONE_ASIO

#include <asio/steady_timer.hpp>

#endif // WT_ASIO_IS_BOOST_ASIO

#include "namespace.hpp"

#endif // WT_ASIO_STEADY_TIMER_H_
