// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SAML_SERVICE_H_
#define SAML_SERVICE_H_

#include <Wt/Auth/Saml/Service.h>

class SamlService final : public Wt::Auth::Saml::Service {
public:
  explicit SamlService(Wt::Auth::AuthService &baseAuth);

  Wt::Auth::Identity assertionToIdentity(const Wt::Auth::Saml::Assertion &assertion) const override;
};

#endif // SAML_SERVICE_H_
