// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WSSL_INFO_H_
#define WT_WSSL_INFO_H_

#include <Wt/WDllDefs.h>
#include <Wt/WSslCertificate.h>
#include <Wt/WValidator.h>

#include <string>
#include <vector>

#ifndef WT_TARGET_JAVA

namespace Wt {

/*! \class WSslInfo Wt/WSslInfo.h Wt/WSslInfo.h.C
 *  \brief Provides SSL information about the current session.
 *  
 * This class provides an interface to the SSL information related
 * to the current session. This class is returned by
 * WEnvironment::sslInfo().
 *
 * Probably the most important use of this class is that it provides
 * access to the client certificate which was presented by the
 * client during an https handshake to authenticate the SSL session.
 * This class collects the information on the verification that was
 * performed by the connector (FCGI, ISAPI, the built-in webserver, ...)
 * and presents it to the application programmer.
 *
 * The verification and the acceptance of the certificate has to be
 * configured in the web server (built-in httpd, Apache, IIS, ...).
 * When WEnvironment::sslInfo() returns a WSslInfo object, this means
 * that the client verification has already passed the verification
 * procedures in the webserver. This does not mean that the
 * certificate is valid; depending on the configuration of your web
 * server, this verification may be weak. Always check the
 * verification result with clientVerificationResult().
 *
 * This class is only available when %Wt was compiled with SSL support.
 */
class WT_API WSslInfo
{
public:
  /*
   * The WSslInfo class will usually be created by the library itself
   * and is therefore not public API.
   */
  WSslInfo(const WSslCertificate &clientCertificate,
	   const std::vector<WSslCertificate> &clientCertificateChain,
	   WValidator::Result clientVerificationResult);

  /*! \brief Returns the certificate used by the client for authentication.
   */
  const WSslCertificate &clientCertificate() const {
    return clientCertificate_; 
  }

  /*! \brief Returns the certificate chain used for client authentication.
   *
   * Warning: for the ISAPI connector, the certificate chain will always be
   * empty.
   */
  const std::vector<WSslCertificate> &clientPemCertificateChain() const {
    return clientCertificateChain_;
  }

  /*! \brief Returns the result of the client certificate verification.
   *
   * WSslInfo (and thus Wt) by itself does not perform any validation:
   * this task is entirely up to the web server, and this class merely
   * reports the validation status reported by the webserver.
   */
  WValidator::Result clientVerificationResult() const { 
    return clientVerificationResult_; 
  }
  
private:
  WSslCertificate              clientCertificate_;
  std::vector<WSslCertificate> clientCertificateChain_;
  WValidator::Result           clientVerificationResult_;

  std::string gdb() const;  
};

}

#endif

#endif // WT_WSSL_INFO_H_
