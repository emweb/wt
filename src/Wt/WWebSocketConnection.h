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

/*! \class OpCode Wt/WWebSocketConnection.h
 *  \brief Indicates the type of the frame/message.
 *
 * This enumeration contains all possible types for a WebSocket frame.
 */
enum class OpCode
{
  /*! Indicates that the content is the same as a previous frame, and
   * that its data needs to be appended to the previous frame.
   */
  Continuation = 0,
  //! Indicates a text message. The data of the frame is a string.
  Text = 1,
  //! Indicates a binary message. The data of the frame is binary.
  Binary = 2,
  /*! Indicates a close message. The data of the frame is an optional
   * string.
   */
  Close = 8,
  //! Indicates a ping message.
  Ping = 9,
  //! Indicates a pong message.
  Pong = 10,
};

/*! \class CloseCode Wt/WWebSocketConnection.h
 *  \brief Indicates the generic code for the close message.
 *
 * This contains all possible codes for a close message.
 */
enum class CloseCode
{
  Normal = 1000, //!< The normal code, this is the most-often used code.
  GoingAway = 1001, //!< Indicates that the server is going down.
  ProtocolError = 1002, //<! Termination due to a protocol error.
  UnexpectedDataType = 1003, //<! The data type cannot be handled.
  Reserved = 1004, //<! Reserved, not to be used.
  NoStatusCode = 1005, //!< Reserved, to be used by applications.
  AbnormalClose = 1006, //!< Reserved, to be used by applications.
  IncorrectData = 1007, //!< Received data is not consistent with type.
  PolicyError = 1008, //!< Generic code for any policy violations.
  MessageTooLarge = 1009, //!< Termination when received message is too big.
  FailedExtensionNegotiation = 1010, //!< Termination by client due to expected extension.
  UnexpectedCondition = 1011, //!< Termination by server due to unexpected condition.
  TLSFailure = 1015 //!< Reserved, to be used by applications.
};

struct WebSocketFrameHeader;

enum class ReadingState
{
  ReadingHeader = 0,
  ReadingData = 1,
  ClosingSocket = 2,
  SkipData = 3,
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

  void setMaximumReceivedFrameSize(std::size_t frameSize);
  void setMaximumReceivedMessageSize(std::size_t messageSize);

  bool skipMessage() const { return skipMessage_; }

protected:
  AsioWrapper::asio::io_service& ioService_;
  AsioWrapper::asio::io_service::strand strand_;

  void handleAsyncRead(const AsioWrapper::error_code& e, std::size_t bytes_transferred);
  void handleAsyncWritten(OpCode type, const AsioWrapper::error_code& e, std::size_t bytes_transferred);

  Buffer readBuffer_;
  Buffer::iterator readBufferPtr_; // first free byte of readBuffer_ when async read was started

  std::size_t skipDataSize_;
  std::size_t frameSize_;
  std::size_t messageSize_;

  virtual void doSocketRead(char* buffer, size_t size) = 0;
  // Performs an async write to the socket, inside the write-done loop.
  virtual void doSocketWrite(const std::vector<AsioWrapper::asio::const_buffer>& buffer, OpCode type) = 0;

private:
  // Buffer & parsing
  ReadingState readingState_;
  bool skipMessage_;
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
  std::vector<AsioWrapper::asio::const_buffer> pendingWritePingBuffer_;
  std::vector<AsioWrapper::asio::const_buffer> pendingWritePongBuffer_;
  std::vector<AsioWrapper::asio::const_buffer> pendingWriteDataBuffer_;

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
  void doSocketWrite(const std::vector<AsioWrapper::asio::const_buffer>& buffer, OpCode type) final;

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
  void doSocketWrite(const std::vector<AsioWrapper::asio::const_buffer>& buffer, OpCode type) final;

private:
  std::unique_ptr<SSLSocket> socket_;

  void stopTcpSocket(const AsioWrapper::error_code& e);
};
#endif

/*! \class WWebSocketConnection Wt/WWebSocketConnection.h
 *  \brief A single connection to a WWebSocketResource.
 *
 * When a connection is set-up to a WWebSocketResource, a new connection
 * is created, which manages the underlying TCP or SSL stream.
 *
 * Upon its creation, it will inherit the setting currently found on the
 * WWebSocketResource. Like the maximum frame and message sizes, the
 * settings for the ping-pong system and whether it takes the application
 * update lock.
 *
 * The connection can be used to listen to incoming requests, or send out
 * messages after the WebSocket set-up has occurred. It also offers
 * methods to manage the connection, like closing it gracefully.
 *
 * Each of the methods that either read an incoming message, or respond
 * to a message are made virtual so that developers can change the
 * implementation slightly, if they desire. This will be limited to being
 * able to log, track, or manipulate the data, but not change the way the
 * framework will create frames or messages.
 *
 * Most socket, and data management is still handled by the framework.
 * That is, everything to do with any of the above mentioned methods,
 * copied from WWebSocketResource, is handled by the framework. A
 * developer will thus never be able to receive a message or frame
 * greater than the imposed limit, unless they increase it, or disable
 * it. If the setTakesUpdateLock() is set to \p true, this lock
 * acquisition will always take place. These settings can always be
 * changed after the connection has been set up.
 */
class WT_API WWebSocketConnection : public WObject
{
public:
  //! Constructor, pointing to its resource and the IO service.
  WWebSocketConnection(WWebSocketResource* resource, AsioWrapper::asio::io_service& ioService);

  //! Destructor.
  virtual ~WWebSocketConnection();

  /*! \brief Handles an incoming text message.
   *
   * A single message (which can consist of multiple frames) has been
   * received, of the text type. The \p text parameter contains the data
   * that was present in the message.
   */
  virtual void handleMessage(const std::string& text);

