/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Assertion.h"

#include <algorithm>

namespace {

std::vector<std::string> emptyVector;

}

namespace Wt {
  namespace Auth {
    namespace Saml {

const std::string *Assertion::attributeValue(const std::string &name) const {
  const auto &values = attributeValues(name);
  if (values.empty()) {
    return nullptr;
  } else {
    return &values.front();
  }
}

const std::vector<std::string> &Assertion::attributeValues(const std::string &name) const {
  for (const auto &attribute : attributes) {
    if (attribute.name == name) {
      return attribute.values;
    }
  }
  return emptyVector;
}

    }
  }
}
