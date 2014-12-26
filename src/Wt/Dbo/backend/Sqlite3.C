/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/backend/Sqlite3"
#include "Wt/Dbo/Exception"

#ifdef SQLITE3_BDB
#include <db.h>
#endif // SQLITE3_BDB
#include <sqlite3.h>
#include <iostream>
#include <math.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

//#define DEBUG(x) x
#define DEBUG(x)
#define USEC_PER_DAY (24.0 * 60 * 60 * 1000 * 1000)

namespace Wt {
  namespace Dbo {
    namespace backend {

inline bool isNaN(double d) {
#ifdef _MSC_VER
  // received bug reports that on 64 bit windows, MSVC2005
  // generates wrong code for d != d.
  return _isnan(d) != 0;
#else
  return !(d == d);
#endif
}

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
  Sqlite3Statement(Sqlite3& db, const std::string& sql)
    : db_(db),
      sql_(sql)
  {
    DEBUG(std::cerr << this << " for: " << sql << std::endl);

#if SQLITE_VERSION_NUMBER >= 3003009
    int err = sqlite3_prepare_v2(db_.connection(), sql.c_str(),
				 static_cast<int>(sql.length() + 1), &st_, 0);
#else
    int err = sqlite3_prepare(db_.connection(), sql.c_str(),
			      static_cast<int>(sql.length() + 1), &st_, 0);
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
    if (st_) {
      int err = sqlite3_reset(st_);
      handleErr(err);

      err = sqlite3_clear_bindings(st_);
      handleErr(err);
    }

    state_ = Done;
  }

  virtual void bind(int column, const std::string& value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    int err = sqlite3_bind_text(st_, column + 1, value.c_str(),
				static_cast<int>(value.length()),
				SQLITE_TRANSIENT);

    handleErr(err);
  }

  virtual void bind(int column, short value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    int err = sqlite3_bind_int(st_, column + 1, value);

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

  virtual void bind(int column, float value)
  {
    bind(column, static_cast<double>(value));
  }

  virtual void bind(int column, double value)
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    int err;
    if (isNaN(value))
      err = sqlite3_bind_text(st_, column + 1, "NaN", 3, SQLITE_TRANSIENT);
    else
      err = sqlite3_bind_double(st_, column + 1, value);

    handleErr(err);
  }

  virtual void bind(int column, const boost::posix_time::time_duration & value)
  {
    DEBUG(std::cerr << this << " bind " << column << " "
	  << boost::posix_time::to_simple_string(value) << std::endl);

    long long msec = value.total_milliseconds();   

    int err = sqlite3_bind_int64(st_, column + 1, msec);

    handleErr(err);
  }

  virtual void bind(int column, const boost::posix_time::ptime& value,
		    SqlDateTimeType type)
  {
    Sqlite3::DateTimeStorage storageType = db_.dateTimeStorage(type);

    switch (storageType) {
    case Sqlite3::ISO8601AsText:
    case Sqlite3::PseudoISO8601AsText: {
      std::string v;
      if (type == SqlDate)
	v = boost::gregorian::to_iso_extended_string(value.date());
      else {
	v = boost::posix_time::to_iso_extended_string(value);

	if (storageType == Sqlite3::PseudoISO8601AsText)
	  v[v.find('T')] = ' ';
      }

      bind(column, v);
      break;
    }
    case Sqlite3::JulianDaysAsReal:
      if (type == SqlDate)
	bind(column, static_cast<double>(value.date().julian_day()));
      else {
	bind(column, value.date().julian_day()
	     + value.time_of_day().total_microseconds() / USEC_PER_DAY);
      }
      break;
    case Sqlite3::UnixTimeAsInteger:
      bind(column,
	   static_cast<long long>
	   ((value - boost::posix_time::ptime
	     (boost::gregorian::date(1970, 1, 1))).total_seconds()));
      break;
    };
  }

  virtual void bind(int column, const std::vector<unsigned char>& value)
  {
    DEBUG(std::cerr << this << " bind " << column << " (blob, size=" <<
	  value.size() << ")" << std::endl);

    int err;

    if (value.size() == 0)
      err = sqlite3_bind_blob(st_, column + 1, "", 0, SQLITE_TRANSIENT);
    else 
      err = sqlite3_bind_blob(st_, column + 1, &(*(value.begin())),
			      static_cast<int>(value.size()), SQLITE_STATIC);

    handleErr(err);
  }

