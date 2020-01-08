// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JSON_VALUE_H_
#define WT_JSON_VALUE_H_

#include <Wt/WAny.h>
#include <Wt/WException.h>
#include <Wt/WString.h>

namespace Wt {
  /*! \brief Namespace for the \ref json
   */
  namespace Json {

class Object;
class Array;

/*! \defgroup json JSON Library (Wt::Json)
 *  \brief A JSON representation and parsing library.
 *
 * The JSON library contains data types to represent a JSON data
 * structure (Value, Object and Array), a JSON parser, and a JSON serializer.
 *
 * Usage example:
 * \code
 * Json::Object result;
 * Json::parse("{ "
 *             "  \"a\": \"That's great\", "
 *             "  \"b\": true "
 *             "}",
 *             result);
 *
 * std::cerr << "Size: " << result.size(); << std::endl; // Size: 2
 * WString s = result.get("a");
 * bool b = result.get("b");
 * std::cerr << "a: " << s << ", b: " << b << std::endl; // a: That's great, b: true
 * \endcode
 *
 * Include the <Wt/Json/Parser.h> header for Wt::Json::parse(), and
 * the <Wt/Json/Serializer.h> header for Wt::Json::serialize().
 */

/*! \brief Enumeration for the type of a JSON value.
 *
 * \sa Value::type()
 *
 * \ingroup json
 */
enum class Type {
  Null,   //!< "null" or missing value
  String, //!< a (unicode) string
  Bool,   //!< "true" or "false"
  Number, //!< a number (integer or floating point)
  Object, //!< an Object
  Array,  //!< an Array
};

/*! \class TypeException Wt/Json/Value.h Wt/Json/Value.h
 *  \brief Exception that indicates a type error.
 *
 * This exception is thrown when a Value is being casted to an
 * incompatible C++ type.
 *
 * \note To avoid exceptions, coerce the type first, and handle Null values.
 *
 * \ingroup json
 */
class TypeException : public WException
{
public:
  TypeException(const std::string& name, Type actualType, Type expectedType);
  TypeException(Type actualType, Type expectedType);
  virtual ~TypeException() throw();

  /*! \brief Returns the object field name (if known) */
  const std::string& name() const { return name_; }

  /*! \brief Returns the value type */
  Type actualType() const { return actualType_; }

  /*! \brief Returns the expected value type */
  Type expectedType() const { return expectedType_; }

private:
  std::string name_;
  Type actualType_, expectedType_;
};

/*! \brief A JSON value
 *
 * This class represents a JSON value, which may be:
 * - a simple type (boolean, number, string)
 * - a composed type (Array or Object)
 * - #Null: this represents values which are represented as "null" in
 *   JSON notations, but also the values returned for missing members in
 *   an Object, or values which are the result of a failed type coercion.
 *
 * \ingroup json
 */
class WT_API Value
{
public:
  /*! \brief Default construtor.
   *
   * This creates a #Null value.
   */
  Value();

  /*! \brief Creates a value from a string.
   *
   * This creates a \link Wt::Json::Type::String
   * Json::Type::String\endlink value.
   */
  Value(const WT_USTRING& value);

  /*! \brief Creates a value from a string.
   *
   * This creates a \link Wt::Json::Type::String
   * Json::Type::String\endlink value.
   */
  Value(WT_USTRING&& value);

  /*! \brief Creates a value from a boolean.
   *
   * This creates a \link Wt::Json::Type::Bool
   * Json::Type::Bool\endlink value.
   */
  Value(bool value);

  /*! \brief Creates a value from an integer.
   *
   * This creates a \link Wt::Json::Type::Number
   * Json::Type::Number\endlink value.
   */
  Value(int value);

  /*! \brief Creates a value from a long.
   *
   * This creates a \link Wt::Json::Type::Number
   * Json::Type::Number\endlink value.
   */
  Value(long long value);

  /*! \brief Creates a value from a double.
   *
   * This creates a \link Wt::Json::Type::Number
   * Json::Type::Number\endlink value.
   */
  Value(double value);

#ifndef WT_TARGET_JAVA

