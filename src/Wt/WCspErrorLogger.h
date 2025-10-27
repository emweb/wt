// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCSP_ERROR_LOGGER_H_
#define WCSP_ERROR_LOGGER_H_

#include <string>

#include <Wt/WResource.h>

namespace Wt {
/*! \class WCspErrorLogger Wt/WCspErrorLogger.h Wt/WCspErrorLogger.h
 *  \brief A resource that logs Content Security Policy (CSP) errors.
 *
 * This resource can be used to log CSP violations reported by the
 * browser.
 */
class WT_API WCspErrorLogger : public WResource
{
public:
  /*! \brief Creates a CSP error logger resource.
   *
   * Creates a CSP error logger resource that will log CSP violation
   * reports to the Wt log. The maximum size of a report is specified
   * by \p maxReportSize. Reports that exceed this size will be ignored.
   */
  WCspErrorLogger(int64_t maxReportSize = 1024 * 50);

  //! Destructor.
  ~WCspErrorLogger();

  void handleRequest(const Http::Request& request,
                     Http::Response& response) override;

protected:
  /*! \brief Logs a CSP report.
   *
   * This method is called when a CSP report is received. The default
   * implementation logs the report to the Wt log. Subclasses can
   * override this method to provide custom handling of CSP reports.
   */
  virtual void logReport(const std::string& report);

private:
  int64_t maxReportSize_;
};

}

#endif // WCSP_ERROR_LOGGER_H_
