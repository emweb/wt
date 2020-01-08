// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_ASIO_NAMESPACE_H_
#define WT_ASIO_NAMESPACE_H_

#if defined(WT_ASIO_IS_BOOST_ASIO)

namespace Wt {
  namespace AsioWrapper {
    namespace asio = ::boost::asio;
  }
}

#elif defined(WT_ASIO_IS_STANDALONE_ASIO)

namespace Wt {
  namespace AsioWrapper {
    namespace asio = ::asio;
  }
}

#endif // WT_ASIO_IS_BOOST_ASIO

#endif // WT_ASIO_NAMESPACE_H_
