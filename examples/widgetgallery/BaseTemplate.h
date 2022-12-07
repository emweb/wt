// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BASE_TEMPLATE_H_
#define BASE_TEMPLATE_H_

#include <Wt/WTemplate.h>

class BaseTemplate : public Wt::WTemplate {
public:
  explicit BaseTemplate(const char* trKey);

  void resolveString(const std::string& varName,
                     const std::vector<Wt::WString>& args,
                     std::ostream& result) override;
};

#endif // BASE_TEMPLATE_H_
