// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_SAML_SERVICEIMPL_H_
#define WT_AUTH_SAML_SERVICEIMPL_H_

#include <Wt/Auth/Saml/Service.h>

#include <memory>

namespace opensaml {
  class SAMLConfig;
  class SecurityPolicyRule;

  namespace saml2md {
    class MetadataProvider;
    class SPSSODescriptor;
  }
}

namespace xmltooling {
  class CredentialResolver;
  class TrustEngine;
  class XMLToolingConfig;
  template<class T> struct cleanup;
}

namespace Wt {
  namespace Auth {
    namespace Saml {

class ServiceImpl final {
public:
#ifndef WT_TARGET_JAVA
  using SecurityPolicyRulePtr = std::unique_ptr<const opensaml::SecurityPolicyRule, xmltooling::cleanup<opensaml::SecurityPolicyRule>>;
#endif // WT_TARGET_JAVA

  explicit ServiceImpl(::Wt::Auth::Saml::Service &service);

  ~ServiceImpl();

  ServiceImpl(const ServiceImpl &) = delete;
  ServiceImpl &operator=(const ServiceImpl &) = delete;

  ServiceImpl(ServiceImpl &&) = delete;
  ServiceImpl &operator=(ServiceImpl &&) = delete;

#ifdef WT_TARGET_JAVA
  static void initialize();
#endif // WT_TARGET_JAVA

  std::string metadata() const;

  static void configureLogging();
  static void configureXmlSecurity();

  static opensaml::SAMLConfig &samlConfig();
  static xmltooling::XMLToolingConfig &xmlToolingConfig();

#ifndef WT_TARGET_JAVA
  const std::vector<SecurityPolicyRulePtr> &rules() const { return rules_; }
#endif // WT_TARGET_JAVA
  opensaml::saml2md::MetadataProvider *metadataProvider() const;
  xmltooling::CredentialResolver *credentialResolver() const;
  xmltooling::TrustEngine *trustEngine() const;

  std::u16string ssoUrl() const;
  static std::pair<std::string, std::u16string> generateIdentifier();

#ifndef WT_TARGET_JAVA
private:
  class WtAppender;

  static std::vector<std::string> loadedCatalogs_;
  static std::unique_ptr<WtAppender> logAppender_;
  static int serviceCount_;
  static bool xmlSecurityConfigured_;

  const ::Wt::Auth::Saml::Service &service_;
  std::vector<SecurityPolicyRulePtr> rules_;
  std::unique_ptr<opensaml::saml2md::MetadataProvider> metadataProvider_;
  std::unique_ptr<xmltooling::CredentialResolver> credentialResolver_;
  std::unique_ptr<xmltooling::TrustEngine> trustEngine_;

  // All of these are used in the ctor
  static void initSaml();
  void initSecurityRules();
  void initXmlSigningPolicyRule();
  void initConditionsPolicyRule();
  void initMessageFlowPolicyRule();
  void initMetadataProvider();
  void initCredentialResolver();
  void initTrustEngine();
  static void initReplayCache();
  static void initTemplateEngine();
  void initXmlCatalogs();
  static void loadCatalog(const std::string &path);

  // Used in metadata()
  void addSigningCredential(opensaml::saml2md::SPSSODescriptor &spSSODescriptor) const;
  void addEncryptionCredential(opensaml::saml2md::SPSSODescriptor &spSSODescriptor) const;
  bool addCredential(opensaml::saml2md::SPSSODescriptor &spSSODescriptor, unsigned int usage, const char16_t *use) const;
#endif // WT_TARGET_JAVA
};

    }
  }
}

#endif // WT_AUTH_SAML_SERVICEIMPL_H_
