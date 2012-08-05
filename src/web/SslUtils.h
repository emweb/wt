// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SSLUTILS_H_
#define SSLUTILS_H_

#include <string>
#include <vector>

#include <Wt/WDateTime>
#include <Wt/WSslCertificate>

#ifdef WT_WITH_SSL
#include <openssl/ssl.h>

namespace Wt {
  namespace Ssl {
    std::vector<Wt::WSslCertificate::DnAttribute>
    getDnAttributes(struct X509_name_st *sn);

    WT_API extern WSslCertificate x509ToWSslCertificate(X509 *x);

    Wt::WDateTime dateToWDate(struct asn1_string_st *date);

    std::string exportToPem(struct x509_st *x509);

    struct x509_st *readFromPem(const std::string &pem);
  }
}
#endif //WT_WITH_SSL

#endif // SSLUTILS_H_