  /*! \brief Handles an incoming binary message.
   *
   * A single message (which can consist of multiple frames) has been
   * received, of the binary type. The \p buffer parameter contains the
   * data that was present in the message.
   */
  virtual void handleMessage(const std::vector<char>& buffer);

  /*! \brief Sends out a text message.
   *
   * This will create a single text message that is to be sent to the
   * client. The \p text data will be added to the message. The message
   * is sent out as a single frame. This method can only be called in a
   * sequential manner. Meaning that after each send event, a done() will
   * be called, and no other send can be executed before the done()
   * signal has been emitted. If this is called before done() fired, and
   * thus when the previous message is still being written, the function
   * will return \p false, and a warning will be logged.
   *
   * Upon success, this will return \p true. This does not indicate that
   * the message has been actually sent, but that it has been successfully
   * queued. When the message has been written to the stream, done() will
   * be called. When done() is fired, the queued message has been
   * handled. This can mean that is has been successfully written, or
   * that and error has occurred. If an error is attached to the done()
   * signal, an error has occurred during sending, and the stream is
   * potentially useless, depending on the type of error.
   */
  virtual bool sendMessage(const std::string& text);

  /*! \brief Sends out a binary message.
   *
   * This will create a single binary message that is to be sent to the
   * client. The \p buffer data will be added to the message. The message
   * is sent out as a single frame. This method can only be called in a
   * sequantial manner. Meaning that after each send event, a done() will
   * be called, and no other send can be executed before the done()
   * signal has been emitted. If this is called before done() fired, and
   * thus when the previous message is still being written, the function
   * will return \p false, and a warning will be logged.
   *
   * Upon success, this will return \p true. This does not indicate that
   * the message has been actually sent, but that it has been successfully
   * queued. When the message has been written to the stream, done() will
   * be called. When done() is fired, the queued message has been
   * handled. This can mean that is has been successfully written, or
   * that and error has occurred. If an error is attached to the done()
   * signal, an error has occurred during sending, and the stream is
   * potentially useless, depending on the type of error.
   */
  virtual bool sendMessage(const std::vector<char>& buffer);

  /*! \brief Send the close signal to the client.
   *
   * This will close the connection in a graceful manner. After the
   * closing frame is sent out, the connection waits for it to be
   * acknowledged by the client. Once this has been received, the
   * connection will be actually terminated.
   *
   * When sending this message, the \p code will indicate a generic
   * cause for termination. Generally CloseCode::Normal will be most
   * often used. An optional \p reason can be provided to the client
   * as an additional specification to the generic code.
   *
   * This method is also considered a sending event, meaning that it
   * also blocks an additional writing events, until done() has been
   * emitted.
   */
  virtual bool close(CloseCode code, const std::string& reason = "");

  /*! \brief Acknowledges a received close frame.
   *
   * A client has sent out a close frame, to which the server responds
   * with an identical message. It sends out a frame with the same code
   * and optionally the same \p reason.
   *
   * Since this is a final response, it does not fall within the normal
   * sending/done() logic. It does still write a frame to the client, but
   * the response here is not important. The done() signal will be fired,
   * and used internally, eventually firing the closed() signal. Which
   * is used to indicate that the stream will be closed down and is to be
   * considered unusable from now on.
   */
  virtual void acknowledgeClose(const std::string & reason = "");

  /*! \brief Sends out a ping message.
   *
   * This sends out a ping message to the connected client. This will
   * happen every \p x seconds where \p x is set by setPingTimeout().
   */
  virtual bool sendPing();

  /*! \brief Acknowlegdes a received ping message.
   *
   * The client has sent out a ping message. The server now needs to
   * respond with a pong.
   */
  virtual void acknowledgePing();

  /*! \brief Handles an incoming pong message.
   *
   * A pong message has been sent by the client, meaning that earlier
   * the server has sent out a ping message with sendPing().
   *
   * This ensures that the ping-pong mechanism is complete. When this
   * method is called, this indicates a full cycle of the ping-pong
   * logic. This will set up the timer (if it is enabled) again.
   */
  virtual void handlePong();

  /*! \brief Handles an incoming bad frame.
   *
   * A frame was received that was not expected. Currently this is only
   * used for when a continuation frame is caught, but no initial frame
   * to which the continuation should apply, was received first.
   */
  virtual void handleError();

  /*! \brief Sets the maximum received frame and message size.
   *
   * \sa WWebSocketResource::setMaximumReceivedSize
   */
  void setMaximumReceivedSize(std::size_t frameSize, std::size_t messageSize);

  /*! \brief Sets the application update lock needs to be taken on
   * sending or receiving messages.
   *
   * \sa WWebSocketResource::setTakesUpdateLock
   */
  void setTakesUpdateLock(bool takesUpdateLock);

  /*! \brief Sets the ping-pong configuration.
   *
   * \sa WWebSocketResource::setPingTimeout
   */
  void setPingTimeout(int pingInterval, int pingTimeout);

  /*! \brief Signal indicating a sending event has been completed.
   *
   * The error code it returns either does not exists, indicating a
   * successful write. Or it can exist, and will then have a value.
   * The number of the value signifies which type of error has occurred.
   * This can leave the underlying stream useless.
   */
  Signal<AsioWrapper::error_code>& done() { return done_; }

  /*! \brief Signal indicating the connection has been closed.
   *
   * The error code it returns either does not exists, indicating a
   * successful close. Or it can exist, and will then have a value.
   * The number of the value signifies which type of error has occurred.
   * Even if an error occurs the underlying stream should never be used
   * again, since it is very likely the client has already closed the
   * stream from their end.
   *
   * The string returned can hold an optional message, which specifies
   * why the connection was closed.
   */
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
