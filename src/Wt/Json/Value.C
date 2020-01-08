/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Json/Value.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Array.h"
#include "Wt/WAny.h"
#include "Wt/WLogger.h"

#include "WebUtils.h"

namespace Wt {

LOGGER("Json.Value");

  namespace Json {

    namespace {
      const char *typeNames[] = {
	"Null",
	"String",
	"Bool",
	"Number",
	"Object",
	"Array"
      };
    }

TypeException::TypeException(const std::string& name,
			     Type actualType, Type expectedType)
  : WException("Type error: " + name + " is " 
	       + typeNames[static_cast<unsigned int>(actualType)]
	       + ", expected " 
	       + typeNames[static_cast<unsigned int>(expectedType)]),
    name_(name),
    actualType_(actualType),
    expectedType_(expectedType)
{ }

TypeException::TypeException(Type actualType, Type expectedType)
  : WException(std::string("Type error: value is ") 
	       + typeNames[static_cast<unsigned int>(actualType)]
	       + ", expected " 
	       + typeNames[static_cast<unsigned int>(expectedType)]),
    actualType_(actualType),
    expectedType_(expectedType)
{ }

TypeException::~TypeException() throw()
{ }

template <typename T> T Value::get(Type requestedType) const
{
  try {
    return cpp17::any_cast<T>(v_);
  } catch (cpp17::bad_any_cast& e) {
    throw TypeException(type(), requestedType);
  }
}

template <typename T> const T& Value::getCR(Type requestedType) const
{
  try {
    return cpp17::any_cast<const T&>(v_);
  } catch (cpp17::bad_any_cast& e) {
    throw TypeException(type(), requestedType);
  }
}

template <typename T> T& Value::getR(Type requestedType)
{
  try {
    return cpp17::any_cast<T&>(v_);
  } catch (cpp17::bad_any_cast& e) {
    throw TypeException(type(), requestedType);
  }
}

const Value Value::Null;
const Value Value::True(true);
const Value Value::False(false);

Value::Value()
{ }

Value::Value(bool value)
  : v_(value)
{ }

Value::Value(const WT_USTRING& value)
  : v_(value)
{ }

Value::Value(WT_USTRING&& value)
  : v_(std::move(value))
{ }

Value::Value(int value)
  : v_(value)
{ }

Value::Value(long long value)
  : v_(value)
{ }

Value::Value(double value)
  : v_(value)
{ }

Value::Value(const char* value)
  : v_(WString(value))
{ }

Value::Value(const Array& value)
  : v_(value)
{ }

Value::Value(const Object& value)
  : v_(value)
{ }

Value::Value(Type type)
{
  switch (type) {
  case Type::Null: break;
  case Type::Bool: v_ = false; break;
  case Type::Number: v_ = double(0.0); break;
  case Type::String: v_ = std::string(); break;
  case Type::Object: v_ = Object(); break;
  case Type::Array: v_ = Array(); break;
  }
}

Value::Value(const Value& other)
  : v_(other.v_)
{ }

Value::Value(Value&& other)
  : v_(std::move(other.v_))
{ }

Value::Value(Object&& other)
  : v_(std::move(other))
{ }

Value::Value(Array&& other)
  : v_(std::move(other))
{ }

Value& Value::operator= (const Value& other)
{
  v_ = other.v_;
  return *this;
}

Value& Value::operator= (Value &&other)
{
  v_ = std::move(other.v_);
  return *this;
}

Value& Value::operator= (Object&& other)
{
  v_ = std::move(other);
  return *this;
}

Value& Value::operator= (Array&& other)
{
  v_ = std::move(other);
  return *this;
}

bool Value::operator== (const Value& other) const
{
  if (typeid(v_) != typeid(other.v_))
    return false;
  else if (!cpp17::any_has_value(v_) || !cpp17::any_has_value(other.v_))
    return cpp17::any_has_value(v_) == cpp17::any_has_value(other.v_);
  else if (v_.type() == typeid(Json::Object))
    return cpp17::any_cast<Json::Object>(v_) ==
      cpp17::any_cast<Json::Object>(other.v_);
  else if (v_.type() == typeid(Json::Array))
    return cpp17::any_cast<Json::Array>(v_) ==
      cpp17::any_cast<Json::Array>(other.v_);
  else if (v_.type() == typeid(bool))
    return cpp17::any_cast<bool>(v_) == cpp17::any_cast<bool>(other.v_);
  else if (v_.type() == typeid(int))
    return cpp17::any_cast<int>(v_) == cpp17::any_cast<int>(other.v_);
  else if (v_.type() == typeid(long long))
    return cpp17::any_cast<long long>(v_) ==
      cpp17::any_cast<long long>(other.v_);
  else if (v_.type() == typeid(double))
    return cpp17::any_cast<double>(v_) == cpp17::any_cast<double>(other.v_);
  else if (v_.type() == typeid(Wt::WString))
    return cpp17::any_cast<Wt::WString>(v_) == cpp17::any_cast<Wt::WString>(other.v_);
  else {
    WStringStream ss;
    ss << "Value::operator== : unknown value type: " << std::string(v_.type().name());
    throw WException(ss.str());
  }
}

bool Value::operator!= (const Value& other) const
{
  return !(*this == other);
}

Type Value::type() const
{
  if (!cpp17::any_has_value(v_))
    return Type::Null;
  else {
    return typeOf(v_.type());
  }
}

bool Value::hasType(const std::type_info& aType) const
{
  return type() == typeOf(aType);
}

Type Value::typeOf(const std::type_info& t)
{
  if (t == typeid(bool))
    return Type::Bool;
  else if (t == typeid(double) || t == typeid(long long) || t == typeid(int))
    return Type::Number;
  else if (t == typeid(WT_USTRING))
    return Type::String;
  else if (t == typeid(Object))
    return Type::Object;
  else if (t == typeid(Array))
    return Type::Array;
  else
    throw WException(std::string("Value::typeOf(): unsupported type ")
		     + t.name());
}

Value::operator const WT_USTRING&() const
{
  return getCR<WT_USTRING>(Type::String);
}

Value::operator std::string() const
{
  return ((const WT_USTRING&)(*this)).toUTF8();
}

Value::operator bool() const
{
  return get<bool>(Type::Bool);
}

Value::operator int() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(double))
    return static_cast<int>(cpp17::any_cast<double>(v_));
  else if (t == typeid(long long))
    return static_cast<int>(cpp17::any_cast<long long>(v_));
  else if (t == typeid(int))
    return cpp17::any_cast<int>(v_);
  else
    throw TypeException(type(), Type::Number);
}

