/*
 * Copyright (C) Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCspErrorLogger.h"
#include "Wt/WLogger.h"

#include "Wt/Http/Response.h"
#include "Wt/Http/Request.h"
#include "Wt/Json/Array.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Serializer.h"
#include "Wt/Json/Value.h"

namespace Wt {

LOGGER("WCspErrorLogger");

WCspErrorLogger::WCspErrorLogger(int64_t maxReportSize)
  : maxReportSize_(maxReportSize)
{ }


WCspErrorLogger::~WCspErrorLogger()
{
  beingDeleted();
}

void WCspErrorLogger::handleRequest(const Http::Request& request,
                                    WT_MAYBE_UNUSED Http::Response& response)
{

  ::int64_t len = request.contentLength();
  std::string type = request.contentType();

  if (type.empty() ||
      (type != "application/reports+json" &&
       type != "application/csp-report")) {
    LOG_INFO("Ignoring CSP report with unexpected content type: " << (type.empty() ? "(null)" : type));
    return;
  }

  if (len > maxReportSize_) {
    LOG_SECURE("Ignoring CSP report that exceeds maximum size of " << maxReportSize_ << " bytes");
    return;
  }

  if (len == 0) {
    LOG_INFO("Ignoring empty CSP report");
    return;
  }

  auto buf = std::unique_ptr<char[]>(new char[len + 1]);
  request.in().read(buf.get(), len);

  if (request.in().gcount() != (int)len) {
    LOG_ERROR("Unexpected short read.");
  }

  buf[len] = 0;
  std::string body = buf.get();
  logReport(body);
}

void WCspErrorLogger::logReport(const std::string& report)
{
  LOG_ERROR("CSP report:\n" << report);
}

}
