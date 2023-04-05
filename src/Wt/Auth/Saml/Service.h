// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_SAML_SERVICE_H_
#define WT_AUTH_SAML_SERVICE_H_

#include <Wt/WJavaScript.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/Identity.h>

#include <chrono>
#include <memory>
#include <vector>

#ifdef WT_THREADED
#include <mutex>
#endif // WT_THREADED

namespace Wt {
  namespace Auth {

    /*! \brief Namespace for the SAML SP implementation
     */
    namespace Saml {

class Process;
class ProcessImpl;
class ServiceImpl;

#ifdef WT_TARGET_JAVA
class CredentialResolver;
class MetadataResolver;
#endif // WT_TARGET_JAVA

/*! \enum SignaturePolicy Wt/Auth/Saml/Service.h
 *  \brief An enum describing how %SAML responses should be signed
 *
 * \note Any signature that is present will still be checked, even if
 * the signature policy does not demand that that signature be present.
 */
enum class SignaturePolicy {
  /*! \brief Do not require any signatures
   *
   * This may be useful for testing, but it is **not** recommended for
   * production use. A %SAML SP that does not check signatures is **not**
   * a secure %SAML SP.
   */
  Unsafe,

  /*! \brief Require that the response is signed
   */
  SignedResponse,

  /*! \brief Require that the assertion is signed
   *
   * This is less secure than SignedResponse. If you can configure your
   * IdP to sign responses, you are recommended to do so rather than changing
   * to SignedAssertion.
   */
  SignedAssertion,

