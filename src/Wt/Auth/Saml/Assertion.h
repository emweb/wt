// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_SAML_ASSERTION_H_
#define WT_AUTH_SAML_ASSERTION_H_

#include <Wt/WDllDefs.h>

#include <memory>
#include <string>
#include <vector>

namespace Wt {
  namespace Auth {
    namespace Saml {

/*! \struct Subject Wt/Auth/Saml/Assertion.h
 *  \brief A %SAML %Subject (saml-core-2.0-os, section 2.4.1)
 *
 * This represents a single `<Subject>` element in a %SAML assertion.
 */
struct Subject {
  /*! \brief The id of the subject.
   *
   * This corresponds to the `<NameID>` element in a %SAML assertion,
   * or the decrypted `<EncryptedID>`.
   *
   * This is an empty string if absent.
   */
  std::string id;
};

/*! \struct Attribute Wt/Auth/Saml/Assertion.h
 *  \brief A %SAML %Attribute (saml-core-2.0-os, section 2.7.3.1)
 *
 * This represents a single `<Attribute>` element in a %SAML assertion.
 */
struct Attribute {
  //! The name of the attribute (`Name` XML attribute of the `<Attribute>` element)
  std::string name;

  /*! \brief The name format of the attribute
   *
   * This corresponds to the `NameFormat` XML attribute of the `<Attribute>` element.
   * This is an empty string if absent.
   */
  std::string nameFormat;

  /*! \brief The friendly name of the attribute
   *
   * This corresponds to the `FriendlyName` XML attribute of the `<Attribute>` element.
   * This is an empty string if absent.
   */
  std::string friendlyName;

  /*! \brief The values of the attribute
   *
   * This corresponds to the `<AttributeValue>` elements.
   */
  std::vector<std::string> values;
};

/*! \struct Assertion Wt/Auth/Saml/Assertion.h
 *  \brief Represents a %SAML assertion (saml-core-2.0-os, section 2.3.3)
 */
struct WT_API Assertion {
  /*! \brief The optional subject
   */
  std::unique_ptr<Subject> subject;

  /*! \brief The attributes
   */
  std::vector<Attribute> attributes;

  /*! \brief Gets a single attribute's value
   *
   * This searches for the attribute with the given name, and returns its
   * value. If the attribute has more than one value the first value is returned.
   * If no attribute with the given name was found (or the attribute has no value),
   * nullptr is returned.
   */
  const std::string *attributeValue(const std::string &name) const;

  /*! \brief Gets the values of an attribute
   *
   * This searches for the attribute with the given name, and returns its values.
   * If no attribute with the given name was found (or the attribute has no value),
   * an empty vector is returned.
   */
  const std::vector<std::string> &attributeValues(const std::string &name) const;
};

    }
  }
}

#endif // WT_AUTH_SAML_ASSERTION_H_
