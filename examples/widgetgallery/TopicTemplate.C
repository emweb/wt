/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/algorithm/string.hpp>

#include "TopicTemplate.h"

#include <Wt/Utils.h>
#include <Wt/WApplication.h>
#include <Wt/WStringStream.h>

TopicTemplate::TopicTemplate(const char *trKey)
  : BaseTemplate(trKey)
{
#ifndef WT_TARGET_JAVA
  bindString("doc-url", "//www.webtoolkit.eu/wt/doc/reference/html/");
#else
  bindString("doc-url", "//www.webtoolkit.eu/"
             "jwt/latest/doc/javadoc/eu/webtoolkit/jwt/");
#endif

  namespaceToPackage["Chart"] = "chart";
  namespaceToPackage["Render"] = "render";
}

std::string TopicTemplate::getString(const std::string& varName)
{
  std::stringstream ss;
  std::vector<Wt::WString> args;

  resolveString(varName, args, ss);

  return ss.str();
}

std::string TopicTemplate::docUrl(const std::string& type,
                                  const std::string& className)
{
  Wt::WStringStream ss;

#if !defined(WT_TARGET_JAVA)
  ss << getString("doc-url") << type << escape("Wt::" + className) << ".html";
#else
  if (type == "namespace") {
    ss << getString("doc-url") << namespaceToPackage[className] << "/package-summary.html";
  } else {
    std::string cn = className;
    boost::replace_all(cn, ".", "/");
    ss << getString("doc-url") << cn << ".html";
  }
#endif

  return ss.str();
}

void TopicTemplate::resolveString(const std::string& varName,
                                  const std::vector<Wt::WString>& args,
                                  std::ostream& result)
{
  if (varName == "doc-link") {
    std::string className = args[0].toUTF8();
    std::string type = "class";
    std::string title;
    for (std::size_t i = 1; i < args.size(); ++i) {
      std::string arg = args[i].toUTF8();
      if (boost::starts_with(arg, "type=")) {
        type = arg.substr(5);
      } else if (boost::starts_with(arg, "title=")) {
        title = arg.substr(6);
      }
    }

#ifndef WT_TARGET_JAVA
    boost::replace_all(className, "-", "::");
#else
    for (auto it = namespaceToPackage.begin(); it != namespaceToPackage.end(); ++it) {
      boost::replace_all(className, it->first + "-", it->second + ".");
    }
#endif

    result << "<a href=\"" << docUrl(type, className) << "\" target=\"_blank\">";

    if (title.empty()) {
      title = className;
      for (auto it = namespaceToPackage.begin(); it != namespaceToPackage.end(); ++it) {
#ifndef WT_TARGET_JAVA
        boost::replace_all(title, it->first + "::", "");
#else
        boost::replace_all(title, it->second + ".", "");
#endif // WT_TARGET_JAVA
      }
    }

    result << Wt::Utils::htmlEncode(title) << "</a>";
  } else if (varName == "src") {
    std::string exampleName = args[0].toUTF8();
    result << "<fieldset class=\"src\">"
           << "<legend>source</legend>"
           << tr("src-" + exampleName).toXhtmlUTF8()
           << "</fieldset>";
  } else
    BaseTemplate::resolveString(varName, args, result);
}

std::string TopicTemplate::escape(const std::string &name)
{
  Wt::WStringStream ss;

  for (unsigned i = 0; i < name.size(); ++i) {
    if (name[i] != ':')
      ss << name[i];
    else
      ss << "_1";
  }

  return ss.str();
}
