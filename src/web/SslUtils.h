// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SSLUTILS_H_
#define SSLUTILS_H_

#include <Wt/WConfig.h>

#include <string>
#include <vector>

#include <Wt/WDateTime>
#include <Wt/WSslCertificate>

#ifdef WT_WITH_SSL
#include <openssl/ssl.h>

#ifdef WT_WIN32
namespace boost {
  namespace asio {
    namespace ssl {
      class context;
    }
  }
}
#endif // WT_WIN32

namespace Wt {
  namespace Ssl {
    std::vector<Wt::WSslCertificate::DnAttribute>
    getDnAttributes(struct X509_name_st *sn);

    WT_API extern WSslCertificate x509ToWSslCertificate(X509 *x);

    Wt::WDateTime dateToWDate(struct asn1_string_st *date);

    std::string exportToPem(struct x509_st *x509);

    struct x509_st *readFromPem(const std::string &pem);

#ifdef WT_WIN32
    extern void addWindowsCACertificates(boost::asio::ssl::context &ctx);
#endif // WT_WIN32
  }
}
#endif //WT_WITH_SSL

#endif // SSLUTILS_H_
