// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_LOGGER_H_
#define WT_DBO_LOGGER_H_

#include <Wt/WConfig.h>
#include <Wt/WLogSink.h>

#include <Wt/Dbo/WDboDllDefs.h>

namespace Wt {
namespace Dbo {

/*! \brief Sets a custom logger to redirect all logging to.
 *
 * Instead of using the server's default logger, this will send
 * all logging to some custom WLogSink.
 *
 * \sa Wt::Dbo::logToWt()
 */
WTDBO_API extern void setCustomLogger(const WLogSink &customLogger);

/*! \brief Redirects all Dbo logging to Wt::log
 *
 * Call this once at the start of your program to send all of Wt::Dbo's
 * logging to Wt's logger.
 *
 * \sa Wt::Dbo::setCustomLogger()
 */
inline void logToWt()
{
  setCustomLogger(*::Wt::Impl::logSink);
}

} // Dbo
} // Wt

#ifdef WT_BUILDING

#define WT_DBO_LOGGER
#include "../WLogger.h"
#undef WT_DBO_LOGGER

#endif // WT_BUILDING

#endif // WT_DBO_LOGGER_H_