  virtual void bindNull(int column)
  {
    DEBUG(std::cerr << this << " bind " << column << " null" << std::endl);

    int err = sqlite3_bind_null(st_, column + 1);

    handleErr(err);
  }

  virtual void execute()
  {
    if (db_.showQueries())
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
    return sqlite3_last_insert_rowid(db_.connection());
  }

  virtual int affectedRowCount()
  {
    return sqlite3_changes(db_.connection());
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
      done();
      throw Sqlite3Exception("Sqlite3: nextRow(): statement already finished");
    }      

    return false;
  }

  virtual bool getResult(int column, std::string *value, int size)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = (const char *)sqlite3_column_text(st_, column);

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
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = 42;
    *value = sqlite3_column_int(st_, column);

    DEBUG(std::cerr << this 
	  << " result int " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, long long *value)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = sqlite3_column_int64(st_, column);

    DEBUG(std::cerr << this 
	  << " result long long " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, float *value)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = static_cast<float>(sqlite3_column_double(st_, column));

    DEBUG(std::cerr << this 
	  << " result float " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, double *value)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = sqlite3_column_double(st_, column);
  
    DEBUG(std::cerr << this 
	  << " result double " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, boost::posix_time::time_duration *value)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    long long msec = sqlite3_column_int64(st_, column);
    boost::posix_time::time_duration::fractional_seconds_type ticks_per_msec =
      boost::posix_time::time_duration::ticks_per_second() / 1000;

    *value = boost::posix_time::time_duration(0, 0, 0,
					      msec * ticks_per_msec);

    DEBUG(std::cerr << this 
	  << " result time_duration " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, boost::posix_time::ptime *value,
			 SqlDateTimeType type)
  {
    Sqlite3::DateTimeStorage storageType = db_.dateTimeStorage(type);

    *value = boost::posix_time::not_a_date_time;

    switch (storageType) {
    case Sqlite3::ISO8601AsText:
    case Sqlite3::PseudoISO8601AsText: {
      std::string v;
      if (!getResult(column, &v, -1))
	return false;

      try {
	if (type == SqlDate)
	  *value = boost::posix_time::ptime(boost::gregorian::from_string(v),
					    boost::posix_time::hours(0));
	else {
	  std::size_t t = v.find('T');

	  if (t != std::string::npos)
	    v[t] = ' ';
	  if (v.length() > 0 && v[v.length() - 1] == 'Z')
	    v.erase(v.length() - 1);

	  *value = boost::posix_time::time_from_string(v);
	}
      } catch (std::exception& e) {
	std::cerr << "Sqlite3::getResult(ptime): " << e.what() << std::endl;
	return false;
      }

      return true;
    }
    case Sqlite3::JulianDaysAsReal: {
      double v;
      if (!getResult(column, &v))
	return false;

      int vi = static_cast<int>(v);

      if (type == SqlDate)
	*value = boost::posix_time::ptime(fromJulianDay(vi),
					  boost::posix_time::hours(0));
      else {
	double vf = modf(v, &v);
	boost::gregorian::date d = fromJulianDay(vi);
	boost::posix_time::time_duration t
          = boost::posix_time::microseconds((long long)(vf * USEC_PER_DAY));
	*value = boost::posix_time::ptime(d, t);
      }

      return true;
    }
    case Sqlite3::UnixTimeAsInteger: {
      long long v;

      if (!getResult(column, &v))
	return false;

      boost::posix_time::ptime t
	= boost::posix_time::from_time_t(static_cast<std::time_t>(v));
      if (type == SqlDate)
	*value = boost::posix_time::ptime(t.date(),
					  boost::posix_time::hours(0));
      else
	*value = t;

      return true;
    }
    }
    std::stringstream ss;
    ss << __FILE__ << ":" << __LINE__ << ": implementation error";
    throw Sqlite3Exception(ss.str());
  }


  virtual bool getResult(int column, std::vector<unsigned char> *value,
			 int size)
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    int s = sqlite3_column_bytes(st_, column);
    unsigned char *v = (unsigned char *)sqlite3_column_blob(st_, column);

    value->resize(s);
    std::copy(v, v + s, value->begin());

    DEBUG(std::cerr << this 
	  << " result blob " << column << " (blob, size = " << s << ")"
	  << std::endl);

    return true;
  }

 
  virtual std::string sql() const {
    return sql_;
  }

private:
  Sqlite3& db_;
  sqlite3_stmt *st_;
  std::string sql_;
  enum { NoFirstRow, FirstRow, NextRow, Done } state_;

