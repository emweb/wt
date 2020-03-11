/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/backend/Sqlite3.h"
#include "Wt/Dbo/Exception.h"
#include "Wt/Dbo/Logger.h"
#include "Wt/Dbo/StringStream.h"

#ifdef SQLITE3_BDB
#include <db.h>
#endif // SQLITE3_BDB
#include <sqlite3.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <math.h>

#ifdef WT_WIN32
#define timegm _mkgmtime
#endif

#define USEC_PER_DAY (24.0 * 60 * 60 * 1000 * 1000)

namespace {
#ifndef WT_WIN32
  thread_local std::tm local_tm;
#endif

  std::tm *thread_local_gmtime(const time_t *timep)
  {
#ifdef WT_WIN32
    return std::gmtime(timep); // Already returns thread-local pointer
#else // !WT_WIN32
    gmtime_r(timep, &local_tm);
    return &local_tm;
#endif // WT_WIN32
  }
}

namespace Wt {
  namespace Dbo {

LOGGER("Dbo.backend.Sqlite3");

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

class Sqlite3Statement final : public SqlStatement
{
public:
  Sqlite3Statement(Sqlite3& db, const std::string& sql)
    : db_(db),
      sql_(sql)
  {
    LOG_DEBUG(this << " for: " << sql);

#if SQLITE_VERSION_NUMBER >= 3003009
    int err = sqlite3_prepare_v2(db_.connection(), sql.c_str(),
				 static_cast<int>(sql.length() + 1), &st_, 
				 nullptr);
#else
    int err = sqlite3_prepare(db_.connection(), sql.c_str(),
			      static_cast<int>(sql.length() + 1), &st_, 
			      nullptr);
#endif

    handleErr(err);

    state_ = Done;
  }

  virtual ~Sqlite3Statement()
  {
    sqlite3_finalize(st_);
  }

  virtual void reset() override
  {
    if (st_) {
      int err = sqlite3_reset(st_);
      handleErr(err);

      err = sqlite3_clear_bindings(st_);
      handleErr(err);
    }

    state_ = Done;
  }

  virtual void bind(int column, const std::string& value) override
  {
    LOG_DEBUG(this << " bind " << column << " " << value);

    int err = sqlite3_bind_text(st_, column + 1, value.c_str(),
				static_cast<int>(value.length()),
				SQLITE_TRANSIENT);

    handleErr(err);
  }

  virtual void bind(int column, short value) override
  {
    LOG_DEBUG(this << " bind " << column << " " << value);

    int err = sqlite3_bind_int(st_, column + 1, value);

    handleErr(err);
  }

  virtual void bind(int column, int value) override
  {
    LOG_DEBUG(this << " bind " << column << " " << value);

    int err = sqlite3_bind_int(st_, column + 1, value);

    handleErr(err);
  }

  virtual void bind(int column, long long value) override
  {
    LOG_DEBUG(this << " bind " << column << " " << value);

    int err = sqlite3_bind_int64(st_, column + 1, value);

    handleErr(err);
  }

  virtual void bind(int column, float value) override
  {
    bind(column, static_cast<double>(value));
  }

  virtual void bind(int column, double value) override
  {
    LOG_DEBUG(this << " bind " << column << " " << value);

    int err;
    if (isNaN(value))
      err = sqlite3_bind_text(st_, column + 1, "NaN", 3, SQLITE_TRANSIENT);
    else
      err = sqlite3_bind_double(st_, column + 1, value);

    handleErr(err);
  }

  virtual void bind(int column, const std::chrono::duration<int, std::milli> & value) override
  {
    LOG_DEBUG(this << " bind " << column << " " << value.count() << "ms");

    long long msec = value.count();
    int err = sqlite3_bind_int64(st_, column + 1, msec);

    handleErr(err);
  }

