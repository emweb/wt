/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ServiceImpl.h"

#include "Wt/Auth/Saml/Process.h"

#include "Wt/WLogger.h"

#include <xmltooling/XMLToolingConfig.h>
#include <xmltooling/security/Credential.h>
#include <xmltooling/security/CredentialCriteria.h>
#include <xmltooling/security/TrustEngine.h>
#include <xmltooling/util/ParserPool.h>
#include <xmltooling/util/ReplayCache.h>
#include <xmltooling/util/TemplateEngine.h>
#include <xmltooling/util/XMLConstants.h>
#include <xmltooling/util/XMLHelper.h>

#include <saml/SAMLConfig.h>
#include <saml/binding/ArtifactMap.h>
#include <saml/binding/SecurityPolicyRule.h>
#include <saml/saml2/metadata/Metadata.h>
#include <saml/saml2/metadata/MetadataProvider.h>
#include <saml/util/SAMLConstants.h>

#include <xsec/dsig/DSIGConstants.hpp>
#include <xsec/utils/XSECPlatformUtils.hpp>

#ifdef XMLTOOLING_LOG4SHIB
#include <log4shib/AppenderSkeleton.hh>
#include <log4shib/Category.hh>
#endif // XMLTOOLING_LOG4SHIB

#ifdef XMLTOOLING_LOG4CPP
#include <log4cpp/AppenderSkeleton.hh>
#include <log4cpp/Category.hh>
#endif // XMLTOOLING_LOG4CPP

#include <algorithm>
#include <fstream>

#ifdef XMLTOOLING_LOG4SHIB
namespace logging = log4shib;
#endif // XMLTOOLING_LOG4SHIB

#ifdef XMLTOOLING_LOG4CPP
namespace logging = log4cpp;
#endif // XMLTOOLING_LOG4CPP

using namespace std::string_literals;

namespace saml2md = opensaml::saml2md;

namespace {

constexpr const char16_t * TAG_POLICY_RULE = u"PolicyRule";
constexpr const char16_t * ATTR_ERROR_FATAL = u"errorFatal";
constexpr const char16_t * ATTR_CHECK_CORRELATION = u"checkCorrelation";
constexpr const char16_t * ATTR_BLOCK_UNSOLICITED = u"blockUnsolicited";
constexpr const char16_t * USE_SIGNING = u"signing";
constexpr const char16_t * USE_ENCRYPTION = u"encryption";

}

namespace Wt {

LOGGER("Auth.Saml.Service");

}

namespace Wt {
  namespace Auth {
    namespace Saml {

std::vector<std::string> ServiceImpl::loadedCatalogs_;
std::unique_ptr<ServiceImpl::WtAppender> ServiceImpl::logAppender_;
int ServiceImpl::serviceCount_ = 0;
bool ServiceImpl::xmlSecurityConfigured_ = false;

class ServiceImpl::WtAppender : public logging::AppenderSkeleton {
public:
  explicit WtAppender(const std::string &name)
    : AppenderSkeleton(name)
  {
    priorityMap_ = {
      { logging::Priority::PriorityLevel::EMERG, "error" },
      { logging::Priority::PriorityLevel::FATAL, "error" },
      { logging::Priority::PriorityLevel::CRIT, "error" },
      { logging::Priority::PriorityLevel::ERROR, "error" },
      { logging::Priority::PriorityLevel::WARN, "warn" },
      { logging::Priority::PriorityLevel::NOTICE, "info" },
      { logging::Priority::PriorityLevel::INFO, "info" },
      { logging::Priority::PriorityLevel::DEBUG, "debug" },
      { logging::Priority::PriorityLevel::NOTSET, "debug" }
    };
  }

  void close() override { }

  bool requiresLayout() const override {
    return false;
  }

