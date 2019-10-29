// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WSSL_CERTIFICATE_H_
#define WT_WSSL_CERTIFICATE_H_

#include <Wt/WDllDefs.h>
#include <Wt/WDateTime.h>

#include <string>
#include <vector>

#ifndef WT_TARGET_JAVA

namespace Wt {

/*! \class WSslCertificate Wt/WSslCertificate.h Wt/WSslCertificate.h.C
 *  \brief An interface to an SSL certificate
 *  
 * This class provides an interface to an X.509 certificate, as used
 * by SSL (server and client cert). The certificates are usually
 * obtained by calling methods of class WSslInfo.
 *
 * This class offers you an interface to the raw (PEM/DER) certificate,
 * as well as a convenient interface to the most common attribute fields.
 * The attributes interpreted by %Wt are limited to those listed in
 * enum DnAttributeName.
 *
 * The raw certificate can be queried in PEM/DER format, and a function
 * is provided to convert PEM (textual format) to DER (binary format).
 *
 * This class is only available when %Wt was compiled with SSL support.
 */
class WT_API WSslCertificate
{
 public:
  /*! \brief Distinguished name's attribute name
   *
   * Note: The values of this enum have no relation with the numerical ID
   * used in the X.509 certificate.
   *
   * \sa DnAttribute
   */
  enum DnAttributeName {
    CountryName,            //!< Country name 
    CommonName,             //!< Common name 
    LocalityName,           //!< Locality name
    Surname,                //!< Surname
    GivenName,              //!< Given name
    SerialNumber,           //!< Serial number
    Title,                  //!< Title
    Initials,               //!< Initials
    OrganizationName,       //!< Name of the organization
    OrganizationalUnitName, //!< Name of the organizational unit
    StateOrProvinceName,    //!< Name of the state or province
    Pseudonym,              //!< Pseudonym
    DnAttributeNameCount
  };

  /*! \brief Distinguished name attribute (also known as relative 
   *  distinguished name)
   *
   * \sa WSslCertificate::subjectDn()
   * \sa WSslCertificate::issuerDn()
   */
  class WT_API DnAttribute {
  public:
    DnAttribute(DnAttributeName name, std::string value) 
      : name_(name),
	value_(value) { }
    
    /*! \brief Returns the attribute name as an enum */
    DnAttributeName name() const { return name_; }

    /*! \brief Returns the attribute's value
     */
    const std::string &value() const { return value_; }

    /*! \brief Returns the attribute's long name.
     */
    std::string longName() const;
    
    /*! \brief Returns the attribute's short name.
     */ 
    std::string shortName() const;

  private:
    DnAttributeName name_;
    std::string     value_;
  };

  /*
   * WSslCertificates are for now always constructed in Wt's connectors.
   */
  WSslCertificate(const std::vector<DnAttribute> &subjectDn,
	   const std::vector<DnAttribute> &issuerDn,
	   const Wt::WDateTime &validityStart,
	   const Wt::WDateTime &validityEnd,
	   const std::string &pemCert);

  /*! \brief Returns the distinguished name attributes of the subject.
   *
   * A distinguished name (DN) defining the entity associated with this 
   * certificate. Only the fields listed in enum DnAttributeName are
   * decoded from the certificate.
   */
  const std::vector<DnAttribute> &subjectDn() const { 
    return subjectDn_; 
  }

  /*! \brief Returns the distinguished name of the subject in 
   *   string format.
   *
   * For example: CN=Pietje Puk,OU=Development,O=Emweb
   */
  std::string subjectDnString() const;
  
  /*! \brief Returns the distinguished name attributes of the issuer.
   *
   * The distinguished name (DN) of the authority that signed and therefore 
   * issued the certificate. This is the Certification Authority (CA),
   * unless a certificate chain is used.
   */
  const std::vector<DnAttribute> &issuerDn() const { 
    return issuerDn_; 
  }
  
  /*! \brief Returns the distinguished name of the issuer in
   *   string format.
   *
   * An example: CN=Pietje Puk,OU=Development,O=Emweb
   */
  std::string issuerDnString() const;
  
  /*! \brief Returns the start time of the validity period of the certificate.
   *
   * The returned date may be invalid if not provided in the certificate.
   *
   * \sa validityEnd()
   */
  const Wt::WDateTime &validityStart() const { 
    return validityStart_; 
  }
  
  /*! \brief Returns the end time of the validity period of the certificate.
   *
   * The returned date may be invalid if not provided in the certificate.
   *
   * \sa validityStart()
   */
  const Wt::WDateTime &validityEnd() const { 
    return validityEnd_; 
  }

  /*! \brief Returns the textual PEM-encoded certificate.
   *
   * \sa pemToDer()
   */
  const std::string &toPem() const { 
    return pemCert_; 
  }
  
  /*! \brief Returns the binary DER-encoded certificate.
   *
   * This function returns WSslCertificate::pemToDer(toPem()). It will therefore throw a
   * WException if the conversion fails.
   *
   * \sa pemToDer()
   */
  std::string toDer() const { 
    return pemToDer(pemCert_);
  }

  /*! \brief Convert a certificate from PEM encoding (textual) to
   * DER encoding (binary).
   *
   * This function throws an WException when the input string is
   * not in the expected format.
   */
  static std::string pemToDer(const std::string &pem);

  std::string gdb() const;

  static std::vector<DnAttribute> dnFromString(const std::string &dnStr);

 private:
  std::vector<DnAttribute>                 subjectDn_;
  std::vector<DnAttribute>                 issuerDn_;
  Wt::WDateTime                            validityStart_;
  Wt::WDateTime                            validityEnd_;
  std::string                              pemCert_;
};

}

#endif

#endif //WT_WSSL_CERTIFICATE_H_