Value::operator long long() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(double))
    return static_cast<long long>(cpp17::any_cast<double>(v_));
  else if (t == typeid(long long))
    return cpp17::any_cast<long long>(v_);
  else if (t == typeid(int))
    return static_cast<long long>(cpp17::any_cast<int>(v_));
  else
    throw TypeException(type(), Type::Number);
}

Value::operator double() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(double))
    return cpp17::any_cast<double>(v_);
  else if (t == typeid(long long))
    return static_cast<double>(cpp17::any_cast<long long>(v_));
  else if (t == typeid(int))
    return static_cast<double>(cpp17::any_cast<int>(v_));
  else
    throw TypeException(type(), Type::Number);
}

Value::operator const Array&() const
{
  return getCR<Array>(Type::Array);
}

Value::operator const Object&() const
{
  return getCR<Object>(Type::Object);
}

Value::operator Array&()
{
  return getR<Array>(Type::Array);
}

Value::operator Object&()
{
  return getR<Object>(Type::Object);
}

const WT_USTRING& Value::orIfNull(const WT_USTRING& v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

std::string Value::orIfNull(const std::string& v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

std::string Value::orIfNull(const char *v) const
{
  return orIfNull(std::string(v));
}

bool Value::orIfNull(bool v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

int Value::orIfNull(int v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

long long Value::orIfNull(long long v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

double Value::orIfNull(double v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

const Array& Value::orIfNull(const Array& v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

const Object& Value::orIfNull(const Object& v) const
{
  if (cpp17::any_has_value(v_))
    return *this;
  else
    return v;
}

Value Value::toString() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(Object) || t == typeid(Array))
    return Null;
  else if (t == typeid(WT_USTRING))
    return *this;
  else if(type() == Type::Number) {
    WString str = asString(v_);
    std::string sstr = str.toUTF8();
    if (sstr.find("nan") != std::string::npos || 
	sstr.find("inf") != std::string::npos)
      throw WException(std::string("Value::toString(): Not a Number"));
    return Value(str);
  } else 
    return Value(asString(v_));
}

Value Value::toBool() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(Object) || t == typeid(Array))
    return Null;
  else if (t == typeid(bool))
    return *this;
  else if (t == typeid(WT_USTRING)) {
    const WT_USTRING& s = cpp17::any_cast<const WT_USTRING& >(v_);
    if (s == "true")
      return True;
    else if (s == "false")
      return False;
    else
      return Null;
  } else
    return Null;
}

Value Value::toNumber() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(Object) || t == typeid(Array))
    return Null;
  else if (t == typeid(double) || t == typeid(long long) || t == typeid(int))
    return *this;
  else if (t == typeid(WT_USTRING)) {
    const WT_USTRING& s = cpp17::any_cast<const WT_USTRING& >(v_);
    try {
      return Utils::stod(s.toUTF8());
    } catch (std::exception& e) {
      LOG_WARN("toNumber() could not cast '" << s << "'");
      return Null;
    }
  } else
    return Null;
}

  }
}