  virtual void bind(int column, const std::chrono::system_clock::time_point& value,
		    SqlDateTimeType type) override
  {
    DateTimeStorage storageType = db_.dateTimeStorage(type);
    std::time_t t = std::chrono::system_clock::to_time_t(value);
    std::tm *tm = thread_local_gmtime(&t);
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(value.time_since_epoch());

    switch (storageType) {
    case DateTimeStorage::ISO8601AsText:
    case DateTimeStorage::PseudoISO8601AsText: {
      std::string v;
      char str[100];
      if (type == SqlDateTimeType::Date){
        std::strftime(str, sizeof(str), "%Y-%m-%d", tm);
        v = str;
      }
      else {
        std::strftime(str, sizeof(str), "%Y-%m-%dT%H:%M:%S", tm);
        v = str;
        std::stringstream ss;
        ss.imbue(std::locale::classic());
        ss << "." << std::setfill('0') << std::setw(3) << ms.count()%1000;
        v.append(ss.str());

	if (storageType == DateTimeStorage::PseudoISO8601AsText)
	  v[v.find('T')] = ' ';
      }

      bind(column, v);
      break;
    }
    case DateTimeStorage::JulianDaysAsReal: {
      int a = floor((14 - tm->tm_mon + 1)/12);
      int nyears = tm->tm_year + 1900 + 4800 - a;
      int nmonths = tm->tm_mon + 1 + 12*a - 3;
      int julianday = tm->tm_mday + floor((153*nmonths + 2)/5) + 365*nyears + floor(nyears/4) - floor(nyears/100) + floor(nyears/400) - 32045;
      if (type == SqlDateTimeType::Date)
        bind(column, static_cast<double>(julianday));
      else {
        long long msec = (tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec)*1000 + ms.count()%1000;
        bind(column, julianday
         + msec / USEC_PER_DAY);
      }
      break;
    }
    case DateTimeStorage::UnixTimeAsInteger:
      std::tm tmEpoch = std::tm();
      tmEpoch.tm_mday = 1;
      tmEpoch.tm_mon = 0;
      tmEpoch.tm_year = 70;
      std::time_t tEpoch = timegm(&tmEpoch);
      std::chrono::system_clock::time_point tpEpoch = std::chrono::system_clock::from_time_t(tEpoch);
      std::chrono::system_clock::time_point diff = std::chrono::system_clock::time_point(value - tpEpoch);
      std::time_t tdiff = std::chrono::system_clock::to_time_t(diff);
      std::tm *tmDiff = thread_local_gmtime(&tdiff);
      std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(diff.time_since_epoch());
      bind(column,
	   static_cast<long long>
       ((tmDiff->tm_hour * 3600 + tmDiff->tm_min * 60 + tmDiff->tm_sec)*1000 + millis.count()%1000));
      break;
    };
  }

  virtual void bind(int column, const std::vector<unsigned char>& value) override
  {
    LOG_DEBUG(this << " bind " << column << " (blob, size=" << value.size() << ")");

    int err;

    if (value.size() == 0)
      err = sqlite3_bind_blob(st_, column + 1, "", 0, SQLITE_TRANSIENT);
    else 
      err = sqlite3_bind_blob(st_, column + 1, &(*(value.begin())),
			      static_cast<int>(value.size()), SQLITE_TRANSIENT);

    handleErr(err);
  }

  virtual void bindNull(int column) override
  {
    LOG_DEBUG(this << " bind " << column << " null");

    int err = sqlite3_bind_null(st_, column + 1);

    handleErr(err);
  }

  virtual void execute() override
  {
    if (db_.showQueries()) {
      LOG_INFO(sql_);
    }

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

  virtual long long insertedId() override
  {
    return sqlite3_last_insert_rowid(db_.connection());
  }

  virtual int affectedRowCount() override
  {
    return sqlite3_changes(db_.connection());
  }
  
  virtual bool nextRow() override
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

  virtual int columnCount() const override {
    return sqlite3_column_count(st_);
  }

  virtual bool getResult(int column, std::string *value, int size) override
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = (const char *)sqlite3_column_text(st_, column);

    LOG_DEBUG(this << " result string " << column << " " << *value);

    return true;
  }

