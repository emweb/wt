// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_ASIO_SSL_H_
#define WT_ASIO_SSL_H_

#include "Wt/WConfig.h"

#include <openssl/opensslv.h>

// Older Asio releases' RFC 2818 verifier uses internals made opaque in
// OpenSSL 4. Use its replacement host name verifier instead.
#ifdef WT_ASIO_IS_BOOST_ASIO

#if defined(OPENSSL_VERSION_MAJOR) && OPENSSL_VERSION_MAJOR >= 4
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/ssl/verify_context.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#else
#include <boost/asio/ssl.hpp>
#endif
#if BOOST_ASIO_VERSION >= 103300 || \
    (defined(OPENSSL_VERSION_MAJOR) && OPENSSL_VERSION_MAJOR >= 4)
namespace boost {
  namespace asio {
    namespace ssl {
      using rfc2818_verification = boost::asio::ssl::host_name_verification;
    }
  }
}
#endif

#else // WT_ASIO_IS_STANDALONE_ASIO

#if defined(OPENSSL_VERSION_MAJOR) && OPENSSL_VERSION_MAJOR >= 4
#include <asio/ssl/context.hpp>
#include <asio/ssl/context_base.hpp>
#include <asio/ssl/error.hpp>
#include <asio/ssl/host_name_verification.hpp>
#include <asio/ssl/stream.hpp>
#include <asio/ssl/stream_base.hpp>
#include <asio/ssl/verify_context.hpp>
#include <asio/ssl/verify_mode.hpp>
#else
#include <asio/ssl.hpp>
#endif
#if ASIO_VERSION >= 103300 || \
    (defined(OPENSSL_VERSION_MAJOR) && OPENSSL_VERSION_MAJOR >= 4)
namespace asio {
  namespace ssl {
    using rfc2818_verification = asio::ssl::host_name_verification;
  }
}
#endif

#endif // WT_ASIO_IS_BOOST_ASIO

#include "namespace.hpp"

#endif // WT_ASIO_SSL_H_
