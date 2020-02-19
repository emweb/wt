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

#include <Wt/WDateTime.h>
#include <Wt/WSslCertificate.h>

#ifdef WT_WITH_SSL
#include <openssl/ssl.h>

#include <Wt/AsioWrapper/asio.hpp>
#include <Wt/AsioWrapper/ssl.hpp>

#ifdef WT_ASIO_IS_BOOST_ASIO
namespace asio = boost::asio;
#endif // WT_ASIO_IS_BOOST_ASIO

namespace Wt {
  namespace Ssl {
    std::vector<Wt::WSslCertificate::DnAttribute>
    getDnAttributes(struct X509_name_st *sn);

    WT_API extern WSslCertificate x509ToWSslCertificate(X509 *x);

    Wt::WDateTime dateToWDate(struct asn1_string_st *date);

    std::string exportToPem(struct x509_st *x509);

    WT_API struct x509_st *readFromPem(const std::string &pem);

    extern asio::ssl::context createSslContext(asio::io_service &io_service,
                                               bool addCACerts);
  }
}
#endif //WT_WITH_SSL

#endif // SSLUTILS_H_