  /*! \brief Creates a value from a const char*.
   *
   * This creates a \link Wt::Json::StringType
   * Json::StringType\endlink value.
   *
   * This constructor first converts const char*
   * to WString using \link Wt::WString(const char*,CharEncoding) WString(const char *)\endlink
   */
  Value(const char* value);

#endif

  /*! \brief Creates a value from a Json::Object.
   *
   * This creates a \link Wt::Json::ArrayType
   * Json::ArrayType\endlink value.
   */
  Value(const Array& value);

  /*! \brief Creates a value from a Json::Object.
   *
   * This creates a \link Wt::Json::ArrayType
   * Json::ArrayType\endlink value.
   */
  Value(Array &&value);

  /*! \brief Creates a value from a Json::Object.
   *
   * This creates a \link Wt::Json::ObjectType
   * Json::ObjectType\endlink value.
   */
  Value(const Object& value);

  /*! \brief Creates a value from a Json::Object.
   *
   * This creates a \link Wt::Json::ObjectType
   * Json::ObjectType\endlink value.
   */
  Value(Object &&value);

  /*! \brief Creates a value with a given type.
   *
   * This creates a value of the given type, using a default constructed value
   * of that type:
   * - \link Wt::Json::Type::Bool Json::Type::Bool\endlink: false
   * - \link Wt::Json::Type::Number Json::Type::Number\endlink: 0
   * - \link Wt::Json::Type::String Json::Type::String\endlink: ""
   * - \link Wt::Json::Type::Array Json::Type::Array\endlink: an empty array
   * - \link Wt::Json::Type::Object Json::Type::Object\endlink: an empty object
   */
  Value(Type type);

  /*! \brief Copy constructor.
   */
  Value(const Value& other);

  /*! \brief Move constructor.
   */
  Value(Value&& other);

  /*! \brief Assignment operator.
   *
   * As a result of an assignment, both value and type are set to the value and
   * type of the \p other value.
   */
  Value& operator= (const Value& other);

  /*! \brief Move Assignment operator.
   *
   * As a result of an assignment, both value and type are set to the value and
   * type of the \p other value.
   */
  Value& operator= (Value&& other);

  /*! \brief Move assignment operator.
   */
  Value& operator= (Object&& other);

  /*! \brief Move assignment operator.
   */
  Value& operator= (Array&& other);

  /*! \brief Comparison operator.
   *
   * Returns whether two values have the same type and value.
   */
  bool operator== (const Value& other) const;

  /*! \brief Comparison operator.
   *
   * Returns whether two values have a different type or value.
   */
  bool operator!= (const Value& other) const;

  /*! \brief Returns the type.
   *
   * Returns the type of this value.
   */
  Type type() const;

  /*! \brief Returns whether the value is #Null.
   *
   * This returns \c true when the type is \link Wt::Json::Type::Null
   * Json::Type::Null\endlink.
   */
  bool isNull() const { return type() == Type::Null; }

  /*! \brief Returns whether the value is compatible with a given C++ type.
   *
   * This returns whether the value type can be contained in the given
   * C++ type, i.e. when a casting operation will not fail throwing a
   * TypeException.
   *
   * \sa typeOf()
   */
  bool hasType(const std::type_info& type) const;

  /*! \brief Returns the JSON type that corresponds to a C++ type.
   *
   * This is a utility method for converting between C++ types and
   * JSON types.
   */
  static Type typeOf(const std::type_info& type);

  /*! \brief Extracts the string value.
   *
   * This returns the value of a \link Wt::Json::Type::String
   * string\endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   const WString& occupation = person.get("occupation");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To coerce a value of another type to a string use toString()
   * first. To provide a fallback in case the value is null or could
   * not be coerced to a string, use orIfNull().
   *
   * For example, the following code does not throw exceptions:
   * \code
   * const Json::Object& person = ...;
   * const WString& occupation = person.get("occupation").toString().orIfNull(WString("manager"));
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::String
   * Json::Type::String\endlink
   */
  operator const WT_USTRING&() const;

