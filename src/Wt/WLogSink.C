/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLogger.h"
#include "Wt/WLogSink.h"

#include <boost/algorithm/string/predicate.hpp>

namespace {

class WtLogSink final : public Wt::WLogSink {
public:
  virtual void log(const std::string &type,
                   const std::string &scope,
                   const std::string &message) const noexcept override
  {
    if (boost::starts_with(message, scope)) {
      Wt::log(type) << scope << message.substr(scope.size());
    } else {
      Wt::log(type) << scope << ": " << message;
    }
  }

  virtual bool logging(const std::string &type,
                       const std::string &scope) const noexcept override
  {
    return Wt::logInstance().logging(type, scope);
  }
};

const WtLogSink instance_;

}

namespace Wt {
namespace Impl {

const WLogSink * const logSink = &instance_;

} // Impl
} // Wt
