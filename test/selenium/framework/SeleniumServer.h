/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WServer.h"

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
    SeleniumServer(const std::string& docroot = ".")
    {
      const char *argv[]
        = { "test",
            "--http-address", ADDRESS,
            "--http-port", "0",
            "--docroot", docroot.c_str()
          };
      setServerConfiguration(7, (char **)argv);
    }

    //! Retrieve the (localhost) URL (and port) the server is hosted on.
    std::string url() const
    {
      return "http://" + std::string(ADDRESS) + ":" + std::to_string(httpPort());
    }
  };
}