  /*! \brief Extracts the string value (UTF-8 encoded).
   *
   * This returns the value of a \link Wt::Json::Type::String
   * string\endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   std::string occupation = person.get("occupation");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To coerce a value of another type to a string use toString()
   * first. To provide a fallback in case the value is null or could
   * not be coerced to a string, use orIfNull().
   *
   * For example, the following code does not throw exceptions:
   * \code
   * const Json::Object& person = ...;
   * const std::string occupation = person.get("occupation").toString().orIfNull("manager");
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::String
   * Json::Type::String\endlink
   */
  operator std::string() const;

  /*! \brief Extracts the boolean value.
   *
   * This returns the value of a \link Wt::Json::Type::Bool
   * boolean \endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   bool happy = person.get("happy");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To coerce a value of another type to a boolean use toBool()
   * first. To provide a fallback in case the value is null or could
   * not be coerced to a boolean, use orIfNull().
   *
   * For example, the following code does not throw exceptions:
   * \code
   * const Json::Object& person = ...;
   * bool happy = person.get("happy").toBool().orIfNull(false);
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Bool
   * Json::Type::Bool\endlink
   */
  operator bool() const;

  /*! \brief Extracts the integer number value.
   *
   * This returns the value of a \link Wt::Json::Type::Number
   * number \endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   int cost = person.get("cost");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To coerce a value of another type to a number use toNumber()
   * first. To provide a fallback in case the value is null or could
   * not be coerced to a number, use orIfNull().
   *
   * For example, the following code does not throw exceptions:
   * \code
   * const Json::Object& person = ...;
   * int cost = person.get("cost").toNumber().orIfNull(0);
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Number
   * Json::Type::Number\endlink
   */
  operator int() const;

  /*! \brief Extracts the integer number value.
   *
   * This returns the value of a \link Wt::Json::Type::Number
   * number \endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   long long cost = person.get("cost");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To coerce a value of another type to a number use toNumber()
   * first. To provide a fallback in case the value is null or could
   * not be coerced to a number, use orIfNull().
   *
   * For example, the following code does not throw exceptions:
   * \code
   * const Json::Object& person = ...;
   * long long cost = person.get("cost").toNumber().orIfNull(0LL);
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Number
   * Json::Type::Number\endlink
   */
  operator long long() const;

  /*! \brief Extracts the floating point number value.
   *
   * This returns the value of a \link Wt::Json::Type::Number
   * number \endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   double cost = person.get("cost");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To coerce a value of another type to a number use toNumber()
   * first. To provide a fallback in case the value is null or could
   * not be coerced to a number, use orIfNull().
   *
   * For example, the following code does not throw exceptions:
   * \code
   * const Json::Object& person = ...;
   * double cost = person.get("cost").toNumber().orIfNull(0.0);
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Number
   * Json::Type::Number\endlink
   */
  operator double() const;

  /*! \brief Extracts the array value.
   *
   * This returns the value of a \link Wt::Json::Type::Array
   * array \endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   const Array& children = person.get("children");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To provide a fallback in case the value is null, use orIfNull().
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Array
   * Json::Type::Array\endlink
   */
  operator const Array&() const;

  /*! \brief Extracts the object value.
   *
   * This returns the value of a \link Wt::Json::Type::Object
   * object \endlink JSON value.
   *
   * For example:
   * \code
   * const Json::Object& person = ...;
   * try {
   *   const Object& employer = person.get("employer");
   *   ...
   * } catch (const std::exception& e) {
   *   ...
   * }
   * \endcode
   *
   * To provide a fallback in case the value is null, use orIfNull().
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Object
   * Json::Type::Object\endlink
   */
  operator const Object&() const;

  /*! \brief Accesses the array value.
   *
   * This returns the value of a \link Wt::Json::Type::Array
   * array \endlink JSON value.
   *
   * Use this method to modify the contained array in-place.
   *
   * For example:
   * \code
   * Json::Object person;
   * person["children"] = Json::Value(Json::Type::Array);
   * Json::Array& children = person.get("children");
   * // add children ...
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Array
   * Json::Type::Array\endlink
   */
  operator Array&();

  /*! \brief Accesses the object value.
   *
   * This returns the value of a \link Wt::Json::Type::Object
   * object \endlink JSON value.
   *
   * Use this method to modify the contained object in-place.
   *
   * For example:
   * \code
   * Json::Array& children = ...;
   * for (unsigned i = 0; i < 3; ++i) {
   *   children.push_back(Json::Value(Json::Type::Object));
   *   Json::Object& child = children.back();
   *   ...
   * }
   * \endcode
   *
   * \throws TypeException if the value type is not \link Wt::Json::Type::Object
   * Json::Type::Object\endlink
   */
  operator Object&();

