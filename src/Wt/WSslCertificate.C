/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSslCertificate.h"

#include "Wt/WException.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/Utils.h"

#include <stdexcept>
#include <cstring>
#include <cctype>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

#define PEM_HEADER "-----BEGIN CERTIFICATE-----"
#define PEM_FOOTER "-----END CERTIFICATE-----"

namespace {
  std::string attributesToString(const std::vector<Wt::WSslCertificate::DnAttribute> &DN) {
    Wt::WStringStream ss;
    bool first = true;
    for (const Wt::WSslCertificate::DnAttribute &rdn : DN) {
      if (!first)
        ss << ',';
      first = false;
      ss << rdn.shortName() << '=' << rdn.value();
    }
    return ss.str();
  }

  const std::string DN_ATTR_SHORT_NAMES[] = {
    "C",
    "CN",
    "L",
    "S",
    "G",
    "SN",
    "T",
    "I",
    "O",
    "OU",
    "ST",
    "P"
  };

  static_assert(sizeof(DN_ATTR_SHORT_NAMES) / sizeof(std::string) == Wt::WSslCertificate::DnAttributeNameCount,
                "There should be as many short names as the cardinality of DnAttributeName");

  const std::string DN_ATTR_LONG_NAMES[] = {
    "countryName",
    "commonName",
    "localityName",
    "surname",
    "givenName",
    "serialNumber",
    "title",
    "initials",
    "organizationName",
    "organizationalUnitName",
    "stateOrProvinceName",
    "pseudonym"
  };

  static_assert(sizeof(DN_ATTR_LONG_NAMES) / sizeof(std::string) == Wt::WSslCertificate::DnAttributeNameCount,
                "There should be as many long names as the cardinality of DnAttributeName");
}

namespace Wt {

LOGGER("WSslCertificate");

WSslCertificate::WSslCertificate(const std::vector<DnAttribute> &subjectDn,
	   const std::vector<DnAttribute> &issuerDn,
	   const Wt::WDateTime &validityStart,
	   const Wt::WDateTime &validityEnd,
	   const std::string &pemCert):
  subjectDn_(subjectDn),
  issuerDn_(issuerDn),
  validityStart_(validityStart),
  validityEnd_(validityEnd),
  pemCert_(pemCert)
{
  LOG_DEBUG("WSslCertificate fields: " 
	    <<  gdb());
}

std::string WSslCertificate::DnAttribute::shortName() const
{
  if (name_ >= DnAttributeNameCount) {
    throw Wt::WException("WSslCertificate::shortName(): Unknown DnAttributeName");
  }
  return DN_ATTR_SHORT_NAMES[static_cast<unsigned int>(name_)];
}

std::string WSslCertificate::DnAttribute::longName() const
{
  if (name_ >= DnAttributeNameCount) {
    throw Wt::WException("WSslCertificate::longName(): Unknown DnAttributeName");
  }
  return DN_ATTR_LONG_NAMES[static_cast<unsigned int>(name_)];
}

std::vector<WSslCertificate::DnAttribute>
WSslCertificate::dnFromString(const std::string &dn)
{
  std::vector<std::string> rdns;
  boost::split(rdns, dn, boost::is_any_of(","));

  std::vector<DnAttribute> result;
  result.reserve(rdns.size());

  for (const auto &rdn : rdns) {
    auto eqPos = rdn.find('=');
    if (eqPos == std::string::npos)
      return std::vector<DnAttribute>();
    std::string attr = rdn.substr(0, eqPos);
    for (int i = 0; i < DnAttributeNameCount; ++i) {
      if (boost::iequals(attr, DN_ATTR_SHORT_NAMES[i]) ||
          boost::iequals(attr, DN_ATTR_LONG_NAMES[i])) {
        std::string value = rdn.substr(eqPos + 1);
        result.push_back(DnAttribute(static_cast<DnAttributeName>(i), value));
        break;
      }
    }
  }

  return result;
}

std::string WSslCertificate::issuerDnString() const
{
  return attributesToString(issuerDn());
}

std::string WSslCertificate::subjectDnString() const 
{
  return attributesToString(subjectDn());
}

std::string WSslCertificate::pemToDer(const std::string &pem)
{
  unsigned pem_header_len = strlen(PEM_HEADER);

  size_t startPos = pem.find(PEM_HEADER);
  if (startPos == std::string::npos)
    throw Wt::WException("WSslCertificate::pemToDer() illegal PEM format");
  size_t endPos = pem.find(PEM_FOOTER, startPos);
  if (endPos == std::string::npos)
    throw Wt::WException("WSslCertificate::pemToDer() illegal PEM format");

  int npos = pem.size() 
    - (startPos + strlen(PEM_HEADER))
    - (pem.size() - endPos);
  std::string cert = pem.substr(startPos + pem_header_len, npos);
  
  std::string cert_base64;
  cert_base64.reserve(cert.size());
  int x=0;
  for (unsigned i = 0; i < cert.size(); ++i) {
    char c = cert[i];
    if (std::isalnum(c) || c == '+' || c == '/' || c == '=') {
      cert_base64 += c;
      ++x;
    }
  }

  std::string der = Utils::base64Decode(cert_base64);
  
  return der; 
}

std::string WSslCertificate::gdb() const
{
  Wt::WStringStream ss;
  ss 
    << "subject DN: " << subjectDnString() << '\n'
    << "issuer DN: " << issuerDnString() << '\n'
    << "validity start: " << validityStart_.toString().toUTF8() << '\n'
    << "validity end: " << validityEnd_.toString().toUTF8() << '\n'
    << "client cert: " << pemCert_ << '\n';
 
  return ss.str();
}

}

