/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Hilary Cheng
 */
#include "Wt/Dbo/backend/Postgres"
#include "Wt/Dbo/Exception"

#include "EscapeOStream.h"

#include <libpq-fe.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>

//#define DEBUG(x) x
#define DEBUG(x)

namespace Wt {
  namespace Dbo {
    namespace backend {

const bool showQueries = true;
			
class PostgresException : public Exception
{
public:
  PostgresException(const std::string& msg)
    : Exception(msg)
  { }
};

class PostgresStatement : public SqlStatement
{
public:
  PostgresStatement(PGconn *conn, const std::string& sql)
    : conn_(conn),
      sql_(convertToNumberedPlaceholders(sql))
  {
    _oid = InvalidOid;
    lastId_ = -1; affectedRows = 0;
    c_params_ = 0;
 
    snprintf(name_, 64, "SQL%p%08X", this, rand());

    DEBUG(std::cerr << this << " for: " << sql_ << std::endl);

    result = PQprepare(conn_, name_, sql_.c_str(), 0, NULL);
    handleErr(PQresultStatus(result));

    state_ = Done;
  }

  virtual ~PostgresStatement()
  {
    PQclear(result);
    if (c_params_)
      free(c_params_);
  }

  virtual void reset()
  {
    state_ = Done;
  }

  virtual void bind(int column, const std::string& value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    for (unsigned i = params_.size(); i <= column; ++i)
      params_.push_back(std::make_pair(std::string(), true));
    
    params_[column].first = value;
    params_[column].second = false;
  }

  virtual void bind(int column, int value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    for (unsigned i = params_.size(); i <= column; ++i)
      params_.push_back(std::make_pair(std::string(), true));
    
    params_[column].first = boost::lexical_cast<std::string>(value);
    params_[column].second = false;
  }

  virtual void bind(int column, long long value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    for (unsigned i = params_.size(); i <= column; ++i)
      params_.push_back(std::make_pair(std::string(), true));

    params_[column].first = boost::lexical_cast<std::string>(value);
    params_[column].second = false;
  }

  virtual void bind(int column, float value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    for (unsigned i = params_.size(); i <= column; ++i)
      params_.push_back(std::make_pair(std::string(), true));

    params_[column].first = boost::lexical_cast<std::string>(value);
    params_[column].second = false;
  }

  virtual void bind(int column, double value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    for (unsigned i = params_.size(); i <= column; ++i)
      params_.push_back(std::make_pair(std::string(), true));

    params_[column].first = boost::lexical_cast<std::string>(value);
    params_[column].second = false;
  }

  virtual void bindNull(int column)
  {
    DEBUG(std::cerr << this << " bind " << column << " null" << std::endl);

    for (unsigned i = params_.size(); i <= column; ++i)
      params_.push_back(std::make_pair(std::string(), true));

    params_[column].second = true;
  }

  virtual void execute()
  {
    if (showQueries)
      std::cerr << sql_ << std::endl;

    if (!params_.empty()) {
      if (!c_params_)
	c_params_ = (char **) malloc(sizeof(char *) * params_.size());
      for (int count = 0; count < params_.size(); count++) {
	if (params_[count].second)
	  c_params_[count] = 0;
	else
	  c_params_[count] = (char *) params_[count].first.c_str();
      }
    }

    PQclear(result);

    result = PQexecPrepared(conn_, name_, params_.size(), c_params_, 0, 0, 0);
    _oid = PQoidValue(result);

    starting = 0;
    if (PQresultStatus(result) == PGRES_COMMAND_OK)
      affectedRows = boost::lexical_cast<int>(PQcmdTuples(result));
    if (PQresultStatus(result) == PGRES_TUPLES_OK)
      affectedRows = PQntuples(result);
    if (affectedRows == 1 && sql_.rfind("returning id") != std::string::npos) {
      state_ = NoFirstRow;
      if (PQntuples(result) == 1 && PQnfields(result) == 1) {
	lastId_ = boost::lexical_cast<int>(PQgetvalue(result, 0, 0));
      }
    } else {
      if (PQntuples(result) == 0) {
	state_ = NoFirstRow;
      } else {
	state_ = FirstRow;
      }
    }

    handleErr(PQresultStatus(result));
  }

  virtual long long insertedId()
  {
    return lastId_;
  }

  virtual int affectedRowCount()
  {
    return affectedRows;
  }
  
  virtual bool nextRow()
  {
    switch (state_) {
    case NoFirstRow:
      state_ = Done;
      return false;
    case FirstRow:
      state_ = NextRow;
      return true;
    case NextRow:
      if (starting + 1 < PQntuples(result)) {
	starting++;
	return true;
      } else {
	state_ = Done;
	return false;
      }
      break;
    case Done:
      throw PostgresException("Postgres: nextRow(): statement already finished");
    }      

    return false;
  }

