// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JSON_ARRAY_H_
#define WT_JSON_ARRAY_H_

#include <string>
#include <vector>
#include <initializer_list>

#include <Wt/Json/Value.h>

namespace Wt {
  namespace Json {

class Object;
class Value;

/*! \brief A JSON array
 *
 * This class represents a JSON array. It is implemented as a
 * <tt>std::vector&lt;Json::Value&gt;</tt> -- no new API to learn
 * here!
 *
 * \ingroup json
 */
class WT_API Array : public std::vector<Value>
{
public:

  /*! \brief Constructor.
   */
  Array();

  /*! \brief Copy constructor.
   */
  Array(const Array& other);

  /*! \brief Assignment operator.
   */
  Array& operator= (const Array& other);

  /*! \brief Move constructor
   */
  Array(Array&& other);

  /*! \brief Initializer list constructor.
   */
  Array(std::initializer_list<Value> list);

  /*! \brief Assignment operator.
   */
  Array& operator= (Array&& other);

  /*! \brief Empty array constant.
   */
  static Array Empty;
};

  }
}

#endif // WT_JSON_ARRAY_H_
