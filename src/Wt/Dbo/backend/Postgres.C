/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Hilary Cheng
 */
#include "Wt/Dbo/backend/Postgres"
#include "Wt/Dbo/Exception"

#include <libpq-fe.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp> 

#ifdef WIN32
#define snprintf _snprintf
#endif

#define BYTEAOID 17

//#define DEBUG(x) x
#define DEBUG(x)

namespace Wt {
  namespace Dbo {
    namespace backend {

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
  PostgresStatement(Postgres& conn, const std::string& sql)
    : conn_(conn),
      sql_(convertToNumberedPlaceholders(sql))
  {
    lastId_ = -1;
    row_ = affectedRows_ = 0;
    result_ = 0;

    paramValues_ = 0;
    paramTypes_ = paramLengths_ = paramFormats_ = 0;
 
    snprintf(name_, 64, "SQL%p%08X", this, rand());

    DEBUG(std::cerr << this << " for: " << sql_ << std::endl);

    state_ = Done;
  }

  virtual ~PostgresStatement()
  {
    PQclear(result_);
    delete[] paramValues_;
    delete[] paramTypes_;
  }

  virtual void reset()
  {
    state_ = Done;
  }

  virtual void bind(int column, const std::string& value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, value);
  }

  virtual void bind(int column, short value)
  {
    bind(column, static_cast<int>(value));
  }

  virtual void bind(int column, int value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, boost::lexical_cast<std::string>(value));
  }

