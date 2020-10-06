#include "IsapiRequest.h"
#include "FileUtils.h"
#include "Server.h"
#include "SslUtils.h"
#include "WebUtils.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <fstream>
#include <stdexcept>
#include <string>

#include <Wt/WSslInfo.h>

#ifdef WT_WITH_SSL
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

namespace {
#ifdef WT_WITH_SSL
X509 *convertCertContext(const CERT_CONTEXT &cc) {
  X509 *x509 = NULL;
  BIO *in = BIO_new_mem_buf(cc.pbCertEncoded, cc.cbCertEncoded);
  x509 = d2i_X509_bio(in, NULL);
  BIO_free_all(in);
  
  return x509;
}
#endif
}

namespace Wt {
  namespace isapi {

// Important: an IsapiRequest must ALWAYS be pushed to the server,
// even on error. Otherwise you created a memory leak.
IsapiRequest::IsapiRequest(LPEXTENSION_CONTROL_BLOCK ecb,
                           IsapiServer *server, bool forceSynchronous)
  : ecb_(ecb),
  server_(server),
  good_(true),
  synchronous_(true),
  reading_(true),
  chunking_(false),
  contentLength_(-1),
  headerSent_(false)
{
  std::string version = envValue("HTTP_VERSION");
  if (version == "HTTP/1.1") {
    version_ = HTTP_1_1;
  } else if (version == "HTTP/1.0") {
    version_ = HTTP_1_0;
  } else {
    version_ = HTTP_1_0;
  }

  if (!forceSynchronous) {
    // Try to configure async mode (synchronous_ must be set right, also
    // if only used for write)
    if (ecb->ServerSupportFunction(ecb->ConnID, HSE_REQ_IO_COMPLETION,
        (LPVOID)&IsapiRequest::completionCallback, 0, (LPDWORD)this)) {
      // Note: we don't expect this to happen
      synchronous_ = false;
    }
  }

  // Read whatever has already been read
  bool done = false;
  bytesToRead_ = ecb->cbTotalBytes;

  bool spooltofile = false;
  if (server_->hasConfiguration()) {
    spooltofile =
      bytesToRead_ >(unsigned)server_
      ->configuration().isapiMaxMemoryRequestSize();
  } else {
    spooltofile = bytesToRead_ > 128*1024;
  }
  if (spooltofile) {
    requestFileName_ = Wt::FileUtils::createTempFileName();
    // First, create the file
    std::ofstream o(requestFileName_.c_str());
    o.close();
    // Now, open it for read/write
    in_ = new std::fstream(requestFileName_.c_str(),
      std::ios::in | std::ios::out | std::ios::binary);
    if (!*in_) {
      // Give up, spool to memory
      requestFileName_ = "";
      delete in_;
      in_ = &in_mem_;
    }
  } else {
    in_ = &in_mem_;
  }

  if (ecb->lpbData && ecb->cbAvailable) {
    in_->write((const char *)ecb_->lpbData, ecb->cbAvailable);
  }
  if (bytesToRead_ != 0xffffffff) {
    bytesToRead_ -= ecb->cbAvailable;
  }
  if (bytesToRead_ == 0) {
    in_->seekg(0);
    server->pushRequest(this);
    return;
  }

  if (!synchronous_) {
    processAsyncRead(0, 0, true);
  } else {
    while (bytesToRead_ != 0 && !done) {
      bufferSize_ = sizeof(buffer_);
      if (bytesToRead_ != 0xffffffff && bytesToRead_ < bufferSize_) {
        bufferSize_ = bytesToRead_;
      }
      if (ecb->ReadClient(ecb->ConnID, (LPVOID)buffer_, &bufferSize_)) {
        if (bytesToRead_ != 0xffffffff) {
          bytesToRead_ -= bufferSize_;
        }
        if (bufferSize_ != 0) {
          in_->write(buffer_, bufferSize_);
        } else {
          done = true;
        }
      } else {
        int err = GetLastError();
        if (server_->hasConfiguration())
          server_->log("error")
          << "ISAPI Synchronous Read failed with error " << err;
        good_ = false; // TODO: retry on timeout?
        done = true;
      }
    }
    in_->seekg(0);
    server->pushRequest(this);
  }
}

IsapiRequest::~IsapiRequest()
{
  if (&in_mem_ != in_) {
    dynamic_cast<std::fstream *>(in_)->close();
    delete in_;
  }
  if (requestFileName_ != "") {
    unlink(requestFileName_.c_str());
  }
  for (unsigned i = 0; i < strings_.size(); ++i)
    delete strings_[i];
}

void WINAPI IsapiRequest::completionCallback(LPEXTENSION_CONTROL_BLOCK lpECB,
                                          PVOID pContext,
                                          DWORD cbIO,
                                          DWORD dwError)
{
  IsapiRequest *req = reinterpret_cast<IsapiRequest *>(pContext);
  if (req->reading_)
    req->processAsyncRead(cbIO, dwError, false);
  else
    req->writeAsync(cbIO, dwError, false);
}

void IsapiRequest::processAsyncRead(DWORD cbIO, DWORD dwError, bool first)
{
  // First, queue up the bytes received
  if (bytesToRead_ != 0xffffffff) {
    bytesToRead_ -= cbIO;
  }
  if (cbIO != 0) {
    in_->write(buffer_, cbIO);
  }

  if (!server_->server()->controller()->requestDataReceived(this,
    ecb_->cbTotalBytes - bytesToRead_, ecb_->cbTotalBytes)) {
    setStatus(/*request_entity_too_large*/ 413);
    good_ = false;
    in_->seekg(0);
    server_->pushRequest(this);
    return;
  }

  // Then, read more, if applicable
  if ((first && bytesToRead_ != 0) ||
      (cbIO != 0 && bytesToRead_ != 0 && dwError == 0)) {
    bufferSize_ = sizeof(buffer_);
    if (bytesToRead_ != 0xffffffff && bytesToRead_ < bufferSize_) {
      bufferSize_ = bytesToRead_;
    }
    DWORD dwFlags = HSE_IO_ASYNC;
    if (!ecb_->ServerSupportFunction(ecb_->ConnID,
          HSE_REQ_ASYNC_READ_CLIENT, (LPVOID)buffer_, &bufferSize_,
          (LPDWORD)&dwFlags)) {
      int err = GetLastError();
      if (server_->hasConfiguration())
        server_->log("error")
          << "ISAPI Asynchronous Read scheduling failed with error " << err;
      good_ = false;
      in_->seekg(0);
      server_->pushRequest(this);
    } else {
      // Don't do anything here, the readclient callback may already
      // be invoked
    }
  } else {
    // Nothing more to read, or error
    if (dwError) {
      server_->log("error")
        << "ISAPI Asynchronous Read failed with error " << dwError;
      good_ = false;
    }
    in_->seekg(0);
    server_->pushRequest(this);
  }
}

bool IsapiRequest::isSynchronous() const
{
  return synchronous_;
}

void IsapiRequest::sendHeader()
{
  // Finish up the header
  if (chunking_) {
    header_ << "Transfer-Encoding: chunked\r\n\r\n";
  } else if (contentLength_ != -1) {
    header_ << "Content-Length: " << contentLength_ << "\r\n\r\n";
  } else {
    header_ << "\r\n";
  }
  // TODO: add proper human-readable description
  std::string status = std::to_string(ecb_->dwHttpStatusCode);
  std::string header = header_.str();
  HSE_SEND_HEADER_EX_INFO hei = { 0 };
  hei.pszStatus = status.c_str();
  hei.cchStatus = static_cast<DWORD>(status.size() + 1);
  hei.pszHeader = header.c_str();
  hei.cchHeader = static_cast<DWORD>(header.size() + 1);
  hei.fKeepConn 
    = version_ == HTTP_1_1 && ((contentLength_ != -1) || chunking_);
  ecb_->ServerSupportFunction(ecb_->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX,
    &hei, 0, 0);
  headerSent_ = true;
}

bool IsapiRequest::isGood()
{
  return good_;
}

void IsapiRequest::abort()
{
  ecb_->ServerSupportFunction(ecb_->ConnID, HSE_REQ_CLOSE_CONNECTION,
    0, 0, 0);
  good_ = false;
  DWORD status = HSE_STATUS_ERROR;
  ecb_->ServerSupportFunction(ecb_->ConnID, HSE_REQ_DONE_WITH_SESSION,
    &status, 0, 0);
}

void IsapiRequest::flush(ResponseState state, const WriteCallback& callback)
{
  reading_ = false;
  if (!headerSent_) {
    // Determine how we will tell the client how long the response is
    if (state != ResponseState::ResponseDone) {
      if (version_ == HTTP_1_1 && contentLength_ == -1) {
        chunking_ = true;
      }
    } else {
      out_.seekg(0, std::ios_base::end);
      contentLength_ = out_.tellg();
      if (contentLength_ == -1) contentLength_ = 0;
      out_.seekg(0);
    }
    sendHeader();
  }

  flushState_ = state;
  if (state == ResponseState::ResponseFlush) {
    setAsyncCallback(callback);
  } else {
    setAsyncCallback(WriteCallback());
  }

  // Reserve some space so that data doesn't get copied around
  writeData_.reserve(5);

  out_.flush();
  out_.seekg(0, std::ios_base::end);
  std::streamsize size = out_.tellg();
  if (size == -1) size = 0;
  out_.seekg(0);
  if (chunking_) {
    std::string chunkPrefix;
    std::stringstream hexsize;
    hexsize << std::hex << size << std::dec << "\r\n";
    writeData_.push_back(chunkPrefix = hexsize.str());
    if (state == ResponseState::ResponseDone) {
      out_ << "\r\n0\r\n\r\n";
    } else {
      out_ << "\r\n";
    }
  }
  writeData_.push_back(out_.str());
  out_.str("");

  if (!synchronous_) {
    writeAsync(0, 0, true);
  } else {
    writeSync();
  }
}

void IsapiRequest::writeSync()
{
  for (unsigned int i = 0; i < writeData_.size(); ++i) {
    bool more = true;
    DWORD offset = 0;
    while (more) {
      DWORD size = static_cast<DWORD>(writeData_[i].size() - offset);
      if (ecb_->WriteClient(ecb_->ConnID, 
			    (LPVOID)(writeData_[i].data() + offset),
          &size, HSE_IO_SYNC)) {
        offset += size;
        if (offset >= writeData_[i].size()) {
          more = false;
        }
      } else {
        int err = GetLastError();
        err = err;
        abort();
        server_->log("error")
          << "ISAPI Synchronous Write failed with error " << err;
        getAsyncCallback()(Wt::WebWriteEvent::Error);
        return;
      }
    }
  }

  // When done:
  writeData_.clear();
  flushDone();
}

void IsapiRequest::writeAsync(DWORD cbIO, DWORD dwError, bool first)
{
  bool error = dwError != 0;
  if (dwError) {
    server_->log("error")
      << "ISAPI Asynchronous Write failed with error " << dwError;
  }
  if (first) {
    writeIndex_ = 0;
    writeOffset_ = 0;
  }
  writeOffset_ += cbIO;
  if (writeIndex_ < writeData_.size() 
      && writeOffset_ >= writeData_[writeIndex_].size()) {
    writeIndex_++;
    writeOffset_ = 0;
  }
  if (!error) {
    if (writeIndex_ < writeData_.size()) {
      DWORD size 
	= static_cast<DWORD>(writeData_[writeIndex_].size() - writeOffset_);
      if (ecb_
	  ->WriteClient(ecb_->ConnID, 
			(LPVOID)(writeData_[writeIndex_].data() + writeOffset_),
			&size, HSE_IO_ASYNC)) {
        // Don't do anything anymore, the callback will take over
        return;
      } else {
        error = true;
        int err = GetLastError();
        server_->log("error")
          << "ISAPI Asynchronous Write schedule failed with error " << err;
      }
    } else {
      // Everything is written, finish up
      writeData_.clear();
      flushDone();
      return;
    }
  }

  if (error) {
    abort();
    getAsyncCallback()(Wt::WebWriteEvent::Error);
    return;
  }
}

void IsapiRequest::flushDone()
{
  if (flushState_ == ResponseState::ResponseDone) {
    DWORD status;
    if (version_ == HTTP_1_0) {
      status = HSE_STATUS_SUCCESS;
    } else {
      status = HSE_STATUS_SUCCESS_AND_KEEP_CONN;
    }
    DWORD err;
    err = ecb_->ServerSupportFunction(ecb_->ConnID, HSE_REQ_DONE_WITH_SESSION,
      &status, 0, 0);
    if (synchronous_) {
      emulateAsync(flushState_);
      return;
    } else {
      delete this;
      return;
    }
  } else if (flushState_ == ResponseState::ResponseFlush) {
    if (synchronous_) {
      emulateAsync(flushState_);
    } else {
      getAsyncCallback()(Wt::WebWriteEvent::Completed);
    }
  }
}

void IsapiRequest::sendSimpleReply(int status, const std::string &msg)
{
  setStatus(status);
  out() << msg;
  flush(ResponseState::ResponseDone);
}

void IsapiRequest::setStatus(int status)
{
  ecb_->dwHttpStatusCode = status;
  header_ << "Status: " << status << "\r\n";
}

void IsapiRequest::setContentLength(std::int64_t length)
{
  contentLength_ = length;
}

void IsapiRequest::setContentType(const std::string& value)
{
  addHeader("Content-Type", value);
}

void IsapiRequest::addHeader(const std::string& name, const std::string& value)
{
  header_ << name << " : " << value << "\r\n";
}

void IsapiRequest::setRedirect(const std::string& url)
{
  header_ << "Location: " << url << "\r\n";
}

const char *IsapiRequest::headerValue(const char *name) const
{
  std::string *retval = persistentEnvValue((std::string("HEADER_") + name).c_str());
  if (retval == 0) {
    std::string hdr = name;
    for (unsigned int i = 0; i < hdr.size(); ++i) {
      if (hdr[i] == '-')
        hdr[i] = '_';
    }
    retval = persistentEnvValue((std::string("HTTP_") + hdr).c_str());
  }
  return retval ? retval->c_str() : 0;
}

std::vector<Wt::Http::Message::Header> IsapiRequest::headers() const
{
  std::vector<Wt::Http::Message::Header> headerVec;
  std::string *pAllRaw = persistentEnvValue("ALL_RAW");
  if (!pAllRaw)
    return headerVec;
  std::string all_raw = *pAllRaw;

  std::size_t headerStart = 0;
  std::size_t headerEnd = all_raw.find("\r\n");
  while (headerEnd != std::string::npos) {
    std::size_t colonPos = all_raw.find(':', headerStart);
    std::string name = all_raw.substr(headerStart, colonPos - headerStart);
    std::string value = all_raw.substr(colonPos + 2, headerEnd - (colonPos + 2));
    headerVec.push_back(Wt::Http::Message::Header(name, value));

    headerStart = headerEnd + 2;
    headerEnd = all_raw.find("\r\n", headerEnd + 2);
  }

  return headerVec;
}

std::string *IsapiRequest::persistentEnvValue(const char *hdr) const
{
  std::string name = boost::algorithm::to_upper_copy(std::string(hdr));
  char buffer[1024];
  DWORD size = sizeof(buffer);
  if (!ecb_->GetServerVariable(ecb_->ConnID, (LPSTR)name.c_str(),
      buffer, &size)) {
    switch (GetLastError()) {
    case ERROR_INVALID_PARAMETER:
      return 0;
      break;
    case ERROR_INVALID_INDEX:
      return 0;
      break;
    case ERROR_INSUFFICIENT_BUFFER:
    {
      char *buf = new char[size];
      std::string *retval = 0;
      if (!ecb_->GetServerVariable(ecb_->ConnID,
        (LPSTR)name.c_str(),
        buf, &size)) {
        // Give up
      } else {
        retval = new std::string(buf, buf + size - 1);
      }
      delete[] buf;
      if (retval != 0)
        strings_.push_back(retval);
      return retval;
    }
      break;
    case ERROR_NO_DATA:
      return 0;
      break;
    }
    return 0;
  } else {
    std::string *retval = new std::string(buffer, buffer + size - 1);
    strings_.push_back(retval);
    return retval;
  }
}

const char *IsapiRequest::envValue(const char *hdr) const
{
  std::string *retval = persistentEnvValue(hdr);
  if (retval)
    return retval->c_str();
  else
    return 0;
}

const std::string &IsapiRequest::scriptName() const {
  std::string *retval = persistentEnvValue("SCRIPT_NAME");
  if (!retval)
    return emptyString_;
  if (entryPoint_) {
    *retval = *retval + entryPoint_->path();
  }
  return *retval;
}

const std::string &IsapiRequest::serverName() const {
  std::string *retval = persistentEnvValue("SERVER_NAME");
  return retval ? *retval : emptyString_;
}

const char *IsapiRequest::requestMethod() const {
  std::string *retval = persistentEnvValue("REQUEST_METHOD");
  return retval ? retval->c_str() : 0;
}

const std::string &IsapiRequest::queryString() const {
  std::string *retval = persistentEnvValue("QUERY_STRING");
  return retval ? *retval : emptyString_;
}

const std::string &IsapiRequest::serverPort() const {
  std::string *retval = persistentEnvValue("SERVER_PORT");
  return retval ? *retval : emptyString_;
}

const std::string &IsapiRequest::pathInfo() const {
  if (entryPoint_) {
    std::string *pi = persistentEnvValue("PATH_INFO");
    if (!pi)
      return emptyString_;
    if (pi->size() >= entryPoint_->path().size()) {
      // assert(boost::starts_with(pi, entryPoint_->path()))
      *pi = pi->substr(entryPoint_->path().size());
      return *pi;
    } else {
      return *pi;
    }
  } else {
    std::string *retval = persistentEnvValue("PATH_INFO");
    return retval ? *retval : emptyString_;
  }
}

const std::string &IsapiRequest::remoteAddr() const {
  std::string *retval = persistentEnvValue("REMOTE_ADDR");
  return retval ? *retval : emptyString_;
}

const char *IsapiRequest::urlScheme() const {
  std::string *https = persistentEnvValue("HTTPS");
  if (*https == "ON" || *https == "on")
    return "https";
  else
    return "http";
}

std::unique_ptr<WSslInfo> IsapiRequest::sslInfo(const Configuration &) const {
#ifdef WT_WITH_SSL
  CERT_CONTEXT_EX cce;
  memset(&cce, 0, sizeof(CERT_CONTEXT_EX));
  char certbuf[64*1024];
  cce.cbAllocated = sizeof(certbuf);
  cce.CertContext.pbCertEncoded = (BYTE *) &certbuf;
  
  if(ecb_->ServerSupportFunction(ecb_->ConnID, 
				 HSE_REQ_GET_CERT_INFO_EX, 
				 &cce, 0, 0)) {
    bool present = (bool)(cce.dwCertificateFlags & 0x001);
    bool invalid = (bool)(cce.dwCertificateFlags & 0x002);
    if (present) {
      X509 *x509 = convertCertContext(cce.CertContext);
      
      std::string clientCert;
      if (x509) {
        Wt::WSslCertificate clientCert = Wt::Ssl::x509ToWSslCertificate(x509);

        X509_free(x509);
	
        // FIXME!!! Not implemented!!
        std::vector<WSslCertificate> clientCertChain;
	
        Wt::WValidator::Result
          clientVerificationResult(invalid ? Wt::ValidationState::Invalid
				   : Wt::ValidationState::Valid);

        return std::make_unique<Wt::WSslInfo>(clientCert,
                                                clientCertChain,
                                                clientVerificationResult);
      }
    }
  } else {
    DWORD error = GetLastError();
    if (error == ERROR_NO_DATA) {
      //do nothing, no certificate was found
    } else {
      server_->log("error")
        << "IsapiRequest::sslInfo(): Error " +
           std::to_string(error);
    }
  }
#endif

  return nullptr;
}

}
}