  virtual bool getResult(int column, short *value) override
  {
    int intValue;
    if (getResult(column, &intValue)) {
      *value = intValue;
      return true;
    } else
      return false;
  }

  virtual bool getResult(int column, int *value) override
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = 42;
    *value = sqlite3_column_int(st_, column);

    LOG_DEBUG(this << " result int " << column << " " << *value);

    return true;
  }

  virtual bool getResult(int column, long long *value) override
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = sqlite3_column_int64(st_, column);

    LOG_DEBUG(this << " result long long " << column << " " << *value);

    return true;
  }

  virtual bool getResult(int column, float *value) override
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = static_cast<float>(sqlite3_column_double(st_, column));
    if (sqlite3_column_type(st_, column) != SQLITE_FLOAT) {
      const char *txt = (const char*)sqlite3_column_text(st_, column);
      if (txt[0] == 'N' && txt[1] == 'a' && txt[2] == 'N' && txt[3] == '\0') {
        *value = std::numeric_limits<float>::quiet_NaN();
      }
    }

    LOG_DEBUG(this << " result float " << column << " " << *value);

    return true;
  }

  virtual bool getResult(int column, double *value) override
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    *value = sqlite3_column_double(st_, column);
    if (sqlite3_column_type(st_, column) != SQLITE_FLOAT) {
      const char *txt = (const char*)sqlite3_column_text(st_, column);
      if (txt[0] == 'N' && txt[1] == 'a' && txt[2] == 'N' && txt[3] == '\0') {
        *value = std::numeric_limits<double>::quiet_NaN();
      }
    }

    LOG_DEBUG(this << " result double " << column << " " << *value);

    return true;
  }

  virtual bool getResult(int column, std::chrono::duration<int, std::milli> *value) override
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    long long msec = sqlite3_column_int64(st_, column);

    *value = std::chrono::milliseconds(msec);

    LOG_DEBUG(this << " result time_duration " << column << " " << value->count() << "ms");

    return true;
  }

  virtual bool getResult(int column, std::chrono::system_clock::time_point *value,
			 SqlDateTimeType type) override
  {
    DateTimeStorage storageType = db_.dateTimeStorage(type);

    switch (storageType) {
    case DateTimeStorage::ISO8601AsText:
    case DateTimeStorage::PseudoISO8601AsText: {
      std::string v;
      if (!getResult(column, &v, -1))
	return false;

      try {
    if (type == SqlDateTimeType::Date){
      int year, month, day;
      std::sscanf(v.c_str(), "%d-%d-%d", &year, &month, &day);
      std::tm tm = std::tm();
      tm.tm_year = year - 1900;
      tm.tm_mon = month - 1;
      tm.tm_mday = day;
      std::time_t t = timegm(&tm);
      *value = std::chrono::system_clock::from_time_t(t);
    } else {
	  std::size_t t = v.find('T');

	  if (t != std::string::npos)
	    v[t] = ' ';
	  if (v.length() > 0 && v[v.length() - 1] == 'Z')
	    v.erase(v.length() - 1);

      int year, month, day, hour, min, sec, ms;
      std::sscanf(v.c_str(), "%d-%d-%d %d:%d:%d.%d", &year, &month, &day, &hour, &min, &sec, &ms);
      std::tm tm = std::tm();
      tm.tm_year = year - 1900;
      tm.tm_mon = month - 1;
      tm.tm_mday = day;
      tm.tm_hour = hour;
      tm.tm_min = min;
      tm.tm_sec = sec;
      std::time_t timet = timegm(&tm);
      *value = std::chrono::system_clock::from_time_t(timet);
      *value += std::chrono::milliseconds(ms);
	}
      } catch (std::exception& e) {
        LOG_ERROR("Sqlite3::getResult(ptime): " << e.what());
	return false;
      }

      return true;
    }
    case DateTimeStorage::JulianDaysAsReal: {
      double v;
      if (!getResult(column, &v))
	return false;

      int vi = static_cast<int>(v);

      if (type == SqlDateTimeType::Date)
        *value = fromJulianDay(vi);
      else {
        double vf = modf(v, &v);
        std::chrono::system_clock::time_point d = fromJulianDay(vi);
        d += std::chrono::microseconds((int)(vf * USEC_PER_DAY));
        *value = d;
      }

      return true;
    }
    case DateTimeStorage::UnixTimeAsInteger: {
      long long v;

      if (!getResult(column, &v))
        return false;
      std::chrono::system_clock::time_point tp =
	std::chrono::system_clock::from_time_t(static_cast<std::time_t>(v));
      if (type == SqlDateTimeType::Date){
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::tm *tm = thread_local_gmtime(&t);
        std::tm time = std::tm();
        time.tm_year = tm->tm_year;
        time.tm_mon = tm->tm_mon;
        time.tm_mday = tm->tm_mday;
        std::time_t t2 = timegm(&time);
        *value = std::chrono::system_clock::from_time_t(t2);
      }
      else
        *value = tp;

      return true;
    }
    }
    std::stringstream ss;
    ss << __FILE__ << ":" << __LINE__ << ": implementation error";
    throw Sqlite3Exception(ss.str());
  }


  virtual bool getResult(int column, std::vector<unsigned char> *value,
			 int size) override
  {
    if (sqlite3_column_type(st_, column) == SQLITE_NULL)
      return false;

    int s = sqlite3_column_bytes(st_, column);
    unsigned char *v = (unsigned char *)sqlite3_column_blob(st_, column);

    value->resize(s);
    std::copy(v, v + s, value->begin());

    LOG_DEBUG(this << " result blob " << column << " (blob, size = " << s << ")");

    return true;
  }

 
  virtual std::string sql() const override {
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

  std::chrono::system_clock::time_point fromJulianDay(int julian) {
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

  std::tm tm = std::tm();
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  std::time_t t = timegm(&tm);

  return std::chrono::system_clock::from_time_t(t);
  }
};

