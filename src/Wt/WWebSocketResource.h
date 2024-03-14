#ifndef WT_WWEBSOCKETRESOURCE_H_
#define WT_WWEBSOCKETRESOURCE_H_

#include "Wt/AsioWrapper/asio.hpp"

#include "Wt/Http/Request.h"
#include "Wt/Http/Response.h"

#include "Wt/WResource.h"

#include <mutex>

namespace http {
  namespace server {
    class Connection;
  }
}

namespace Wt {
class WebSocketConnection;
class WWebSocketConnection;
class WWebSocketResource;

class WebSocketHandlerResource final : public WResource
{
public:
  explicit WebSocketHandlerResource(WWebSocketResource* resource);
  ~WebSocketHandlerResource();

  void moveSocket(const Http::Request& request, const std::shared_ptr<WebSocketConnection>& socketConnection);

protected:
  void handleRequest(const Http::Request& request, Http::Response& response) final;

private:
  WWebSocketResource* resource_ = nullptr;
  WWebSocketConnection* moveTo_;
};

class WT_API WWebSocketResource : public WObject
{
public:
  WWebSocketResource();

  ~WWebSocketResource();

  virtual std::unique_ptr<WWebSocketConnection> handleConnect(const Http::Request& request) = 0;
  virtual void shutdown();

  void setInternalPath(const std::string& path);
  std::string internalPath() const;
  std::string url() const;

  std::shared_ptr<WebSocketHandlerResource> handleResource() const { return resource_; }

  void setMaximumSize(size_t frame, size_t message);
  void setTakesUpdateLock(bool canTake);
  void setPingTimeout(int intervalSeconds, int timeoutSeconds);

  size_t maximumReceivedFrameSize() const { return frameSize_; }
  size_t maximumReceivedMessageSize() const { return messageSize_; }

  bool takesUpdateLock() const { return takesUpdateLock_; }
  int pingInterval() const { return pingInterval_; }
  int pingTimeout() const { return pingTimeout_; }

  WApplication* app() { return app_; }

private:
  std::shared_ptr<WebSocketHandlerResource> resource_;

  std::recursive_mutex clientsMutex_;
  std::vector<std::unique_ptr<WWebSocketConnection>> clients_;

  WWebSocketConnection* registerConnection(std::unique_ptr<WWebSocketConnection> connection);

  size_t frameSize_;
  size_t messageSize_;

  bool takesUpdateLock_;

  int pingInterval_;
  int pingTimeout_;

  WApplication* app_ = nullptr;

  friend class WebSocketHandlerResource;
  friend class WWebSocketConnection;
};
}
#endif // WT_WWEBSOCKETRESOURCE_H_
