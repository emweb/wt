/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "TopicWidget.h"

#include <Wt/WString.h>
#include <Wt/WText.h>

#include <sstream>
#include <boost/algorithm/string.hpp>

namespace {

int countSpaces(const std::string& line) {
  for (unsigned int pos=0; pos<line.length(); ++pos) {
    if (line[pos] != ' ')
      return pos;
  }
  return line.length();
}

std::string skipSpaces(const std::string& line, unsigned int count) {
  if (line.length() >= count)
    return line.substr(count);
  else
    return std::string();
}

}

TopicWidget::TopicWidget()
  : WContainerWidget()
{ }

void TopicWidget::populateSubMenu(Wt::WMenu *menu)
{ }

std::string TopicWidget::escape(const std::string &name) const
{
  std::stringstream ss;
  for(unsigned int i = 0; i < name.size(); ++i) {
    if (name[i] != ':') {
      ss << name[i];
    } else {
      ss << "_1";
    }
  }
  return ss.str();
}

std::string TopicWidget::docAnchor(const std::string &classname) const
{
  std::stringstream ss;

#if !defined(WT_TARGET_JAVA)
  ss << "<a href=\"https://www.webtoolkit.eu/wt/doc/reference/html/class"
     << escape("Wt::" + classname)
     << ".html\" target=\"_blank\">doc</a>";
#else
  std::string cn = classname;
  cn = boost::replace_all(cn, "Chart::","chart/");
  ss << "<a href=\"https://www.webtoolkit.eu/"
     << "jwt/latest/doc/javadoc/eu/webtoolkit/jwt/"
     << cn
     << ".html\" target=\"_blank\">doc</a>";
#endif

  return ss.str();
}

Wt::WText *TopicWidget::addText(const Wt::WString& s, Wt::WContainerWidget *parent)
{
  auto text = parent->addNew<Wt::WText>(s);
  bool literal;
#ifndef WT_TARGET_JAVA
  literal = s.literal();
#else
  literal = Wt::WString(s).literal();
#endif
  if (!literal)
    text->setInternalPathEncoding(true);
  return text;
}

Wt::WString TopicWidget::reindent(const Wt::WString& text)
{
  std::vector<std::string> lines;
  std::string s = text.toUTF8();
  boost::split(lines, s, boost::is_any_of("\n"));

  std::string result;
  int indent = -1;
  int newlines = 0;
  for (unsigned i = 0; i < lines.size(); ++i) {
    const std::string& line = lines[i];

    if (line.empty()) {
      ++newlines;
    } else {
      if (indent == -1) {
        indent = countSpaces(line);
      } else {
        for (int j = 0; j < newlines; ++j)
          result += '\n';
      }

      newlines = 0;

      if (!result.empty())
        result += '\n';

      result += skipSpaces(line, indent);
    }
  }
  return Wt::WString(result);
}
