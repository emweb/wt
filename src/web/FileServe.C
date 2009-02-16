/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cassert>
#include <cstring>

#include "FileServe.h"
#include "WtException.h"

namespace Wt {

FileServe::FileServe(const char *contents)
  : template_(contents),
    currentPos_(0)
{ }

void FileServe::setVar(const std::string& name, const std::string& value)
{
  vars_[name] = value;
}

void FileServe::setVar(const std::string& name, const char *value)
{
  vars_[name] = std::string(value);
}

void FileServe::setVar(const std::string& name, bool value)
{
  setVar(name, value ? "true" : "false");
}

void FileServe::stream(std::ostream& out)
{
  streamUntil(out, std::string());
}

void FileServe::streamUntil(std::ostream& out, const std::string& until)
{
  std::string currentVar;
  bool readingVar = false;

  int start = currentPos_;

  for (; template_[currentPos_]; ++currentPos_) {
    const char *s = template_ + currentPos_;

    if (readingVar) {
      if (std::strncmp(s, "_$_", 3) == 0) {
	if (currentVar == until) {
	  currentPos_ += 3;
	  return;
	}

	std::map<std::string, std::string>::const_iterator i
	  = vars_.find(currentVar);

	if (i == vars_.end())
	  throw WtException("Internal error: could not find variable: "
			    + currentVar);

	out << i->second;

	readingVar = false;
	start = currentPos_ + 3;
	currentPos_ += 2;
      } else
	currentVar.push_back(*s);
    } else {
      if (std::strncmp(s, "_$_", 3) == 0) {
	if (currentPos_ - start > 0)
	  out.write(template_ + start, currentPos_ - start);

	currentPos_ += 2;
	readingVar = true;
	currentVar.clear();
      }
    }
  }

  if (currentPos_ - start > 0)
    out.write(template_ + start, currentPos_ - start);
}

}