  /*! \brief Require that both the response and the assertion are signed
   */
  SignedResponseAndAssertion
};

/*! \enum AuthnContextComparison Wt/Auth/Saml/Service.h
 *  \brief An enum describing the comparison attribute of the `AuthnContext`.
 */
enum class AuthnContextComparison {
  //! Exact
  Exact,
  //! Better
  Better,
  //! Minimum
  Minimum,
  //! Maximum
  Maximum
};

/*! \class Service Wt/Auth/Saml/Service.h
 *  \brief A minimal implementation of a %SAML service provider.
 *
 * This class implements a minimal %SAML 2.0 service provider for single sign on
 * based on the C++ version of [OpenSAML 3](https://wiki.shibboleth.net/confluence/display/OS30/Home),
 * part of [Shibboleth](https://www.shibboleth.net).
 *
 * Part of the Web Browser SSO Profile (saml-profiles-2.0-os, section 4.1) is implemented, including:
 *
 *  - AuthnRequest (saml-core-2.0-os, section 3.4.1) using HTTP Redirect binding (saml-bindings-2.0-os, section 3.4)
 *  - An %Assertion Consumer %Service receiving Response (saml-core-2.0-os, section 3.3.3) using
 *    HTTP POST binding (saml-bindings-2.0-os, section 3.5)
 *
 * Only SP-initiated login is supported.
 *
 * ## Metadata
 *
 * It's possible to get the SP's metadata in XML format through the metadata() function, and it's possible to set
 * a metadata endpoint with setMetadataPath(). This may be useful when adding this SP's information to the IdP.
 *
 * ## Tested IdPs
 *
 * Single sign on was tested with:
 *
 * - [SimpleSAMLphp](https://simplesamlphp.org/)
 * - Microsoft Azure AD
 * - Google
 * - [samltest.id](https://www.samltest.id) (based on [Shibboleth](https://www.shibboleth.net))
 * - Auth0
 *
 * ## Thread safety
 *
 * Once configured and initialized, the service may safely be used from multiple threads: it is stateless
 * or locks are acquired where necessary, so a `const Service` is thread safe.
 *
 * ## Usage
 *
 * This Service is an abstract base class, assertionToIdentity() needs to be specialized in order to retrieve
 * an Identity from the Assertion, for example:
 *
 * ```cpp
 * class MySamlService : public Wt::Auth::Saml::Service {
 * public:
 *   explicit MySamlService(const Wt::Auth::AuthService &baseAuth)
 *     : Wt::Auth::Saml::Service(baseAuth)
 *   { }
 *
 *   Wt::Auth::Identity assertionToIdentity(const Wt::Auth::Saml::Assertion &assertion) const override
 *   {
 *     auto name = assertion.attributeValue("name");
 *     auto email = assertion.attributeValue("email");
 *     if (!email || !name) {
 *       return {};
 *     }
 *     return Wt::Auth::Identity("saml", *email, *name, *email, true);
 *   }
 * };
 * ```
 *
 * The Service must first be configured and then initialized. We recommend that you do this once at
 * application startup.
 *
 * \note The Service's destructor should run before `main()` exits.
 *       Otherwise the cleanup may cause a crash.
 *
 * Example configuration:
 *
 * ```cpp
 * // baseAuth is a const AuthService&
 * MySamlService service(baseAuth);
 * // It's common for the SP entityId to be the same as the metadata endpoint URL, but this
 * // could be anything
 * service.setSPEntityId("https://sp.example.com/metadata.xml");
 * service.setAcsUrl("https://sp.example.com/acs");
 * service.setMetadataPath("/metadata.xml");
 * service.setIdpEntityId("https://idp.example.com/sso");
 * service.setMetadataProviderPath("metadata_provider.xml");
 * service.setAuthnRequestSigningEnabled(true);
 * service.setCredentialResolverPath("credential_resolver.xml");
 * service.initialize();
 * ```
 *
 * ## Standards compliance notes
 *
 * The `RelayState` used by Wt exceeds the limit of 80 bytes mandated by the standard
 * (saml-bindings-2.0-os, section 3.4.3). However, most implementations allow this
 * limit to be exceeded.
 *
 * The SP in %Wt does not implement the full SP Lite operational mode
 * (saml-conformance-2.0-os, section 3.1). Missing are:
 *
 *  - HTTP Artifact binding
 *  - Artifact resolution (over SOAP)
 *  - Single logout (SLO)
 *  - Enhanced Client/Proxy SSO, PAOS
 *
 * ## References
 *
 * - saml-core-2.0-os: [Assertions and Protocols for the OASIS Security Assertion Markup Language (SAML) V2.0](https://docs.oasis-open.org/security/saml/v2.0/saml-core-2.0-os.pdf)
 * - saml-bindings-2.0-os: [Bindings for the OASIS Security Security Assertion Markup Language (SAML) V2.0](https://docs.oasis-open.org/security/saml/v2.0/saml-bindings-2.0-os.pdf)
 * - saml-conformance-2.0-os: [Conformance Requirements for the OASIS Security Assertion Markup Language (SAML) V2.0](https://docs.oasis-open.org/security/saml/v2.0/saml-conformance-2.0-os.pdf)
 * - saml-profiles-2.0-os: [Profiles for the OASIS Security Markup Language (SAML) V2.0](https://docs.oasis-open.org/security/saml/v2.0/saml-profiles-2.0-os.pdf)
 * - saml-metadata-2.0-os: [Metadata for the OASIS Security Markup Language (SAML) V2.0](https://docs.oasis-open.org/security/saml/v2.0/saml-metadata-2.0-os.pdf)
 */
class WT_API Service {
public:
  /*! \brief Constructor
   */
  explicit Service(const AuthService &baseAuth);

  /*! \brief Destructor
   *
   * \note The destructor should be run before the `main()` function exits.
   */
  virtual ~Service();

#ifndef WT_TARGET_JAVA
  /*! \brief Initializes the Service so it is ready for use
   *
   * This function should be called after the Service is configured and before the service
   * can be used (i.e. before metadata() or createProcess() are called).
   *
   * No settings should be changed after the Service has been initialized.
   *
   * \note This will call configureLogging() and configureXmlSecurity() in order to set up the global
   *       logging configuration of log4shib and algorithm whitelists or blacklist. If you do not want
   *       the Saml Service to configure these, or configure them differently, you can override
   *       configureLogging() and configureXmlSecurity().
   *
   * \sa configureLogging()
   * \sa configureXmlSecurity()
   */
  void initialize();
#else // WT_TARGET_JAVA
  /*! \brief Initializes OpenSAML so it is ready for use
   */
  static void initialize();
#endif // WT_TARGET_JAVA