  /*! \brief Extracts the string value, using a fallback when null.
   *
   * This is similar to the string cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::String
   *                       Json::Type::String\endlink
   */
  const WT_USTRING& orIfNull(const WT_USTRING& v) const;

  /*! \brief Extracts the UTF-8 encoded string value, using a fallback when null.
   *
   * This is similar to the string cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::String
   *                       Json::Type::String\endlink
   */
  std::string orIfNull(const char *v) const;

#ifndef WT_TARGET_JAVA
  /*! \brief Extracts the UTF-8 encoded string value, using a fallback when null.
   *
   * This is similar to the string cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::String
   *                       Json::Type::String\endlink
   */
  std::string orIfNull(const std::string& v) const;
#endif

  /*! \brief Extracts the boolean value, using a fallback when null.
   *
   * This is similar to the boolean cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::Bool
   *                       Json::Type::Bool\endlink
   */
  bool orIfNull(bool v) const;

  /*! \brief Extracts the number value, using a fallback when null.
   *
   * This is similar to the int cast operator, but this method returns
   * a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::Number
   *                       Json::Type::Number\endlink
   */
  int orIfNull(int v) const;

  /*! \brief Extracts the number value, using a fallback when null.
   *
   * This is similar to the long long cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::Number
   *                       Json::Type::Number\endlink
   */
  long long orIfNull(long long v) const;

  /*! \brief Extracts the number value, using a fallback when null.
   *
   * This is similar to the double cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::Number
   *                       Json::Type::Number\endlink
   */
  double orIfNull(double v) const;

  /*! \brief Extracts the array value, using a fallback when null.
   *
   * This is similar to the Array cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::Array
   *                       Json::Type::Array\endlink
   */
  const Array& orIfNull(const Array& v) const;

  /*! \brief Extracts the object value, using a fallback when null.
   *
   * This is similar to the Object cast operator, but this method
   * returns a fallback when the value is null instead of throwing an
   * exception.
   *
   * \throws TypeException if the value is not null and has a type
   *                       other than \link Wt::Json::Type::Object
   *                       Json::Type::Object\endlink
   */
  const Object& orIfNull(const Object& v) const;

  /*! \brief Converts the value to a string.
   *
   * The value is lexically casted to a string. For an object or array
   * value, this coercion is not defined and #Null is returned.
   *
   * \throws WException if the Value is a number initialized to NaN
   *
   */
  Value toString() const;

  /*! \brief Converts the value to a boolean.
   *
   * A string value of "true" or "false" is interpreted as a boolean.
   * Otherwise, #Null is returned.
   */
  Value toBool() const;

  /*! \brief Converts the value to a number.
   *
   * A string value is lexically casted to a number. If this fails, or
   * for a boolean, array or object type, #Null is returned.
   */
  Value toNumber() const;

  /*! \brief Null constant.
   *
   * A constant value with type \link Wt::Json::Type::Null
   * Json::Type::Null\endlink, i.e. as constructed by
   * <tt>Json::Value()</tt>.
   */
  static const Value Null;

  /*! \brief True constant.
   *
   * A constant value of type \link Wt::Json::Type::Bool
   * Json::Type::Bool\endlink with value \c true, i.e. as
   * constructed by <tt>Json::Value(true)</tt>.
   */
  static const Value True;

  /*! \brief False constant.
   *
   * A constant value of type \link Wt::Json::Type::Bool
   * Json::Type::Bool\endlink with value \c false, i.e. as
   * constructed by <tt>Json::Value(false)</tt>
   */
  static const Value False;

#ifdef WT_TARGET_JAVA
  std::string getAsString();
  long getAsLong();
  int getAsInt();
#endif

private:
  cpp17::any v_;

  template <typename T> T get(Type type) const;
  template <typename T> const T& getCR(Type type) const;
  template <typename T> T& getR(Type type);
};

  }
}

#endif // WT_JSON_OBJECT_H_
