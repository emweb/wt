// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JSON_SERIALIZER_H
#define WT_JSON_SERIALIZER_H

#include <Wt/WDllDefs.h>
#include <string>

namespace Wt {
class EscapeOStream;
  namespace Json {

class Object;
class Array;

/*! \brief Serialization function for an Object.
 *
 * Serializes a Object into a string. All unicode in the object is
 * UTF-8 encoded in the output. The output is indented to make it
 * readable. The indentation argument to this function is used in
 * recursive calls and should not be set.
 * 
 * \throws WException when the trying to serialize a number which is a Nan
 * 
 * \ingroup json
 */
std::string WT_API serialize(const Object& obj, int indentation = 1);
void serialize(const Object& obj, int indentation, EscapeOStream& result);

/*! \brief Serialization function for an Array.
 *
 * Serializes a Array into a string. All unicode in the object is
 * UTF-8 encoded in the output. The output is indented to make it
 * readable. The indentation argument to this function is used in
 * recursive calls and should not be set.
 *
 * \throws WException when the trying to serialize a number which is a Nan
 *
 * \ingroup json
 */
std::string WT_API serialize(const Array& arr, int indentation = 1);
void serialize(const Array& arr, int indentation, EscapeOStream& result);

  }
}

#endif
