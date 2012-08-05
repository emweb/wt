// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FILE_SERVE_H_
#define FILE_SERVE_H_

#include <string>
#include <iostream>
#include <map>

namespace Wt {

/*
 * A simple template streaming class.
 *
 * It supports:
 * - variables (_$_name_$_)
 * - conditions:
 *  _$_$if_condition_$_
 *     ...
 *  _$_$endif_$_
 * and
 *  _$_$ifnot_condition_$_;
 *     ...
 *  _$_$endif_$_;
 */
class FileServe
{
public:
  FileServe(const char *contents);

  void setVar(const std::string& name, const std::string& value);
  void setVar(const std::string& name, const char *value);
  void setVar(const std::string& name, bool value);
  void setVar(const std::string& name, int value);
  void setVar(const std::string& name, unsigned value);
  void setCondition(const std::string& name, bool value);
  void stream(std::ostream& out);
  void streamUntil(std::ostream& out, const std::string& until);

private:
  const char *template_;
  int currentPos_;
  std::map<std::string, std::string> vars_;
  std::map<std::string, bool> conditions_;
};

}

#endif // FILE_SERVE_H_
