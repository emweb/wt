#include "WebSocketApplication.h"

#include "../mywebsocket.h"

#include <web/Configuration.h>
#include <Wt/WServer.h>
#include <Wt/WWebSocketResource.h>

int main(int argc, char** argv)
{
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::EntryPointType::Application, [](const Wt::WEnvironment& env) { return std::make_unique<WebSocketApplication>(env); }, "/app");

    server.run();

  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