Sqlite3::Sqlite3(const std::string& db)
  : conn_(db)
{
  dateTimeStorage_[static_cast<unsigned>(SqlDateTimeType::Date)]
    = DateTimeStorage::ISO8601AsText;
  dateTimeStorage_[static_cast<unsigned>(SqlDateTimeType::DateTime)]
    = DateTimeStorage::ISO8601AsText;

  int err = sqlite3_open(conn_.c_str(), &db_);

  if (err != SQLITE_OK)
    throw Sqlite3Exception(sqlite3_errmsg(db_));

  init();
}

Sqlite3::Sqlite3(const Sqlite3& other)
  : SqlConnection(other),
    conn_(other.conn_)
{
  dateTimeStorage_[static_cast<unsigned>(SqlDateTimeType::Date)] 
    = other
    .dateTimeStorage_[static_cast<unsigned>(SqlDateTimeType::Date)];
  dateTimeStorage_[static_cast<unsigned>(SqlDateTimeType::DateTime)] 
    = other
    .dateTimeStorage_[static_cast<unsigned>(SqlDateTimeType::DateTime)];

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

std::unique_ptr<SqlConnection> Sqlite3::clone() const
{
  return std::unique_ptr<SqlConnection>(new Sqlite3(*this));
}

std::unique_ptr<SqlStatement> Sqlite3::prepareStatement(const std::string& sql)
{
  return std::unique_ptr<SqlStatement>(
      new Sqlite3Statement(*this, sql));
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
  if (type == SqlDateTimeType::Time)
    return "integer";
  else
    switch (dateTimeStorage(type)) {
    case DateTimeStorage::ISO8601AsText:
    case DateTimeStorage::PseudoISO8601AsText:
      return "text";
    case DateTimeStorage::JulianDaysAsReal:
      return "real";
    case DateTimeStorage::UnixTimeAsInteger:
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
  dateTimeStorage_[static_cast<unsigned>(type)] = storage;
}

DateTimeStorage Sqlite3::dateTimeStorage(SqlDateTimeType type) const
{
  return dateTimeStorage_[static_cast<unsigned>(type)];
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
