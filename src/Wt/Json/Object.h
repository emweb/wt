// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JSON_OBJECT_H_
#define WT_JSON_OBJECT_H_

#include <initializer_list>
#include <map>
#include <set>
#include <Wt/WException.h>
#include <Wt/Json/Value.h>

namespace Wt {
  namespace Json {

class Array;

/*! \brief A JSON object
 *
 * This class represents a JSON object, which defines a value map. It
 * is implemented as a <tt>std::map&lt;std::string,Json::Value&gt;</tt>.
 * It is therefore possible to use the std::map API to deal with it.
 *
 * However, a number of additional methods have been added to more
 * conveniently query the existence and the type of contained
 * objects. A #get() method can be used to return a member value,
 * which returns Value::Null if the member is not defined.
 *
 * \ingroup json
 */
class WT_API Object : public std::map<std::string, Value>
{
public:
  /*! \brief Default constructor.
   */
  Object();

  /*! \brief Copy constructor.
   */
  Object(const Object& other);

  /*! \brief Swap operation.
   *
   * This swaps the contents of two objects.
   */
  void swap(Object& other);

  /*! \brief Assignment operator.
   */
  Object& operator= (const Object& other);
  //bool operator== (const Object& other) const;
  //bool operator!= (const Object& other) const;
  /*! \brief Move constructor.
   */
  Object(Object&& other);

  /*! \brief Initializer list constructor.
   */
  Object(std::initializer_list<std::pair<const std::string,Value> > list);

  /*! \brief Move assignment operator.
   */
  Object& operator= (Object&& other);

  /*! \brief Returns the key set.
   *
   * This returns the member names: these are the members for which
   * values have been associated in this object.
   *
   * \note This may include members with explicit Value::Null values.
   */
  std::set<std::string> names() const;

  /*! \brief Returns whether a member exists.
   *
   * This returns whether the \p name is in the names() set.
   */
  bool contains(const std::string& name) const;

  /*! \brief Returns the type of a given member.
   *
   * This method returns the type of the value for members that are included,
   * or returns \link Wt::Json::Type::Null Json::Type::Null\endlink for when no
   * value is associated with \p name.
   */
  Type type(const std::string& name) const;

  /*! \brief Returns whether the value for a member is null (or not defined).
   *
   * \sa get(), Value::isNull()
   */
  bool isNull(const std::string& name) const;

  /*! \brief Returns the value for a member (or null if not defined).
   *
   * Returns the value associated with member \p name or null if no value
   * has been associated with this member name.
   */
  const Value& get(const std::string& name) const;

  /*! \brief Empty object constant.
   */
  static Object Empty;

#ifdef WT_TARGET_JAVA
  bool isNull() const;
  Value& operator[](const std::string& name);
#endif
};

  }
}

#endif // WT_JSON_OBJECT_H_
