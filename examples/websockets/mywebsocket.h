#ifndef MYWEBSOCKET_H_
#define MYWEBSOCKET_H_

#include <Wt/AsioWrapper/system_error.hpp>

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <Wt/Utils.h>
#include <Wt/WDateTime.h>
#include <Wt/WServer.h>
#include <Wt/WWebSocketResource.h>
#include <Wt/WWebSocketConnection.h>

class MyWebSocketConnection final : public Wt::WWebSocketConnection
{
public:
  MyWebSocketConnection(Wt::WWebSocketResource* resource, Wt::AsioWrapper::asio::io_service& ioService)
    : Wt::WWebSocketConnection(resource, ioService)
  {
  }

  void handleMessage(const std::string& text) final
  {
    Wt::WWebSocketConnection::handleMessage(text);
    textMessageReceived_.emit(text);
  }

  void handleMessage(const std::vector<char>& data) final
  {
    Wt::WWebSocketConnection::handleMessage(data);
    binaryMessageReceived_.emit(data);
  }

  bool sendPing() final
  {
    bool val = Wt::WWebSocketConnection::sendPing();
    pingSent_.emit();
    return val;
  }

  void handlePong() final
  {
    Wt::WWebSocketConnection::handlePong();
    pongSent_.emit();
  }

  Wt::Signal<const std::string&>& textMessageReceived() { return textMessageReceived_; }
  Wt::Signal<std::vector<char>>& binaryMessageReceived() { return binaryMessageReceived_; }
  Wt::Signal<>& pingSent() { return pingSent_; }
  Wt::Signal<>& pongSent() { return pongSent_; }

private:
  Wt::Signal<const std::string&> textMessageReceived_;
  Wt::Signal<std::vector<char>> binaryMessageReceived_;
  Wt::Signal<> pingSent_;
  Wt::Signal<> pongSent_;
};

class MyWebSocketResource final : public Wt::WWebSocketResource
{
public:
  std::unique_ptr<Wt::WWebSocketConnection> handleConnect(const Wt::Http::Request& request) final
  {
    auto connection = std::make_unique<MyWebSocketConnection>(this, Wt::WServer::instance()->ioService());
    newConnectionMade_.emit(connection.get());
    return connection;
  }

  Wt::Signal<MyWebSocketConnection*>& newConnectionMade() { return newConnectionMade_; }

private:
  Wt::Signal<MyWebSocketConnection*> newConnectionMade_;
};
#endif // MYWEBSOCKET_H_
