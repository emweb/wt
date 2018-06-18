#include <Wt/WRestResource.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>

int main(int argc, char *argv[])
{
  Wt::WRestResource hello;
  hello.setHandler(Wt::Http::Method::Get, [](const Wt::Http::Request &,
                                             Wt::Http::Response &){
    return Wt::utf8("Hello world!");
  });

  try {
    Wt::WServer server{argc, argv, WTHTTP_CONFIGURATION};

    server.addResource(&hello, "/hello");

    server.run();
  } catch (const Wt::WServer::Exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "exception: " << e.what() << '\n';
    return 1;
  }
}
