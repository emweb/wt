#include "WebSocketApplication.h"

#include "../mywebsocket.h"

#include "Wt/Utils.h"
#include "Wt/WApplication.h"
#include "Wt/WBootstrap5Theme.h"
#include "Wt/WGlobal.h"
#include "Wt/WWebSocketConnection.h"
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WTimer.h>
#include <Wt/WWebSocketResource.h>

#include <chrono>
#include <string>

WebSocketApplication::WebSocketApplication(const Wt::WEnvironment& env)
  : Wt::WApplication(env)
{
  enableUpdates(true);
  setTheme(std::make_shared<Wt::WBootstrap5Theme>());

  createResource();

  createLayout();

  createOutputField();
  createWebSocketListeners();
  createWebSocketField();

  createDataFields();
  createActionButtons();
}

WebSocketApplication::~WebSocketApplication()
{
}

void WebSocketApplication::createResource()
{
  websocketResource_.reset(new MyWebSocketResource());
}

void WebSocketApplication::createLayout()
{
  templ_ = root()->addNew<Wt::WTemplate>("<div class='container'>"
                                         "  <div class='row'>"
                                         "    <div class='col-xs-10'>"
                                         "      ${connect}${disconnect}${serverClose}"
                                         "      <p>A failure to connect will be logged in the browser's console.</p>"
                                         "      <p class='bold'>Only a single connection at a time is supported.</p>"
                                         "    </div>"
                                         "  </div>"
                                         "  <div class='row'>"
                                         "    <div class='col-xs-10'>"
                                         "      Note that while the WebSocket connection's maximum frame/message size will depend"
                                         "      on the limits set on the <a href='https://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1WWebSocketResource.html'>WWebSocketResource</a>"
                                         "      the message that is sent to the server to push the information to the socket will still"
                                         "      contain the data entered in the fields here. If this becomes very large, it will encounter the"
                                         "      limited size of a request (max-memory-request-size, max-request-size)"
                                         "      which can be set in the <code>wt_config.xml</code>, or passed as an argument."
                                         "    </div>"
                                         "    <div class='col-xs-10' style='height:320px'>"
                                         "      <h3>Output:</h3>"
                                         "      <div class='scrollItem border' style='height:280px; overflow-y:scroll;'>"
                                         "        ${output}"
                                         "      </div>"
                                         "    </div>"
                                         "  </div>"
                                         "  <div class='row'>"
                                         "    <div class='col-xs-4 mt-4'>"
                                         "      <h3>Client pushes:</h3>"
                                         "      <p>This field will make the client (this web browser) "
                                         "      push data to the server.</p>"
                                         "      ${dataInput}"
                                         "    </div>"
                                         "    <div class='col-xs-4 mt-2'>"
                                         "      ${sendText}${sendBinary}"
                                         "    </div>"
                                         "  </div>"
                                         "  <div class='row mt-5'>"
                                         "    <div class='col-xs-4 mt-4'>"
                                         "      <h3>Server pushes:</h3>"
                                         "      <p>Instead of making the client push info to the server, "
                                         "      the server can also push data to the client.</p>"
                                         "      <p><strong>Note:</strong> This does not produce an output in the above"
                                         "      field, but prints the received messages to the browser console.</p>"
                                         "      ${serverDataInput}"
                                         "    </div>"
                                         "    <div class='col-xs-4 mt-2'>"
                                         "      ${sendServerText}${sendServerBinary}"
                                         "    </div>"
                                         "  </div>"
                                         "</div>");
}

void WebSocketApplication::createOutputField()
{
  output_ = templ_->bindNew<Wt::WText>("output");
  output_->setTextFormat(Wt::TextFormat::XHTML);
}

void WebSocketApplication::createWebSocketListeners()
{
  websocketResource_->newConnectionMade().connect(this, &WebSocketApplication::addWebSocketConnection);
}

