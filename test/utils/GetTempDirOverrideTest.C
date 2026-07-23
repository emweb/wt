/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/FileUtils.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <unistd.h>

namespace {
  // This executable overrides only getTempDir().
  std::string overrideTempDir;
}

namespace Wt {
  namespace FileUtils {
    // Strong definition replacing the weak one in the Wt library.
    std::string getTempDir()
    {
      return overrideTempDir;
    }
  }
}

BOOST_AUTO_TEST_CASE( FileUtils_getTempDir_override )
{
  char tempDir[] = "/tmp/wt-temp-dir-override-XXXXXX";
  BOOST_REQUIRE(mkdtemp(tempDir));

  overrideTempDir = tempDir;
  const std::string fileName = Wt::FileUtils::createTempFileName();
  overrideTempDir.clear();

  const bool usedOverrideTempDir
    = fileName.compare(0, std::string(tempDir).size() + 1,
                       std::string(tempDir) + "/") == 0;
  const bool fileExists = Wt::FileUtils::exists(fileName);

  if (fileExists) {
    std::remove(fileName.c_str());
  }
  rmdir(tempDir);

  BOOST_CHECK(usedOverrideTempDir);
  BOOST_CHECK(fileExists);
}
