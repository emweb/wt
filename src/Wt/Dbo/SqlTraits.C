/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/SqlTraits"

namespace Wt {
  namespace Dbo {

FieldInfo::FieldInfo(const std::string& name,
		     const std::type_info *type,
		     int flags)
  : name_(name),
    type_(type),
    flags_(flags)
{ }

void FieldInfo::setQualifier(const std::string& qualifier)
{
  qualifier_ = qualifier;
}

std::string FieldInfo::sql() const
{
  std::string result;

  if (!qualifier_.empty())
    result = '"' + qualifier_ + "\".";

  if (needsQuotes())
    result += '"' + name() + '"';
  else
    result += name();

  return result;
}

  }
}
