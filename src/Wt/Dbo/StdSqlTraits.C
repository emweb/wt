/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/StdSqlTraits"
#include "Wt/Dbo/SqlStatement"

#include <boost/lexical_cast.hpp>

#ifndef DOXYGEN_ONLY

namespace Wt {
  namespace Dbo {

std::string sql_value_traits<std::string>::type(SqlConnection *conn, int size)
{
  if (size == -1)
    return "text not null";
  else
    return "varchar(" + boost::lexical_cast<std::string>(size) + ") not null";
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

const char *sql_value_traits<long long>::type(SqlConnection *conn, int size)
{
  return "bigint not null";
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
