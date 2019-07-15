/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Hilary Cheng
 */
#include "Wt/WConfig.h"

#ifdef WT_WIN32
#define NOMINMAX
// WinSock2.h warns that it should be included before windows.h
#include <WinSock2.h>
#endif // WT_WIN32

#include "Wt/Dbo/backend/Postgres.h"
#include "Wt/Dbo/Exception.h"

#include <libpq-fe.h>
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <cstring>
#include <ctime>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/karma.hpp>

#include "Wt/Date/date.h"

#ifdef WT_WIN32
#define snprintf _snprintf
#define strcasecmp _stricmp
#else // WT_WIN32
#include <sys/select.h>
#endif // WT_WIN32

#define BYTEAOID 17

//#define DEBUG(x) x
#define DEBUG(x)

namespace karma = boost::spirit::karma;

namespace {

  inline struct timeval toTimeval(std::chrono::microseconds ms)
  {
    std::chrono::seconds s = date::floor<std::chrono::seconds>(ms);
    struct timeval result;
    result.tv_sec = s.count();
    result.tv_usec = (ms - s).count();
    return result;
  }

  // adjust rendering for JS flaots
  template <typename T, int Precision>
  struct PostgresPolicy : karma::real_policies<T>
  {
    // not 'nan', but 'NaN'
    template <typename CharEncoding, typename Tag, typename OutputIterator>
    static bool nan (OutputIterator& sink, T n, bool force_sign)
    {
      return karma::string_inserter<CharEncoding, Tag>::call(sink, "NaN");
    }

    // not 'inf', but 'Infinity'
    template <typename CharEncoding, typename Tag, typename OutputIterator>
    static bool inf (OutputIterator& sink, T n, bool force_sign)
    {
      return karma::sign_inserter::call(sink, false, (n<0), force_sign) &&
        karma::string_inserter<CharEncoding, Tag>::call(sink, "Infinity");
    }

    static int floatfield(T t) {
      return (t != 0.0) && ((t < 0.001) || (t > 1E8)) ?
        karma::real_policies<T>::fmtflags::scientific :
        karma::real_policies<T>::fmtflags::fixed;
    }

    // 7 significant numbers; about float precision
    static unsigned precision(T) { return Precision; }

  };

  using PostgresReal = karma::real_generator<float, PostgresPolicy<float, 7> >;
  using PostgresDouble = karma::real_generator<double, PostgresPolicy<double, 15> >;

  static inline std::string double_to_s(const double d)
  {
    char buf[30];
    char *p = buf;
    if (d != 0) {
      karma::generate(p, PostgresDouble(), d);
    } else {
      *p++ = '0';
    }
    *p = '\0';
    return std::string(buf, p);
  }

  static inline std::string float_to_s(const float f)
  {
    char buf[30];
    char *p = buf;
    if (f != 0) {
      karma::generate(p, PostgresReal(), f);
    } else {
      *p++ = '0';
    }
    *p = '\0';
    return std::string(buf, p);
  }
}

namespace Wt {
  namespace Dbo {
    namespace backend {

// do not reconnect in a transaction unless we exceed the lifetime by 120s.
const std::chrono::seconds TRANSACTION_LIFETIME_MARGIN = std::chrono::seconds(120);
    
class PostgresException : public Exception
{
public:
  PostgresException(const std::string& msg)
    : Exception(msg)
  { }

  PostgresException(const std::string& msg, const std::string& code)
    : Exception(msg, code)
  { }
};

class PostgresStatement final : public SqlStatement
{
public:
  PostgresStatement(Postgres& conn, const std::string& sql)
    : conn_(conn),
      sql_(sql)
  {
    convertToNumberedPlaceholders();

    lastId_ = -1;
    row_ = affectedRows_ = 0;
    result_ = nullptr;

    paramValues_ = nullptr;
    paramTypes_ = paramLengths_ = paramFormats_ = nullptr;
 
    snprintf(name_, 64, "SQL%p%08X", (void*)this, rand());

    DEBUG(std::cerr << this << " for: " << sql_ << std::endl);

    state_ = Done;
  }

