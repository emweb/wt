/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/backend/Sqlite3"
#include "Wt/Dbo/Exception"

#include <sqlite3.h>
#include <iostream>

//#define DEBUG(x) x
#define DEBUG(x)

namespace Wt {
  namespace Dbo {
    namespace backend {

const bool showQueries = true;

class Sqlite3Exception : public Exception
{
public:
  Sqlite3Exception(const std::string& msg)
    : Exception(msg)
  { }
};

class Sqlite3Statement : public SqlStatement
{
public:
  Sqlite3Statement(sqlite3 *db, const std::string& sql)
    : db_(db),
      sql_(sql)
  {
    DEBUG(std::cerr << this << " for: " << sql << std::endl);
    
#if SQLITE3_VERSION_NUMBER >= 3009009
    int err = sqlite3_prepare_v2(db_, sql.c_str(), sql.length() + 1, &st_, 0);
#else
    int err = sqlite3_prepare(db_, sql.c_str(), sql.length() + 1, &st_, 0);
#endif

    handleErr(err);

    state_ = Done;
  }

  virtual ~Sqlite3Statement()
  {
    sqlite3_finalize(st_);
  }

  virtual void reset()
  {
    int err = sqlite3_reset(st_);

    handleErr(err);

    state_ = Done;
  }

  virtual void bind(int column, const std::string& value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    int err = sqlite3_bind_text(st_, column + 1, value.c_str(),
				value.length(), SQLITE_TRANSIENT);

    handleErr(err);
  }

  virtual void bind(int column, int value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    int err = sqlite3_bind_int(st_, column + 1, value);

    handleErr(err);
  }

  virtual void bind(int column, long long value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    int err = sqlite3_bind_int64(st_, column + 1, value);

    handleErr(err);
  }

  virtual void execute()
  {
    if (showQueries)
      std::cerr << sql_ << std::endl;

    int result = sqlite3_step(st_);

    if (result == SQLITE_ROW)
      state_ = FirstRow;
    else if (result == SQLITE_DONE)
      state_ = NoFirstRow;
    else {
      state_ = Done;

      handleErr(result);
    }
  }

  virtual long long insertedId()
  {
    return sqlite3_last_insert_rowid(db_);
  }

  virtual int affectedRowCount()
  {
    return sqlite3_changes(db_);
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
      {
	int result = sqlite3_step(st_);

	if (result == SQLITE_ROW)
	  return true;
	else {
	  state_ = Done;
	  if (result == SQLITE_DONE)
	    return false;

	  handleErr(result);
	}
      }
      break;
    case Done:
      throw Sqlite3Exception("Sqlite3: nextRow(): statement already finished");
    }      

    return false;
  }

  virtual bool getResult(int column, std::string *value)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = (const char *)sqlite3_column_text(st_, column);

    DEBUG(std::cerr << this << " result string " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, int *value)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = 42;
    *value = sqlite3_column_int(st_, column);

    DEBUG(std::cerr << this << " result int " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, long long *value)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = sqlite3_column_int64(st_, column);

    DEBUG(std::cerr << this << " result long long " << column << " " << *value << std::endl);

    return true;
  }

  virtual std::string sql() const {
    return sql_;
  }

private:
  sqlite3 *db_;
  sqlite3_stmt *st_;
  std::string sql_;
  enum { NoFirstRow, FirstRow, NextRow, Done } state_;

  void handleErr(int err)
  {
    if (err != SQLITE_OK)
      throw Sqlite3Exception(sqlite3_errmsg(db_));
  }
};

Sqlite3::Sqlite3(const std::string& db)
{
  int err = sqlite3_open(db.c_str(), &db_);

  if (err != SQLITE_OK)
    throw Sqlite3Exception(sqlite3_errmsg(db_));
}

Sqlite3::~Sqlite3()
{
  clearStatementCache();

  sqlite3_close(db_);
}

SqlStatement *Sqlite3::prepareStatement(const std::string& sql)
{
  return new Sqlite3Statement(db_, sql);
}

    }
  }
}
