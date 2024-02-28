#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

#include <memory>
#include <mutex>

namespace Wt
{
  class WLineEdit;
  class WPushButton;
  class WTemplate;
  class WText;
}

class MyWebSocketConnection;
class MyWebSocketResource;

class WebSocketApplication : public Wt::WApplication
{
public:
  WebSocketApplication(const Wt::WEnvironment& env);
  ~WebSocketApplication();

private:
  void createResource();
  void createLayout();
  void createOutputField();
  void createWebSocketListeners();
  void createWebSocketField();
  void createDataFields();
  void createActionButtons();

  void addWebSocketConnection(MyWebSocketConnection* connection);

  void connectWebSocket();
  void disconnectWebSocket();
  void serverDisconnectWebSocket();
  bool checkConnection();
  void connectionClosed();

  void sendTextData();
  void sendBinaryData();
  void sendServerTextData();
  void sendServerBinaryData();

  void addToOutput(const std::string& value);

  bool isConnected_ = false;

  std::unique_ptr<MyWebSocketResource> websocketResource_;
  MyWebSocketConnection* connection_ = nullptr;

  Wt::WTemplate* templ_ = nullptr;
  Wt::WText* output_ = nullptr;

  Wt::WLineEdit* wsData_ = nullptr;
  Wt::WLineEdit* wsServerData_ = nullptr;

  Wt::WPushButton* connect_ = nullptr;
  Wt::WPushButton* disconnect_ = nullptr;
  Wt::WPushButton* serverDisconnect_ = nullptr;
};