  void handleErr(int err)
  {
    if (err != SQLITE_OK) {
      std::string msg = "Sqlite3: " + sql_ + ": "
	+ sqlite3_errmsg(db_.connection());
      try {
	done();
      }	catch (...) { }

      throw Sqlite3Exception(msg);
    }
  }

  boost::gregorian::date fromJulianDay(int julian) {
    int day, month, year;

    if (julian < 0) {
      julian = 0;
    }

    int a = julian;

    if (julian >= 2299161) {
      int jadj = (int)(((float)(julian - 1867216) - 0.25) / 36524.25);
      a += 1 + jadj - (int)(0.25 * jadj);
    }

    int b = a + 1524;
    int c = (int)(6680.0 + ((float)(b - 2439870) - 122.1) / 365.25);
    int d = (int)(365 * c + (0.25 * c));
    int e = (int)((b - d) / 30.6001);

    day = b - d - (int)(30.6001 * e);
    month = e - 1;

  if (month > 12) {
    month -= 12;
  }

  year = c - 4715;

  if (month > 2) {
    --year;
  }

  if (year <= 0) {
    --year;
  }

  return boost::gregorian::date(year, month, day);
  }
};

Sqlite3::Sqlite3(const std::string& db)
  : conn_(db)
{
  dateTimeStorage_[SqlDate] = ISO8601AsText;
  dateTimeStorage_[SqlDateTime] = ISO8601AsText;

  int err = sqlite3_open(conn_.c_str(), &db_);

  if (err != SQLITE_OK)
    throw Sqlite3Exception(sqlite3_errmsg(db_));

  init();
}

Sqlite3::Sqlite3(const Sqlite3& other)
  : SqlConnection(other),
    conn_(other.conn_)
{
  dateTimeStorage_[SqlDate] = other.dateTimeStorage_[SqlDate];
  dateTimeStorage_[SqlDateTime] = other.dateTimeStorage_[SqlDateTime];

  int err = sqlite3_open(conn_.c_str(), &db_);

  if (err != SQLITE_OK)
    throw Sqlite3Exception(sqlite3_errmsg(db_));

  init();
}

void Sqlite3::init()
{
  executeSql("pragma foreign_keys = ON");

  sqlite3_busy_timeout(db_, 1000);
}

Sqlite3::~Sqlite3()
{
  clearStatementCache();

  sqlite3_close(db_);
}

Sqlite3 *Sqlite3::clone() const
{
  return new Sqlite3(*this);
}

SqlStatement *Sqlite3::prepareStatement(const std::string& sql)
{
  return new Sqlite3Statement(*this, sql);
}

std::string Sqlite3::autoincrementType() const
{
  return "integer";
}

std::string Sqlite3::autoincrementSql() const
{
  return "autoincrement";
}

std::vector<std::string> 
Sqlite3::autoincrementCreateSequenceSql(const std::string &table,
					const std::string &id) const
{
  return std::vector<std::string>();
}

std::vector<std::string> 
Sqlite3::autoincrementDropSequenceSql(const std::string &table,
				      const std::string &id) const
{
  return std::vector<std::string>();
}

std::string Sqlite3::autoincrementInsertSuffix(const std::string& id) const
{
  return std::string();
}

const char *Sqlite3::dateTimeType(SqlDateTimeType type) const
{
  if (type == SqlTime)
    return "integer";
  else
    switch (dateTimeStorage(type)) {
    case ISO8601AsText:
    case PseudoISO8601AsText:
      return "text";
    case JulianDaysAsReal:
      return "real";
    case UnixTimeAsInteger:
      return "integer";
    }

  std::stringstream ss;
  ss << __FILE__ << ":" << __LINE__ << ": implementation error";
  throw Sqlite3Exception(ss.str());
}

const char *Sqlite3::blobType() const
{
  return "blob not null";
}

bool Sqlite3::supportDeferrableFKConstraint() const
{
  return true;
}

void Sqlite3::setDateTimeStorage(SqlDateTimeType type,
				 DateTimeStorage storage)
{
  dateTimeStorage_[type] = storage;
}

Sqlite3::DateTimeStorage Sqlite3::dateTimeStorage(SqlDateTimeType type) const
{
  return dateTimeStorage_[type];
}

void Sqlite3::startTransaction() 
{
  executeSql("begin transaction");
}

void Sqlite3::commitTransaction() 
{
  executeSql("commit transaction");
}

void Sqlite3::rollbackTransaction() 
{
  executeSql("rollback transaction");
}
    }
  }
}
