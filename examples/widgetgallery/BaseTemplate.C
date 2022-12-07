/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "BaseTemplate.h"

#include <Wt/Utils.h>
#include <Wt/WApplication.h>

#include <boost/algorithm/string.hpp>

BaseTemplate::BaseTemplate(const char *trKey)
  : Wt::WTemplate(tr(trKey))
{
  setInternalPathEncoding(true);
  addFunction("tr", &Functions::tr);

#ifndef WT_TARGET_JAVA
  setCondition("if:cpp", true);
  setCondition("if:java", false);
#else
  setCondition("if:cpp", false);
  setCondition("if:java", true);
#endif
}

void BaseTemplate::resolveString(const std::string& varName,
                                 const std::vector<Wt::WString>& args,
                                 std::ostream& result)
{
  if (varName == "img") {
    std::string src;
    std::string alt;
    std::string style;

    for (const auto& arg : args) {
      const auto argS = arg.toUTF8();
      if (boost::starts_with(argS, "src=")) {
        src = argS.substr(4);
      } else if (boost::starts_with(argS, "alt=")) {
        alt = argS.substr(4);
      } else if (boost::starts_with(argS, "style=")) {
        style = argS.substr(6);
      }
    }

    const Wt::WApplication* app = Wt::WApplication::instance();
    result << "<img src=\"" << Wt::Utils::htmlAttributeValue(app->resolveRelativeUrl(src)) << "\" ";
    if (!alt.empty()) {
      result << "alt=\"" << Wt::Utils::htmlAttributeValue(alt) << "\" ";
    }
    if (!style.empty()) {
      result << "style=\"" << Wt::Utils::htmlAttributeValue(style) << "\" ";
    }
    result << "/>";
  } else {
    WTemplate::resolveString(varName, args, result);
  }
}
