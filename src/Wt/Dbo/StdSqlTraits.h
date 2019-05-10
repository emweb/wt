// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_STD_SQL_TRAITS_H_
#define WT_DBO_STD_SQL_TRAITS_H_

#include <string>
#include <type_traits>
#include <vector>
#include <chrono>
#include <Wt/Dbo/SqlTraits.h>
#include <Wt/Dbo/SqlStatement.h>

namespace boost {
  template<typename T> class optional;
}

#ifdef WT_CXX17
#if __has_include(<optional>)
#include <optional>
#endif // __has_include(<optional>)
#endif // WT_CXX17

namespace Wt {
  namespace Dbo {

template<>
struct WTDBO_API sql_value_traits<std::string, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size);
  static void bind(const std::string& v, SqlStatement *statement, int column,
		   int size);
  static bool read(std::string& v, SqlStatement *statement, int column,
		   int size);
};

template<>
struct WTDBO_API sql_value_traits<long long, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size);
  static void bind(long long v, SqlStatement *statement, int column, int size);
  static bool read(long long& v, SqlStatement *statement, int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<int, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(int v, SqlStatement *statement, int column, int size);
  static bool read(int& v, SqlStatement *statement, int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<long, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size);
  static void bind(long v, SqlStatement *statement, int column, int size);
  static bool read(long& v, SqlStatement *statement, int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<short, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(short v, SqlStatement *statement, int column, int size);
  static bool read(short& v, SqlStatement *statement, int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<bool, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size);
  static void bind(bool v, SqlStatement *statement, int column, int size);
  static bool read(bool& v, SqlStatement *statement, int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<float, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(float v, SqlStatement *statement, int column, int size);
  static bool read(float& v, SqlStatement *statement, int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<double, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(double v, SqlStatement *statement, int column, int size);
  static bool read(double& v, SqlStatement *statement, int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<std::chrono::system_clock::time_point, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(const std::chrono::system_clock::time_point& v, SqlStatement *statement,
		   int column, int size);
  static bool read(std::chrono::system_clock::time_point& v, SqlStatement *statement,
		   int column, int size);
};

template<>
struct WTDBO_API sql_value_traits<std::chrono::duration<int, std::milli>, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(const std::chrono::duration<int, std::milli>& v,
		   SqlStatement *statement, int column, int size);
  static bool read(std::chrono::duration<int, std::milli>& v,
		   SqlStatement *statement, int column, int size);
};

template <typename Enum>
struct WTDBO_API sql_value_traits<Enum,
	         typename std::enable_if<std::is_enum<Enum>::value>::type> 
: public sql_value_traits<int>
{
  static void bind(Enum v, SqlStatement *statement, int column, int size) {
    sql_value_traits<int>::bind(static_cast<int>(v), statement, column, size);
  }

  static bool read(Enum& v, SqlStatement *statement, int column, int size) {
    return sql_value_traits<int>::read(reinterpret_cast<int&>(v), statement,
				       column, size);
  }
};

template<>
struct WTDBO_API sql_value_traits<std::vector<unsigned char>, void>
{
  static const bool specialized = true;

  static const char *type(SqlConnection *conn, int size);
  static void bind(const std::vector<unsigned char>& v,
		   SqlStatement *statement, int column, int size);
  static bool read(std::vector<unsigned char>& v, SqlStatement *statement,
		   int column, int size);
};

template<typename T>
struct sql_value_traits<boost::optional<T>, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size) {
    std::string nested = sql_value_traits<T>::type(conn, size);
    if (nested.length() > 9
	&& nested.substr(nested.length() - 9) == " not null")
      return nested.substr(0, nested.length() - 9);
    else
      return nested;
  }

  static void bind(const boost::optional<T>& v,
		   SqlStatement *statement, int column, int size) {
    if (v)
      sql_value_traits<T>::bind(v.get(), statement, column, size);
    else
      statement->bindNull(column);
  }

  static bool read(boost::optional<T>& v, SqlStatement *statement, int column,
                   int size) {
    T result;
    if (sql_value_traits<T>::read(result, statement, column, size)) {
      v = result;
      return true;
    } else {
      v = boost::optional<T>();
      return false;
    }
  }
};

#ifdef WT_CXX17
#if __has_include(<optional>)
template<typename T>
struct sql_value_traits<std::optional<T>, void>
{
  static const bool specialized = true;

  static std::string type(SqlConnection *conn, int size) {
    std::string nested = sql_value_traits<T>::type(conn, size);
    if (nested.length() > 9
        && nested.substr(nested.length() - 9) == " not null")
      return nested.substr(0, nested.length() - 9);
    else
      return nested;
  }

  static void bind(const std::optional<T>& v,
                   SqlStatement *statement, int column, int size) {
    if (v)
      sql_value_traits<T>::bind(*v, statement, column, size);
    else
      statement->bindNull(column);
  }

  static bool read(std::optional<T>& v, SqlStatement *statement, int column,
                   int size) {
    T result;
    if (sql_value_traits<T>::read(result, statement, column, size)) {
      v = result;
      return true;
    } else {
      v = std::nullopt;
      return false;
    }
  }
};
#endif // __has_include(<optional>)
#endif // WT_CXX17

  }
}

#endif // WT_DBO_STD_SQL_TRAITS_H_
