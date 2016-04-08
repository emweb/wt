/*
 * Copyright (C) 2012 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/algorithm/string.hpp>

#include "TopicTemplate.h"

#include <Wt/WStringStream>

TopicTemplate::TopicTemplate(const char *trKey)
  : Wt::WTemplate(tr(trKey))
{
  setInternalPathEncoding(true);
  addFunction("tr", &Functions::tr);

#ifndef WT_TARGET_JAVA
  setCondition("if:cpp", true);
  setCondition("if:java", false);
  bindString("doc-url", "//www.webtoolkit.eu/wt/doc/reference/html/");
#else
  setCondition("if:cpp", false);
  setCondition("if:java", true);
  bindString("doc-url", "//www.webtoolkit.eu/"
	     "jwt/latest/doc/javadoc/eu/webtoolkit/jwt/");
#endif
}

std::string TopicTemplate::getString(const std::string& varName)
{
  std::stringstream ss;
  std::vector<Wt::WString> args;

  resolveString(varName, args, ss);

  return ss.str();
}

std::string TopicTemplate::docUrl(const std::string& className)
{
  Wt::WStringStream ss;

#if !defined(WT_TARGET_JAVA)
  ss << getString("doc-url") << "class" << escape("Wt::" + className)
     << ".html";
#else
  std::string cn = className;
  boost::replace_all(cn, ".", "/");
  ss << getString("doc-url") << cn << ".html";
#endif

  return ss.str();
}

void TopicTemplate::resolveString(const std::string& varName,
				  const std::vector<Wt::WString>& args,
				  std::ostream& result)
{
  if (varName == "doc-link") {
    std::string className = args[0].toUTF8();

#ifndef WT_TARGET_JAVA
    boost::replace_all(className, "-", "::");
#else
    boost::replace_all(className, "Render-", "render.");    
#endif

    result << "<a href=\"" << docUrl(className)
	   << "\" target=\"_blank\">";

#ifdef WT_TARGET_JAVA
    boost::replace_all(className, "render.", "");
#endif // WT_TARGET_JAVA

    result << className << "</a>";
  } else if (varName == "src") {
    std::string exampleName = args[0].toUTF8();
    result << "<fieldset class=\"src\">"
	   << "<legend>source</legend>"
           << tr("src-" + exampleName).toUTF8()
	   << "</fieldset>";
  } else
    WTemplate::resolveString(varName, args, result);
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
