// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WLOGSINK_H_

#ifndef WT_TARGET_JAVA

#include <Wt/WDllDefs.h>
#include <string>

namespace Wt {
/*! \class WLogSink Wt/WLogSink.h Wt/WLogSink.h
 *  \brief An abstract log sink
 *
 * You can implement this to redirect Wt or Wt::Dbo's logging to another
 * logging system.
 *
 * For Wt, you can configure the logger on a WServer level using
 * WServer::setCustomLogger(). For Wt::Dbo, you can configure the logger
 * globally using Wt::Dbo::setCustomLogger().
 *
 * \sa WServer::setCustomLogger()
 * \sa Wt::Dbo::setCustomLogger()
 * \sa Wt::Dbo::logToWt()
 */
class WLogSink {
public:
  /*! \brief Logs a message
   *
   * \param type The type of the log message (e.g. debug, info, ...)
   * \param scope The scope of the message (e.g. WWidget, wthttp, ...)
   * \param message The message itself (usually starts with the scope too)
   */
  virtual void log(const std::string &type,
                   const std::string &scope,
                   const std::string &message) const noexcept = 0;

  /*! \brief Returns whether messages of the given type should be logged
   *
   * \param type The type of the log message (e.g. debug, info, ...)
   * \param scope The scope of the message (e.g. WWidget, wthttp, ...)
   *
   * The default implementation always returns true. You can reimplement this
   * as an optimization: if the underlying logger doesn't log messages of a certain
   * type, then Wt does not need to generate the log message.
   */
  virtual bool logging(const std::string &type,
                       const std::string &scope) const noexcept {
    (void)type; (void)scope; return true;
  }
};

namespace Impl {

WT_API extern const WLogSink * const logSink;

}

} // Wt

#endif // WT_TARGET_JAVA

#define WT_WLOGSINK_H_

#endif // WT_WLOGSINK_H_
