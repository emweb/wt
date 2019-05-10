/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/StdSqlTraits.h"
#include "Wt/Dbo/SqlStatement.h"

#include <string>

#ifndef DOXYGEN_ONLY

namespace Wt {
  namespace Dbo {

  /*
   * std::string
   */
std::string sql_value_traits<std::string>::type(SqlConnection *conn, int size)
{
  return std::string(conn->textType(size)) + " not null";
}

void sql_value_traits<std::string>::bind(const std::string& v,
					 SqlStatement *statement, int column,
					 int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<std::string>::read(std::string& v,
					 SqlStatement *statement, int column,
					 int size)
{
  if (!statement->getResult(column, &v, size)) {
    v.clear();
    return false;
  } else
    return true;
}

  /*
   * long long 
   */

std::string sql_value_traits<long long>::type(SqlConnection *conn, int size)
{
  return conn->longLongType() + " not null";
}

void sql_value_traits<long long>::bind(long long v,
				       SqlStatement *statement, int column,
				       int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<long long>::read(long long& v,
				       SqlStatement *statement, int column,
				       int size)
{
  return statement->getResult(column, &v);
}


  /*
   * long
   */

std::string sql_value_traits<long>::type(SqlConnection *conn, int size)
{
  if (sizeof(long) == 4)
    return "integer not null";
  else
    return conn->longLongType() + " not null";
}

void sql_value_traits<long>::bind(long v,
				  SqlStatement *statement, int column,
				  int size)
{
  if (sizeof(long) == 4)
    statement->bind(column, (int)v);
  else
    statement->bind(column, (long long)v);
}

bool sql_value_traits<long>::read(long& v,
				  SqlStatement *statement, int column,
				  int size)
{
  if (sizeof(long) == 4) {
    int v2;
    bool result = statement->getResult(column, &v2);
    v = v2;
    return result;
  } else {
    long long v2;
    bool result = statement->getResult(column, &v2);
    v = v2;
    return result;
  }
}

  /*
   * int
   */

const char *sql_value_traits<int>::type(SqlConnection *conn, int size)
{
  return "integer not null";
}

void sql_value_traits<int>::bind(int v, SqlStatement *statement, int column,
				 int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<int>::read(int& v, SqlStatement *statement, int column,
				 int size)
{
  return statement->getResult(column, &v);
}

  /*
   * short
   */

const char *sql_value_traits<short>::type(SqlConnection *conn,
					  int size)
{
  return "smallint not null";
}

void sql_value_traits<short>::bind(short v, SqlStatement *statement, int column,
				   int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<short>::read(short& v, SqlStatement *statement,
				   int column, int size)
{
  return statement->getResult(column, &v);
}

  /*
   * bool
   */

std::string sql_value_traits<bool>::type(SqlConnection *conn, int size)
{
  return std::string(conn->booleanType()) + " not null";
}

void sql_value_traits<bool>::bind(bool v, SqlStatement *statement, int column,
				  int size)
{
  statement->bind(column, v ? 1 : 0);
}

bool sql_value_traits<bool>::read(bool& v, SqlStatement *statement, int column,
				  int size)
{
  int intValue;
  bool result = statement->getResult(column, &intValue);
  if (result)
    v = intValue ? true : false;

  return result;
}

  /*
   * float
   */

const char *sql_value_traits<float>::type(SqlConnection *conn, int size)
{
  return "real not null";
}

void sql_value_traits<float>::bind(float v, SqlStatement *statement,
				   int column, int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<float>::read(float& v, SqlStatement *statement,
				   int column, int size)
{
  return statement->getResult(column, &v);
}

  /*
   * double
   */

const char *sql_value_traits<double>::type(SqlConnection *conn, int size)
{
  return "double precision not null";
}

void sql_value_traits<double>::bind(double v, SqlStatement *statement,
				    int column, int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<double>::read(double& v, SqlStatement *statement,
				    int column, int size)
{
  return statement->getResult(column, &v);
}

  /*
   * std::chrono::system_clock::time_point
   */

const char *sql_value_traits<std::chrono::system_clock::time_point>
::type(SqlConnection *conn, int size)
{
  return conn->dateTimeType(SqlDateTimeType::DateTime);
}

void sql_value_traits<std::chrono::system_clock::time_point>
::bind(const std::chrono::system_clock::time_point& v, SqlStatement *statement,
       int column, int size)
{
    statement->bind(column, v, SqlDateTimeType::DateTime);
}

bool sql_value_traits<std::chrono::system_clock::time_point>
::read(std::chrono::system_clock::time_point& v, SqlStatement *statement, int column,
       int size)
{
  return statement->getResult(column, &v, SqlDateTimeType::DateTime);
}

  /*
   * std::chrono::duration<int, std::milli>
   */

const char *sql_value_traits<std::chrono::duration<int, std::milli>>
::type(SqlConnection *conn, int size)
{
  return conn->dateTimeType(SqlDateTimeType::Time);
}

void sql_value_traits<std::chrono::duration<int, std::milli>>
::bind(const std::chrono::duration<int, std::milli>& v, SqlStatement *statement,
       int column, int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<std::chrono::duration<int, std::milli>>
::read(std::chrono::duration<int, std::milli>& v, SqlStatement *statement,
       int column, int size)
{
  return statement->getResult(column, &v);
}

   /*
    * std::vector<unsigned char>
    */

const char *sql_value_traits<std::vector<unsigned char> >
::type(SqlConnection *conn, int size)
{
  return conn->blobType();
}

void sql_value_traits<std::vector<unsigned char> >
::bind(const std::vector<unsigned char>& v, SqlStatement *statement,
       int column, int size)
{
  statement->bind(column, v);
}

bool sql_value_traits<std::vector<unsigned char> >
::read(std::vector<unsigned char>& v, SqlStatement *statement, int column,
       int size)
{
  return statement->getResult(column, &v, size);
}

  }
}

#endif