  virtual ~PostgresStatement()
  {
    if (result_)
      PQclear(result_);
    delete[] paramValues_;
    delete[] paramTypes_;
  }

  virtual void reset() override
  {
    params_.clear();

    state_ = Done;
  }

  void rebuild()
  {
    if (result_) {
      PQclear(result_);
      result_ = 0;
      delete[] paramValues_;
      paramValues_ = 0;
      delete[] paramTypes_;
      paramTypes_ = paramLengths_ = paramFormats_ = 0;
    }
  }

  virtual void bind(int column, const std::string& value) override
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, value);
  }

  virtual void bind(int column, short value) override
  {
    bind(column, static_cast<int>(value));
  }

  virtual void bind(int column, int value) override
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, std::to_string(value));
  }

  virtual void bind(int column, long long value) override
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, std::to_string(value));
  }

  virtual void bind(int column, float value) override
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, float_to_s(value));
  }

  virtual void bind(int column, double value) override
  {
    DEBUG(std::cerr << this << " bind " << column << " " << value << std::endl);

    setValue(column, double_to_s(value));
  }

  virtual void bind(int column, const std::chrono::duration<int, std::milli> & value) override
  {
    auto absValue = value < std::chrono::milliseconds::zero() ? -value : value;
    auto hours = date::floor<std::chrono::hours>(absValue);
    auto minutes = date::floor<std::chrono::minutes>(absValue) - hours;
    auto seconds = date::floor<std::chrono::seconds>(absValue) - hours - minutes;
    auto milliseconds = date::floor<std::chrono::milliseconds>(absValue) - hours - minutes - seconds;

    std::stringstream ss;
    ss.imbue(std::locale::classic());
    if (absValue != value)
      ss << '-';
    ss << std::setfill('0')
       << std::setw(2) << hours.count() << ':'
       << std::setw(2) << minutes.count() << ':'
       << std::setw(2) << seconds.count() << '.'
       << std::setw(3) << milliseconds.count();

    DEBUG(std::cerr << this << " bind " << column << " " << ss.str() << std::endl);

    setValue(column, ss.str());
  }

  virtual void bind(int column, const std::chrono::system_clock::time_point& value,
		    SqlDateTimeType type) override
  {
    std::stringstream ss;
    ss.imbue(std::locale::classic());
    if (type == SqlDateTimeType::Date) {
      auto daypoint = date::floor<date::days>(value);
      auto ymd = date::year_month_day(daypoint);
      ss << (int)ymd.year() << '-' << (unsigned)ymd.month() << '-' << (unsigned)ymd.day();
    } else {
      auto daypoint = date::floor<date::days>(value);
      auto ymd = date::year_month_day(daypoint);
      auto tod = date::make_time(value - daypoint);
      ss << (int)ymd.year() << '-' << (unsigned)ymd.month() << '-' << (unsigned)ymd.day() << ' ';
      ss << std::setfill('0')
         << std::setw(2) << tod.hours().count() << ':'
         << std::setw(2) << tod.minutes().count() << ':'
         << std::setw(2) << tod.seconds().count() << '.'
         << std::setw(3) << date::floor<std::chrono::milliseconds>(tod.subseconds()).count();
      /*
       * Add explicit timezone offset. Postgres will ignore this for a TIMESTAMP
       * column, but will treat the timestamp as UTC in a TIMESTAMP WITH TIME
       * ZONE column -- possibly in a legacy table.
       */
      ss << "+00";
    }
    DEBUG(std::cerr << this << " bind " << column << " " << ss.str() << std::endl);
    setValue(column, ss.str());
  }

  virtual void bind(int column, const std::vector<unsigned char>& value) override
  {
    DEBUG(std::cerr << this << " bind " << column << " (blob, size=" <<
	  value.size() << ")" << std::endl);

    for (int i = (int)params_.size(); i <= column; ++i)
      params_.push_back(Param());

    Param& p = params_[column];
    p.value.resize(value.size());
    if (value.size() > 0)
      std::memcpy(const_cast<char *>(p.value.data()), &(*value.begin()),
	     value.size());
    p.isbinary = true;
    p.isnull = false;

    // FIXME if first null was bound, check here and invalidate the prepared
    // statement if necessary because the type changes
  }

  virtual void bindNull(int column) override
  {
    DEBUG(std::cerr << this << " bind " << column << " null" << std::endl);

    for (int i = (int)params_.size(); i <= column; ++i)
      params_.push_back(Param());

    params_[column].isnull = true;
  }

  virtual void execute() override
  {
    conn_.checkConnection(TRANSACTION_LIFETIME_MARGIN);
    
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
      handleErr(PQresultStatus(result_), result_);
    }

    for (unsigned i = 0; i < params_.size(); ++i) {
      if (params_[i].isnull)
	paramValues_[i] = nullptr;
      else
	if (params_[i].isbinary) {
	  paramValues_[i] = const_cast<char *>(params_[i].value.data());
	  paramLengths_[i] = params_[i].value.length();
	} else
	  paramValues_[i] = const_cast<char *>(params_[i].value.c_str());
    }

    int err = PQsendQueryPrepared(conn_.connection(), name_, params_.size(),
				  paramValues_, paramLengths_, paramFormats_, 0);
    if (err != 1)
      throw PostgresException(PQerrorMessage(conn_.connection()));

    if (conn_.timeout() > std::chrono::microseconds{0}) {
      fd_set rfds;
      FD_ZERO(&rfds);
      FD_SET(PQsocket(conn_.connection()), &rfds);
      struct timeval timeout = toTimeval(conn_.timeout());

      for (;;) {
	int result = select(FD_SETSIZE, &rfds, 0, 0, &timeout);

	if (result == 0) {
	  std::cerr << "Postgres: timeout while executing query" << std::endl;
	  conn_.disconnect();
	  throw PostgresException("Database timeout");
	} else if (result == -1) {
	  if (errno != EINTR) {
	    perror("select");
	    throw PostgresException("Error waiting for result");
	  } else {
	    // EINTR, try again
	  }
	} else {
	  err = PQconsumeInput(conn_.connection());
	  if (err != 1)
	    throw PostgresException(PQerrorMessage(conn_.connection()));

	  if (PQisBusy(conn_.connection()) != 1)
	    break;
	}
      }
    }

    std::string error;

    PQclear(result_);
    result_ = PQgetResult(conn_.connection());
    
    row_ = 0;
    if (PQresultStatus(result_) == PGRES_COMMAND_OK) {
      std::string s = PQcmdTuples(result_);
      if (!s.empty())
	affectedRows_ = std::stoi(s);
      else
	affectedRows_ = 0;
    } else if (PQresultStatus(result_) == PGRES_TUPLES_OK)
      affectedRows_ = PQntuples(result_);

    bool isInsertReturningId = false;
    if (affectedRows_ == 1) {
      const std::string returning = " returning ";
      std::size_t j = sql_.rfind(returning);
      if (j != std::string::npos
	  && sql_.find(' ', j + returning.length()) == std::string::npos)
	isInsertReturningId = true;
    }

    if (isInsertReturningId) {
      state_ = NoFirstRow;
      if (PQntuples(result_) == 1 && PQnfields(result_) == 1) {
	lastId_ = std::stoll(PQgetvalue(result_, 0, 0));
      }
    } else {
      if (PQntuples(result_) == 0) {
	state_ = NoFirstRow;
      } else {
	state_ = FirstRow;
      }
    }

    PGresult *nullResult = PQgetResult(conn_.connection());
    if (nullResult != 0) {
      throw PostgresException("PQgetResult() returned more results");
    }

    handleErr(PQresultStatus(result_), result_);
  }

  virtual long long insertedId() override
  {
    return lastId_;
  }

  virtual int affectedRowCount() override
  {
    return affectedRows_;
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

  virtual bool getResult(int column, std::string *value, int size) override
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = PQgetvalue(result_, row_, column);

    DEBUG(std::cerr << this 
	  << " result string " << column << " " << *value << std::endl);

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
    if (PQgetisnull(result_, row_, column))
      return false;

    const char *v = PQgetvalue(result_, row_, column);

    /*
     * booleans are mapped to int values
     */
    if (*v == 'f')
	*value = 0;
    else if (*v == 't')
	*value = 1;
    else
      *value = std::stoi(v);

    DEBUG(std::cerr << this 
	  << " result int " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, long long *value) override
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = std::stoll(PQgetvalue(result_, row_, column));

    DEBUG(std::cerr << this 
	  << " result long long " << column << " " << *value << std::endl);

    return true;
  }
  
  virtual bool getResult(int column, float *value) override
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = std::stof(PQgetvalue(result_, row_, column));

    DEBUG(std::cerr << this 
	  << " result float " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column, double *value) override
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    *value = std::stod(PQgetvalue(result_, row_, column));

    DEBUG(std::cerr << this 
	  << " result double " << column << " " << *value << std::endl);

    return true;
  }

  virtual bool getResult(int column,
			 std::chrono::system_clock::time_point *value,
			 SqlDateTimeType type) override
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    std::string v = PQgetvalue(result_, row_, column);

    if (type == SqlDateTimeType::Date){
      std::istringstream in(v);
      in.imbue(std::locale::classic());
      in >> date::parse("%F", *value);
    } else {
      /*
       * Handle timezone offset. Postgres will append a timezone offset [+-]dd
       * if a column is defined as TIMESTAMP WITH TIME ZONE -- possibly
       * in a legacy table. If offset is present, subtract it for UTC output.
       */
      int offsetHour = 0;
      if(v.size() >= 3 && std::strchr("+-", v[v.size() - 3])){
          offsetHour = std::stoi(v.substr(v.size() - 3));
          v = v.substr(0, v.size() - 3);
      }
      std::istringstream in(v);
      in.imbue(std::locale::classic());
      in >> date::parse("%F %T", *value);
      *value -= std::chrono::hours{ offsetHour };
    }

    return true;
  }

  virtual bool getResult(int column, std::chrono::duration<int, std::milli> *value) override
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    std::string v = PQgetvalue(result_, row_, column);
    bool neg = false;
    if (!v.empty() && v[0] == '-') {
      neg = true;
      v = v.substr(1);
    }

    std::istringstream in(v);
    in.imbue(std::locale::classic());
    in >> date::parse("%T", *value);
    if (neg)
      *value = -(*value);

    return true;
  }

  virtual bool getResult(int column, std::vector<unsigned char> *value,
			 int size) override
  {
    if (PQgetisnull(result_, row_, column))
      return false;

    const char *escaped = PQgetvalue(result_, row_, column);

    std::size_t vlength;
    unsigned char *v = PQunescapeBytea((unsigned char *)escaped, &vlength);

    value->resize(vlength);
    std::copy(v, v + vlength, value->begin());
    PQfreemem(v);

    DEBUG(std::cerr << this 
	  << " result blob " << column << " (blob, size = " << vlength << ")"
	  << std::endl);

    return true;
  }

  virtual std::string sql() const override {
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

  int paramCount_;
  char **paramValues_;
  int *paramTypes_, *paramLengths_, *paramFormats_;
 
  int lastId_, row_, affectedRows_;

  void handleErr(int err, PGresult *result)
  {
    if (err != PGRES_COMMAND_OK && err != PGRES_TUPLES_OK) {
      std::string code;

      if (result) {
	char *v = PQresultErrorField(result, PG_DIAG_SQLSTATE);
	if (v)
	  code = v;
      }

      throw PostgresException(PQerrorMessage(conn_.connection()), code);
    }
  }

  void setValue(int column, const std::string& value) {
    if (column >= paramCount_)
      throw PostgresException("Binding too much parameters");

    for (int i = (int)params_.size(); i <= column; ++i)
      params_.push_back(Param());

    params_[column].value = value;
    params_[column].isnull = false;
  }

  void convertToNumberedPlaceholders()
  {
    std::stringstream result;

    enum { Statement, SQuote, DQuote } state = Statement;
    int placeholder = 1;

    for (unsigned i = 0; i < sql_.length(); ++i) {
      switch (state) {
      case Statement:
	if (sql_[i] == '\'')
	  state = SQuote;
	else if (sql_[i] == '"')
	  state = DQuote;
	else if (sql_[i] == '?') {
          if (i + 1 != sql_.length() &&
              sql_[i + 1] == '?') {
            // escape question mark with double question mark
            result << '?';
            ++i;
          } else {
	    result << '$' << placeholder++;
          }
	  continue;
	}
	break;
      case SQuote:
	if (sql_[i] == '\'') {
	  if (i + 1 == sql_.length())
	    state = Statement;
	  else if (sql_[i + 1] == '\'') {
	    result << sql_[i];
	    ++i; // skip to next
	  } else
	    state = Statement;
	}
	break;
      case DQuote:
	if (sql_[i] == '"')
	  state = Statement;
	break;
      }
      result << sql_[i];
    }

    paramCount_ = placeholder - 1;
    sql_ = result.str();
  }
};