  void setLayout(logging::Layout *layout) override { }

protected:
  void _append(const logging::LoggingEvent &event) override {
    std::string logLevel;
    auto it = std::find_if(begin(priorityMap_), end(priorityMap_), [priority=event.priority](auto &pair) {
      return pair.first > priority;
    });
    if (it == begin(priorityMap_)) {
      logLevel = priorityMap_.front().second;
    } else {
      logLevel = (it - 1)->second;
    }

    Wt::log(logLevel) << Wt::logger << ": " << event.categoryName << " (thread: " << event.threadName << "): " << event.message;
  }

private:
  std::vector<std::pair<logging::Priority::PriorityLevel, std::string>> priorityMap_;
};

ServiceImpl::ServiceImpl(::Wt::Auth::Saml::Service &service)
  : service_(service)
{
  initSaml();
  initSecurityRules();
  initMetadataProvider();
  initCredentialResolver();
  initTrustEngine();
  initReplayCache();
  initTemplateEngine();
  initXmlCatalogs();

  ++serviceCount_;
}

ServiceImpl::~ServiceImpl()
{
  --serviceCount_;

  rules_.clear();
  metadataProvider_ = nullptr;
  credentialResolver_ = nullptr;
  trustEngine_ = nullptr;

  samlConfig().term();

  if (serviceCount_ == 0 && logAppender_) {
    auto &root = logging::Category::getRoot();
    root.removeAppender(logAppender_.get());
    logAppender_ = nullptr;
  }
}

void ServiceImpl::initSaml()
{
  // This can be done multiple times, since inits and terms are counted by OpenSAML
  if (!samlConfig().init()) {
    throw Wt::WException("Failed to initialize OpenSAML");
  }
}

void ServiceImpl::initSecurityRules()
{
  initXmlSigningPolicyRule();
  initConditionsPolicyRule();
  initMessageFlowPolicyRule();
}

void ServiceImpl::initXmlSigningPolicyRule()
{
  xercesc::DOMDocument *signingRuleConfig = xmlToolingConfig().getParser().newDocument();
  xmltooling::XercesJanitor <xercesc::DOMDocument> janitor(signingRuleConfig);
  signingRuleConfig->appendChild(signingRuleConfig->createElementNS(nullptr, TAG_POLICY_RULE));
  signingRuleConfig->getDocumentElement()->setAttributeNS(nullptr, ATTR_ERROR_FATAL, xmlconstants::XML_TRUE);

  rules_.emplace_back(
    samlConfig().SecurityPolicyRuleManager.newPlugin(
      XMLSIGNING_POLICY_RULE, signingRuleConfig->getDocumentElement(), false));
}

void ServiceImpl::initConditionsPolicyRule()
{
  rules_.emplace_back(samlConfig().SecurityPolicyRuleManager.newPlugin(CONDITIONS_POLICY_RULE, nullptr, false));
}

void ServiceImpl::initMessageFlowPolicyRule()
{
  xercesc::DOMDocument *messageFlowRuleConfig = xmlToolingConfig().getParser().newDocument();
  xmltooling::XercesJanitor <xercesc::DOMDocument> janitor(messageFlowRuleConfig);
  messageFlowRuleConfig->appendChild(messageFlowRuleConfig->createElementNS(nullptr, TAG_POLICY_RULE));
  messageFlowRuleConfig->getDocumentElement()->setAttributeNS(nullptr, ATTR_CHECK_CORRELATION, xmlconstants::XML_TRUE);
  messageFlowRuleConfig->getDocumentElement()->setAttributeNS(nullptr, ATTR_BLOCK_UNSOLICITED, xmlconstants::XML_TRUE);

  rules_.emplace_back(
    samlConfig().SecurityPolicyRuleManager.newPlugin(
      MESSAGEFLOW_POLICY_RULE, messageFlowRuleConfig->getDocumentElement(), false));
}

void ServiceImpl::initMetadataProvider()
{
  std::ifstream is(service_.metadataProviderPath_, std::ios::binary);
  if (!is) {
    throw Wt::WException("Could not open metadata provider path for reading: '" + service_.metadataProviderPath_ + "'");
  }
  try {
    xercesc::DOMDocument *doc = xmlToolingConfig().getParser().parse(is);
    xmltooling::XercesJanitor<xercesc::DOMDocument> janitor(doc);
    auto type = doc->getDocumentElement()->getAttribute(u"type");
    if (u""s == type) {
      throw Wt::WException("Missing type for metadata provider at path: '" + service_.metadataProviderPath_ + "'");
    }
    xmltooling::auto_ptr_char typeChr(type);
    metadataProvider_.reset(
      samlConfig().MetadataProviderManager.newPlugin(
        typeChr.get(), doc->getDocumentElement(), false));
    metadataProvider_->init();
  } catch (const xmltooling::XMLToolingException &ex) {
    throw Wt::WException("Failed to initialize metadata provider at path: '" + service_.metadataProviderPath_ + "', " +
                         "detail: " + ex.what());
  }
}

void ServiceImpl::initCredentialResolver()
{
  if (service_.credentialResolverPath_.empty())
    return;

  std::ifstream is(service_.credentialResolverPath_, std::ios::binary);
  if (!is) {
    throw Wt::WException("Could not open credential resolver path for reading: '" + service_.credentialResolverPath_ + "'");
  }
  try {
    xercesc::DOMDocument *doc = xmlToolingConfig().getParser().parse(is);
    xmltooling::XercesJanitor<xercesc::DOMDocument> janitor(doc);
    auto type = doc->getDocumentElement()->getAttribute(u"type");
    if (u""s == type) {
      throw Wt::WException("Missing type for credential resolver at path: '" + service_.credentialResolverPath_ + "'");
    }
    xmltooling::auto_ptr_char typeChr(type);
    credentialResolver_.reset(
      xmlToolingConfig().CredentialResolverManager.newPlugin(
        typeChr.get(), doc->getDocumentElement(), false));
  } catch (const xmltooling::XMLToolingException &ex) {
    throw Wt::WException("Failed to initialize credential resolver at path: '" + service_.credentialResolverPath_ + "', " +
                         "detail: " + ex.what());
  }
}

void ServiceImpl::initTrustEngine()
{
  trustEngine_.reset(xmlToolingConfig().TrustEngineManager.newPlugin(EXPLICIT_KEY_TRUSTENGINE, nullptr, false));
}

void ServiceImpl::initReplayCache()
{
  if (!xmlToolingConfig().getReplayCache()) {
    xmlToolingConfig().setReplayCache(new xmltooling::ReplayCache());
  }
}

void ServiceImpl::initTemplateEngine()
{
  if (!xmlToolingConfig().getTemplateEngine()) {
    xmlToolingConfig().setTemplateEngine(new xmltooling::TemplateEngine());
  }
}

void ServiceImpl::initXmlCatalogs()
{
  if (service_.xmlValidationEnabled()) {
    loadCatalog(service_.xmlToolingCatalogPath_);
    loadCatalog(service_.samlCatalogPath_);
  }
}

void ServiceImpl::loadCatalog(const std::string &path)
{
  auto it = std::find(begin(loadedCatalogs_), end(loadedCatalogs_), path);
  if (it != end(loadedCatalogs_)) {
    return; // already loaded
  }
  auto &validatingParser = xmlToolingConfig().getValidatingParser();
  bool result = validatingParser.loadCatalog(path.c_str());
  if (!result) {
    throw Wt::WException("Failed to load catalog: '" + path + "'");
  }
}

void ServiceImpl::configureLogging()
{
  if (logAppender_) {
    return;
  }

  logAppender_ = std::make_unique<WtAppender>("default");
  auto &root = logging::Category::getRoot();
#ifdef WT_DEBUG_ENABLED
  root.setPriority(logging::Priority::DEBUG);
  logAppender_->setThreshold(logging::Priority::DEBUG);
#else
  root.setPriority(logging::Priority::NOTICE);
  logAppender_->setThreshold(logging::Priority::NOTICE);
#endif
  root.addAppender(*logAppender_);
}

void ServiceImpl::configureXmlSecurity()
{
  if (xmlSecurityConfigured_)
    return;

  // ALlowed hashing algorithms (for digest)
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_SHA256);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_SHA384);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_SHA512);

