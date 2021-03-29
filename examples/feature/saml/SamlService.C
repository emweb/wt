/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "SamlService.h"

#include <Wt/Auth/Identity.h>

#include <Wt/Auth/Saml/Assertion.h>

SamlService::SamlService(Wt::Auth::AuthService &baseAuth)
  : Service(baseAuth)
{
  setName("samltestid");
  setDescription("SAML test IdP");

  setSPEntityId("WtTestSP");
  setAcsUrl("http://localhost:8080/sp/acs");
  setMetadataPath("/sp/metadata.xml");

  setIdpEntityId("https://samltest.id/saml/idp");
  setMetadataProviderPath("xml/metadata_provider.xml");

  setXmlToolingCatalogPath("/home/roel/libraries/saml/install-dir/share/xml/xmltooling/catalog.xml");
  setSamlCatalogPath("/home/roel/libraries/saml/install-dir/share/xml/opensaml/saml20-catalog.xml");

  initialize();
}

Wt::Auth::Identity SamlService::assertionToIdentity(const Wt::Auth::Saml::Assertion &assertion) const
{
  auto email = assertion.attributeValue("urn:oasis:names:tc:SAML:attribute:subject-id");
  auto name = assertion.attributeValue("urn:oid:2.16.840.1.113730.3.1.241");
  if (!email || !name) {
    return Wt::Auth::Identity();
  }
  return Wt::Auth::Identity(this->name(), *email, *name, *email, true);
}