Postgres::Postgres()
  : conn_(nullptr),
    timeout_(0),
    maximumLifetime_(std::chrono::seconds{-1})
{ }

Postgres::Postgres(const std::string& db)
  : conn_(nullptr),
    timeout_(0),
    maximumLifetime_(std::chrono::seconds{-1})
{
  if (!db.empty())
    connect(db);
}

Postgres::Postgres(const Postgres& other)
  : SqlConnection(other),
    conn_(NULL),
    timeout_(other.timeout_),
    maximumLifetime_(other.maximumLifetime_)
{
  if (!other.connInfo_.empty())
    connect(other.connInfo_);
}

void Postgres::setMaximumLifetime(std::chrono::seconds seconds)
{
  maximumLifetime_ = seconds;    
}

Postgres::~Postgres()
{
  clearStatementCache();
  if (conn_)
    PQfinish(conn_);
}

void Postgres::disconnect()
{
  if (conn_)
    PQfinish(conn_);

  conn_ = 0;

  std::vector<SqlStatement *> statements = getStatements();

  /* Evict also the statements -- the statements themselves can stay,
     only running statements behavior is affected (but we are dealing with
     that while calling disconnect) */
  for (std::size_t i = 0; i < statements.size(); ++i) {
    SqlStatement *s = statements[i];
    PostgresStatement *ps = dynamic_cast<PostgresStatement *>(s);
    ps->rebuild();
  }
}
    