  /*! \brief Configure if a popup should be used for the login.
   *
   * When JavaScript is available and this is set to true, a popup will
   * open for the authentication. By default, this is disabled, the session
   * will be suspended and a redirect is used instead.
   *
   * A timeout can be configured in wt_config.xml (see <tt>saml-redirect-timeout</tt>),
   * or with setRedirectTimeout().
   *
   * \sa popupEnabled()
   */
  void setPopupEnabled(bool enable);

  /*! \brief Returns if a popup is used for the login.
   *
   * \sa setPopupEnabled()
   */
  bool popupEnabled() const { return usePopup_; }

  /*! \brief Creates a new authorization process.
   *
   * The service needs to be correctly configured  and
   * initialized before being able to call this function.
   *
   * \throws WException if the Service was not initialized
   *
   * \sa initialize()
   */
  std::unique_ptr<Process> createProcess() const;

  /*! \brief Derives a state value from the session ID.
   *
   * The state value protects the authorization protocol against
   * misuse, and is used to connect an authorization code response
   * with a particular request.
   *
   * In the default implementation the state is the \p sessionId,
   * cryptographically signed. This signature is verified in
   * decodeState().
   */
  std::string encodeState(const std::string &url) const;

  /*! \brief Validates and decodes a state parameter.
   *
   * This function returns the sessionId that is encoded in the state,
   * if the signature validates that it is an authentic state
   * generated by encodeState().
   */
  std::string decodeState(const std::string &state) const;

  Service(const Service &other) = delete;
  Service& operator=(const Service &other) = delete;

  Service(Service &&other) = delete;
  Service& operator=(Service &&other) = delete;

  /*! \brief Returns the basic authentication service.
   */
  const AuthService& baseAuth() const { return baseAuth_; }

  /*! \brief Returns the SPs metadata as XML
   *
   * The result is an XML string with an `<EntityDescriptor>` containing
   * one `<SPSSODescriptor>` describing this SP.
   *
   * See saml-metadata-2.0-os, sections 2.3.2 and 2.4.4
   */
  std::string metadata() const;

  /*! \brief Creates an entity from the given Assertion
   *
   * This function should be specialized to return an Identity based on the
   * given Assertion, e.g.:
   *
   * ```cpp
   *   Wt::Auth::Identity assertionToIdentity(const Wt::Auth::Saml::Assertion &assertion) const override
   *   {
   *     auto name = assertion.attributeValue("name");
   *     auto email = assertion.attributeValue("email");
   *     if (!email || !name) {
   *       return {};
   *     }
   *     return Wt::Auth::Identity("saml", *email, *name, *email, true);
   *   }
   * ```
   */
  virtual Identity assertionToIdentity(const Assertion &assertion) const = 0;

  /*! @name Configuration properties
   * @{
   */

  /*! \brief Sets the secret used when encoding the RelayState
   *
   * This RelayState is used so that responses can be routed to the right WApplication.
   *
   * By default the secret is retrieved from the "saml-secret" configuration property,
   * or if that property is not present, the secret generated automatically.
   *
   * Note that when dedicated session processes are used, a fixed secret has to be set.
   *
   * \sa encodeState()
   * \sa decodeState()
   */
  void setSecret(const std::string &secret);

  /*! \brief Sets the timeout to login when there is no popup.
   *
   * If the user does not return to the application in time, then the
   * session is destroyed.
   *
   * By default the timeout is retrieved from the "saml-redirect-timeout" configuration property,
   * or if that property is not present, the default timeout is equal to 10 minutes.
   */
  void setRedirectTimeout(std::chrono::seconds timeout);