  virtual void bind(int column, long long value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, boost::lexical_cast<std::string>(value));
  }

  virtual void bind(int column, float value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, boost::lexical_cast<std::string>(value));
  }

  virtual void bind(int column, double value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, boost::lexical_cast<std::string>(value));
  }

  virtual void bind(int column, const boost::posix_time::time_duration & value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << boost::posix_time::to_simple_string(value) << std::endl);

    std::string v = boost::posix_time::to_simple_string(value);   

    setValue(column, v);
  }

  virtual void bind(int column, const boost::posix_time::ptime& value,
		    SqlDateTimeType type)
  {
    DEBUG(std::cerr << this << " bind " << column << " "
	  << boost::posix_time::to_simple_string(value) << std::endl);

    std::string v;
    if (type == SqlDate)
      v = boost::gregorian::to_iso_extended_string(value.date());
    else {
      v = boost::posix_time::to_iso_extended_string(value);
      v[v.find('T')] = ' ';
    }

    setValue(column, v);
  }

  virtual void bind(int column, const std::vector<unsigned char>& value)
  {
    DEBUG(std::cerr << this << " bind " << column << " (blob, size=" <<
	  value.size() << ")" << std::endl);

    for (int i = (int)params_.size(); i <= column; ++i)
      params_.push_back(Param());

    Param& p = params_[column];
    p.value.resize(value.size());
    memcpy(const_cast<char *>(p.value.data()), &(*value.begin()), value.size());
    p.isbinary = true;
    p.isnull = false;

    // FIXME if first null was bound, check here and invalidate the prepared
    // statement if necessary because the type changes
  }

  virtual void bindNull(int column)
  {
    DEBUG(std::cerr << this << " bind " << column << " null" << std::endl);

    for (int i = (int)params_.size(); i <= column; ++i)
      params_.push_back(Param());

    params_[column].isnull = true;
  }

  virtual void execute()
  {
    if (conn_.showQueries())
      std::cerr << sql_ << std::endl;

    if (!result_) {
      paramValues_ = new char *[params_.size()];

      for (unsigned i = 0; i < params_.size(); ++i) {
	if (params_[i].isbinary) {
	  paramTypes_ = new int[params_.size() * 3];
	  paramLengths_ = paramTypes_ + params_.size();
	  paramFormats_ = paramLengths_ + params_.size();
	  for (unsigned j = 0; j < params_.size(); ++j) {
	    paramTypes_[j] = params_[j].isbinary ? BYTEAOID : 0;
	    paramFormats_[j] = params_[j].isbinary ? 1 : 0;
	    paramLengths_[j] = 0;
	  }

	  break;
	}
      }

      result_ = PQprepare(conn_.connection(), name_, sql_.c_str(),
			  paramTypes_ ? params_.size() : 0, (Oid *)paramTypes_);
      handleErr(PQresultStatus(result_));
    }

    for (unsigned i = 0; i < params_.size(); ++i) {
      if (params_[i].isnull)
	paramValues_[i] = 0;
      else
	if (params_[i].isbinary) {
	  paramValues_[i] = const_cast<char *>(params_[i].value.data());
	  paramLengths_[i] = params_[i].value.length();
	} else
	  paramValues_[i] = const_cast<char *>(params_[i].value.c_str());
    }

    PQclear(result_);
    result_ = PQexecPrepared(conn_.connection(), name_, params_.size(),
			     paramValues_, paramLengths_, paramFormats_, 0);

    row_ = 0;
    if (PQresultStatus(result_) == PGRES_COMMAND_OK) {
      std::string s = PQcmdTuples(result_);
      if (!s.empty())
	affectedRows_ = boost::lexical_cast<int>(s);
      else
	affectedRows_ = 0;
    } else if (PQresultStatus(result_) == PGRES_TUPLES_OK)
      affectedRows_ = PQntuples(result_);
    if (affectedRows_ == 1 && sql_.rfind("returning id") != std::string::npos) {
      state_ = NoFirstRow;
      if (PQntuples(result_) == 1 && PQnfields(result_) == 1) {
	lastId_ = boost::lexical_cast<int>(PQgetvalue(result_, 0, 0));
      }
    } else {
      if (PQntuples(result_) == 0) {
	state_ = NoFirstRow;
      } else {
	state_ = FirstRow;
      }
    }

    handleErr(PQresultStatus(result_));
  }

  virtual long long insertedId()
  {
    return lastId_;
  }

  virtual int affectedRowCount()
  {
    return affectedRows_;
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
      if (row_ + 1 < PQntuples(result_)) {
	row_++;
	return true;
      } else {
	state_ = Done;
	return false;
      }
      break;
    case Done:
      throw PostgresException("Postgres: nextRow(): statement already "
			      "finished");
    }

    return false;
  }

  virtual bool getResult(int column, std::string *value, int size)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = PQgetvalue(result_, row_, column);

    DEBUG(std::cerr << this 
	  << " result string " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, short *value)
  {
    int intValue;
    if (getResult(column, &intValue)) {
      *value = intValue;
      return true;
    } else
      return false;
  }

  virtual bool getResult(int column, int *value)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = boost::lexical_cast<int>(PQgetvalue(result_, row_, column));

    DEBUG(std::cerr << this 
	  << " result int " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, long long *value)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value
      = boost::lexical_cast<long long>(PQgetvalue(result_, row_, column));

    DEBUG(std::cerr << this 
	  << " result long long " << column << " " << *value << std::endl);

    return true;
  }
  
  virtual bool getResult(int column, float *value)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = boost::lexical_cast<float>(PQgetvalue(result_, row_, column));

    DEBUG(std::cerr << this 
	  << " result float " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, double *value)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = boost::lexical_cast<double>(PQgetvalue(result_, row_, column));

    DEBUG(std::cerr << this 
	  << " result double " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, boost::posix_time::ptime *value,
			 SqlDateTimeType type)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    std::string v;
    v = PQgetvalue(result_, row_, column);

    if (type == SqlDate)
      *value = boost::posix_time::ptime(boost::gregorian::from_string(v),
					boost::posix_time::hours(0));
    else
      *value = boost::posix_time::time_from_string(v);

    DEBUG(std::cerr << this 
	  << " result time_duration " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, boost::posix_time::time_duration *value)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    std::string v;
    v = PQgetvalue(result_, row_, column);

    *value = boost::posix_time::time_duration
      (boost::posix_time::duration_from_string(v));

    return true;
  }

  virtual bool getResult(int column, std::vector<unsigned char> *value,
			 int size)
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    const char *escaped = PQgetvalue(result_, row_, column);

    std::size_t vlength;
    unsigned char *v = PQunescapeBytea((unsigned char *)escaped, &vlength);

    value->resize(vlength);
    std::copy(v, v + vlength, value->begin());
    free(v);

    DEBUG(std::cerr << this 
	  << " result blob " << column << " (blob, size = " << vlength << ")"
	  << std::endl);

    return true;
  }

  virtual std::string sql() const {
    return sql_;
  }

