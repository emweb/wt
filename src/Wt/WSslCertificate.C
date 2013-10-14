/*
 * Copyright (C) 2012 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSslCertificate"

#include "Wt/WException"
#include "Wt/WLogger"
#include "Wt/Utils"

#include <sstream>
#include <stdexcept>

#ifdef WT_WITH_SSL

#define PEM_HEADER "-----BEGIN CERTIFICATE-----"
#define PEM_FOOTER "-----END CERTIFICATE-----"

namespace {
  std::string 
  attributesToString(const std::vector<Wt::WSslCertificate::DnAttribute> &DN) {
    std::stringstream ss;
    for (unsigned i = 0; i < DN.size(); ++i) {
      ss << DN[i].shortName() << "=" << DN[i].value();
      if (i != DN.size() - 1)
	ss << ",";
    }
    return ss.str();
  }
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
  switch (name_) {
  case CountryName:
    return "C";
  case CommonName:
    return "CN";
  case LocalityName:
    return "L";
  case Surname:
    return "S";
  case GivenName:
    return "G";
  case SerialNumber:
    return "SN";
  case Title:
    return "T";
  case Initials:
    return "I";
  case OrganizationName:
    return "O";
  case OrganizationalUnitName:
    return "OU";
  case StateOrProvinceName:
    return "ST";
  case Pseudonym:
    return "P";
  default:
    throw Wt::WException("WSslCertificate::shortName(): "
      "Unknown DnAttributeName");
  }
}

std::string WSslCertificate::DnAttribute::longName() const
{
  switch (name_) {
  case CountryName:
    return "countryName";
  case CommonName:
    return "commonName";
  case LocalityName:
    return "localityName";
  case Surname:
    return "surname";
  case GivenName:
    return "givenName";
  case SerialNumber:
    return "serialNumber";
  case Title:
    return "title";
  case Initials:
    return "initials";
  case OrganizationName:
    return "organizationName";
  case OrganizationalUnitName:
    return "organizationalUnitName";
  case StateOrProvinceName:
    return "stateOrProvinceName";
  case Pseudonym:
    return "pseudonym";
  default:
    throw Wt::WException("WSslCertificate::DnAttribute::longName(): "
      "Unknown DnAttributeName");
  }
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
  std::stringstream ss;
  ss 
    << "subject DN: " << subjectDnString() << std::endl
    << "issuer DN: " << issuerDnString() << std::endl
    << "validity start: " << validityStart_.toString() << std::endl
    << "validity end: " << validityEnd_.toString() << std::endl
    << "client cert: " << pemCert_ << std::endl;
 
  return ss.str();
}

}

#endif