void Postgres::setTimeout(std::chrono::microseconds timeout)
{
  timeout_ = timeout;
}

std::unique_ptr<SqlConnection> Postgres::clone() const
{
  return std::unique_ptr<SqlConnection>(new Postgres(*this));
}

bool Postgres::connect(const std::string& db)
{
  connInfo_ = db;
  conn_ = PQconnectdb(db.c_str());

  if (PQstatus(conn_) != CONNECTION_OK) {
    std::string error = PQerrorMessage(conn_);
    PQfinish(conn_);
    conn_ = nullptr;
    connectTime_ = std::chrono::steady_clock::time_point{};
    throw PostgresException("Could not connect to: " + error);
  } else
    connectTime_ = std::chrono::steady_clock::now();

  PQsetClientEncoding(conn_, "UTF8");

  return true;
}

bool Postgres::reconnect()
{
  std::cerr << this << " reconnecting..." << std::endl;
  
  if (conn_) {
    if (PQstatus(conn_) == CONNECTION_OK) {
      PQfinish(conn_);
    }

    conn_ = 0;
  }

  clearStatementCache();

  if (!connInfo_.empty()) {
    bool result = connect(connInfo_);

    if (result) {
      const std::vector<std::string>& statefulSql = getStatefulSql();
      for (unsigned i = 0; i < statefulSql.size(); ++i)
	executeSql(statefulSql[i]);
    }

    return result;
  } else
    return false;
}

