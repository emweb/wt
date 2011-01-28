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
		     const std::string& sqlType,
		     int flags)
  : name_(name),
    sqlType_(sqlType),
    type_(type),
    flags_(flags)
{ }

FieldInfo::FieldInfo(const std::string& name, const std::type_info *type,
		     const std::string& sqlType,
		     const std::string& foreignKeyTable,
		     const std::string& foreignKeyName,
		     int flags, int fkConstraints)
  : name_(name),
    sqlType_(sqlType),
    foreignKeyName_(foreignKeyName),
    foreignKeyTable_(foreignKeyTable),
    type_(type),
    flags_(flags),
    fkConstraints_(fkConstraints)
{ }


void FieldInfo::setQualifier(const std::string& qualifier, bool firstDboField)
{
  qualifier_ = qualifier;
  if (firstDboField)
    flags_ |= FirstDboField;
}

std::string FieldInfo::sql() const
{
  std::string result;

  if (!qualifier_.empty())
    result = qualifier_ + '.';

  if (needsQuotes())
    result += '"' + name() + '"';
  else
    result += name();

  return result;
}

  }
}