  /*! \brief Sets the provider name.
   *
   * This is a short identifier.
   *
   * \sa Identity::provider()
   * \sa setDescription()
   */
  void setName(const std::string &name);

  /*! \brief Returns the provider name.
   *
   * This is a short identifier.
   *
   * \sa Identity::provider()
   * \sa description()
   * \sa setName()
   */
  const std::string &name() const { return name_; }

  /*! \brief Sets the provider description.
   *
   * This sets a description useful for e.g. tooltips on a login icon.
   *
   * \sa setName()
   */
  void setDescription(const std::string &description);

  /*! \brief Returns the provider description.
   *
   * This returns a description useful for e.g. tooltips on a login icon.
   *
   * \sa name()
   * \sa setDescription()
   */
  const std::string &description() const { return description_; }

  /*! \brief Sets the desired width for the popup window.
   *
   * Defaults to 670 pixels.
   *
   * \sa setPopupHeight()
   */
  void setPopupWidth(int popupWidth);

  /*! \brief Returns the desired width for the popup window.
   *
   * Defaults to 670 pixels.
   *
   * \sa setPopupWidth()
   * \sa popupHeight()
   */
  int popupWidth() const { return popupWidth_; }

  /*! \brief Sets the desired height for the popup window.
   *
   * Defaults to 400 pixels.
   *
   * \sa setPopupWidth()
   */
  void setPopupHeight(int popupHeight);

  /*! \brief Returns the desire height for the popup window.
   *
   * Defaults to 400 pixels.
   *
   * \sa setPopupHeight()
   * \sa popupWidth()
   */
  int popupHeight() const { return popupHeight_; }

  /*! \brief Sets the entity ID of the IdP.
   */
  void setIdpEntityId(const std::string &entityID);

  /*! \brief Gets the entity ID of the IdP.
   *
   * \sa setIdpEntityId()
   */
  const std::string &idpEntityId() const { return idpEntityId_; }

  /*! \brief Sets the format for the name id policy in the authentication request.
   *
   * This defaults to `urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified` (see saml-core-2.0-os, section 8.3).
   *
   * Set to empty to omit the name id policy altogether.
   *
   * \sa setNameIdPolicyAllowCreate()
   */
  void setNameIdPolicyFormat(const std::string &format);

  /*! \brief Gets the format for the name id policy in the authentication request.
   *
   * \sa setNameIdPolicyFormat()
   * \sa nameIdPolicyAllowCreate()
   */
  const std::string &nameIdPolicyFormat() const { return nameIdPolicyFormat_; }

  /*! \brief Sets the AllowCreate parameter for the name id policy in the authentication request.
   *
   * This defaults to true.
   *
   * \sa setNameIdPolicyFormat()
   */
  void setNameIdPolicyAllowCreate(bool allowCreate);

  /*! \brief Gets the AllowCreate parameter for the name id policy in the authentication request.
   *
   * \sa setNameIdPolicyAllowCreate()
   * \sa nameIdPolicyFormat()
   */
  bool nameIdPolicyAllowCreate() const { return nameIdPolicyAllowCreate_; }

  /*! \brief Sets the authentication context class to use in the authentication request.
   *
   * This defaults to `urn:oasis:names:tc:SAML:2.0:ac:classes:Password`.
   *
   * Set to empty to omit the authentication context class altogether.
   *
   * \sa setAuthnContextComparison
   */
  void setAuthnContextClassRef(const std::string &authnContextClassRef);

  /*! \brief Gets the authentication context class to use in the authentication request.
   *
   * \sa setAuthnContextClassRef()
   * \sa authnContextComparison()
   */
  const std::string &authnContextClassRef() const { return authnContextClassRef_; }

  /*! \brief Sets the comparison to use in the authentication context of the authentication request.
   *
   * This defaults to AuthnContextComparison::Exact.
   *
   * \sa setAuthnContextClassRef()
   */
  void setAuthnContextComparison(AuthnContextComparison comparison);

