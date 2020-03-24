#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "WebRequest.h"
#include <sstream>
#include <httpext.h>

namespace Wt {
  namespace isapi {

class IsapiServer;

class IsapiRequest : public WebResponse
{
public:
  IsapiRequest(LPEXTENSION_CONTROL_BLOCK ecb, IsapiServer *server,
    bool forceSynchronous);

  ~IsapiRequest();

  // Signal ISAPI that this connection is to be closed and that we're
  // done with it. The IsapiRequest object must be deleted after calling
  // this method.
  void abort();

  // Returns true if the HTTP request was received without errors
  bool isGood();

  virtual bool isSynchronous() const;

  virtual void flush(ResponseState state = ResponseState::ResponseDone,
		     const WriteCallback& callback = WriteCallback());

  // Sends a simple text reply
  void sendSimpleReply(int status, const std::string &msg);

  virtual std::istream& in() { return *in_; }
  virtual std::ostream& out() { return out_; }
  virtual std::ostream& err() { return err_; }

  virtual void setStatus(int status);

  virtual void setContentLength(::int64_t length);

  virtual void setContentType(const std::string& value);

  virtual void addHeader(const std::string& name, const std::string& value);

  virtual void setRedirect(const std::string& url);

  virtual const char *headerValue(const char *name) const;

  virtual std::vector<Wt::Http::Message::Header> headers() const;

  virtual const char *envValue(const char *name) const;

  virtual const std::string &scriptName() const;

  virtual const std::string &serverName() const;

  virtual const char *requestMethod() const;

  virtual const std::string &queryString() const;

  virtual const std::string &serverPort() const;

  virtual const std::string &pathInfo() const;

  virtual const std::string &remoteAddr() const;

  virtual const char *urlScheme() const;

  virtual std::unique_ptr<WSslInfo> sslInfo(bool behindReverseProxy) const;

private:
  LPEXTENSION_CONTROL_BLOCK ecb_;
  IsapiServer *server_;
  bool good_;

  bool synchronous_;
  bool reading_;
  DWORD bytesToRead_;
  char buffer_[1024];
  DWORD bufferSize_;

  //boost::mutex intermediateBufferedLock_;
  //int intermediateBufferedCounter_;

  void processAsyncRead(DWORD cbIO, DWORD dwError, bool first);
  static void WINAPI completionCallback(LPEXTENSION_CONTROL_BLOCK lpECB,
    PVOID pContext, DWORD cbIO, DWORD dwError);
  void writeSync();
  void writeAsync(DWORD cbIO, DWORD dwError, bool first);
  void flushDone();

  std::vector<std::string> writeData_;
  unsigned int writeIndex_; // next index to be written in writeData_
  unsigned int writeOffset_; // offset withing current item of writeData_
  ResponseState flushState_;

  std::stringstream header_, in_mem_, out_, err_;
  std::iostream *in_;
  std::string requestFileName_;

  bool chunking_;
  std::int64_t contentLength_;
  bool headerSent_;
  void sendHeader();
  enum {HTTP_1_0, HTTP_1_1} version_;

  // Returns a reference to a string that's safe to use until this object is
  // deleted (we used to return by value but the built-in httpd optimizations
  // required a different approach)
  std::string *persistentEnvValue(const char *name) const;

  // storage used by persistentEnvValue
  mutable std::vector<std::string *> strings_;
  std::string emptyString_;
};

}
}