void WebSocketApplication::addWebSocketConnection(MyWebSocketConnection* connection)
{
  isConnected_ = true;
  connect_->disable();
  disconnect_->enable();
  serverDisconnect_->enable();
  connection_ = connection;

  connection_->textMessageReceived().connect([this](const std::string& text) {
    std::string values = text;
    if (values.size() > 75) {
      auto length  = values.size();
      values = values.substr(0, 75);
      values += " ... (length: " + std::to_string(length) + ")";
    }
    addToOutput("SERVER received: " + values + "\n");
  });

  connection_->binaryMessageReceived().connect([this](std::vector<char> data) {
    std::string values;
    for (char& c : data) {
      values += std::to_string(c) + " ";

      if (values.size() > 75) {
        auto length  = data.size();
        values = values.substr(0, 75);
        values += " ... (length: " + std::to_string(length) + ")";
        break;
      }
    }
    addToOutput("SERVER received: " + values + "\n");
  });

  connection_->pingSent().connect([this] {
    std::unique_ptr<Wt::WApplication::UpdateLock> updateLock(new Wt::WApplication::UpdateLock(this));
    if (*updateLock.get()) {
      addToOutput("SERVER sent ping\n");
      updateLock.reset(nullptr);
    }
  });

  connection_->pongSent().connect([this] {
    std::unique_ptr<Wt::WApplication::UpdateLock> updateLock(new Wt::WApplication::UpdateLock(this));
    if (*updateLock.get()) {
      addToOutput("SERVER sent pong\n");
      updateLock.reset(nullptr);
    }
  });

  std::string newText = "New connection was made to " + Wt::Utils::htmlEncode(websocketResource_->url());
  addToOutput(newText);

  connection->closed().connect(this, &WebSocketApplication::connectionClosed);
}

void WebSocketApplication::createWebSocketField()
{
  connect_ = templ_->bindNew<Wt::WPushButton>("connect", "Connect");
  disconnect_ = templ_->bindNew<Wt::WPushButton>("disconnect", "Disconnect");
  disconnect_->disable();
  serverDisconnect_ = templ_->bindNew<Wt::WPushButton>("serverClose", "Force server close");
  serverDisconnect_->disable();

  connect_->clicked().connect(this, &WebSocketApplication::connectWebSocket);
  disconnect_->clicked().connect(this, &WebSocketApplication::disconnectWebSocket);
  serverDisconnect_->clicked().connect(this, &WebSocketApplication::serverDisconnectWebSocket);
}

void WebSocketApplication::createDataFields()
{
  wsData_ = templ_->bindNew<Wt::WLineEdit>("dataInput");
  wsData_->setPlaceholderText("Input WS data");
  wsData_->setTextSize(100);

  wsServerData_ = templ_->bindNew<Wt::WLineEdit>("serverDataInput");
  wsServerData_->setPlaceholderText("Input WS data");
  wsServerData_->setTextSize(100);
}

void WebSocketApplication::createActionButtons()
{
  auto dataBtn = templ_->bindNew<Wt::WPushButton>("sendText", "Send text TO server");
  auto binaryDataBtn = templ_->bindNew<Wt::WPushButton>("sendBinary", "Send binary TO server");

  dataBtn->clicked().connect(this, &WebSocketApplication::sendTextData);
  binaryDataBtn->clicked().connect(this, &WebSocketApplication::sendBinaryData);

  auto serverDataBtn = templ_->bindNew<Wt::WPushButton>("sendServerText", "Send text FROM server");
  auto serverBinaryDataBtn = templ_->bindNew<Wt::WPushButton>("sendServerBinary", "Send binary FROM server");

  serverDataBtn->clicked().connect(this, &WebSocketApplication::sendServerTextData);
  serverBinaryDataBtn->clicked().connect(this, &WebSocketApplication::sendServerBinaryData);
}

