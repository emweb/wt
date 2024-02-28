#ifndef WT_WWEBSOCKETCONNECTION_H_
#define WT_WWEBSOCKETCONNECTION_H_

#include "Wt/AsioWrapper/asio.hpp"
#ifdef WT_WITH_SSL
#include "Wt/AsioWrapper/ssl.hpp"
#endif
#include "Wt/AsioWrapper/system_error.hpp"

#include "Wt/WObject.h"
#include "Wt/WIOService.h"
#include "Wt/WSignal.h"
#include "Wt/WStringStream.h"
#include <functional>

namespace Wt {

class WebSocketHandlerResource;
class WWebSocketResource;

using Buffer = std::array<char, 8192>;
using Socket = AsioWrapper::asio::ip::tcp::socket;
#ifdef WT_WITH_SSL
using SSLSocket = AsioWrapper::asio::ssl::stream<AsioWrapper::asio::ip::tcp::socket>;
#endif

enum class OpCode
{
  Continuation = 0,
  Text = 1,
  Binary = 2,
  Close = 8,
  Ping = 9,
  Pong = 10,
};

enum class CloseCode
{
  Normal = 1000,
  GoingAway = 1001,
  ProtocolError = 1002,
  UnexpectedDataType = 1003,
  Reserved = 1004,
  NoStatusCode = 1005, //
  AbnormalClose = 1006, //
  IncorrectData = 1007,
  PolicyError = 1008,
  MessageTooLarge = 1009,
  FailedExtensionNegotiation = 1010,
  UnexpectedCondition = 1011,
  TLSFailure = 1015 //
};

struct WebSocketFrameHeader;

enum class ReadingState
{
  ReadingHeader = 0,
  ReadingData = 1,
  ClosingSocket = 2,
};

class WT_API WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection>
{
public:
  WebSocketConnection(AsioWrapper::asio::io_service& ioService);

  virtual ~WebSocketConnection();

  virtual Socket& socket() = 0;

  virtual void doClose() = 0;

  void startReading();
  bool doAsyncWrite(const std::vector<char>& frameHeader, const std::vector<char>& data = {});

  void setDataReadCallback(const std::function<void()>& callback);
  void setDataWrittenCallback(const std::function<void(const AsioWrapper::error_code&, std::size_t)>& callback);

  WebSocketFrameHeader* header() const { return header_.get(); }
  const WStringStream& dataBuffer() const { return dataBuffer_; }

  bool isOpen();

protected:
  AsioWrapper::asio::io_service::strand strand_;

  void handleAsyncRead(const AsioWrapper::error_code& e, std::size_t bytes_transferred);
  void handleAsyncWritten(const AsioWrapper::error_code& e, std::size_t bytes_transferred);

  Buffer readBuffer_;
  char *readBufferPtr_; // first free byte of readBuffer_ when async read was started

  virtual void doSocketRead(char* buffer, size_t size) = 0;
  virtual void doSocketWrite(const std::vector<boost::asio::const_buffer>& buffer) = 0;

private:
  ReadingState readingState_;
  std::unique_ptr<WebSocketFrameHeader> header_;

  WStringStream dataBuffer_;

  bool isWriting_;

  std::function<void()> hasDataReadCallback_;
  std::function<void(const AsioWrapper::error_code&, std::size_t)> hasDataWrittenCallback_;

  void doAsyncRead(char* buffer, size_t size);
  std::size_t parseBuffer(const char* begin, const char* end);
  void doEmitAndCleanBuffers();
};

class WT_API WebSocketTcpConnection final : public WebSocketConnection
{
public:
  explicit WebSocketTcpConnection(AsioWrapper::asio::io_service& ioService, std::unique_ptr<Socket> socket);

  ~WebSocketTcpConnection() final;

  Socket& socket() final;

  void doClose() final;

protected:
  void doSocketRead(char* input, size_t offset) final;
  void doSocketWrite(const std::vector<boost::asio::const_buffer>& buffer) final;

private:
  std::unique_ptr<Socket> socket_;
};

#ifdef WT_WITH_SSL
class WT_API WebSocketSslConnection final : public WebSocketConnection
{
public:
  explicit WebSocketSslConnection(AsioWrapper::asio::io_service& ioService, std::unique_ptr<SSLSocket> socket);

  ~WebSocketSslConnection() final;

  Socket& socket() final;

  void doClose() final;

protected:
  void doSocketRead(char* input, size_t offset) final;
  void doSocketWrite(const std::vector<boost::asio::const_buffer>& buffer) final;

private:
  std::unique_ptr<SSLSocket> socket_;

  void stopTcpSocket(const AsioWrapper::error_code& e);
};
#endif

class WT_API WWebSocketConnection : public WObject
{
public:
  WWebSocketConnection(WWebSocketResource* resource);

  virtual ~WWebSocketConnection();

  virtual void handleMessage(const std::string& text);
  virtual void handleMessage(const std::vector<char>& buffer);

  virtual bool sendMessage(const std::string& text);
  virtual bool sendMessage(const std::vector<char>& buffer);

  virtual bool close(CloseCode code, const std::string& reason = "");
  virtual void acknowledgeClose(const std::string & reason = "");

  virtual void handleError();

  void setMaximumReceivedSize(std::size_t frameSize, std::size_t messageSize);

  void setTakesUpdateLock(bool takesUpdateLock);
  void setPingTimeout(int pingInterval, int pingTimeout);

  Signal<AsioWrapper::error_code>& done() { return done_; }
  Signal<AsioWrapper::error_code, const std::string&>& closed() { return closed_; }

private:
  WWebSocketResource* resource_;
  std::shared_ptr<WebSocketConnection> socketConnection_;

  Signal<AsioWrapper::error_code> done_;
  Signal<AsioWrapper::error_code, const std::string&> closed_;

  void setSocket(const std::shared_ptr<WebSocketConnection>& connection);

  bool sendEmptyFrame(OpCode opcode);
  bool sendDataFrame(const std::vector<char>& buffer, OpCode opcode);

  void writeFrame(const AsioWrapper::error_code& e, std::size_t bytes_transferred);
  void receiveFrame();

  void acknowledgePing();
  void closeSocket(const AsioWrapper::error_code& e, const std::string& reason);

  bool wantsToClose_;

  std::size_t frameSize_;
  std::size_t messageSize_;

  bool takesUpdateLock_;

  int pingInterval_;
  int pingTimeout_;

  WStringStream continuationBuffer_;
  OpCode continuationOpCode_;

  friend class WebSocketHandlerResource;
};
}
#endif // WT_WWEBSOCKETCONNECTION_H_
