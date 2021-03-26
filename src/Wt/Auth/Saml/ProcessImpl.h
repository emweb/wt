// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_SAML_PROCESSIMPL_H_
#define WT_AUTH_SAML_PROCESSIMPL_H_

#include <Wt/WGlobal.h>

#include <memory>

#ifndef WT_TARGET_JAVA
namespace opensaml {
  namespace saml2 {
    class Assertion;
    class SAML2AssertionPolicy;
  }

  namespace saml2p {
    class Response;
  }
}
#endif // WT_TARGET_JAVA

namespace Wt {
  namespace Auth {
    namespace Saml {

class Process;

class ProcessImpl final
{
public:
  explicit ProcessImpl(Process &process);

  bool createAuthnRequest(Http::Response &response);
  bool handleResponse(const Http::Request &request);

private:
#ifndef WT_TARGET_JAVA
  class Request;
  class Response;

  Process &process_;

  std::unique_ptr<opensaml::saml2p::Response> decodeResponse(const Http::Request &request);
  std::unique_ptr<opensaml::saml2::SAML2AssertionPolicy> createPolicy(const char16_t *idpEntityId,
                                                            const char16_t *spEntityId,
                                                            const char16_t *inResponseTo);
  std::pair<const opensaml::saml2::Assertion *, std::unique_ptr<opensaml::saml2::Assertion>>
  getAssertion(const char16_t *spEntityId, const opensaml::saml2p::Response &response);
  std::unique_ptr<Subject> getSubject(const char16_t *spEntityId,
                                      const opensaml::saml2::Assertion &assertion);
  std::vector<Attribute> getAttributes(const char16_t *spEntityId,
                                       const opensaml::saml2::Assertion &assertion);
#endif // WT_TARGET_JAVA
};

    }
  }
}

#endif // WT_AUTH_SAML_PROCESSIMPL_H_