  /*! \brief Gets the comparison to use in the authentication context of the application request.
   *
   * \sa setAuthnContextComparison()
   * \sa authnContextClassRef()
   */
  AuthnContextComparison authnContextComparison() const { return authnContextComparison_; }

  /*! \brief Sets the entity ID of this SP.
   */
  void setSPEntityId(const std::string &entityID);

  /*! \brief Gets the entity ID of this SP.
   *
   * \sa setSPEntityId()
   */
  const std::string &spEntityId() const { return spEntityId_; }

  /*! \brief Sets the %Assertion Consumer %Service URL
   *
   * This should be an absolute URL. E.g. if your application
   * is deployed on `https://sp.example.com`, then your ACS URL
   * could be `https://sp.example.com/acs`. This is the URL that
   * responses will be sent to.
   */
  void setAcsUrl(const std::string &url);

  /*! \brief Gets the %Assertion Consumer %Service URL
   *
   * \sa setAcsUrl()
   */
  const std::string &acsUrl() const { return acsUrl_; };

  /*! \brief Gets the %Assertion Consumer %Service path
   *
   * This strips the domain name from the acsUrl(), e.g. if the
   * acsUrl() is `https://sp.example.com/acs` then this will return
   * `/acs`. This is the path at which the WResource for the
   * %Assertion Consumer %Service will be deployed.
   */
  std::string acsPath() const;

  /*! \brief Sets the path at which the metadata of this SP will be made available
   *
   * For example, if your application is deployed at `https://sp.example.com`, and
   * the \p path is set to `/metadata.xml`, then the metadata XML will be made available
   * at `https://sp.example.com/metadata.xml`.
   *
   * This is left empty by default. If left empty, no metadata endpoint will be created.
   */
  void setMetadataPath(const std::string &path);

