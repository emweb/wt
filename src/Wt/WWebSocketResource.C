#include "Wt/WWebSocketResource.h"

#include "Wt/WApplication.h"
#include "Wt/WException.h"
#include "Wt/WLogger.h"
#include "Wt/WRandom.h"
#include "Wt/WWebSocketConnection.h"

#include "Wt/Http/Request.h"
#include "Wt/Http/Response.h"

namespace Wt {
LOGGER("WWebSocketResource");

const std::string KEY_ACCEPT_APPEND = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

WebSocketHandlerResource::WebSocketHandlerResource(WWebSocketResource* resource)
  : resource_(resource)
{
}

WebSocketHandlerResource::~WebSocketHandlerResource()
{
  beingDeleted();
}

void WebSocketHandlerResource::handleRequest(const Http::Request& request, Http::Response& response)
{
  response.setStatus(101);

  response.insertHeader("Content-Length", "0");
  response.insertHeader("Upgrade", "websocket");
  response.insertHeader("Connection", "Upgrade");
  response.insertHeader("Sec-Websocket-Version", "13");

  // No extensions
  response.insertHeader("Sec-Websocket-Protocol", "");
  response.insertHeader("Sec-Websocket-Extensions", "");
}

void WebSocketHandlerResource::moveSocket(const Http::Request& request, const std::shared_ptr<WebSocketConnection>& socketConnection)
{
  std::unique_ptr<WApplication::UpdateLock> updateLock;
  if (resource_ && resource_->takesUpdateLock() && resource_->app_) {
    updateLock.reset(new Wt::WApplication::UpdateLock(resource_->app_));
  }

  auto connection = resource_->handleConnect(request);
  updateLock.reset(nullptr);

  connection->setSocket(socketConnection);
  resource_->registerConnection(std::move(connection));
}

WWebSocketResource::WWebSocketResource()
  : frameSize_(10485760), // 1024 * 1024 * 10 => 10MB
    messageSize_(52428800), // 1024 * 1024 * 50 => 50MB
    takesUpdateLock_(true),
    pingInterval_(180),
    pingTimeout_(360)
{
  resource_ = std::make_shared<WebSocketHandlerResource>(this);

  WApplication* app = WApplication::instance();
  if (app) {
    app_ = app;
    app->addWebSocketResource(this);
  }
}

WWebSocketResource::~WWebSocketResource()
{
  std::unique_lock<std::recursive_mutex> lock(clientsMutex_);
  for (auto& connection : clients_) {
    connection.reset(nullptr);
  }
}

WWebSocketConnection* WWebSocketResource::registerConnection(std::unique_ptr<WWebSocketConnection> connection)
{
  // Pass set-up
  connection->setMaximumReceivedSize(frameSize_, messageSize_);
  connection->setTakesUpdateLock(takesUpdateLock_);
  connection->setPingTimeout(pingInterval_, pingTimeout_);

  {
    std::unique_lock<std::recursive_mutex> lock(clientsMutex_);
    LOG_INFO("A new connection was made.");
    clients_.push_back(std::move(connection));
  }
  return clients_.back().get();
}

void WWebSocketResource::shutdown()
{
  std::unique_lock<std::recursive_mutex> lock(clientsMutex_);
  for (auto& client : clients_) {
    client.reset(nullptr);
  }
}

void WWebSocketResource::setInternalPath(const std::string& path)
{
  resource_->setInternalPath(path);
}

std::string WWebSocketResource::internalPath() const
{
  return resource_->internalPath();
}

std::string WWebSocketResource::url() const
{
  return resource_->url();
}

void WWebSocketResource::setMaximumReceivedSize(size_t frame, size_t message)
{
  frameSize_ = frame;
  messageSize_ = message;
}

void WWebSocketResource::setTakesUpdateLock(bool canTake)
{
  takesUpdateLock_ = canTake;
}

void WWebSocketResource::setPingTimeout(int intervalSeconds, int timeoutSeconds)
{
  pingInterval_ = intervalSeconds;
  pingTimeout_ = timeoutSeconds;
}
}
