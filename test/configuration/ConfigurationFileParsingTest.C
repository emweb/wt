/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <web/Configuration.h>
#include <Wt/WRandom.h>

#include <Wt/cpp17/filesystem.hpp>
#include <fstream>
#include <string>

namespace {
  class ConfigFileFixture {
  public:
    ConfigFileFixture(const std::string& content)
      : filePath_(Wt::cpp17::filesystem::temp_directory_path() /
                  ("wt_test_config_" + Wt::WRandom::generateId() + ".xml"))
    {
      std::ofstream file(filePath());
      BOOST_REQUIRE(file.is_open());
      file.write(content.data(), content.size());
      file.close();
      BOOST_REQUIRE(file.good());
    }

    ~ConfigFileFixture()
    {
      Wt::cpp17::fs_error_code error;
      Wt::cpp17::filesystem::remove(filePath(), error);
    }

    std::string filePath() const { return filePath_.string(); }

  private:
    Wt::cpp17::filesystem::path filePath_;
  };
}

BOOST_AUTO_TEST_CASE( parse_original_ip_header_in_trusted_proxy_config )
{
  ConfigFileFixture confFile("<server>"
                               "<application-settings location=\"*\">"
                                 "<trusted-proxy-config>"
                                   "<original-ip-header>TestHeader</original-ip-header>"
                                 "</trusted-proxy-config>"
                               "</application-settings>"
                             "</server>");

  Wt::Configuration conf("", "", confFile.filePath(), nullptr);

  BOOST_CHECK_EQUAL(conf.originalIPHeader(), "TestHeader");
}