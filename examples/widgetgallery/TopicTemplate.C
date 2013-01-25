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

#ifndef WT_TARGET_JAVA
  bindString("doc-url", "http://www.webtoolkit.eu/wt/doc/reference/html/");
#else
  bindString("doc-url", "http://www.webtoolkit.eu/"
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
  ss << getString("doc-url") << className << ".html";
#endif

  return ss.str();
}

void TopicTemplate::resolveString(const std::string& varName,
				  const std::vector<Wt::WString>& args,
				  std::ostream& result)
{
  if (varName == "doc-link") {
    std::string className = args[0].toUTF8();

    boost::replace_all(className, "-", "::");

    result << "<a href=\"" << docUrl(className)
	   << "\" target=\"_blank\">"
	   << className << "</a>";
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
