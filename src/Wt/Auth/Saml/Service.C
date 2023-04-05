/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Service.h"

#include "Assertion.h"
#include "Process.h"

#include "ServiceImpl.h"

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLogger.h>
#include <Wt/WRandom.h>
#include <Wt/WResource.h>
#include <Wt/WServer.h>

#include <Wt/Auth/AuthUtils.h>

#include <Wt/Http/Client.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include "web/WebUtils.h"

#include <string>

namespace {

const char * const PASSWORD_AUTHN_CTX_CLASS_REF = "urn:oasis:names:tc:SAML:2.0:ac:classes:Password";

}

namespace Wt {

LOGGER("Auth.Saml.Service");

}

namespace Wt {
  namespace Auth {
    namespace Saml {

class Service::StaticAcsResource final : public WResource {
public:
  explicit StaticAcsResource(const Service &service);

  ~StaticAcsResource() final;

protected:
  void handleRequest(const Http::Request &request,
                     Http::Response &response) final;

private:
  const Service &service_;

  std::string appendOriginalUrl(const std::string &baseUrl,
                                const Http::Request &request) const;
  static void sendError(Http::Response &response);
};

class Service::MetadataResource final : public WResource {
public:
  explicit MetadataResource(const Service &service);

  ~MetadataResource() final;

protected:
  void handleRequest(const Http::Request &request,
                     Http::Response &response) final;

private:
  const Service &service_;
};

Service::StaticAcsResource::StaticAcsResource(const Service &service)
  : service_(service)
{ }

Service::StaticAcsResource::~StaticAcsResource()
{
  beingDeleted();
}

void Service::StaticAcsResource::handleRequest(const Http::Request &request,
                                               Http::Response &response)
{
  if (request.method() != "POST") {
    LOG_DEBUG("Invalid request to Assertion Consumer Service: wrong method: " << request.method() << " instead of POST");
    sendError(response);
    return;
  }

  const std::string *const relayStatePtr = request.getParameter("RelayState");
  if (!relayStatePtr) {
    LOG_DEBUG("Invalid request to Assertion Consumer Service: missing RelayState");
    sendError(response);
    return;
  }

  const std::string &relayState = *relayStatePtr;
  std::string url = service_.decodeState(relayState);
  if (url.empty()) {
    LOG_SECURE("Invalid request to Assertion Consumer Service: failed to decode RelayState");
    sendError(response);
    return;
  }

  url = appendOriginalUrl(url, request);

  // Temporary redirect, use same method (POST)
  response.setStatus(307);
  response.addHeader("Location", url);
}

std::string Service::StaticAcsResource::appendOriginalUrl(const std::string &baseUrl,
                                                          const Http::Request &request) const
{
  std::string originalUrl =
    request.urlScheme() +
    "://" +
    request.hostName() +
#ifndef WT_TARGET_JAVA
    request.path() +
    request.pathInfo();
#else // WT_TARGET_JAVA
    request.requestURI();
#endif // WT_TARGET_JAVA
  const std::string queryString = request.queryString();
  if (
#ifdef WT_TARGET_JAVA
      // queryString may be null in Java, doesn't make sense in C++
      &queryString != nullptr &&
#endif // WT_TARGET_JAVA
      !queryString.empty()) {
    originalUrl += "?" + queryString;
  }
  const std::string originalUrlState = service_.encodeState(originalUrl);

  std::string result  = baseUrl;
  if (result.find('?') != std::string::npos) {
    result += '&';
  } else {
    result += '?';
  }
  result += "originalUrl=" + originalUrlState;

  return result;
}

void Service::StaticAcsResource::sendError(Http::Response &response)
{
  response.setStatus(400);
  response.setMimeType("text/html");
  response.out() << "<html><body>"
                 << "<h1>SAML Authentication error</h1>"
                 << "</body></html>";
}

Service::MetadataResource::MetadataResource(const Service &service)
  : service_(service)
{ }

Service::MetadataResource::~MetadataResource()
{
  beingDeleted();
}

void Service::MetadataResource::handleRequest(const Http::Request &request,
                                              Http::Response &response)
{
  response.setMimeType("text/xml");
  response.out() << service_.metadata();
}


Service::Service(const AuthService &baseAuth)
  : baseAuth_(baseAuth),
    redirectTimeout_(std::chrono::minutes(10)),
    name_("SAML"),
    description_("SAML Single Sign-On"),
    popupWidth_(670),
    popupHeight_(400),
    authnContextClassRef_(PASSWORD_AUTHN_CTX_CLASS_REF),
    authnContextComparison_(AuthnContextComparison::Exact),
#ifdef WT_TARGET_JAVA
    metadataResolver_(nullptr),
    credentialResolver_(nullptr),
#endif
    signaturePolicy_(SignaturePolicy::SignedResponseAndAssertion),
    xmlValidationEnabled_(true),
    nameIdPolicyAllowCreate_(true),
    usePopup_(false)
{
  nameIdPolicyFormat_ = "urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified";

  try {
    secret_ = Service::configurationProperty("saml-secret");
  } catch (std::exception &e) {
    secret_ = WRandom::generateId(32);
  }
  try {
    std::string redirectTimeoutStr = Service::configurationProperty("saml-redirect-timeout");
    redirectTimeout_ = std::chrono::seconds(Wt::Utils::stoll(redirectTimeoutStr));
  } catch (std::exception &e) {
  }

#ifndef WT_TARGET_JAVA
#ifdef WT_WIN32
  xmlToolingCatalogPath_ = R"=(C:\ProgramData\Shibboleth\SP\xml\xmltooling\catalog.xml)=";
  samlCatalogPath_ = R"=(C:\ProgramData\Shibboleth\SP\xml\opensaml\saml20-catalog.xml)=";
#endif

#ifndef WT_WIN32
  xmlToolingCatalogPath_ = "/usr/share/xml/xmltooling/catalog.xml";
  samlCatalogPath_ = "/usr/share/xml/opensaml/saml20-catalog.xml";
#endif // WT_WIN32
#endif // WT_TARGET_JAVA
}

Service::~Service()
{ }

void Service::initialize()
{
#ifndef WT_TARGET_JAVA
  if (impl_)
    return;

  checkConfig();
  configureLogging();
  impl_ = std::make_unique<ServiceImpl>(*this);
  configureXmlSecurity();
  generateAcsEndpoint();
  generateMetadataEndpoint();
#else // WT_TARGET_JAVA
  ServiceImpl::initialize();
#endif // WT_TARGET_JAVA
}

void Service::setPopupEnabled(bool enable)
{
  usePopup_ = enable;
}

#ifndef WT_TARGET_JAVA
void Service::configureLogging()
{
  ServiceImpl::configureLogging();
}

void Service::configureXmlSecurity()
{
  ServiceImpl::configureXmlSecurity();
}
#endif // WT_TARGET_JAVA

void Service::setSecret(const std::string &secret)
{
  secret_ = secret;
}

void Service::setRedirectTimeout(std::chrono::seconds timeout)
{
  redirectTimeout_ = timeout;
}

void Service::setName(const std::string &name)
{
  name_ = name;
}

void Service::setDescription(const std::string &description)
{
  description_ = description;
}

void Service::setPopupWidth(int popupWidth)
{
  popupWidth_ = popupWidth;
}

void Service::setPopupHeight(int popupHeight)
{
  popupHeight_ = popupHeight;
}

std::string Service::encodeState(const std::string &url) const
{
  return Utils::encodeState(secret_, url);
}

std::string Service::decodeState(const std::string &state) const
{
  return Utils::decodeState(secret_, state);
}

void Service::setIdpEntityId(const std::string &entityID)
{
  idpEntityId_ = entityID;
}

void Service::setNameIdPolicyFormat(const std::string &format)
{
  nameIdPolicyFormat_ = format;
}

void Service::setNameIdPolicyAllowCreate(bool allowCreate)
{
  nameIdPolicyAllowCreate_ = allowCreate;
}

void Service::setAuthnContextClassRef(const std::string &authnContextClassRef)
{
  authnContextClassRef_ = authnContextClassRef;
}

void Service::setAuthnContextComparison(Wt::Auth::Saml::AuthnContextComparison comparison)
{
  authnContextComparison_ = comparison;
}

void Service::setSPEntityId(const std::string &entityID)
{
  spEntityId_ = entityID;
}

std::string Service::acsPath() const
{
  Http::Client::URL url;
  if (!Http::Client::parseUrl(acsUrl_, url)) {
    LOG_ERROR("Could not parse Assertion Consumer Service URL: " << acsUrl_);
    return "";
  }
  return url.path;
}

void Service::setAcsUrl(const std::string &url)
{
  acsUrl_ = url;
}

void Service::setMetadataPath(const std::string &path)
{
  metadataPath_ = path;
}

#ifndef WT_TARGET_JAVA
void Service::setMetadataProviderPath(const std::string &path)
{
  metadataProviderPath_ = path;
}
#else // WT_TARGET_JAVA
void Service::setMetadataResolver(MetadataResolver *resolver)
{
  metadataResolver_ = resolver;
}
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
void Service::setCredentialResolverPath(const std::string &path)
{
  credentialResolverPath_ = path;
}
#else // WT_TARGET_JAVA
void Service::setCredentialResolver(CredentialResolver *resolver)
{
  credentialResolver_ = resolver;
}
#endif // WT_TARGET_JAVA

void Service::setXmlValidationEnabled(bool enabled)
{
  xmlValidationEnabled_ = enabled;
}

#ifndef WT_TARGET_JAVA
void Service::setXmlToolingCatalogPath(const std::string &path)
{
  xmlToolingCatalogPath_ = path;
}

void Service::setSamlCatalogPath(const std::string &path)
{
  samlCatalogPath_ = path;
}
#endif // WT_TARGET_JAVA

std::unique_ptr<Process> Service::createProcess() const
{
  checkInitialized();
  return std::unique_ptr<Process>(new Process(*this));
}

void Service::generateAcsEndpoint()
{
  auto resource = std::make_shared<StaticAcsResource>(*this);
  std::string path = acsPath();

  LOG_INFO("deploying endpoint at " << path);
  WApplication *app = WApplication::instance();
  WServer *server = nullptr;
  if (app) {
    server = app->environment().server();
  } else {
    server = WServer::instance();
  }

  server->addResource(resource, path);
}

void Service::generateMetadataEndpoint()
{
  if (metadataPath_.empty())
    return;

  auto resource = std::make_shared<MetadataResource>(*this);
  WApplication *app = WApplication::instance();
  WServer *server = nullptr;
  if (app) {
    server = app->environment().server();
  } else {
    server = WServer::instance();
  }

  server->addResource(resource, metadataPath_);
}

std::string Service::metadata() const
{
  checkInitialized();
  return impl_->metadata();
}

void Service::setSignaturePolicy(SignaturePolicy policy)
{
  signaturePolicy_ = policy;
}

void Service::checkConfig() const
{
  const std::string prefix = "Saml::Service: incorrect configuration: ";
  if (secret_.empty()) {
    throw Wt::WException(prefix + "missing secret");
  }
  if (idpEntityId_.empty()) {
    throw Wt::WException(prefix + "missing IdP entityID, please set it with setIdpEntityId()");
  }
  if (spEntityId_.empty()) {
    throw Wt::WException(prefix + "missing SP entityID, please set it with setSPEntityId()");
  }
  if (acsUrl_.empty()) {
    throw Wt::WException(prefix + "missing ACS URL, please set it with setAcsUrl()");
  }
  Http::Client::URL url;
  if (!Http::Client::parseUrl(acsUrl_, url)) {
    throw Wt::WException(prefix + "could not parse ACS URL: '" + acsUrl_ + "'");
  }
  if (!metadataPath_.empty() && metadataPath_[0] != '/') {
    throw Wt::WException(prefix + "invalid metadata path (should start with slash): '" + metadataPath_ + "'");
  }
#ifndef WT_TARGET_JAVA
  if (metadataProviderPath_.empty()) {
    throw Wt::WException(prefix + "missing metadata provider path, please set it with setMetadataProviderPath()");
  }
#else // WT_TARGET_JAVA
  if (!metadataResolver_) {
    throw Wt::WException(prefix + "missing metadata resolver, please set it with setMetadataResolver()");
  }
#endif // WT_TARGET_JAVA
  if (xmlValidationEnabled_) {
#ifndef WT_TARGET_JAVA
    if (samlCatalogPath_.empty()) {
      throw Wt::WException(prefix + "XML validation enabled, but no SAML catalog path set, please set it with setSamlCatalogPath()");
    }
    if (xmlToolingCatalogPath_.empty()) {
      throw Wt::WException(prefix + "XML validation enabled, but no XMLTooling catalog path set, please set it with setXmlToolingCatalogPath()");
    }
#endif // WT_TARGET_JAVA
  } else {
    LOG_WARN("configured without XML validation");
  }
  if (signaturePolicy_ == SignaturePolicy::Unsafe) {
    LOG_WARN("configured with Unsafe SignaturePolicy");
  }
}

void Service::checkInitialized() const
{
  if (!impl_) {
    throw Wt::WException("Saml::Service not initialized, please call initialize() before use");
  }
}

std::string Service::configurationProperty(const std::string &property)
{
  return Utils::configurationProperty("SAML", property);
}

    }
  }
}
