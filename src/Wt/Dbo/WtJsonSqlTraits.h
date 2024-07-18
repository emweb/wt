// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WT_DBO_JSON_IMPL_H_
#define WT_DBO_JSON_IMPL_H_

#include <Wt/Dbo/Json.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WString.h>

namespace Wt {
  namespace Dbo {

template<>
void JsonSerializer::act(FieldRef<WString> field)
{
  writeFieldName(field.name());
  fastJsStringLiteral(field.value().toUTF8());
}

template<>
void JsonSerializer::act(FieldRef<WDate> field)
{
  writeFieldName(field.name());

  std::string s;
  auto format = sql_value_traits<WDate, void>::format;
  if (format) {
    s = field.value().toString(format).toUTF8();
  } else {
    s = field.value().toString().toUTF8();
  }

  fastJsStringLiteral(s);
}

template<>
void JsonSerializer::act(FieldRef<WDateTime> field)
{
  writeFieldName(field.name());

  std::string s;
  auto format = sql_value_traits<WDateTime, void>::format;
  if (format) {
    s = field.value().toString(format).toUTF8();
  } else {
    s = field.value().toString().toUTF8();
  }
  
  fastJsStringLiteral(s);
}

template<>
void JsonSerializer::act(FieldRef<WTime> field)
{
  writeFieldName(field.name());

  std::string s;
  auto format = sql_value_traits<WTime, void>::format;
  if (format) {
    s = field.value().toString(format).toUTF8();
  } else { 
    s = field.value().toString().toUTF8();
  }

  fastJsStringLiteral(s);
}

const char * sql_value_traits<WDate, void>::format = nullptr;
const char * sql_value_traits<WDateTime, void>::format = nullptr;
const char * sql_value_traits<WTime, void>::format = nullptr;

  }
}

#endif // WT_DBO_JSON_IMPL_H_