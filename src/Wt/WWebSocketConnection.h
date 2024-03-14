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
#include <mutex>

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
  // Performs an async write to the socket, inside the write-done loop.
  bool doAsyncWrite(OpCode type, const std::vector<char>& frameHeader, const std::vector<char>& data = {});
  // Performs an async write to the socket, outside the write-done loop.
  // This will schedule the write to happen on the strand, which will execute ones the socket is no longer
  // busy writing (or "immediately" if it isn't busy. This will not trigger the callback (hasDataWrittenCallback_)
  // so the WWebsocketResource isn't aware of this frame being sent, since it will not be notified.
  void doControlFrameWrite(const std::vector<char>& frameHeader, OpCode opcode);

  void setDataReadCallback(const std::function<void()>& callback);
  void setDataWrittenCallback(const std::function<void(const AsioWrapper::error_code&, std::size_t)>& callback);

  WebSocketFrameHeader* header() const { return header_.get(); }
  const WStringStream& dataBuffer() const { return dataBuffer_; }

  bool isOpen();

protected:
  AsioWrapper::asio::io_service& ioService_;
  AsioWrapper::asio::io_service::strand strand_;

  void handleAsyncRead(const AsioWrapper::error_code& e, std::size_t bytes_transferred);
  void handleAsyncWritten(OpCode type, const AsioWrapper::error_code& e, std::size_t bytes_transferred);

  Buffer readBuffer_;
  char *readBufferPtr_; // first free byte of readBuffer_ when async read was started

  virtual void doSocketRead(char* buffer, size_t size) = 0;
  // Performs an async write to the socket, inside the write-done loop.
  virtual void doSocketWrite(const std::vector<boost::asio::const_buffer>& buffer, OpCode type) = 0;

private:
  ReadingState readingState_;
  std::unique_ptr<WebSocketFrameHeader> header_;

  WStringStream dataBuffer_;

  // Continuation skipping & size
  bool isContinuation_;
  std::size_t continuationSize_;

  // Socket state, part of the regular flow (write - done).
  bool isWriting_;

  // Socket state, part of the control system, outside the normal flow.
  bool isWritingToSocket_;
  // Socket writing mutex, guarding (control) writing state, which can
  // be set by sendMessage or ping-pong frames.
  // It guards:
  //  - isWritingToSocket_
  //  - hasPendingDataWrite_
  //  - hasPendingPingWrite_
  //  - hasPendingPingWrite_
  //  - pendingWriteDataType_
  //  - pendingWritePingBuffer_
  //  - pendingWritePongBuffer_
  //  - pendingWriteDataBuffer_
  std::mutex writingMutex_;

  // Writing buffer. Necessary for delayed frames, so that data isn't lost,
  // since `const_buffer` does NOT own its underlying data.
  WStringStream writeBuffer_;
  // Temporary writing storage, when sending collides with other socket
  // operation. The type will indicate the delayed request type.
  // In case of Ping or Pong, the respective buffer will be used.
  // In case of other frames, the data buffer will be used.
  bool hasPendingDataWrite_;
  bool hasPendingPingWrite_;
  bool hasPendingPongWrite_;
  OpCode pendingWriteDataType_;
  std::vector<boost::asio::const_buffer> pendingWritePingBuffer_;
  std::vector<boost::asio::const_buffer> pendingWritePongBuffer_;
  std::vector<boost::asio::const_buffer> pendingWriteDataBuffer_;

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
  void doSocketWrite(const std::vector<boost::asio::const_buffer>& buffer, OpCode type) final;

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
  void doSocketWrite(const std::vector<boost::asio::const_buffer>& buffer, OpCode type) final;

private:
  std::unique_ptr<SSLSocket> socket_;

  void stopTcpSocket(const AsioWrapper::error_code& e);
};
#endif

class WT_API WWebSocketConnection : public WObject
{
public:
  WWebSocketConnection(WWebSocketResource* resource, AsioWrapper::asio::io_service& ioService);

  virtual ~WWebSocketConnection();

  virtual void handleMessage(const std::string& text);
  virtual void handleMessage(const std::vector<char>& buffer);

  virtual bool sendMessage(const std::string& text);
  virtual bool sendMessage(const std::vector<char>& buffer);

  virtual bool close(CloseCode code, const std::string& reason = "");
  virtual void acknowledgeClose(const std::string & reason = "");

  virtual bool sendPing();
  virtual void acknowledgePing();
  virtual void handlePong();

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

  // Send an empty frame INSIDE the write-done loop (blocking async write & done() signal)
  bool sendEmptyFrame(OpCode opcode);
  // Send an empty frame OUTSIDE the write-done loop (blocking async write & done() signal)
  bool sendControlFrame(OpCode opcode);
  bool sendDataFrame(const std::vector<char>& buffer, OpCode opcode);

  void writeFrame(const AsioWrapper::error_code& e, std::size_t bytes_transferred);
  void receiveFrame();

  void startPingTimer();
  void doSendPing(const AsioWrapper::error_code& e);
  void missingPong(const AsioWrapper::error_code& e);
  void closeSocket(const AsioWrapper::error_code& e, const std::string& reason);

  WApplication* app();

  bool wantsToClose_;

  std::size_t frameSize_;
  std::size_t messageSize_;

  bool takesUpdateLock_;

  int pingInterval_;
  int pingTimeout_;

  AsioWrapper::asio::steady_timer pingSignalTimer_;
  AsioWrapper::asio::steady_timer pongTimeoutTimer_;

  WStringStream continuationBuffer_;
  OpCode continuationOpCode_;

  friend class WebSocketHandlerResource;
};
}
#endif // WT_WWEBSOCKETCONNECTION_H_