  /*! \brief Returns the path at which the metadata of this SP will be made available
   *
   * \sa setMetadataPath()
   */
  const std::string &metadataPath() const { return metadataPath_; }

#ifndef WT_TARGET_JAVA
  /*! \brief Sets the path to the XML file that describes the metadata provider
   *
   * This is used to get the metadata of the IdP, like the signing key and Single Sign On endpoint.
   *
   * Example XML file:
   *
   * ```xml
   * <?xml version="1.0" encoding="UTF-8"?>
   * <MetadataProvider type="XML" path="/path/to/metadata.xml" validate="0">
   *   <KeyInfoResolver type="Inline" keyInfoReferences="true" />
   * </MetadataProvider>
   * ```
   *
   * See the [Shibboleth SP documentation on MetadataProviders](https://wiki.shibboleth.net/confluence/display/SP3/MetadataProvider) for more info.
   */
  void setMetadataProviderPath(const std::string &path);
#else // WT_TARGET_JAVA
  /*! \brief Sets the metadata reolver
   *
   * This is used to get the metadata of the IdP, like the signing key and Single Sign On endpoint.
   */
  void setMetadataResolver(MetadataResolver *resolver);
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Returns the path to the XML file that describes the metadata provider
   *
   * \sa setMatadataProviderPath()
   */
  const std::string &metadataProviderPath() const { return metadataProviderPath_; }
#else // WT_TARGET_JAVA
  /*! \brief Returns the metadata resolver
   */
  MetadataResolver *metadataResolver() const { return metadataResolver_; }
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Sets the path to the XML that describes the credential resolver
   *
   * This is used to resolve signing (and encryption) credentials for this SP, used to
   * sign authentication requests and decrypt encrypted assertions, subject ids, or attributes.
   *
   * Example XML file:
   *
   * ```xml
   * <?xml version="1.0" encoding="UTF-8"?>
   * <CredentialResolver type="Chaining"
   *   <CredentialResolver type="File" use="signing">
   *     <Key>
   *       <Path>/path/to/signing.pem</Path>
   *     </Key>
   *     <Certificate>
   *       <Path>/path/to/signing.crt</Path>
   *     </Certificate>
   *   </CredentialResolver>
   *   <CredentialResolver type="File" use="encryption">
   *     <Key>
   *       <Path>/path/to/encryption.pem</Path>
   *     </Key>
   *     <Certificate>
   *       <Path>/path/to/encryption.crt</Path>
   *     </Certificate>
   *   </CredentialResolver>
   * </CredentialResolver>
   * ```
   *
   * See the [Shibboleth SP documentation on CredentialResolvers](https://wiki.shibboleth.net/confluence/display/SP3/CredentialResolver) for more info.
   */
  void setCredentialResolverPath(const std::string &path);
#else // WT_TARGET_JAVA
  /*! \brief Sets the credential resolver
   *
   * This is used to resolve signing (and encryption) credentials for this SP, used to
   * sign authentication requests and decrypt encrypted assertions, subject ids, or attributes.
   */
  void setCredentialResolver(CredentialResolver *resolver);
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Returns the path to the XML file that describes the credential resolver
   *
   * \sa setCredentialResolverPath()
   */
  const std::string &credentialResolverPath() const { return credentialResolverPath_; }
#else // WT_TARGET_JAVA
  /*!\brief Returns the credential resolver
   *
   */
  CredentialResolver *credentialResolver() const { return credentialResolver_; }
#endif // WT_TARGET_JAVA

  /*! \brief Sets whether responses should be validated against the XML schema
   *
   * This is enabled by default, and should preferably not be disabled.
   */
  void setXmlValidationEnabled(bool enabled);

  /*! \brief Retursn whether responses should be validated against the XML schema
   *
   * \sa setXmlValidationEnabled()
   */
  bool xmlValidationEnabled() const { return xmlValidationEnabled_; }

#ifndef WT_TARGET_JAVA
  /*! \brief Sets the path of XMLTooling's `catalog.xml` file
   *
   * This path is used to validate XML responses from the IdP.
   * The schemas from XMLTooling are used to validate the XML of
   * things like XML signatures.
   *
   * On non-Windows, this defaults to `/usr/share/xml/xmltooling/catalog.xml`.
   * On Windows, this defaults to `C:\ProgramData\Shibboleth\SP\xml\xmltooling\catalog.xml`.
   *
   * This file is mandatory if XML validation is enabled (the default).
   * Disabling XML validation is not recommended.
   *
   * \sa setXmlValidationEnabled()
   */
  void setXmlToolingCatalogPath(const std::string &path);

  /*! \brief Returns the path of XMLTooling's `catalog.xml` file
   *
   * \sa setXMLToolingCatalogPath()
   */
  const std::string &xmlToolingCatalogPath() const { return xmlToolingCatalogPath_; }

  /*! \brief Sets the path of OpenSAML's `saml20-catalog.xml` file
   *
   * This path is used to validate XML responses from the IdP.
   * The schemas from OpenSAML are used to validate the XML of
   * things like responses, assertions,...
   *
   * On non-Windows, this defaults to `/usr/share/xml/opensaml/saml20-catalog.xml`.
   * On Windows, this defaults to `C:\ProgramData\Shibboleth\SP\xml\opensaml\saml20-catalog.xml`.
   *
   * This file is mandatory if XML validation is enabled (the default).
   * Disabling XML validation is not recommended.
   *
   * \sa setX
   */
  void setSamlCatalogPath(const std::string &path);

  /*! \brief Returns the path of OpenSAML's `saml20-catalog.xml` file
   *
   * \sa setSamlCatalogPath()
   */
  const std::string &samlCatalogPath() const { return samlCatalogPath_; }
#endif // WT_TARGET_JAVA

  /*! \brief Sets the signature policy
   *
   * The SignaturePolicy determines which signatures should be
   * present in the response of the identity provider (IdP).
   *
   * This defaults to the most strict value of
   * SignaturePolicy::SignedResponseAndAssertion.
   *
   * \note SignaturePolicy::Unsafe is **not** recommended for
   *       production use. As the name suggests is **not**
   *       secure and makes exploits trivial.
   */
  void setSignaturePolicy(SignaturePolicy policy);

  /*! \brief Returns the current signature policy
   *
   * \sa setSignaturePolicy()
   */
  SignaturePolicy signaturePolicy() const { return signaturePolicy_; }

  /*! @} */

#ifndef WT_TARGET_JAVA
protected:
  /*! \brief Sets up logging
   *
   * By default, this makes all log4shib logging redirect to Wt's logger.
   *
   * You can configure log4shib differently, or not set up logging altogether
   * by overriding this function.
   */
  virtual void configureLogging();

  /*! \brief Configures the algorithm whitelist
   *
   * You can configure the whitelist differently, or not set up the whitelist altogether
   * by overriding this function.
   *
   * By default, the following algorithms are supported:
   *
   * - Hashing algorithms (for digest)
   *   - http://www.w3.org/2001/04/xmlenc#sha256
   *   - http://www.w3.org/2001/04/xmldsig-more#sha384
   *   - http://www.w3.org/2001/04/xmlenc#sha512
   * - Signing algorithms
   *   - http://www.w3.org/2001/04/xmldsig-more#rsa-sha256
   *   - http://www.w3.org/2001/04/xmldsig-more#rsa-sha384
   *   - http://www.w3.org/2001/04/xmldsig-more#rsa-sha512
   * - Encryption algorithms
   *   - http://www.w3.org/2001/04/xmlenc#aes128-cbc
   *   - http://www.w3.org/2001/04/xmlenc#aes192-cbc
   *   - http://www.w3.org/2001/04/xmlenc#aes256-cbc
   *   - http://www.w3.org/2009/xmlenc11#aes128-gcm
   *   - http://www.w3.org/2009/xmlenc11#aes192-gcm
   *   - http://www.w3.org/2009/xmlenc11#aes256-gcm
   */
  virtual void configureXmlSecurity();
#endif // WT_TARGET_JAVA

private:
  class StaticAcsResource;
  class MetadataResource;

  const AuthService &baseAuth_;

  // state
  std::unique_ptr<ServiceImpl> impl_;

  // configuration
  std::string secret_;
  std::chrono::seconds redirectTimeout_;
  std::string name_;
  std::string description_;
  int popupWidth_;
  int popupHeight_;
  std::string idpEntityId_;
  std::string nameIdPolicyFormat_;
  std::string authnContextClassRef_;
  AuthnContextComparison authnContextComparison_;
  std::string spEntityId_;
  std::string acsUrl_;
  std::string metadataPath_;
#ifndef WT_TARGET_JAVA
  std::string metadataProviderPath_;
#else // WT_TARGET_JAVA
  MetadataResolver *metadataResolver_;
#endif // WT_TARGET_JAVA
#ifndef WT_TARGET_JAVA
  std::string credentialResolverPath_;
#else // WT_TARGET_JAVA
  CredentialResolver *credentialResolver_;
#endif // WT_TARGET_JAVA
#ifndef WT_TARGET_JAVA
  std::string xmlToolingCatalogPath_;
  std::string samlCatalogPath_;
#endif // WT_TARGET_JAVA
  SignaturePolicy signaturePolicy_;
  bool xmlValidationEnabled_;
  bool nameIdPolicyAllowCreate_;
  bool usePopup_;

  void checkInitialized() const; // Throws if not initialized
  void checkConfig() const; // Throws if configuration is not ok
  void generateAcsEndpoint();
  void generateMetadataEndpoint();

  static std::string configurationProperty(const std::string &property);

  friend class Process;
  friend class ProcessImpl;
  friend class ServiceImpl;
};

    }
  }
}

#endif // WT_AUTH_SAML_SERVICE_H_
