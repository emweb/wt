#include "WebSocketApplication.h"

#include "../mywebsocket.h"

#include <Wt/WEnvironment.h>
#include <Wt/WServer.h>

const std::string WS_PATH = "/websocket";

int main(int argc, char** argv)
{
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    auto websocketResource = std::make_shared<MyWebSocketResource>();

    server.addResource(websocketResource, WS_PATH);
    server.addEntryPoint(Wt::EntryPointType::Application, [](const Wt::WEnvironment& env) { return std::make_unique<WebSocketApplication>(env, WS_PATH); }, "/app");

    server.run();

  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
