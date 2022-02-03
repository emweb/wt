/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "Topic.h"

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

Topic::Topic()
{ }

void Topic::populateSubMenu(Wt::WMenu *menu)
{ }

Wt::WString Topic::reindent(const Wt::WString& text)
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
