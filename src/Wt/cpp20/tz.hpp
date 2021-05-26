// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CPP20_TZ_HPP
#define WT_CPP20_TZ_HPP

#include <Wt/WConfig.h>

#if defined(WT_DATE_TZ_USE_STD)

#include <chrono>

namespace Wt::cpp20 {

namespace date = std::chrono;

}

#else

#include <Wt/Date/tz.h>

namespace Wt {
namespace cpp20 {

namespace date = ::date;

}
}

#endif

#endif // WT_CPP20_TZ_HPP
