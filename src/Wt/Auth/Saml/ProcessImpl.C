/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ProcessImpl.h"

#include "Wt/WApplication.h"

#include "Wt/Auth/Saml/Process.h"
#include "Wt/Auth/Saml/Service.h"
#include "Wt/Auth/Saml/ServiceImpl.h"

#include "Wt/Http/Client.h"
#include "Wt/Http/Response.h"

#include "web/WebUtils.h"

#include <xsec/dsig/DSIGConstants.hpp>

#include <xmltooling/XMLToolingConfig.h>
#include <xmltooling/io/HTTPRequest.h>
#include <xmltooling/io/HTTPResponse.h>
#include <xmltooling/security/Credential.h>
#include <xmltooling/security/CredentialCriteria.h>
#include <xmltooling/util/ParserPool.h>

#include <saml/version.h>
#include <saml/SAMLConfig.h>
#include <saml/binding/MessageDecoder.h>
#include <saml/binding/MessageEncoder.h>
#include <saml/saml2/core/Assertions.h>
#include <saml/saml2/core/Protocols.h>
#include <saml/saml2/metadata/Metadata.h>
#include <saml/saml2/metadata/MetadataProvider.h>
#include <saml/saml2/profile/SAML2AssertionPolicy.h>

namespace saml2 = opensaml::saml2;
namespace saml2p = opensaml::saml2p;
namespace saml2md = opensaml::saml2md;

namespace Wt {

LOGGER("Auth.Saml.Process");

}

namespace {

// Casts a pointer to an owned base class
// to a unique_ptr of type derived. If the
// cast fails the object does not leak.
template<typename Derived, typename Base>
std::unique_ptr<Derived> unique_ptr_cast(Base *ownedBase)
{
  std::unique_ptr<Base> basePtr(ownedBase);
  if (basePtr) {
    auto result = dynamic_cast<Derived *>(ownedBase);
    if (result) {
      (void) basePtr.release();
      return std::unique_ptr<Derived>(result);
    }
  }
  return nullptr;
}

}

namespace Wt {
  namespace Auth {
    namespace Saml {

// Wraps a Wt::Http::Request to decode responses
class ProcessImpl::Request final : public xmltooling::HTTPRequest {
public:
  explicit Request(const Process &process,
                   const Wt::Http::Request &request)
          : process_(process),
            request_(request)
  {
    const std::string *state = request.getParameter("originalUrl");
    if (state) {
      url_ = process_.service_.decodeState(*state);
    }

    method_ = request_.method();
    Http::Client::URL url;
    if (Http::Client::parseUrl(url_, url)) {
      requestPath_ = url.path;
    }
    queryString_ = request_.queryString();
    urlScheme_ = request_.urlScheme();
    hostname_ = request_.hostName();
    body_ = std::string(std::istreambuf_iterator<char>(request_.in()), {});
  }

  ~Request() final = default;

  const char *getMethod() const override
  {
    return method_.c_str();
  }

  const char *getRequestURI() const override
  {
    return requestPath_.c_str();
  }

  const char *getRequestURL() const override
  {
    return url_.c_str();
  }

  const char *getQueryString() const override
  {
    return queryString_.c_str();
  }

  std::string getHeader(const char *name) const override
  {
    return request_.headerValue(name);
  }

  const char *getScheme() const override
  {
    return urlScheme_.c_str();
  }

  const char *getHostname() const override
  {
    return hostname_.c_str();
  }

  int getPort() const override
  {
    return Utils::stoi(request_.serverPort());
  }

  std::string getContentType() const override
  {
    return request_.contentType();
  }

  long getContentLength() const override
  {
    return request_.contentLength();
  }

  const char *getRequestBody() const override
  {
    return body_.c_str();
  }

  const char *getParameter(const char *name) const override
  {
    auto strPtr = request_.getParameter(name);
    if (strPtr) {
      return strPtr->c_str();
    } else {
      return nullptr;
    }
  }

  std::size_t getParameters(const char *name, std::vector<const char *> &values) const override
  {
    auto paramValues = request_.getParameterValues(name);
    for (auto &value : paramValues) {
      values.push_back(value.c_str());
    }
    return paramValues.size();
  }