  // Alllowed signature algorithms
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_RSA_SHA256);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_RSA_SHA384);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_RSA_SHA512);

  // Allowed encryption algorithms
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_AES128_CBC);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_AES192_CBC);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_AES256_CBC);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_AES128_GCM);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_AES192_GCM);
  XSECPlatformUtils::whitelistAlgorithm(u"" URI_ID_AES256_GCM);

  xmlSecurityConfigured_ = true;
}

std::string ServiceImpl::metadata() const {
  xmltooling::auto_ptr_XMLCh postBinding(samlconstants::SAML20_BINDING_HTTP_POST);
  xmltooling::auto_ptr_XMLCh acsUrl(service_.acsUrl_.c_str());

  // <EntityDescriptor>
  std::unique_ptr<saml2md::EntityDescriptor> entityDescriptor(
    saml2md::EntityDescriptorBuilder::buildEntityDescriptor());
  xmltooling::auto_ptr_XMLCh entityId(service_.spEntityId_.c_str());
  entityDescriptor->setEntityID(entityId.get());

  // <SPSSODescriptor>
  entityDescriptor->getSPSSODescriptors().push_back(saml2md::SPSSODescriptorBuilder::buildSPSSODescriptor());

  auto spSSODescriptor = entityDescriptor->getSPSSODescriptors().back();
  spSSODescriptor->setProtocolSupportEnumeration(samlconstants::SAML20P_NS);

  addSigningCredential(*spSSODescriptor);
  addEncryptionCredential(*spSSODescriptor);

  if (service_.signaturePolicy_ == SignaturePolicy::SignedAssertion ||
      service_.signaturePolicy_ == SignaturePolicy::SignedResponseAndAssertion) {
    spSSODescriptor->setWantAssertionsSigned(xmlconstants::XML_TRUE);
  }

  spSSODescriptor->getAssertionConsumerServices().push_back(
    saml2md::AssertionConsumerServiceBuilder::buildAssertionConsumerService());
  auto acs = spSSODescriptor->getAssertionConsumerServices().back();
  acs->setIndex(1);
  acs->setBinding(postBinding.get());
  acs->setLocation(acsUrl.get());

  auto rootElement = entityDescriptor->marshall();
  std::ostringstream result;
  result.imbue(std::locale::classic());
  xmltooling::XMLHelper::serialize(rootElement, result, true);

  return result.str();
}