  virtual bool getResult(int column, std::string *value)
  {
    if (PQgetisnull(result, starting, column))
      return false;

    *value = PQgetvalue(result, starting, column);

    DEBUG(std::cerr << this 
	  << " result string " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, int *value)
  {
    if (PQgetisnull(result, starting, column))
      return false;

    *value = boost::lexical_cast<int>(PQgetvalue(result, starting, column));

    DEBUG(std::cerr << this 
	  << " result int " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, long long *value)
  {
    if (PQgetisnull(result, starting, column))
      return false;

    *value = boost::lexical_cast<long long>(PQgetvalue(result, starting, column));

    DEBUG(std::cerr << this 
	  << " result long long " << column << " " << *value << std::endl);

    return true;
  }
  
  virtual bool getResult(int column, float *value)
  {
    if (PQgetisnull(result, starting, column))
      return false;

    *value = boost::lexical_cast<float>(PQgetvalue(result, starting, column));

    DEBUG(std::cerr << this 
	  << " result float " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, double *value)
  {
    if (PQgetisnull(result, starting, column))
      return false;

    *value = boost::lexical_cast<double>(PQgetvalue(result, starting, column));

    DEBUG(std::cerr << this 
	  << " result double " << column << " " << *value << std::endl);

    return true;
  }

  virtual std::string sql() const {
    return sql_;
  }

private:
  PGconn *conn_;
  std::string sql_;
  char name_[64];
  PGresult *result;
  enum { NoFirstRow, FirstRow, NextRow, Done } state_;
  std::vector< std::pair<std::string, bool> > params_;
  char **c_params_;
  int _oid, lastId_, starting, affectedRows;

  void handleErr(int err)
  {
    if (err != PGRES_COMMAND_OK && err != PGRES_TUPLES_OK)
      throw PostgresException(PQerrorMessage(conn_));
  }

  std::string convertToNumberedPlaceholders(const std::string& sql)
  {
    SStream result;

    enum { Statement, SQuote, DQuote } state = Statement;
    int placeholder = 1;

    for (unsigned i = 0; i < sql.length(); ++i) {
      switch (state) {
      case Statement:
	if (sql[i] == '\'')
	  state = SQuote;
	else if (sql[i] == '"')
	  state = DQuote;
	else if (sql[i] == '?') {
	  result << '$' << placeholder++;
	  continue;
	}
	break;
      case SQuote:
	if (sql[i] == '\'') {
	  if (i + 1 == sql.length() || sql[i + 1] != '\'')
	    state = Statement;
	}
	break;
      case DQuote:
	if (sql[i] == '"')
	  state = Statement;
	break;
      }
      result << sql[i];
    }

    return result.str();
  }
};

Postgres::Postgres(const std::string& db)
  : conn(NULL)
{
  if (!db.empty())
    connect(db);
}

Postgres::~Postgres()
{
  clearStatementCache();
  if (conn) PQfinish(conn);
}

bool Postgres::connect(const std::string& db)
{
  conn = PQconnectdb(db.c_str());
  if (PQstatus(conn) != CONNECTION_OK) {
    conn = NULL;
    DEBUG(fprintf(stderr, "Connection Failed %s %s\n", db.c_str(), PQerrorMessage(conn)));
    return false;
  }

  return true;
}

SqlStatement *Postgres::prepareStatement(const std::string& sql)
{
  return new PostgresStatement(conn, sql);
}

void Postgres::executeSql(const std::string &sql)
{
  PGresult *result;
  int err;

  fprintf(stderr, "Postgres::executeSql %s\n", sql.c_str());
			
  result = PQexec(conn, sql.c_str());
  err = PQresultStatus(result);
  if (err != PGRES_COMMAND_OK && err != PGRES_TUPLES_OK) {
    PQclear(result);
    throw PostgresException(PQerrorMessage(conn));
  }
  PQclear(result);
}

std::string Postgres::autoincrementType()
{
  return "serial";
}
  
std::string Postgres::autoincrementSql()
{
  return std::string();
}

std::string Postgres::autoincrementInsertSuffix()
{
  return " returning id";
}
  
void Postgres::startTransaction()
{
  fprintf(stderr, "Postgres::startTransaction\n");
  PGresult *result = PQexec(conn, "start transaction");
  PQclear(result);
}

void Postgres::commitTransaction()
{
  fprintf(stderr, "Postgres::commitTransaction\n");
  PGresult *result = PQexec(conn, "commit transaction");
  PQclear(result);
}

void Postgres::rollbackTransaction()
{
  fprintf(stderr, "Postgres::rollbackTransaction\n");
  PGresult *result = PQexec(conn, "rollback transaction");
  PQclear(result);
}

    }
  }
}