  std::string getRemoteUser() const override
  {
    LOG_ERROR("getRemoteUser(): unsupported function");
    return "";
  }

  std::string getRemoteAddr() const override
  {
    return request_.clientAddress();
  }

  const std::vector<XSECCryptoX509 *> &getClientCertificates() const override
  {
    LOG_ERROR("getClientCertificates(): unsupported function");
    return clientCerts_;
  }

private:
  const Process &process_;
  const Wt::Http::Request &request_;
  std::string url_;
  std::string method_;
  std::string queryString_;
  std::string urlScheme_;
  std::string body_;
  std::string requestPath_;
  std::string hostname_;
  std::vector<XSECCryptoX509 *> clientCerts_;
};

// Wraps a Wt::Http::Response to redirect the AuthnRequest
class ProcessImpl::Response final : public xmltooling::HTTPResponse {
public:
  explicit Response(Wt::Http::Response &response)
          : response_(response)
  { }

  // NOTE: there is not normally any input to send
  long sendResponse(std::istream &inputStream, long status) override
  {
    response_.setStatus(static_cast<int>(status));
    return status;
  }

  long sendRedirect(const char *url) override
  {
    auto result = HTTPResponse::sendRedirect(url);
    response_.setStatus(static_cast<int>(result));
    response_.addHeader("Location", url);
    return result;
  }

private:
  Wt::Http::Response &response_;
};

ProcessImpl::ProcessImpl(Process &process)
  : process_(process)
{ }

bool ProcessImpl::createAuthnRequest(Wt::Http::Response &response)
{
  process_.error_ = Wt::WString::Empty;
  process_.assertion_.subject = nullptr;
  process_.assertion_.attributes.clear();

  auto &service = process_.service_;
  xmltooling::auto_ptr_XMLCh redirectBinding(samlconstants::SAML20_BINDING_HTTP_REDIRECT);
  xmltooling::auto_ptr_XMLCh postBinding(samlconstants::SAML20_BINDING_HTTP_POST);
  xmltooling::auto_ptr_XMLCh nameIdPolicyFormat(service.nameIdPolicyFormat().c_str());
  xmltooling::auto_ptr_XMLCh authnContextClassRef(service.authnContextClassRef().c_str());
  xmltooling::auto_ptr_XMLCh acsEndpoint(service.acsUrl().c_str());
  xmltooling::auto_ptr_XMLCh issuerName(service.spEntityId().c_str());
  auto identifier = service.impl_->generateIdentifier();
  std::u16string location;

  std::unique_ptr<saml2p::AuthnRequest> authnRequest(saml2p::AuthnRequestBuilder::buildAuthnRequest());

  {
    auto now = std::chrono::system_clock::now();
    authnRequest->setIssueInstant(std::chrono::system_clock::to_time_t(now));
  }

  {
    location = service.impl_->ssoUrl();
    if (location.empty()) {
      LOG_ERROR("Failed to find Single Sign On endpoint for IdP '" << service.idpEntityId() << "'");
      process_.error_ = Wt::utf8("Failed to find Single Sign On endpoint in metadata.");
      return false;
    }
    authnRequest->setDestination(location.c_str());
  }

  authnRequest->setProtocolBinding(postBinding.get());

  authnRequest->setAssertionConsumerServiceURL(acsEndpoint.get());

  process_.requestIdentifier_ = identifier.first;
  authnRequest->setID(identifier.second.c_str());

  {
    std::unique_ptr<saml2::Issuer> issuer(saml2::IssuerBuilder::buildIssuer());
    issuer->setName(issuerName.get());
    authnRequest->setIssuer(issuer.release());
  }

  if (!service.nameIdPolicyFormat().empty()) {
    std::unique_ptr<saml2p::NameIDPolicy> nameIdPolicy(saml2p::NameIDPolicyBuilder::buildNameIDPolicy());
    nameIdPolicy->setFormat(nameIdPolicyFormat.get());
    nameIdPolicy->setAllowCreate(service.nameIdPolicyAllowCreate() ? xmlconstants::XML_TRUE : xmlconstants::XML_FALSE);
    authnRequest->setNameIDPolicy(nameIdPolicy.release());
  }

  if (!service.authnContextClassRef().empty()) {
    std::unique_ptr<saml2p::RequestedAuthnContext> requestedAuthnContext(
            saml2p::RequestedAuthnContextBuilder::buildRequestedAuthnContext());

    {
      std::unique_ptr<saml2::AuthnContextClassRef> passwordAuthnContextClassRef(
              saml2::AuthnContextClassRefBuilder::buildAuthnContextClassRef());
      passwordAuthnContextClassRef->setReference(authnContextClassRef.get());
      requestedAuthnContext->getAuthnContextClassRefs().push_back(passwordAuthnContextClassRef.release());
    }

    switch (service.authnContextComparison()) {
      case AuthnContextComparison::Exact: {
        requestedAuthnContext->setComparison(saml2p::RequestedAuthnContext::COMPARISON_EXACT);
        break;
      }
      case AuthnContextComparison::Better: {
        requestedAuthnContext->setComparison(saml2p::RequestedAuthnContext::COMPARISON_BETTER);
        break;
      }
      case AuthnContextComparison::Maximum: {
        requestedAuthnContext->setComparison(saml2p::RequestedAuthnContext::COMPARISON_MAXIMUM);
        break;
      }
      case AuthnContextComparison::Minimum: {
        requestedAuthnContext->setComparison(saml2p::RequestedAuthnContext::COMPARISON_MINIMUM);
        break;
      }
    }

    authnRequest->setRequestedAuthnContext(requestedAuthnContext.release());
  }

  xercesc::DOMDocument *encoderConfig = ::Wt::Auth::Saml::ServiceImpl::xmlToolingConfig().getParser().newDocument();
  xmltooling::XercesJanitor<xercesc::DOMDocument> documentJanitor(encoderConfig);
  std::unique_ptr<opensaml::MessageEncoder> encoder(
          ::Wt::Auth::Saml::ServiceImpl::samlConfig().MessageEncoderManager.newPlugin(
                  samlconstants::SAML20_BINDING_HTTP_REDIRECT,
                  encoderConfig->getDocumentElement(),
                  false
          ));

  auto app = Wt::WApplication::instance();
  auto url = app->makeAbsoluteUrl(process_.privateAcsResourceUrl());
  auto relayState = service.encodeState(url);

  xmltooling::auto_ptr_char u8Location(location.c_str());
  Response responseWrapper(response);
  xmltooling::CredentialCriteria cc;
  cc.setUsage(xmltooling::Credential::SIGNING_CREDENTIAL);
  auto credentialResolver = service.impl_->credentialResolver();
  const xmltooling::Credential *cred = nullptr;
  if (credentialResolver) {
    xmltooling::Locker credentialLocker(credentialResolver);
    cred = credentialResolver->resolve(&cc);
  }

  encoder->encode(
          responseWrapper,
          authnRequest.release(),
          u8Location.get(),
          nullptr,
          relayState.c_str(),
          nullptr,
          cred,
          u"" URI_ID_RSA_SHA256
  );

  return true;
}

bool ProcessImpl::handleResponse(const Http::Request &request)
{
  auto &service = process_.service_;

  std::unique_ptr<saml2p::Response> response = decodeResponse(request);
  if (!response) {
    return false;
  }

  xmltooling::auto_ptr_XMLCh spEntityId(service.spEntityId_.c_str());

  auto assertion = getAssertion(spEntityId.get(), *response);

  if (!assertion.first) {
    return false;
  }

  auto subject = getSubject(spEntityId.get(), *assertion.first);
  auto attributes = getAttributes(spEntityId.get(), *assertion.first);

  process_.assertion_.subject = std::move(subject);
  process_.assertion_.attributes = std::move(attributes);

  return true;
}

std::unique_ptr<saml2p::Response> ProcessImpl::decodeResponse(const Http::Request &request)
{
  auto &service = process_.service();

  xmltooling::auto_ptr_XMLCh idpEntityId(service.idpEntityId_.c_str());
  xmltooling::auto_ptr_XMLCh spEntityId(service.spEntityId_.c_str());
  xmltooling::auto_ptr_XMLCh inResponseTo(process_.requestIdentifier_.c_str());

  std::unique_ptr<saml2p::Response> response;

  auto policy = createPolicy(idpEntityId.get(), spEntityId.get(), inResponseTo.get());

  std::unique_ptr<opensaml::MessageDecoder> decoder(
          ::Wt::Auth::Saml::ServiceImpl::samlConfig().MessageDecoderManager.newPlugin(
                  samlconstants::SAML20_BINDING_HTTP_POST, nullptr, false
          ));

  Request req(process_, request);

  std::string relayState;

  try {
#if OPENSAML_VERSION_MAJOR > 3 || (OPENSAML_VERSION_MAJOR == 3 && OPENSAML_VERSION_MINOR >= 1)
    response = unique_ptr_cast<saml2p::Response>(decoder->decode(relayState, req, nullptr, *policy));
#else
    response = unique_ptr_cast<saml2p::Response>(decoder->decode(relayState, req, *policy));
#endif
  } catch (std::exception &exception) {
    LOG_ERROR("An exception occurred when trying to decode SAML response: " << exception.what());
    process_.error_ = Wt::utf8("Failed to decode SAML response");
    return nullptr;
  }

  if (!response) {
    LOG_ERROR("Failed to decode SAML response");
    process_.error_ = Wt::utf8("Failed to decode SAML response");
    return nullptr;
  }

  if ((service.signaturePolicy_ == SignaturePolicy::SignedResponse ||
       service.signaturePolicy_ == SignaturePolicy::SignedResponseAndAssertion) &&
      !policy->isAuthenticated()) {
    LOG_ERROR("SAML response not signed");
    process_.error_ = Wt::utf8("SAML response not signed");
    return nullptr;
  }

  const std::string url = service.decodeState(relayState);
  Wt::WApplication *app = Wt::WApplication::instance();
  if (url != app->makeAbsoluteUrl(process_.privateAcsResourceUrl())) {
    LOG_ERROR("RelayState does not match private ACS resource URL");
    process_.error_ = Wt::utf8("SAML response RelayState mismatch");
    return nullptr;
  }

  return response;
}

std::unique_ptr<saml2::SAML2AssertionPolicy> ProcessImpl::createPolicy(const char16_t * const idpEntityId,
                                                                         const char16_t * const spEntityId,
                                                                         const char16_t * const inResponseTo)
{
  auto &service = process_.service_;
  auto metadataProvider = service.impl_->metadataProvider();
  auto trustEngine = service.impl_->trustEngine();
  xmltooling::Locker locker(metadataProvider);
  auto criteria = std::make_unique<saml2md::MetadataProvider::Criteria>(idpEntityId, &opensaml::saml2md::IDPSSODescriptor::ELEMENT_QNAME);
  auto entityDescriptor = metadataProvider->getEntityDescriptor(*criteria);
  if (!entityDescriptor.first) {
    LOG_ERROR("Could not retrieve metadata for IdP '" << service.idpEntityId() << "'");
    return nullptr;
  }
  auto idpRoleDescriptor = entityDescriptor.second;
  auto policy = std::make_unique<saml2::SAML2AssertionPolicy>(
          metadataProvider,
          &opensaml::saml2md::IDPSSODescriptor::ELEMENT_QNAME,
          trustEngine,
          service.xmlValidationEnabled()
  );
  policy->setMetadataProviderCriteria(criteria.release());
  policy->setIssuerMetadata(idpRoleDescriptor);
  policy->getAudiences().push_back(spEntityId);

  policy->setCorrelationID(inResponseTo);

  for (auto &rule : service.impl_->rules()) {
    policy->getRules().push_back(rule.get());
  }

  return policy;
}

std::pair<const saml2::Assertion *, std::unique_ptr<saml2::Assertion>> ProcessImpl::getAssertion(
        const char16_t * const spEntityId,
        const saml2p::Response &response)
{
  const saml2::Assertion *assertion = nullptr;
  std::unique_ptr<saml2::Assertion> ownedAssertion;

  auto &service = process_.service();
  auto credentialResolver = service.impl_->credentialResolver();

  const auto &encryptedAssertions = response.getEncryptedAssertions();
  const auto &assertions = response.getAssertions();
  if (credentialResolver && !encryptedAssertions.empty()) {
    auto encryptedAssertion = encryptedAssertions.front();
    xmltooling::Locker locker(credentialResolver);
    ownedAssertion = unique_ptr_cast<saml2::Assertion>(
            encryptedAssertion->decrypt(*credentialResolver, spEntityId));
    assertion = ownedAssertion.get();
    if (!assertion) {
      LOG_ERROR("Could not decrypt assertion");
      process_.error_ = Wt::utf8("Failed to decrypt assertion in response");
      return { nullptr, nullptr };
    }
  } else if (!assertions.empty()) {
    assertion = assertions.front();
  }

  if (!assertion) {
    LOG_ERROR("Could not get assertion from response");
    process_.error_ = Wt::utf8("No assertion in response");
    return { nullptr, nullptr };
  }

  if (service.signaturePolicy_ == SignaturePolicy::SignedAssertion ||
      service.signaturePolicy_ == SignaturePolicy::SignedResponseAndAssertion) {
    if (!assertion->getSignature()) {
      LOG_ERROR("SignaturePolicy demands that assertions are signed, but no signature found");
      process_.error_ = Wt::utf8("SAML assertion not signed");
      return { nullptr, nullptr };
    }
  }

  return { assertion, std::move(ownedAssertion) };
}

std::unique_ptr<Subject> ProcessImpl::getSubject(const char16_t *spEntityId, const saml2::Assertion &assertion)
{
  auto subject = assertion.getSubject();
  if (!subject) {
    return nullptr;
  }
  auto credentialResolver = process_.service().impl_->credentialResolver();
  std::unique_ptr<saml2::NameID> ownedNameId;
  auto nameId = subject->getNameID();
  auto encryptedId = subject->getEncryptedID();
  if (credentialResolver && encryptedId) {
    xmltooling::Locker locker(credentialResolver);
    ownedNameId = unique_ptr_cast<saml2::NameID>(
            encryptedId->decrypt(*credentialResolver, spEntityId));
    nameId = ownedNameId.get();
    if (!nameId) {
      LOG_ERROR("Could not decrypt NameID");
      process_.error_ = Wt::utf8("Could not decrypt subject's NameID");
      return nullptr;
    }
  }
  if (!nameId) {
    LOG_ERROR("Could not get NameID from subject");
    process_.error_ = Wt::utf8("No NameID in subject");
    return nullptr;
  }

  xmltooling::auto_ptr_char s(nameId->getName());
  auto result = std::make_unique<Subject>();
  result->id = s.get();
  return result;
}

std::vector<Attribute> ProcessImpl::getAttributes(const char16_t *spEntityId, const saml2::Assertion &assertion)
{
  std::vector<Attribute> result;
  for (auto &statement : assertion.getAttributeStatements()) {
    auto addAttribute = [&result](const saml2::Attribute &attribute) {
      result.emplace_back();
      auto &attributeStruct = result.back();
      if (attribute.getName()) {
        xmltooling::auto_ptr_char attrName(attribute.getName());
        attributeStruct.name = attrName.get();
      }
      if (attribute.getNameFormat()) {
        xmltooling::auto_ptr_char nameFormat(attribute.getNameFormat());
        attributeStruct.nameFormat = nameFormat.get();
      }
      if (attribute.getFriendlyName()) {
        xmltooling::auto_ptr_char friendlyName(attribute.getFriendlyName());
        attributeStruct.friendlyName = friendlyName.get();
      }
      for (auto &value : attribute.getAttributeValues()) {
        xmltooling::auto_ptr_char text(value->getTextContent());
        attributeStruct.values.emplace_back(text.get());
      }
    };
    for (auto &attribute : statement->getAttributes()) {
      addAttribute(*attribute);
    }
    for (auto &encryptedAttribute : statement->getEncryptedAttributes()) {
      auto credentialResolver = process_.service().impl_->credentialResolver();
      xmltooling::Locker locker(credentialResolver);
      auto attribute = unique_ptr_cast<saml2::Attribute>(
              encryptedAttribute->decrypt(*credentialResolver, spEntityId));
      if (attribute) {
        addAttribute(*attribute);
      } else {
        LOG_ERROR("Could not decrypt attribute");
        process_.error_ = "Could not decrypt attribute";
      }
    }
  }
  return result;
}

    }
  }
}