void ServiceImpl::addSigningCredential(opensaml::saml2md::SPSSODescriptor &spSSODescriptor) const
{
  bool result = addCredential(spSSODescriptor, xmltooling::Credential::SIGNING_CREDENTIAL, USE_SIGNING);
  if (result) {
    spSSODescriptor.setAuthnRequestsSigned(xmlconstants::XML_TRUE);
  }
}

void ServiceImpl::addEncryptionCredential(opensaml::saml2md::SPSSODescriptor &spSSODescriptor) const
{
  addCredential(spSSODescriptor, xmltooling::Credential::ENCRYPTION_CREDENTIAL, USE_ENCRYPTION);
}

bool ServiceImpl::addCredential(opensaml::saml2md::SPSSODescriptor &spSSODescriptor,
                            unsigned int usage,
                            const char16_t *use) const
{
  xmltooling::CredentialCriteria cc;
  cc.setUsage(usage);
  auto credentialResolver = service_.impl_->credentialResolver_.get();
  if (!credentialResolver) {
    return false;
  }
  xmltooling::Locker credentialLocker(credentialResolver);
  const xmltooling::Credential *cred = credentialResolver->resolve(&cc);
  if (!cred) {
    return false;
  }
  std::unique_ptr<saml2md::KeyDescriptor> keyDescriptor(saml2md::KeyDescriptorBuilder::buildKeyDescriptor());
  keyDescriptor->setUse(use);
  keyDescriptor->setKeyInfo(cred->getKeyInfo());
  spSSODescriptor.getKeyDescriptors().push_back(keyDescriptor.release());
  return true;
}

opensaml::SAMLConfig &ServiceImpl::samlConfig()
{
  return opensaml::SAMLConfig::getConfig();
}

xmltooling::XMLToolingConfig &ServiceImpl::xmlToolingConfig()
{
  return xmltooling::XMLToolingConfig::getConfig();
}

saml2md::MetadataProvider *ServiceImpl::metadataProvider() const
{
  return metadataProvider_.get();
}

xmltooling::CredentialResolver *ServiceImpl::credentialResolver() const
{
  return credentialResolver_.get();
}

xmltooling::TrustEngine *ServiceImpl::trustEngine() const
{
  return trustEngine_.get();
}

std::u16string ServiceImpl::ssoUrl() const
{
  std::u16string redirectBinding = Wt::utf8(samlconstants::SAML20_BINDING_HTTP_REDIRECT).toUTF16();
  xmltooling::auto_ptr_XMLCh idpEntityId(service_.idpEntityId().c_str());
  saml2md::MetadataProvider::Criteria criteria(idpEntityId.get(), &saml2md::IDPSSODescriptor::ELEMENT_QNAME);
  auto metadataProvider = this->metadataProvider();
  xmltooling::Locker locker(metadataProvider);
  auto idpEntityDescriptor = metadataProvider->getEntityDescriptor(criteria).first;
  if (!idpEntityDescriptor) {
    return u"";
  }
  std::u16string result;
  const auto &idpSsoDescriptors = idpEntityDescriptor->getIDPSSODescriptors();
  for (auto &ssoDescriptor : idpSsoDescriptors) {
    for (auto &ssoService : ssoDescriptor->getSingleSignOnServices()) {
      if (redirectBinding == ssoService->getBinding()) {
        return ssoService->getLocation();
      }
    }
  }
  return u"";
}

std::pair<std::string, std::u16string> ServiceImpl::generateIdentifier()
{
  auto newIdentifier = samlConfig().generateIdentifier();
  xmltooling::auto_ptr_char identifier(newIdentifier);
  std::u16string u16Identifier(newIdentifier);
  XMLString::release(&newIdentifier);
  return { identifier.get(), u16Identifier };
}

    }
  }
}