std::unique_ptr<SqlStatement> Postgres::prepareStatement(const std::string& sql)
{
  if (PQstatus(conn_) != CONNECTION_OK)  {
    std::cerr << "Postgres: connection lost to server, trying to reconnect..."
	      << std::endl;
    if (!reconnect()) {
      throw PostgresException("Could not reconnect to server...");
    }
  }

  return std::unique_ptr<SqlStatement>(new PostgresStatement(*this, sql));
}

void Postgres::executeSql(const std::string &sql)
{
  exec(sql, true);
}

/*
 * margin: a grace period beyond the lifetime
 */
void Postgres::checkConnection(std::chrono::seconds margin)
{
  if (maximumLifetime_ > std::chrono::seconds{0} && connectTime_ != std::chrono::steady_clock::time_point{}) {
    auto t = std::chrono::steady_clock::now();
    if (t - connectTime_ > maximumLifetime_ + margin) {
      std::cerr << "Postgres: maximum connection lifetime passed, trying to reconnect..."
		<< std::endl;
      if (!reconnect()) {
	throw PostgresException("Could not reconnect to server...");
      }
    }
  }
}
    
void Postgres::exec(const std::string& sql, bool showQuery)
{
  checkConnection(std::chrono::seconds(0));
  
  if (PQstatus(conn_) != CONNECTION_OK)  {
    std::cerr << "Postgres: connection lost to server, trying to reconnect..."
	      << std::endl;
    if (!reconnect()) {
      throw PostgresException("Could not reconnect to server...");
    }
  }

  if (showQuery && showQueries())
    std::cerr << sql << std::endl;
  
  int err;

  err = PQsendQuery(conn_, sql.c_str());
  if (err != 1)
    throw PostgresException(PQerrorMessage(conn_));

  if (timeout_ > std::chrono::microseconds{0}) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(PQsocket(conn_), &rfds);
    struct timeval timeout = toTimeval(timeout_);

    for (;;) {
      int result = select(FD_SETSIZE, &rfds, 0, 0, &timeout);

      if (result == 0) {
	std::cerr << "Postgres: timeout while executing query" << std::endl;
	disconnect();
	throw PostgresException("Database timeout");
      } else if (result == -1) {
	if (errno != EINTR) {
	  perror("select");
	  throw PostgresException("Error waiting for result");
	} else {
	  // EINTR, try again
	}
      } else {
	err = PQconsumeInput(conn_);
	if (err != 1)
	  throw PostgresException(PQerrorMessage(conn_));

	if (PQisBusy(conn_) != 1)
	  break;
      }
    }
  }

  std::string error;

  for (;;) {
    PGresult *result = PQgetResult(conn_);
    if (result == 0)
      break;

    err = PQresultStatus(result);

    if (err != PGRES_COMMAND_OK && err != PGRES_TUPLES_OK)
      error += PQerrorMessage(conn_);

    PQclear(result);
  }

  if (!error.empty())
    throw PostgresException(error);
}

