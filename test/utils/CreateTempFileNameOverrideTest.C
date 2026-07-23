/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/FileUtils.h"

#include <string>

namespace {
  const std::string overrideName = "application-temp-file-name";
}

namespace Wt {
  namespace FileUtils {
    // Strong definition replacing the weak one in the Wt library.
    std::string createTempFileName()
    {
      return overrideName;
    }
  }
}

BOOST_AUTO_TEST_CASE( FileUtils_createTempFileName_override )
{
  const std::string overridden = Wt::FileUtils::createTempFileName();

  BOOST_CHECK_EQUAL(overridden, overrideName);
}
