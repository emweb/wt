/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Json/Object"

namespace Wt {
  namespace Json {

Object Object::Empty;

typedef std::map<std::string, Value> Impl;

Object::Object()
{ }

Object::Object(const Object& other)
  : Impl(other)
{ }

void Object::swap(Object& other)
{
  Impl::swap(other);
}

Object& Object::operator= (const Object& other)
{
  Impl::operator=(other);
  return *this;
}

std::set<std::string> Object::names() const
{
  std::set<std::string> result;

  for (const_iterator i = begin(); i != end(); ++i)
    result.insert(i->first);

  return result;
}

bool Object::contains(const std::string& name) const
{
  return find(name) != end();
}

Type Object::type(const std::string& name) const
{
  const_iterator i = find(name);
  if (i == end())
    return NullType;
  else
    return i->second.type();
}

bool Object::isNull(const std::string& name) const
{
  return type(name) == NullType;
}

const Value& Object::get(const std::string& name) const
{
  const_iterator i = find(name);

  if (i == end())
    return Value::Null;
  else
    return i->second;
}

  }
}
