// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_ASIO_SYSTEM_ERROR_H_
#define WT_ASIO_SYSTEM_ERROR_H_

#include "Wt/WConfig.h"

#ifdef WT_ASIO_IS_BOOST_ASIO

#include <boost/system/system_error.hpp>

namespace Wt {
  namespace AsioWrapper {
    using error_code = boost::system::error_code;
    using system_error = boost::system::system_error;
  }
}

#else // WT_ASIO_IS_STANDALONE_ASIO

#include <system_error>

namespace Wt {
  namespace AsioWrapper {
    using error_code = std::error_code;
    using system_error = std::system_error;
  }
}

#endif // WT_ASIO_IS_BOOST_ASIO

#endif // WT_ASIO_SYSTEM_ERROR_H_
