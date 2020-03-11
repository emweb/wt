// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_WT_TYPE_TRAITS_H_
#define WT_DBO_WT_TYPE_TRAITS_H_

#include <string>
#include <iostream>

#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WLogger.h>
#include <Wt/WTime.h>
#include <Wt/WString.h>
#include <Wt/cpp17/any.hpp>

#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Parser.h>
#include <Wt/Json/Serializer.h>

#include <Wt/Dbo/SqlTraits.h>
#include <Wt/Dbo/SqlStatement.h>

namespace Wt {
  namespace Dbo {
    namespace Impl {
      extern void WTDBO_API msecsToHMS(std::chrono::duration<int, std::milli> msecs, int &h, int &m, int &s, int &ms);
    }

template<>
struct sql_value_traits<WDate, void>
{
  static const bool specialized = true;

  static const char *format;

  static const char *type(SqlConnection *conn, int size);
  static void bind(const WDate& v, SqlStatement *statement, int column, int size);
  static bool read(WDate& v, SqlStatement *statement, int column, int size);
};

template<>
struct sql_value_traits<WDateTime, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(const WDateTime& v, SqlStatement *statement, int column,
		   int size);
  static bool read(WDateTime& v, SqlStatement *statement, int column, int size);
};

template<>
struct sql_value_traits<WTime, void>
{
  static const bool specialized = true;

  static const char *format;

  static const char *type(SqlConnection *conn, int size);
  static void bind(const WTime& v, SqlStatement *statement, int column,
		   int size);
  static bool read(WTime& v, SqlStatement *statement, int column, int size);
};

template<>
struct sql_value_traits<WString, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size);
  static void bind(const WString& v, SqlStatement *statement, int column,
		   int size);
  static bool read(WString& v, SqlStatement *statement, int column, int size);
};

template<>
struct sql_value_traits<Json::Object, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size);
  static void bind(const Json::Object& v, SqlStatement *statement, int column,
		   int size);
  static bool read(Json::Object& v, SqlStatement *statement, int column, int size);
};

template<>
struct sql_value_traits<Json::Array, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size);
  static void bind(const Json::Array& v, SqlStatement *statement, int column,
		   int size);
  static bool read(Json::Array& v, SqlStatement *statement, int column, int size);
};

    /*
     * WDate
     */

inline const char *sql_value_traits<WDate, void>::type(SqlConnection *conn,
						       int /* size */)
{
  return conn->dateTimeType(SqlDateTimeType::Date);
}

inline void sql_value_traits<WDate, void>
::bind(const WDate& v, SqlStatement *statement, int column, int /* size */)
{
  if (v.isNull())
    statement->bindNull(column);
  else
    statement->bind(column, v.toTimePoint(), SqlDateTimeType::Date);
}

inline bool sql_value_traits<WDate, void>
::read(WDate& v, SqlStatement *statement, int column, int /* size */)
{
  std::chrono::system_clock::time_point tp;

  if (statement->getResult(column, &tp, SqlDateTimeType::Date)) {
    v = WDate(tp);
    return true;
  } else {
    v = WDate();
    return false;
  }
}

    /*
     * WTime
     */

inline const char *sql_value_traits<WTime, void>::type(SqlConnection *conn,
						       int /* size */)
{
  return conn->dateTimeType(SqlDateTimeType::Time);
}

inline void sql_value_traits<WTime, void>
::bind(const WTime& v, SqlStatement *statement, int column, int /* size */)
{
  if (v.isNull())
    statement->bindNull(column);
  else
    statement->bind(column, v.toTimeDuration());
}

inline bool sql_value_traits<WTime, void>
::read(WTime& v, SqlStatement *statement, int column, int /* size */)
{
  std::chrono::duration<int, std::milli> t;
 
  if (statement->getResult(column, &t)) {
    int h = -1, m = -1, s = -1, ms = -1;
    Impl::msecsToHMS(t, h, m, s, ms);
    if (!v.setHMS(h, m, s, ms)) {
      Wt::log("warning") << "Dbo/WtSqlTraits" << ": WTime can only hold durations < 24h";
      return true;
    } else
      return false;

  } else {
    v = WTime();
    return false;
  }
}

    /*
     * WDateTime
     */

inline const char *sql_value_traits<WDateTime, void>::type(SqlConnection *conn,
							   int /* size */)
{
  return conn->dateTimeType(SqlDateTimeType::DateTime);
}

inline void sql_value_traits<WDateTime, void>
::bind(const WDateTime& v, SqlStatement *statement, int column, int /* size */)
{
  if (v.isNull())
    statement->bindNull(column);
  else
    statement->bind(column, v.toTimePoint(), SqlDateTimeType::DateTime);
}

inline bool sql_value_traits<WDateTime, void>
::read(WDateTime& v, SqlStatement *statement, int column, int /* size */)
{
  std::chrono::system_clock::time_point t;

  if (statement->getResult(column, &t, SqlDateTimeType::DateTime)) {
    v = WDateTime::fromTimePoint(t);
    return true;
  } else {
    v = WDateTime();
    return false;
  }
}

    /*
     * WString
     */

inline std::string sql_value_traits<WString, void>::type(SqlConnection *conn,
							 int size)
{
    return conn->textType(size) + " not null";
}

inline void sql_value_traits<WString, void>
::bind(const WString& v, SqlStatement *statement, int column, int /* size */)
{
  statement->bind(column, v.toUTF8());
}

inline bool sql_value_traits<WString, void>
::read(WString& v, SqlStatement *statement, int column, int size)
{
  std::string d;
  if (statement->getResult(column, &d, size)) {
    v = WString::fromUTF8(d);
    return true;
  } else {
    v = WString::Empty;
    return false;
  }
}

    /*
     * Json::Object
     */

inline std::string sql_value_traits<Json::Object, void>::type(
    SqlConnection *conn,
    int size)
{
  return conn->textType(size) + " not null";
}

inline void sql_value_traits<Json::Object, void>
::bind(const Json::Object& v, SqlStatement *statement, int column, int size)
{
  statement->bind(column, Json::serialize(v,false));
}

inline bool sql_value_traits<Json::Object, void>
::read(Json::Object& v, SqlStatement *statement, int column, int size)
{
  std::string d;
  if (statement->getResult(column, &d, size)) {
    Json::ParseError e;
    if (Json::parse(d,v,e,false)) {
      return true;
    }
  }
  v = Json::Object::Empty;
  return false;
}

    /*
     * Json::Array
     */

inline std::string sql_value_traits<Json::Array, void>::type(
    SqlConnection *conn,
    int size)
{
  return conn->textType(size) + " not null";
}

inline void sql_value_traits<Json::Array, void>
::bind(const Json::Array& v, SqlStatement *statement, int column, int size)
{
  statement->bind(column, Json::serialize(v,false));
}

inline bool sql_value_traits<Json::Array, void>
::read(Json::Array& v, SqlStatement *statement, int column, int size)
{
  std::string d;
  if (statement->getResult(column, &d, size)) {
    Json::ParseError e;
    if (Json::parse(d,v,e,false)) {
      return true;
    }
  }
  v = Json::Array::Empty;
  return false;
}

  }
}

#endif // WT_DBO_WT_TYPE_TRAITS_H_
