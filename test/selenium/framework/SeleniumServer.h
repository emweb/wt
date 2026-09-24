/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WServer.h"
#include <Wt/WRandom.h>

#include <Wt/cpp17/filesystem.hpp>
#include <fstream>
#include <string>

namespace Selenium {
  /*! \class SeleniumServer "test/selenium/framework/SeleniumServer.h"
   *  \brief A very basic test server. Used by SeleniumFixture.
   *
   *  This server is very simple, and will bind to a random port. The
   *  docroot is simply ".", and the address is bound to localhost
   *  (`127.0.0.1`).
   */
  class SeleniumServer : public Wt::WServer
  {
    static constexpr char ADDRESS[] = "127.0.0.1";
  public:
    SeleniumServer(const std::string& docroot = ".", const std::string& configFile = "")
    {
      if (!configFile.empty()) {
        createConfigFile(configFile);
      }

      std::vector<std::string> args {
        "--http-address", ADDRESS,
        "--http-port", "0",
        "--docroot", docroot.c_str(),
        "--resources-dir", WT_SELENIUM_RESOURCES_DIR
      };

      if (!configFile.empty()) {
        args.push_back("--config");
        args.push_back(configFilePath_.string());
      }

      setServerConfiguration("test", args);
    }

    ~SeleniumServer()
    {
      if (!configFilePath_.empty()) {
        Wt::cpp17::fs_error_code error;
        Wt::cpp17::filesystem::remove(configFilePath_, error);
      }
    }

    //! Retrieve the (localhost) URL (and port) the server is hosted on.
    std::string url() const
    {
      return "http://" + std::string(ADDRESS) + ":" + std::to_string(httpPort());
    }

  private:
    Wt::cpp17::filesystem::path configFilePath_;

    void createConfigFile(const std::string& content)
    {
      configFilePath_ = Wt::cpp17::filesystem::path(Wt::cpp17::filesystem::temp_directory_path() /
                                                    ("wt_selenium_test_config_" + Wt::WRandom::generateId() + ".xml"));
      std::ofstream file(configFilePath_);
      if (!file.is_open()) {
        throw std::runtime_error("Failed to create config file: " + configFilePath_.string());
      }
      file.write(content.data(), content.size());
      file.close();
      if (!file.good()) {
        throw std::runtime_error("Failed to create config file: " + configFilePath_.string());
      }
    }
  };
}

