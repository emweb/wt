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

    /*
bool Object::operator==(const Object& other) const
{
  return Impl::operator==(other);
}

bool Object::operator!=(const Object& other) const
{
  return Impl::operator!=(other);
}
    */

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

    /*
const Array& Object::getArray(const std::string& name) const
{
  return getMember<Array>(name, Array::Empty);
}

const Object& Object::getObject(const std::string& name) const
{
  return getMember<Object>(name, Object::Empty);
}

const WT_USTRING& Object::getString(const std::string& name,
				    const WT_USTRING& fallback) const
{
  return getMember<WT_USTRING>(name, fallback);
}

bool Object::getBool(const std::string& name, bool fallback) const
{
  return getV<bool>(name, fallback);
}

int Object::getInt(const std::string& name, int fallback) const
{
  return getV<int>(name, fallback);
}

long long Object::getLong(const std::string& name, long long fallback) const
{
  return getV<long>(name, fallback);
}

double Object::getDouble(const std::string& name, double fallback) const
{
  return getV<double>(name, fallback);
}

WT_USTRING Object::getAsString(const std::string& name,
			       const WT_USTRING& fallback) const
{
  return getValue(name).asString(fallback);
}

bool Object::getAsBool(const std::string& name, bool fallback) const
{
  return getValue(name).asString(fallback);
}

int Object::getAsInt(const std::string& name, int fallback = 0) const
{
  return getValue(name).asInt(fallback);
}

long Object::getAsLong(const std::string& name, long fallback = 0) const
{
  return getValue(name).asLong(fallback);
}

double Object::getAsDouble(const std::string& name, double fallback = 0) const
{
  return getValue(name).asDouble(fallback);
}
    */

  }
}