std::string Postgres::autoincrementType() const
{
  return "bigserial";
}
  
std::string Postgres::autoincrementSql() const
{
  return std::string();
}

std::vector<std::string> 
Postgres::autoincrementCreateSequenceSql(const std::string &table,
					 const std::string &id) const
{
  return std::vector<std::string>();
}

std::vector<std::string> 
Postgres::autoincrementDropSequenceSql(const std::string &table,
				       const std::string &id) const
{
  return std::vector<std::string>();
}

std::string Postgres::autoincrementInsertSuffix(const std::string& id) const
{
  return " returning \"" + id + "\"";
}
  
const char *Postgres::dateTimeType(SqlDateTimeType type) const
{
  switch (type) {
  case SqlDateTimeType::Date:
    return "date";
  case SqlDateTimeType::DateTime:
    return "timestamp";
  case SqlDateTimeType::Time:
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

bool Postgres::supportAlterTable() const
{
  return true;
}

bool Postgres::supportDeferrableFKConstraint() const
{
  return true;
}

bool Postgres::requireSubqueryAlias() const
{
  return true;
}

void Postgres::startTransaction()
{
  exec("start transaction", false);
}

void Postgres::commitTransaction()
{
  exec("commit transaction", false);
}

void Postgres::rollbackTransaction()
{
  exec("rollback transaction", false);
}

    }
  }
}
