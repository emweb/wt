// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CPP17_FILESYSTEM_HPP
#define WT_CPP17_FILESYSTEM_HPP
#include <Wt/WConfig.h>
#if defined(WT_FILESYSTEM_IMPL_BOOST)
#include <boost/filesystem.hpp>
namespace Wt {
  namespace cpp17 {
    namespace filesystem = boost::filesystem;
    using fs_error_code = boost::system::error_code;
  }
}
#elif defined(WT_FILESYSTEM_IMPL_STD)
#include <filesystem>
namespace Wt {
  namespace cpp17 {
    namespace filesystem = std::filesystem;
    using fs_error_code = std::error_code;
  }
}
#endif
#endif // WT_CPP17_FILESYSTEM_HPP