void WebSocketApplication::addToOutput(const std::string& value)
{
  const Wt::WString outputValue = Wt::WString(value, Wt::CharEncoding::UTF8);
  if (!output_->text().empty()) {
    output_->setText(output_->text() + "<br/>" + outputValue);
  } else {
    output_->setText(outputValue);
  }

  doJavaScript("var item = document.getElementsByClassName('scrollItem border')[0]; item.scrollTop = item.scrollHeight;");
  triggerUpdate();
}

void WebSocketApplication::connectWebSocket()
{
  connect_->disable();
  const std::string urlScheme = environment().urlScheme();
  std::string webSocketProtocol;
  if (urlScheme == "http" || urlScheme == "ws") {
    webSocketProtocol = "ws";
  } else {
    webSocketProtocol = "wss";
  }
  const std::string host = environment().hostName();

  const std::string webSocketEndpoint = webSocketProtocol + "://" + host + "/" + websocketResource_->url();

  doJavaScript("window.webSockConn = new WebSocket('" + webSocketEndpoint + "');");
  doJavaScript("window.webSockConn.onopen = () => { console.log('CONNECTION OPENED'); };");
  doJavaScript("window.webSockConn.onclose = () => { console.log('CONNECTION CLOSED'); };");
  doJavaScript("window.webSockConn.onerror = (e) => { console.log('ERROR: '); console.log(e); };");
  doJavaScript("window.webSockConn.onmessage = (e) => { let length = e.data.length; let byteLength = e.data.byteLength; let size = e.data.size; console.log('Message received. Depending on the WebSocket type the length is either: ' + length + ' for blob or, ' + size + ' for arraybuffer (binary). Contents: ' + e.data); };");
}

void WebSocketApplication::disconnectWebSocket()
{
  doJavaScript("window.webSockConn.close();");
}

void WebSocketApplication::serverDisconnectWebSocket()
{
  connection_->close(Wt::CloseCode::Normal, "This has gone on long enough. Stop it!");
}

void WebSocketApplication::connectionClosed()
{
  isConnected_ = false;
  connect_->enable();
  disconnect_->disable();
  serverDisconnect_->disable();
  connection_ = nullptr;
  std::string newText = "Connection to " + Wt::Utils::htmlEncode(websocketResource_->url()) + " was closed.";
  addToOutput(newText);
}

bool WebSocketApplication::checkConnection()
{
  if (!isConnected_) {
    addToOutput("<strong>Not connected</strong>");
    return false;
  }

  return true;
}

void WebSocketApplication::sendTextData()
{
  if (!checkConnection()) {
    return;
  }

  std::string text = wsData_->text().toUTF8();

  doJavaScript("window.webSockConn.send('" + Wt::Utils::htmlEncode(text) + "');");

  if (text.size() > 75) {
    auto length = text.size();
    text = text.substr(0, 75);
    text += " ... (length: " + std::to_string(length) + ")";
  }

  addToOutput("Sending text: " + text);
}

void WebSocketApplication::sendBinaryData()
{
  if (!checkConnection()) {
    return;
  }

  std::string text = wsData_->text().toUTF8();

  doJavaScript("if (window.encoder === undefined) { window.encoder = new TextEncoder(); }"
               "window.webSockConn.send(window.encoder.encode('" + Wt::Utils::htmlEncode(text) + "'));");

  std::string binaryText;
  for (char& c : text) {
    auto length = text.size();
    binaryText += std::to_string(c) + " ";

    if (binaryText.size() > 75) {
      binaryText += " ... (length: " + std::to_string(length) + ")";
      break;
    }
  }

  addToOutput("Sending binary: " + binaryText);
}

void WebSocketApplication::sendServerTextData()
{
  if (!checkConnection()) {
    return;
  }

  std::string text = wsServerData_->text().toUTF8();

  connection_->sendMessage(text);
}

void WebSocketApplication::sendServerBinaryData()
{
  if (!checkConnection()) {
    return;
  }

  std::string text = wsServerData_->text().toUTF8();
  std::vector<char> data(text.begin(), text.end());

  connection_->sendMessage(data);
}

