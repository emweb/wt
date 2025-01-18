// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_ASIO_SSL_H_
#define WT_ASIO_SSL_H_

#include "Wt/WConfig.h"

#ifdef WT_ASIO_IS_BOOST_ASIO

#include <boost/asio/ssl.hpp>
#if BOOST_ASIO_VERSION >= 103300
namespace boost {
  namespace asio {
    namespace ssl {
      using rfc2818_verification = boost::asio::ssl::host_name_verification;
    }
  }
}
#endif

#else // WT_ASIO_IS_STANDALONE_ASIO

#include <asio/ssl.hpp>
#if ASIO_VERSION >= 103300
namespace boost {
  namespace asio {
    namespace ssl {
      using rfc2818_verification = boost::asio::ssl::host_name_verification;
    }
  }
}
#endif

#endif // WT_ASIO_IS_BOOST_ASIO

#include "namespace.hpp"

#endif // WT_ASIO_SSL_H_