private:
  struct Param {
    std::string value;
    bool isnull, isbinary;

    Param() : isnull(true), isbinary(false) { }
  };

  Postgres& conn_;
  std::string sql_;
  char name_[64];
  PGresult *result_;
  enum { NoFirstRow, FirstRow, NextRow, Done } state_;
  std::vector<Param> params_;

  char **paramValues_;
  int *paramTypes_, *paramLengths_, *paramFormats_;
 
  int lastId_, row_, affectedRows_;

  void handleErr(int err)
  {
    if (err != PGRES_COMMAND_OK && err != PGRES_TUPLES_OK)
      throw PostgresException(PQerrorMessage(conn_.connection()));
  }

  void setValue(int column, const std::string& value) {
    for (int i = (int)params_.size(); i <= column; ++i)
      params_.push_back(Param());

    params_[column].value = value;
    params_[column].isnull = false;
  }

  std::string convertToNumberedPlaceholders(const std::string& sql)
  {
    std::stringstream result;

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
	  if (i + 1 == sql.length())
	    state = Statement;
	  else if (sql[i + 1] == '\'') {
	    result << sql[i];
	    ++i; // skip to next
	  } else
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
  : conn_(NULL)
{
  if (!db.empty())
    connect(db);
}

Postgres::Postgres(const Postgres& other)
  : SqlConnection(other)
{
  if (!other.connInfo_.empty())
    connect(other.connInfo_);
}

Postgres::~Postgres()
{
  clearStatementCache();
  if (conn_)
    PQfinish(conn_);
}

Postgres *Postgres::clone() const
{
  return new Postgres(*this);
}

bool Postgres::connect(const std::string& db)
{
  connInfo_ = db;
  conn_ = PQconnectdb(db.c_str());

  if (PQstatus(conn_) != CONNECTION_OK) {
    std::string error = PQerrorMessage(conn_);
    PQfinish(conn_);
    conn_ = 0;
    throw PostgresException("Could not connect to: " + error);
  }

  return true;
}

SqlStatement *Postgres::prepareStatement(const std::string& sql)
{
  return new PostgresStatement(*this, sql);
}

void Postgres::executeSql(const std::string &sql)
{
  PGresult *result;
  int err;

  if (showQueries())
    std::cerr << sql << std::endl;
			
  result = PQexec(conn_, sql.c_str());
  err = PQresultStatus(result);
  if (err != PGRES_COMMAND_OK && err != PGRES_TUPLES_OK) {
    PQclear(result);
    throw PostgresException(PQerrorMessage(conn_));
  }
  PQclear(result);
}

std::string Postgres::autoincrementType() const
{
  return "serial";
}
  
std::string Postgres::autoincrementSql() const
{
  return std::string();
}

std::string Postgres::autoincrementInsertSuffix() const
{
  return " returning id";
}
  
const char *Postgres::dateTimeType(SqlDateTimeType type) const
{
  switch (type) {
  case SqlDate:
    return "date";
  case SqlDateTime:
    return "timestamp";
  case SqlTime:
    return "interval";
  }

  std::stringstream ss;
  ss << __FILE__ << ":" << __LINE__ << ": implementation error";
  throw PostgresException(ss.str());
}

const char *Postgres::blobType() const
{
  return "bytea not null";
}

void Postgres::startTransaction()
{
  PGresult *result = PQexec(conn_, "start transaction");
  PQclear(result);
}

void Postgres::commitTransaction()
{
  PGresult *result = PQexec(conn_, "commit transaction");
  PQclear(result);
}

void Postgres::rollbackTransaction()
{
  PGresult *result = PQexec(conn_, "rollback transaction");
  PQclear(result);
}

    }
  }
}
