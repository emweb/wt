/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cstring>

#include "Wt/WException.h"
#include "Wt/WStringStream.h"

#include "FileServe.h"

namespace Wt {

FileServe::FileServe(const char *contents)
  : template_(contents),
    currentPos_(0)
{ }

void FileServe::setCondition(const std::string& name, bool value)
{
  conditions_[name] = value;
}

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

void FileServe::setVar(const std::string& name, int value)
{
  setVar(name, std::to_string(value));
}

void FileServe::setVar(const std::string& name, long value)
{
  setVar(name, std::to_string(value));
}

void FileServe::setVar(const std::string& name, long long value)
{
  setVar(name, std::to_string(value));
}

void FileServe::setVar(const std::string& name, unsigned value)
{
  setVar(name, std::to_string(value));
}

void FileServe::stream(WStringStream& out)
{
  streamUntil(out, std::string());
}

void FileServe::streamUntil(WStringStream& out, const std::string& until)
{
  std::string currentVar;
  bool readingVar = false;

  int start = currentPos_;
  int noMatchConditions = 0;

  for (; template_[currentPos_]; ++currentPos_) {
    const char *s = template_ + currentPos_;

    if (readingVar) {
      if (std::strncmp(s, "_$_", 3) == 0) {
	if (currentVar[0] == '$') {
	  std::size_t _pos = currentVar.find('_');
	  std::string fname = currentVar.substr(1, _pos - 1);

	  currentPos_ += 2; // skip ()

	  if (fname == "endif") {
	    if (noMatchConditions)
	      --noMatchConditions;
	  } else {
	    std::string farg = currentVar.substr(_pos + 1);

	    std::map<std::string, bool>::const_iterator
	      i = conditions_.find(farg);

	    if (i == conditions_.end())
	      throw WException("Internal error: could not find condition: "
			       + farg);
	    bool c = i->second;

	    if (fname == "if")
	      ;
	    else if (fname == "ifnot")
	      c = !c;

	    if (!c || noMatchConditions)
	      ++noMatchConditions;
	  }
	} else {
	  if (currentVar == until) {
	    currentPos_ += 3;
	    return;
	  }

	  std::map<std::string, std::string>::const_iterator i
	    = vars_.find(currentVar);

	  if (i == vars_.end())
	    throw WException("Internal error: could not find variable: "
			     + currentVar);

	  if (!noMatchConditions)
	    out << i->second;
	}

	readingVar = false;
	start = currentPos_ + 3;
	currentPos_ += 2;
      } else
	currentVar.push_back(*s);
    } else {
      if (std::strncmp(s, "_$_", 3) == 0) {
	if (!noMatchConditions && (currentPos_ - start > 0))
	  out.append(template_ + start, currentPos_ - start);

	currentPos_ += 2;
	readingVar = true;
	currentVar.clear();
      }
    }
  }

  if (!noMatchConditions && (currentPos_ - start > 0))
    out.append(template_ + start, currentPos_ - start);
}

}
