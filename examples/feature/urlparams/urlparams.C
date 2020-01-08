/*
 * Copyright (C) 2018 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WResource.h>
#include <Wt/WServer.h>

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <iostream>

class Resource : public Wt::WResource {
public:
  virtual void handleRequest(const Wt::Http::Request &request,
                             Wt::Http::Response &response) override
  {
    response.setMimeType("text/plain");

    response.out() << "Request path:\n"
                   << request.path() << "\n\n";

    auto pathInfo = request.pathInfo();
    if (pathInfo.empty())
      pathInfo = "(empty)";
    response.out() << "Request path info:\n"
                   << pathInfo << "\n\n";

    response.out() << "Request URL parameters\n"
                      "----------------------\n";

    auto params = request.urlParams();

    if (params.empty())
      response.out() << "(empty)\n";

    for (const auto &param : params) {
      const auto &name = param.first;
      const auto &value = param.second;
      response.out() << name << ": " << value << '\n';
    }
  }
};

int main(int argc, char *argv[])
{
  Resource resource;

  try {
    Wt::WServer server{argc, argv, WTHTTP_CONFIGURATION};

    server.addResource(&resource, "/users");
    server.addResource(&resource, "/users/${user}");
    server.addResource(&resource, "/users/${user}/posts");
    server.addResource(&resource, "/users/${user}/posts/${post}");

    server.run();
  } catch (const Wt::WServer::Exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "exception: " << e.what() << '\n';
    return 1;
  }
}
