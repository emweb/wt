/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Json/Value"
#include "Wt/Json/Object"
#include "Wt/Json/Array"
#include "Wt/WBoostAny"
#include "Wt/WLogger"

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
  : WException("Type error: " + name_ + " is " + typeNames[actualType]
	       + ", expected " + typeNames[expectedType]),
    name_(name),
    actualType_(actualType),
    expectedType_(expectedType)
{ }

TypeException::TypeException(Type actualType, Type expectedType)
  : WException(std::string("Type error: value is ") + typeNames[actualType]
	       + ", expected " + typeNames[expectedType]),
    actualType_(actualType),
    expectedType_(expectedType)
{ }

TypeException::~TypeException() throw()
{ }

template <typename T> T Value::get(Type requestedType) const
{
  try {
    return boost::any_cast<T>(v_);
  } catch (boost::bad_any_cast& e) {
    throw TypeException(type(), requestedType);
  }
}

template <typename T> const T& Value::getCR(Type requestedType) const
{
  try {
    return boost::any_cast<const T&>(v_);
  } catch (boost::bad_any_cast& e) {
    throw TypeException(type(), requestedType);
  }
}

template <typename T> T& Value::getR(Type requestedType)
{
  try {
    return boost::any_cast<T&>(v_);
  } catch (boost::bad_any_cast& e) {
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

Value::Value(int value)
  : v_(value)
{ }

Value::Value(long long value)
  : v_(value)
{ }

Value::Value(double value)
  : v_(value)
{ }

Value::Value(Type type)
{ 
  switch (type) {
  case NullType: break;
  case BoolType: v_ = false; break;
  case NumberType: v_ = double(0.0); break;
  case StringType: v_ = std::string(); break;
  case ObjectType: v_ = Object(); break;
  case ArrayType: v_ = Array(); break;
  }
}

Value::Value(const Value& other)
  : v_(other.v_)
{ }

Value& Value::operator= (const Value& other)
{
  v_ = other.v_;
  return *this;
}

bool Value::operator== (const Value& other) const
{
  if (typeid(v_) != typeid(other.v_))
    return false;
  else if (v_.empty() == other.v_.empty())
    return true;
  else if (typeid(v_) == typeid(Json::Object))
    return boost::any_cast<Json::Object>(v_) ==
      boost::any_cast<Json::Object>(other.v_);
  else if (typeid(v_) == typeid(Json::Array))
    return boost::any_cast<Json::Array>(v_) ==
      boost::any_cast<Json::Array>(other.v_);
  else if (typeid(v_) == typeid(bool))
    return boost::any_cast<bool>(v_) == boost::any_cast<bool>(other.v_);
  else if (typeid(v_) == typeid(int))
    return boost::any_cast<int>(v_) == boost::any_cast<int>(other.v_);
  else if (typeid(v_) == typeid(long long))
    return boost::any_cast<long long>(v_) ==
      boost::any_cast<long long>(other.v_);
  else if (typeid(v_) == typeid(double))
    return boost::any_cast<double>(v_) == boost::any_cast<double>(other.v_);
  else
    throw WException("Value::operator== : unknown value type\n");
}

bool Value::operator!= (const Value& other) const
{
  return !(*this == other);
}

Type Value::type() const
{
  if (v_.empty())
    return NullType;
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
    return BoolType;
  else if (t == typeid(double) || t == typeid(long long) || t == typeid(int))
    return NumberType;
  else if (t == typeid(WT_USTRING))
    return StringType;
  else if (t == typeid(Object))
    return ObjectType;
  else if (t == typeid(Array))
    return ArrayType;
  else
    throw WException(std::string("Value::typeOf(): unsupported type ")
		     + t.name());
}

Value::operator const WT_USTRING&() const
{
  return getCR<WT_USTRING>(StringType);
}

Value::operator std::string() const
{
  return ((const WT_USTRING&)(*this)).toUTF8();
}

Value::operator bool() const
{
  return get<bool>(BoolType);
}

Value::operator int() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(double))
    return static_cast<int>(boost::any_cast<double>(v_));
  else if (t == typeid(long long))
    return static_cast<int>(boost::any_cast<long long>(v_));
  else if (t == typeid(int))
    return boost::any_cast<int>(v_);
  else
    throw TypeException(type(), NumberType);
}

Value::operator long long() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(double))
    return static_cast<long long>(boost::any_cast<double>(v_));
  else if (t == typeid(long long))
    return boost::any_cast<long long>(v_);
  else if (t == typeid(int))
    return boost::any_cast<long long>(boost::any_cast<int>(v_));
  else
    throw TypeException(type(), NumberType);
}

Value::operator double() const
{
  const std::type_info& t = v_.type();

  if (t == typeid(double))
    return boost::any_cast<double>(v_);
  else if (t == typeid(long long))
    return static_cast<double>(boost::any_cast<long long>(v_));
  else if (t == typeid(int))
    return boost::any_cast<double>(boost::any_cast<int>(v_));
  else
    throw TypeException(type(), NumberType);
}

Value::operator const Array&() const
{
  return getCR<Array>(ArrayType);
}

Value::operator const Object&() const
{
  return getCR<Object>(ObjectType);
}

Value::operator Array&()
{
  return getR<Array>(ArrayType);
}

Value::operator Object&()
{
  return getR<Object>(ObjectType);
}

const WT_USTRING& Value::orIfNull(const WT_USTRING& v) const
{
  if (!v_.empty())
    return *this;
  else
    return v;
}

std::string Value::orIfNull(const std::string& v) const
{
  if (!v_.empty())
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
  if (!v_.empty())
    return *this;
  else
    return v;
}

int Value::orIfNull(int v) const
{
  if (!v_.empty())
    return *this;
  else
    return v;
}

long long Value::orIfNull(long long v) const
{
  if (!v_.empty())
    return *this;
  else
    return v;
}

double Value::orIfNull(double v) const
{
  if (!v_.empty())
    return *this;
  else
    return v;
}

const Array& Value::orIfNull(const Array& v) const
{
  if (!v_.empty())
    return *this;
  else
    return v;
}

const Object& Value::orIfNull(const Object& v) const
{
  if (!v_.empty())
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
  else
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
    const WT_USTRING& s = boost::any_cast<const WT_USTRING& >(v_);
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
    const WT_USTRING& s = boost::any_cast<const WT_USTRING& >(v_);
    try {
      return boost::lexical_cast<double>(s);
    } catch (boost::bad_lexical_cast& e) {
      LOG_WARN("toNumber() could not cast '" << s << "'");
      return Null;
    }
  } else
    return Null;
}

  }
}
