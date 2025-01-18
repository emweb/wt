#include "Wt/WWebSocketConnection.h"

#include "Wt/AsioWrapper/system_error.hpp"

#include "Wt/WApplication.h"
#include "Wt/WLogger.h"
#include "Wt/WString.h"
#include "Wt/WWebSocketResource.h"

#include <chrono>
#include <system_error>

namespace Wt {
LOGGER("WWebSocketConnection");

std::string OpCodeToString(OpCode code)
{
  switch(code) {
    case OpCode::Continuation:
      return "Continuation";
    case OpCode::Text:
      return "Text";
    case OpCode::Binary:
      return "Binary";
    case OpCode::Close:
      return "Close";
    case OpCode::Ping:
      return "Ping";
    case OpCode::Pong:
      return "Pong";
  }
  return "";
}

struct WebSocketFrameHeader
{
public:
  bool isFinished;
  bool reserved1;
  bool reserved2;
  bool reserved3;
  OpCode opCode;
  bool isMasked;

  bool readFrameHeader(const char* begin, std::size_t maxLength);
  std::vector<char> generateFrameHeader(std::size_t dataSize, const std::vector<char>& mask = {}) const;

  int getHeaderSize() const;

  std::string toString() const;

  unsigned char lengthCode() const { return lengthCode_; }
  std::size_t length() const { return length_; }
  unsigned char* mask() { return mask_; }

private:
  unsigned char lengthCode_;
  uint64_t length_;
  unsigned char mask_[4];
};

bool WebSocketFrameHeader::readFrameHeader(const char* begin, std::size_t maxLength)
{
  /* FRAME:
  0                1                2
  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7
  F R R R OPCODE   M PAYLOAD LENGTH EXTENDED LENGTH
  I S S S          A
  N V V V          S
    1 2 3          K

  3/5/11           ... 5/9/15
  0 1 2 3 4 5 6 7  ... 0 1 2 3 4 5 6 7
              MASKING KEY
  */
  if (maxLength < 2) {
    return false;
  }

  // First byte
  unsigned char codes = begin[0];
  // Second byte
  unsigned char maskAndLength = begin[1];

  // Byte 0 bit 0
  isFinished = codes & 0x80;
  // Byte 0 bit 1
  reserved1 = codes & 0x40;
  // Byte 0 bit 2
  reserved2 = codes & 0x20;
  // Byte 0 bit 3
  reserved3 = codes & 0x10;
  // Byte 0 bit 4-7
  opCode = static_cast<OpCode>(codes & 0xF);
  // Byte 1 bit 0
  isMasked = maskAndLength & 0x80;
  // Byte 7 bit 1-7
  lengthCode_ = maskAndLength & 0x7F;

  if (lengthCode_ == 126) {
    if (maxLength < 4) {
      return false;
    }

    // Read next 2 bytes
    unsigned char length1 = begin[2];
    unsigned char length2 = begin[3];
    length_ = (length1 << 8) | length2;
  } else if (lengthCode_ == 127) {
    // Read next 8 bytes (where most significant must be 0)
    // assert length & (1 << 64) == 0
    if (maxLength < 10) {
      return false;
    }

    unsigned char length1 = begin[2];
    unsigned char length2 = begin[3];
    unsigned char length3 = begin[4];
    unsigned char length4 = begin[5];
    unsigned char length5 = begin[6];
    unsigned char length6 = begin[7];
    unsigned char length7 = begin[8];
    unsigned char length8 = begin[9];
    length_ = (static_cast<uint64_t>(length1) << 56) | (static_cast<uint64_t>(length2) << 48)
             | (static_cast<uint64_t>(length3) << 40) | (static_cast<uint64_t>(length4) << 32)
             | (static_cast<uint64_t>(length5) << 24) | (static_cast<uint64_t>(length6) << 16)
             | (static_cast<uint64_t>(length7) << 8) | static_cast<uint64_t>(length8);
  } else {
    length_ = lengthCode_;
  }

  // Normally starts at offset 2
  unsigned int maskOffset = 2;
  if (lengthCode_ == 126) {
    // Additional 2 bytes for medium payload length
    maskOffset += 2;
  } else if (lengthCode_ == 127) {
    // Additional 8 bytes for long payload length
    maskOffset += 8;
  }

  if (maxLength < maskOffset + 4) {
    return false;
  }

  if (isMasked) {
    // Third to sixth byte if (length <= 125)
    // Fifth to eighth byte if (length == 126)
    // Eleventh to fourteenth byte if (length == 127)
    mask_[0] = begin[maskOffset];
    mask_[1] = begin[maskOffset + 1];
    mask_[2] = begin[maskOffset + 2];
    mask_[3] = begin[maskOffset + 3];
  }

  return true;
}

std::vector<char> WebSocketFrameHeader::generateFrameHeader(std::size_t dataLength, const std::vector<char>& mask) const
{
  char FIN = static_cast<char>(isFinished);
  char RSV1 = static_cast<char>(reserved1);
  char RSV2 = static_cast<char>(reserved2);
  char RSV3 = static_cast<char>(reserved3);
  char code = static_cast<char>(opCode);

  char frameByte = (FIN << 7) + (RSV1 << 6) + (RSV2 << 5) + (RSV3 << 4) + code;

  std::vector<char> frameHeader;

  frameHeader.push_back(frameByte);

  // Masking & length
  // MASKING must NOT be done by the server
  char MASK = isMasked;
  char length = 0;

  if (dataLength < 126) {
    length = dataLength;
  } else if (dataLength < 65536) {
    length = 126;
  } else {
    length = 127;
  }

  char maskAndLength = (MASK << 7) + length;

  frameHeader.push_back(maskAndLength);

  if (length == 126) {
    // Optional extended length (2 byte)
    unsigned int l = dataLength;
    char l1 = (l >> 8) & 0xFF;
    char l2 = l & 0xFF;
    frameHeader.push_back(l1);
    frameHeader.push_back(l2);
  } else if (length == 127) {
    // Optional extended length (8 byte)
    std::size_t l = dataLength;
    char l1 = (l >> 56) & 0xFF;
    char l2 = (l >> 48) & 0xFF;
    char l3 = (l >> 40) & 0xFF;
    char l4 = (l >> 32) & 0xFF;
    char l5 = (l >> 24) & 0xFF;
    char l6 = (l >> 16) & 0xFF;
    char l7 = (l >> 8) & 0xFF;
    char l8 = l & 0xFF;
    frameHeader.push_back(l1);
    frameHeader.push_back(l2);
    frameHeader.push_back(l3);
    frameHeader.push_back(l4);
    frameHeader.push_back(l5);
    frameHeader.push_back(l6);
    frameHeader.push_back(l7);
    frameHeader.push_back(l8);
  }

  if (isMasked && mask.size() == 4) {
    // LOG
    frameHeader.push_back(mask[0]);
    frameHeader.push_back(mask[1]);
    frameHeader.push_back(mask[2]);
    frameHeader.push_back(mask[3]);
  }

  return frameHeader;
}

int WebSocketFrameHeader::getHeaderSize() const
{
  // Minimum size is 2
  int size = 2;

  // Length is 2 bytes
  if (lengthCode_ == 126) {
    size += 2;
  // Length it 8 bytes
  } else if (lengthCode_ == 127) {
    size += 8;
  }

  // Masking is 4 bytes
  if (isMasked) {
    size += 4;
  }

  return size;
}

std::string WebSocketFrameHeader::toString() const
{
  std::string maskValues;
  if (isMasked) {
    maskValues = std::to_string(mask_[0]) + " " + std::to_string(mask_[1]) + " "
                 + std::to_string(mask_[2]) + " " + std::to_string(mask_[3]);
  }
  WString result = WString("WebSocket Header with\nFinished: {1}\n"
                                   "RSV1: {2}\nRSV2: {3}\nRSV3: {4}\n"
                                   "OpCode: {5} ({6})\nMasking: {7}\n"
                                   "Mask: {8}\nPayload Length: {9}")
                       .arg(isFinished)
                       .arg(reserved1)
                       .arg(reserved2)
                       .arg(reserved3)
                       .arg(static_cast<int>(opCode))
                       .arg(OpCodeToString(opCode))
                       .arg(isMasked)
                       .arg(maskValues)
                       .arg(length_);
  return result.toUTF8();
}

WebSocketConnection::WebSocketConnection(AsioWrapper::asio::io_service& ioService)
  : ioService_(ioService),
    strand_(ioService),
    readBuffer_(),
    readBufferPtr_(readBuffer_.begin()),
    skipDataSize_(0),
    readingState_(ReadingState::ReadingHeader),
    skipMessage_(false),
    header_(new WebSocketFrameHeader()),
    isContinuation_(false),
    continuationSize_(0),
    isWriting_(false),
    isWritingToSocket_(false),
    hasPendingDataWrite_(false)
{
}

WebSocketConnection::~WebSocketConnection()
{
}

void WebSocketConnection::startReading()
{
  doAsyncRead(&(*readBufferPtr_), readBuffer_.end() - readBufferPtr_);
}

void WebSocketConnection::doAsyncRead(char* buffer, size_t size)
{
  if (readingState_ == ReadingState::ClosingSocket) {
    LOG_WARN("doAsyncRead() was called after a closing signal has been sent or received.");
    return;
  }

  doSocketRead(buffer, size);
}

// Returns amount of bytes consumed; 0 if more data is needed before
// more can be consumed.
std::size_t WebSocketConnection::parseBuffer(const char* begin, const char* end)
{
  const char *current = begin;

  if (readingState_ == ReadingState::ReadingHeader) {
    auto isCompleteHeader = header_->readFrameHeader(begin, end - begin);

    if (!isCompleteHeader) {
      // By returning 0; indicate that we need more data
      return 0;
    } else {
      current += header_->getHeaderSize();
      readingState_ = ReadingState::ReadingData;

      if (header_->length() > frameSize_) {
        LOG_ERROR("parseBuffer: A frame was received that exceeded the limit (" << header_->length() << " > " << frameSize_ << " (bytes))");
        readingState_ = ReadingState::SkipData;
        if (!header_->isFinished) {
          LOG_ERROR("parseBuffer: The received frame will be followed by a continuation that should be skipped as well.");
          skipMessage_ = true;
        }
      } else if (skipMessage_) {
        readingState_ = ReadingState::SkipData;
        LOG_ERROR("parseBuffer: A continuation frame was received that is part of a skipped message.");
      } else if (isContinuation_ && header_->length() + continuationSize_ > messageSize_) {
        readingState_ = ReadingState::SkipData;
        LOG_ERROR("parseBuffer: A continuation frame was received that made the whole message exceed the limit (" << header_->length() + continuationSize_ << " > " << messageSize_ << " (bytes))");
        if (!header_->isFinished) {
          LOG_ERROR("parseBuffer: The received frame will be followed by a continuation that should be skipped as well.");
          skipMessage_ = true;
        }
      }
    }
  }

  if (readingState_ == ReadingState::ReadingData) {
    std::size_t bytes_to_add = std::min(header_->length() - dataBuffer_.length(), (size_t)(end - current));
    dataBuffer_.append(current, bytes_to_add);
    if (dataBuffer_.length() == header_->length()) {
      // received a full frame
      doEmitAndCleanBuffers();
    }
    current += bytes_to_add;
  }

  // Case where data is skipped due to size limitation.
  if (readingState_ == ReadingState::SkipData) {
    std::size_t bytes_to_add = std::min(header_->length() - skipDataSize_, (size_t)(end - current));
    skipDataSize_ += bytes_to_add;
    if (skipDataSize_ == header_->length()) {
      // received a full skipped frame
      doEmitAndCleanBuffers();
    }

    current += bytes_to_add;
  }

  return current - begin;
}

void WebSocketConnection::doEmitAndCleanBuffers()
{
  // Indicates that a message or frame falls within the defined limited size.
  if (readingState_ != ReadingState::SkipData) {
    hasDataReadCallback_();
  } else {
    LOG_INFO("doEmitAndCleanBuffers: not calling read callback due to frame or message size being exceeded.");
  }

  readingState_ = ReadingState::ReadingHeader;

  // If the header marks a complete message, no longer skip any frames, if before
  // they were skipped due to the message limit.
  if (header_->isFinished) {
    skipMessage_ = false;
    isContinuation_ = false;
    continuationSize_ = 0;
  } else {
    isContinuation_ = true;
    continuationSize_ += header_->length();
  }

  // If closing received, mark as wanting to close, to avoid further listening,
  // and cancel active async (read) operations.
  if (header_->opCode == OpCode::Close) {
    readingState_ = ReadingState::ClosingSocket;
  }

  dataBuffer_.clear();
  skipDataSize_ = 0;
}

void WebSocketConnection::handleAsyncRead(const AsioWrapper::error_code& e, std::size_t bytes_transferred)
{
  if (e) {
    // Normal shutdown
    if (e == AsioWrapper::asio::error::operation_aborted) {
      dataBuffer_.clear();
      return;
    }

    LOG_ERROR("handleAsyncRead: error encountered " << e.value());

    dataBuffer_.clear();

    doClose();
    return;
  }

  LOG_DEBUG("handleAsyncRead: incoming frame of size " << bytes_transferred << " bytes");

  // parse, starting at the beginning of the read buffer
  const char *current = &(*readBuffer_.begin());
  // when calculating end, start after bytes that were still in buffer
  const char *end = &(*readBufferPtr_) + bytes_transferred;

  // parse the input buffer until we don't make progress anymore
  std::size_t parsedBytes = parseBuffer(current, end);
  while (parsedBytes != 0) {
    current += parsedBytes;
    parsedBytes = parseBuffer(current, end);
  }
  // move remainder of rx buffer back at the start. Buffer should be empty,
  // unless a partial header was received
  std::size_t restSize = end - current;
  std::memmove(&(*readBuffer_.begin()), current, restSize);
  readBufferPtr_ = readBuffer_.begin() + restSize;

  if (readingState_ != ReadingState::ClosingSocket) {
    doAsyncRead(&(*readBufferPtr_), readBuffer_.end() - readBufferPtr_);
  }
}

bool WebSocketConnection::doAsyncWrite(OpCode type, const std::vector<char>& frameHeader, const std::vector<char>& data)
{
  // Check for the write-done loop, developer driven
  if (isWriting_) {
    LOG_WARN("The connection is already being used to send data over. Wait for the done() signal to fire.");
    return false;
  }

  // Concat header and data to stream
  writeBuffer_.append(frameHeader.data(), frameHeader.size());
  writeBuffer_.append(data.data(), data.size());

  // Put data into asio::buffer actual write buffer
  using namespace AsioWrapper::asio;

  std::vector<const_buffer> buffer;
  writeBuffer_.asioBuffers(buffer);

  // Check for functionality outside write-done loop (ping/pong), framework driven
  std::unique_lock<std::mutex> lock(writingMutex_);
  if (isWritingToSocket_) {
    LOG_DEBUG("doAsyncWrite: The connection is being written to (Ping/Pong), the frame will be delayed.");
    hasPendingDataWrite_ = true;
    pendingWriteDataType_ = type;
    pendingWriteDataBuffer_ = buffer;
    return true;
  }

  isWriting_ = true;
  isWritingToSocket_ = true;

  doSocketWrite(buffer, type);

  return true;
}

void WebSocketConnection::doControlFrameWrite(const std::vector<char>& frameHeader, OpCode opcode)
{
  // Put data into asio::buffer actual write buffer
  writeBuffer_.append(frameHeader.data(), frameHeader.size());

  using namespace AsioWrapper::asio;

  std::vector<const_buffer> buffer;
  writeBuffer_.asioBuffers(buffer);

  // Check for functionality outside write-done loop (ping/pong), framework driven
  std::unique_lock<std::mutex> lock(writingMutex_);
  if (isWritingToSocket_) {
    if (opcode == OpCode::Ping) {
      LOG_DEBUG("doControlFrameWrite: Delay the sending of the control (Ping) frame, since the socket is busy");
      hasPendingPingWrite_ = true;
      pendingWritePingBuffer_ = buffer;
    }
    if (opcode == OpCode::Pong) {
      LOG_DEBUG("doControlFrameWrite: Delay the sending of the control (Pong) frame, since the socket is busy");
      hasPendingPongWrite_ = true;
      pendingWritePongBuffer_ = buffer;
    }
    return;
  }

  isWritingToSocket_ = true;

  doSocketWrite(buffer, opcode);
}

void WebSocketConnection::handleAsyncWritten(OpCode type, const AsioWrapper::error_code& e, std::size_t bytes_transferred)
{
  if (e) {
    LOG_ERROR("handleAsyncWritten: error encountered " << e.value());
    return;
  }

  // Check for functionality outside write-done loop (ping/pong), framework driven
  std::unique_lock<std::mutex> lock(writingMutex_);

  // Only perform a singular write of a delayed frame per handler.
  if (hasPendingPingWrite_) {
    // A control write (ping) event took place while the socket was busy sending the data frame.
    LOG_DEBUG("handleAsyncWritten: Handling a delayed Ping frame");
    doSocketWrite(pendingWritePingBuffer_, OpCode::Ping);
    hasPendingPingWrite_ = false;
    pendingWritePingBuffer_.clear();
  } else if (hasPendingPongWrite_) {
    // A control write (pong) event took place while the socket was busy sending the data frame.
    LOG_DEBUG("handleAsyncWritten: Handling a delayed Pong frame");
    doSocketWrite(pendingWritePongBuffer_, OpCode::Pong);
    hasPendingPongWrite_ = false;
    pendingWritePongBuffer_.clear();
  }else if (hasPendingDataWrite_) {
    LOG_DEBUG("handleAsyncControl: Handling a delayed data frame after: " << OpCodeToString(type));
    doSocketWrite(pendingWriteDataBuffer_, pendingWriteDataType_);
    hasPendingDataWrite_ = false;
    pendingWriteDataBuffer_.clear();
  }

  isWriting_= false;
  isWritingToSocket_ = false;
  writeBuffer_.clear();

  LOG_DEBUG("handleAsyncWritten: Outgoing frame of size: " << bytes_transferred << " bytes has been written");

  if (type == OpCode::Binary || type == OpCode::Text) {
    hasDataWrittenCallback_(e, bytes_transferred);
  }
}

void WebSocketConnection::setDataReadCallback(const std::function<void()>& callback)
{
  hasDataReadCallback_ = callback;
}

void WebSocketConnection::setDataWrittenCallback(const std::function<void(const AsioWrapper::error_code&, std::size_t)>& callback)
{
  hasDataWrittenCallback_ = callback;
}

bool WebSocketConnection::isOpen()
{
  return socket().is_open();
}

void WebSocketConnection::setMaximumReceivedFrameSize(std::size_t frameSize)
{
  frameSize_ = frameSize;
}

void WebSocketConnection::setMaximumReceivedMessageSize(std::size_t messageSize)
{
  messageSize_ = messageSize;
}

WebSocketTcpConnection::WebSocketTcpConnection(AsioWrapper::asio::io_service& ioService, std::unique_ptr<Socket> socket)
  : WebSocketConnection(ioService),
    socket_(std::move(socket))
{
}

WebSocketTcpConnection::~WebSocketTcpConnection()
{
  if (isOpen()) {
    doClose();
  }
}

void WebSocketTcpConnection::doClose()
{
  try {
    AsioWrapper::error_code ignored_ec;
    auto handle = socket().native_handle();
    socket().cancel();
    socket().shutdown(AsioWrapper::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    socket().close();
    LOG_INFO("Socket " << handle << " was shut down correctly.");
  } catch (AsioWrapper::system_error& e) {
    LOG_DEBUG("Socket " << socket().native_handle() << " encountered an error while shutting down: " << e.what());
  }
}

void WebSocketTcpConnection::doSocketRead(char* buffer, size_t size)
{
  socket_->async_read_some(AsioWrapper::asio::buffer(buffer, size), strand_.wrap(std::bind(&WebSocketTcpConnection::handleAsyncRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
}

void WebSocketTcpConnection::doSocketWrite(const std::vector<AsioWrapper::asio::const_buffer>& buffer, OpCode type)
{
  async_write(socket(), buffer, strand_.wrap(std::bind(&WebSocketTcpConnection::handleAsyncWritten, shared_from_this(), type, std::placeholders::_1, std::placeholders::_2)));
}

Socket& WebSocketTcpConnection::socket()
{
  return *socket_.get();
}

#ifdef WT_WITH_SSL
WebSocketSslConnection::WebSocketSslConnection(AsioWrapper::asio::io_service& ioService, std::unique_ptr<SSLSocket> socket)
  : WebSocketConnection(ioService),
    socket_(std::move(socket))
{
}

WebSocketSslConnection::~WebSocketSslConnection()
{
  if (isOpen()) {
    doClose();
  }
}

void WebSocketSslConnection::doClose()
{
  // Non-static pointer
  std::shared_ptr<WebSocketSslConnection> ptr = std::static_pointer_cast<WebSocketSslConnection>(shared_from_this());

  socket_->async_shutdown(strand_.wrap(std::bind(&WebSocketSslConnection::stopTcpSocket, ptr, std::placeholders::_1)));
}

void WebSocketSslConnection::stopTcpSocket(const AsioWrapper::error_code& e)
{
  if (e) {
    LOG_ERROR("doClose: SSL socket shutdown failed " << e.value());
  }

  try {
    AsioWrapper::error_code ignored_ec;
    auto handle = socket().native_handle();
    socket().cancel();
    socket().shutdown(AsioWrapper::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    socket().close();
    LOG_INFO("Socket " << handle << " was shut down correctly.");
  } catch (AsioWrapper::system_error& e) {
    LOG_DEBUG("Socket " << socket().native_handle() << " encountered an error while shutting down: " << e.what());
  }

}

void WebSocketSslConnection::doSocketRead(char* buffer, size_t size)
{
  socket_->async_read_some(AsioWrapper::asio::buffer(buffer, size), strand_.wrap(std::bind(&WebSocketSslConnection::handleAsyncRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
}

void WebSocketSslConnection::doSocketWrite(const std::vector<AsioWrapper::asio::const_buffer>& buffer, OpCode type)
{
  async_write(*socket_, buffer, strand_.wrap(std::bind(&WebSocketSslConnection::handleAsyncWritten, shared_from_this(), type, std::placeholders::_1, std::placeholders::_2)));
}

Socket& WebSocketSslConnection::socket()
{
  return socket_->next_layer();
}
#endif

WWebSocketConnection::WWebSocketConnection(WWebSocketResource* resource, AsioWrapper::asio::io_service& ioService)
  : resource_(resource),
    wantsToClose_(false),
    frameSize_(0),
    messageSize_(0),
    takesUpdateLock_(true),
    pingInterval_(180),
    pingTimeout_(360),
    pingSignalTimer_(ioService),
    pongTimeoutTimer_(ioService)
{
  LOG_DEBUG("New connection to track on endpoint: " << resource->url());
}

WWebSocketConnection::~WWebSocketConnection()
{
  AsioWrapper::error_code ignored_ec;
  closeSocket(ignored_ec, "");
}

void WWebSocketConnection::setSocket(const std::shared_ptr<WebSocketConnection>& connection)
{
  socketConnection_ = connection;
  socketConnection_->setMaximumReceivedFrameSize(frameSize_);
  socketConnection_->setMaximumReceivedMessageSize(messageSize_);

  socketConnection_->startReading();
  // Set up listeners to socket changes (written & read)
  socketConnection_->setDataWrittenCallback(std::bind(&WWebSocketConnection::writeFrame, this, std::placeholders::_1, std::placeholders::_2));
  socketConnection_->setDataReadCallback(std::bind(&WWebSocketConnection::receiveFrame, this));
}

void WWebSocketConnection::handleMessage(const std::string& text)
{
  LOG_INFO("handleMessage(text): text of length: " << std::to_string(text.size()));
}

void WWebSocketConnection::handleMessage(const std::vector<char>& buffer)
{
  LOG_INFO("handleMessage(binary): array of length: " + std::to_string(buffer.size()));
}

bool WWebSocketConnection::sendMessage(const std::string& text)
{
  std::vector<char> buff;
  std::copy(text.begin(), text.end(), std::back_inserter(buff));
  return sendDataFrame(buff, OpCode::Text);
}

bool WWebSocketConnection::sendMessage(const std::vector<char>& buffer)
{
  return sendDataFrame(buffer, OpCode::Binary);
}

bool WWebSocketConnection::close(CloseCode code, const std::string& reason)
{
  wantsToClose_ = true;

  if (reason.empty()) {
    return sendEmptyFrame(OpCode::Close);
  }

  std::vector<char> buff;
  std::copy(reason.begin(), reason.end(), std::back_inserter(buff));
  return sendDataFrame(buff, OpCode::Close);
}

bool WWebSocketConnection::sendDataFrame(const std::vector<char>& data, OpCode opcode)
{
  WebSocketFrameHeader header;
  header.isFinished = true;
  header.reserved1 = false;
  header.reserved2 = false;
  header.reserved3 = false;
  header.opCode = opcode;
  header.isMasked = false;

  std::vector<char> frameHeader = header.generateFrameHeader(data.size());;

  LOG_DEBUG("sendDataFrame: writing data frame with a header of " << frameHeader.size() << " bytes and data of " << data.size() << " bytes");

  return socketConnection_->doAsyncWrite(opcode, frameHeader, data);
}

bool WWebSocketConnection::sendEmptyFrame(OpCode opcode)
{
  if (opcode == OpCode::Ping || opcode == OpCode::Pong) {
    LOG_WARN("sendEmptyFrame: sending a ping or pong frame inside the write/done loop. This will trigger the done() signal upon successfully writing.");
  }

  WebSocketFrameHeader header;
  header.isFinished = true;
  header.reserved1 = false;
  header.reserved2 = false;
  header.reserved3 = false;
  header.opCode = opcode;
  header.isMasked = false;

  std::vector<char> frameHeader = header.generateFrameHeader(0);

  LOG_DEBUG("sendEmptyFrame: writing a control frame with a header of " << frameHeader.size() << " bytes and OpCode: " << OpCodeToString(header.opCode));

  return socketConnection_->doAsyncWrite(opcode, frameHeader);
}

bool WWebSocketConnection::sendControlFrame(OpCode opcode)
{
  if (opcode == OpCode::Close) {
    LOG_WARN("sendControlFrame: sending a close frame outside the write/done loop. This will NOT trigger the done() or closed() signals upon successfully writing.");
  }

  WebSocketFrameHeader header;
  header.isFinished = true;
  header.reserved1 = false;
  header.reserved2 = false;
  header.reserved3 = false;
  header.opCode = opcode;
  header.isMasked = false;

  std::vector<char> frameHeader = header.generateFrameHeader(0);

  LOG_DEBUG("sendControlFrame: writing a control frame with a header of " << frameHeader.size() << " bytes and OpCode: " << OpCodeToString(header.opCode));

  socketConnection_->doControlFrameWrite(frameHeader, opcode);
  return true;
}

void WWebSocketConnection::writeFrame(const AsioWrapper::error_code& e, std::size_t bytes_transferred)
{
  LOG_DEBUG("writeFrame: written data of size: " << bytes_transferred << " bytes");

  std::unique_ptr<WApplication::UpdateLock> updateLock;
  if (takesUpdateLock_ && app()) {
    updateLock.reset(new Wt::WApplication::UpdateLock(app()));
    if (!*updateLock) {
      LOG_ERROR("writeFrame: Cannot take the update lock");
      return;
    }
  }

  done_.emit(e);
  updateLock.reset(nullptr);
}

void WWebSocketConnection::receiveFrame()
{
  WebSocketFrameHeader* header = socketConnection_->header();

  std::string data;
  std::string rawData = socketConnection_->dataBuffer().str();
  std::size_t dataSize = rawData.size();
  data.reserve(dataSize);

  if (header->isMasked) {
    for (unsigned int i = 0; i < dataSize; ++i) {
      data += (rawData[i] ^ header->mask()[(i) % 4]);
    }
  } else {
    data += rawData;
  }

  if (socketConnection_->skipMessage()) {
    LOG_DEBUG("receiveFrame: Received a finished frame or message, that exceeded the received frame limit (" << header->length() << " > " << frameSize_ << " (bytes))");
    return;
  }

  // Continuation frame case
  if (!header->isFinished) {
    // Remember current unmasked data for when frame finishes.
    continuationBuffer_.append(data.c_str(), data.size());
    if (header->opCode != OpCode::Continuation) {
      continuationOpCode_ = header->opCode;
    }
    LOG_DEBUG("Received a non-finished frame, and expect a continuation. Current OpCode: " << OpCodeToString(continuationOpCode_) << " and data size: " << data.size());
    return;
  } else if (header->isFinished && header->opCode == OpCode::Continuation) {
    header->opCode = continuationOpCode_;
    continuationBuffer_.append(data.c_str(), data.size());

    if (continuationBuffer_.length() > messageSize_) {
      LOG_ERROR("receiveFrame: A message was received that exceeded the limit (" << continuationBuffer_.length() << " > " << messageSize_ << " (bytes))");
      continuationBuffer_.clear();
      return;
    }

    data = continuationBuffer_.str();
    LOG_DEBUG("Received a finished frame, after a continuation. Current OpCode: " << OpCodeToString(continuationOpCode_) << " and data size: " << data.size());
    continuationBuffer_.clear();
  }

  // Handle all non-update locked functionality:
  if (header->opCode == OpCode::Ping) {
    acknowledgePing();
    return;
  } else if (header->opCode == OpCode::Pong) {
    handlePong();
    return;
  }

  std::unique_ptr<WApplication::UpdateLock> updateLock;
  if (takesUpdateLock_ && app()) {
    updateLock.reset(new Wt::WApplication::UpdateLock(app()));
    if (!*updateLock) {
      LOG_ERROR("receiveFrame: Cannot take the update lock");
      return;
    }
  }

  if (header->opCode == OpCode::Close) {
    if (wantsToClose_) {
      AsioWrapper::error_code ignored_ec;
      closeSocket(ignored_ec, data);
    } else {
      acknowledgeClose(data);
    }
  } else if (header->opCode == OpCode::Text) {
    handleMessage(data);
  } else if (header->opCode == OpCode::Binary) {
    std::vector<char> binaryData(data.begin(), data.end());
    handleMessage(binaryData);
  } else if (header->opCode == OpCode::Continuation) {
    handleError();
  }

  updateLock.reset(nullptr);
}

void WWebSocketConnection::acknowledgeClose(const std::string& reason)
{
  std::vector<char> buff;
  std::copy(reason.begin(), reason.end(), std::back_inserter(buff));
  sendDataFrame(buff, OpCode::Close);
  done_.connect(this, std::bind(&WWebSocketConnection::closeSocket, this, std::placeholders::_1, reason));
}

void WWebSocketConnection::doSendPing(const AsioWrapper::error_code& e)
{
  if (e) {
    // Normal shutdown
    if (e == AsioWrapper::asio::error::operation_aborted) {
      return;
    }
    LOG_ERROR("doSendPing: encountered an error " << e.value());
    return;
  }

  sendPing();
}

bool WWebSocketConnection::sendPing()
{
  if (pingTimeout_ > 0) {
    pongTimeoutTimer_.expires_after(std::chrono::seconds(pingTimeout_));
    pongTimeoutTimer_.async_wait(std::bind(&WWebSocketConnection::missingPong, this, std::placeholders::_1));
  }

  return sendControlFrame(OpCode::Ping);
}

void WWebSocketConnection::acknowledgePing()
{
  sendControlFrame(OpCode::Pong);
  startPingTimer();
}

void WWebSocketConnection::handleError()
{
  LOG_ERROR("handleError: A had request was caught, currently specifically used for a bad OpCode.");
}

void WWebSocketConnection::handlePong()
{
  LOG_DEBUG("handlePong: a Pong frame was received.");
  startPingTimer();
}

void WWebSocketConnection::missingPong(const AsioWrapper::error_code& e)
{
  if (e) {
    // Normal shutdown
    if (e == AsioWrapper::asio::error::operation_aborted) {
      return;
    }
    LOG_ERROR("missingPong: encountered an error " << e.value());
    return;
  }

  LOG_ERROR("missingPong: the socket has send a Ping request, but received no Pong after " << pingTimeout_ << " seconds.");
  AsioWrapper::error_code ignored_ec;
  closeSocket(ignored_ec, "Missing Pong");
}

void WWebSocketConnection::setMaximumReceivedSize(std::size_t frameSize, std::size_t messageSize)
{
  frameSize_ = frameSize;
  messageSize_ = messageSize;

  if (socketConnection_) {
    socketConnection_->setMaximumReceivedFrameSize(frameSize_);
    socketConnection_->setMaximumReceivedMessageSize(messageSize_);
  }
}

void WWebSocketConnection::setTakesUpdateLock(bool takesUpdateLock)
{
  takesUpdateLock_ = takesUpdateLock;
}

void WWebSocketConnection::setPingTimeout(int pingInterval, int pingTimeout)
{
  pingInterval_ = pingInterval;
  pingTimeout_ = pingTimeout;

  startPingTimer();
}

void WWebSocketConnection::startPingTimer()
{
  pingSignalTimer_.cancel();
  pongTimeoutTimer_.cancel();

  // Socket "keep-alive", performed through ping-pong mechanism.
  if (pingInterval_ > 0) {
    pingSignalTimer_.expires_after(std::chrono::seconds(pingInterval_));
    pingSignalTimer_.async_wait(std::bind(&WWebSocketConnection::doSendPing, this, std::placeholders::_1));
  }
}

void WWebSocketConnection::closeSocket(const AsioWrapper::error_code& e, const std::string& reason)
{
  if (e) {
    LOG_ERROR("closeSocket: encountered an error: " << e.value());
    return;
  }

  pingSignalTimer_.cancel();
  pongTimeoutTimer_.cancel();

  if (socketConnection_->isOpen()) {
    socketConnection_->doClose();
  }
  if (takesUpdateLock_ && app()) {
    WApplication::UpdateLock lock(app());
    if (lock) {
      closed_.emit(e, reason);
    }
  } else {
    closed_.emit(e, reason);
  }
}

WApplication* WWebSocketConnection::app()
{
  if (resource_) {
    return resource_->app_;
  }
  return nullptr;
}
}

