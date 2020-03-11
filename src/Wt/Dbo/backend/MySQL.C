/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Paul Harrison
 */
#include "Wt/WConfig.h"

#ifdef WT_WIN32
#define NOMINMAX
 // WinSock2.h warns that it should be included before windows.h
#include <WinSock2.h>
#endif // WT_WIN32

#include "Wt/Dbo/backend/MySQL.h"
#include "Wt/Dbo/Exception.h"
#include "Wt/Dbo/Logger.h"
#include "Wt/Dbo/StringStream.h"

#include "Wt/Date/date.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <cstring>

#ifdef WT_WIN32
#define snprintf _snprintf
#define timegm _mkgmtime
#include <ctime>
#endif
#include <mysql.h>
#include <errmsg.h>

#define BYTEAOID 17

#define DEBUG(x)
//#define DEBUG(x) x

#if defined(LIBMYSQL_VERSION_ID) && LIBMYSQL_VERSION_ID >= 80000
#define WT_MY_BOOL bool
#else
#define WT_MY_BOOL my_bool
#endif

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

LOGGER("Dbo.backend.MySQL");

    namespace backend {

class MySQLException : public Exception
{
  public:
  MySQLException(const std::string& msg)
  : Exception(msg)
  { }
};

class MySQL_impl{
public:
  MYSQL *mysql;

  MySQL_impl():
    mysql(nullptr)
  {}

  MySQL_impl(MYSQL *newmysql):
    mysql(newmysql)
  {}

  MYSQL *ptr()
  {
    return mysql;
  }

};

namespace {
  struct LibraryInitializer {
    LibraryInitializer() {
#if !defined(MYSQL_NO_LIBRARY_INIT)
      mysql_library_init(0, nullptr, nullptr);
#endif
    }
  } libraryInitializer;
}

/*!
 * \brief MySQL prepared statement.
 * @todo should the getResult requests all be type checked...
 */
class MySQLStatement final : public SqlStatement
{
  public:
    MySQLStatement(MySQL& conn, const std::string& sql)
    : conn_(conn),
      sql_(sql)
    {
      lastId_ = -1;
      row_ = affectedRows_ = 0;
      columnCount_ = 0;
      result_ = nullptr;
      out_pars_ = nullptr;
      errors_ = nullptr;
      is_nulls_ = nullptr;
      lastOutCount_ = 0;

      conn_.checkConnection();
      stmt_ =  mysql_stmt_init(conn_.connection()->mysql);
      mysql_stmt_attr_set(stmt_, STMT_ATTR_UPDATE_MAX_LENGTH, &mysqltrue_);
      if(mysql_stmt_prepare(stmt_, sql_.c_str(), sql_.length()) != 0) {
        throw MySQLException("error creating prepared statement: '"
			      + sql + "': " + mysql_stmt_error(stmt_));
      }

      columnCount_ = static_cast<int>(mysql_stmt_field_count(stmt_));

      paramCount_ = mysql_stmt_param_count(stmt_);

      if (paramCount_ > 0) {
          in_pars_ =
	    (MYSQL_BIND *)malloc(sizeof(MYSQL_BIND) * paramCount_);
	  std::memset(in_pars_, 0, sizeof(MYSQL_BIND) * paramCount_);
      } else {
        in_pars_ = nullptr;
      }

      LOG_DEBUG("new SQLStatement for: " << sql_);

      state_ = Done;
    }

    virtual ~MySQLStatement()
    {
      LOG_DEBUG("closing prepared stmt " << sql_);
      for(unsigned int i = 0;   i < mysql_stmt_param_count(stmt_) ; ++i)
          freeColumn(i);
      if (in_pars_) free(in_pars_);

      if(out_pars_) free_outpars();

      if (errors_) delete[] errors_;

      if (is_nulls_) delete[] is_nulls_;

      if(result_) {
        mysql_free_result(result_);
      }

      mysql_stmt_close(stmt_);
      stmt_ = nullptr;
    }

    virtual void reset() override
    {
      state_ = Done;
      has_truncation_ = false;
    }

    virtual void bind(int column, const std::string& value) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      LOG_DEBUG(this << " bind " << column << " " << value);

      unsigned long * len = (unsigned long *)malloc(sizeof(unsigned long));

      char * data;
      //memset(&in_pars_[column], 0, sizeofin_pars_[column]);// Check
      in_pars_[column].buffer_type = MYSQL_TYPE_STRING;

      unsigned long bufLen = value.length() + 1;
      *len = value.length();
      data = (char *)malloc(bufLen);
      std::memcpy(data, value.c_str(), value.length());
      freeColumn(column);
      in_pars_[column].buffer = data;
      in_pars_[column].buffer_length = bufLen;
      in_pars_[column].length = len;
      in_pars_[column].is_null = nullptr;
    }

    virtual void bind(int column, short value) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      LOG_DEBUG(this << " bind " << column << " " << value);
      short * data = (short *)malloc(sizeof(short));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_SHORT;
      in_pars_[column].buffer = data;
      in_pars_[column].length = nullptr;
      in_pars_[column].is_null = nullptr;

    }

    virtual void bind(int column, int value) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      LOG_DEBUG(this << " bind " << column << " " << value);
      int * data = (int *)malloc(sizeof(int));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_LONG;
      in_pars_[column].buffer = data;
      in_pars_[column].length = nullptr;
      in_pars_[column].is_null = nullptr;
    }

    virtual void bind(int column, long long value) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      LOG_DEBUG(this << " bind " << column << " " << value);
      long long * data = (long long *)malloc(sizeof(long long));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_LONGLONG;
      in_pars_[column].buffer = data;
      in_pars_[column].length = nullptr;
      in_pars_[column].is_null = nullptr;
    }

    virtual void bind(int column, float value) override
    {
      LOG_DEBUG(this << " bind " << column << " " << value);
      float * data = (float *) malloc(sizeof(float));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_FLOAT;
      in_pars_[column].buffer = data;
      in_pars_[column].length = nullptr;
      in_pars_[column].is_null = nullptr;
   }

    virtual void bind(int column, double value) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      LOG_DEBUG(this << " bind " << column << " " << value);
      double * data = (double *)malloc(sizeof(double));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_DOUBLE;
      in_pars_[column].buffer = data;
      in_pars_[column].length = nullptr;
      in_pars_[column].is_null = nullptr;
    }

    virtual void bind(int column, const std::chrono::system_clock::time_point& value,
                      SqlDateTimeType type) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      std::time_t t = std::chrono::system_clock::to_time_t(value);
      std::tm *tm = thread_local_gmtime(&t);
      char mbstr[100];
      std::strftime(mbstr, sizeof(mbstr), "%Y-%b-%d %H:%M:%S", tm);
      LOG_DEBUG(this << " bind " << column << " " << mbstr);

      MYSQL_TIME*  ts = (MYSQL_TIME*)malloc(sizeof(MYSQL_TIME));

      ts->year = tm->tm_year + 1900;
      ts->month = tm->tm_mon + 1;
      ts->day = tm->tm_mday;
      ts->neg = 0;

      if (type == SqlDateTimeType::Date){
        in_pars_[column].buffer_type = MYSQL_TYPE_DATE;
	ts->hour = 0;
	ts->minute = 0;
	ts->second = 0;
	ts->second_part = 0;

      } else{
        in_pars_[column].buffer_type = MYSQL_TYPE_DATETIME;
        ts->hour = tm->tm_hour;
        ts->minute = tm->tm_min;
        ts->second = tm->tm_sec;
        if(conn_.getFractionalSecondsPart() > 0){
            std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(value.time_since_epoch());
            ts->second_part = (unsigned long) ms.count()%1000;
        } else
            ts->second_part = 0;
      }
      freeColumn(column);
      in_pars_[column].buffer = ts;
      in_pars_[column].length = nullptr;
      in_pars_[column].is_null = nullptr;

     }

    virtual void bind(int column, const std::chrono::duration<int, std::milli>& value) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      auto absValue = value < std::chrono::duration<int, std::milli>::zero() ? -value : value;
      auto hours = date::floor<std::chrono::hours>(absValue);
      auto minutes = date::floor<std::chrono::minutes>(absValue) - hours;
      auto seconds = date::floor<std::chrono::seconds>(absValue) - hours - minutes;
      auto msecs = date::floor<std::chrono::milliseconds>(absValue) - hours - minutes - seconds;

      LOG_DEBUG(this << " bind " << column << " " << value.count() << "ms");

      MYSQL_TIME* ts  = (MYSQL_TIME *)malloc(sizeof(MYSQL_TIME));

      //IMPL note that there is not really a "duration" type in mysql...
      //mapping to a datetime
      in_pars_[column].buffer_type = MYSQL_TYPE_TIME;//MYSQL_TYPE_DATETIME;

      ts->year = 0;
      ts->month = 0;
      ts->day = 0;
      ts->neg = absValue != value;

      ts->hour = hours.count();
      ts->minute = minutes.count();
      ts->second = seconds.count();

      if (conn_.getFractionalSecondsPart() > 0)
        ts->second_part = std::chrono::microseconds(msecs).count();
      else
        ts->second_part = 0;

      freeColumn(column);
      in_pars_[column].buffer = ts;
      in_pars_[column].length = nullptr;
      in_pars_[column].is_null = nullptr;
    }

    virtual void bind(int column, const std::vector<unsigned char>& value) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      LOG_DEBUG(this << " bind " << column << " (blob, size=" << value.size() << ")");

      unsigned long * len = (unsigned long *)malloc(sizeof(unsigned long));

      char * data;
      in_pars_[column].buffer_type = MYSQL_TYPE_BLOB;

      *len = value.size();
      data = (char *)malloc(*len);
      if (value.size() > 0) // must not dereference begin() for empty vectors
        std::memcpy(data, &(*value.begin()), *len);

      freeColumn(column);
      in_pars_[column].buffer = data;
      in_pars_[column].buffer_length = *len;
      in_pars_[column].length = len;
      in_pars_[column].is_null = nullptr;

      // FIXME if first null was bound, check here and invalidate the prepared
      // statement if necessary because the type changes
    }

    virtual void bindNull(int column) override
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      LOG_DEBUG(this << " bind " << column << " null");

      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_NULL;
      in_pars_[column].is_null = const_cast<WT_MY_BOOL*>(&mysqltrue_);
      unsigned long * len = (unsigned long *)malloc(sizeof(unsigned long));
      in_pars_[column].buffer = nullptr;
      in_pars_[column].buffer_length = 0;
      in_pars_[column].length = len;
    }

    virtual void execute() override
    {
      if (conn_.showQueries())
        LOG_INFO(sql_);

      conn_.checkConnection();
      if(mysql_stmt_bind_param(stmt_, &in_pars_[0]) == 0){
        if (mysql_stmt_execute(stmt_) == 0) {
          if(columnCount_ == 0) { // assume not select
            affectedRows_ = mysql_stmt_affected_rows(stmt_);
           state_ = NoFirstRow;
            if (affectedRows_ == 1 ) {
              lastId_ = mysql_stmt_insert_id(stmt_);
            }
          }
          else {
            row_ = 0;

            if(result_){
              mysql_free_result(result_);
            }

            result_ = mysql_stmt_result_metadata(stmt_);
            mysql_stmt_store_result(stmt_); //possibly not efficient,
            //but suffer from "commands out of sync" errors with the usage
            //patterns that Wt::Dbo uses if not called.
            if( result_ ) {
              if(mysql_num_fields(result_) > 0){
                state_ = NextRow;
              }
              else {
                state_ = NoFirstRow; // not sure how/if this can happen
              }
            }
            else {
              throw MySQLException(std::string("error getting result metadata ")
                                   + mysql_stmt_error(stmt_));
            }
          }
        }
        else {
          throw MySQLException(
                std::string("error executing prepared statement ")+
                mysql_stmt_error(stmt_));
        }
      }
      else {
        throw MySQLException(std::string("error binding parameters")+
                             mysql_stmt_error(stmt_));
      }
    }

    virtual long long insertedId() override
    {
      return lastId_;
    }

    virtual int affectedRowCount() override
    {
      return (int)affectedRows_;
    }

    virtual bool nextRow() override
    {
      int status = 0;
      switch (state_) {
        case NoFirstRow:
          state_ = Done;
          return false;
        case NextRow:
          //bind the output..
          bind_output();
          if ((status = mysql_stmt_fetch(stmt_)) == 0 ||
	      status == MYSQL_DATA_TRUNCATED) {
	    if (status == MYSQL_DATA_TRUNCATED)
	      has_truncation_ = true;
	    else
	      has_truncation_ = false;
            row_++;
            return true;
          } else {
            if(status == MYSQL_NO_DATA ) {
              lastOutCount_ = mysql_num_fields(result_);
              mysql_free_result(result_);
              mysql_stmt_free_result(stmt_);
              result_ = nullptr;
              state_ = Done;
              return false;
            } else {
              throw MySQLException(std::string("MySQL: row fetch failure: ") +
                                   mysql_stmt_error(stmt_));
            }
          }
        break;
      case Done:
        throw MySQLException("MySQL: nextRow(): statement already "
                             "finished");
      }

      return false;
    }

    virtual int columnCount() const override {
      return columnCount_;
    }

    virtual bool getResult(int column, std::string *value, int size) override
    {
      if (*(out_pars_[column].is_null) == 1)
        return false;

      if(*(out_pars_[column].length) > 0){
        char * str;
	if (*(out_pars_[column].length) + 1 > out_pars_[column].buffer_length) {
	  free(out_pars_[column].buffer);
	  out_pars_[column].buffer = malloc(*(out_pars_[column].length)+1);
	  out_pars_[column].buffer_length = *(out_pars_[column].length)+1;
	}
        mysql_stmt_fetch_column(stmt_,  &out_pars_[column], column, 0);

        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column " + std::to_string(column));


	str = static_cast<char*>( out_pars_[column].buffer);
        *value = std::string(str, *out_pars_[column].length);

        LOG_DEBUG(this << " result string " << column << " " << value);

        return true;
      }
      else
        return false;
    }

    virtual bool getResult(int column, short *value) override
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for "
			     "column " + std::to_string(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

      *value = *static_cast<short*>(out_pars_[column].buffer);

      return true;
    }

    virtual bool getResult(int column, int *value) override
    {

      if (*(out_pars_[column].is_null) == 1)
        return false;
      switch (out_pars_[column].buffer_type ){
      case MYSQL_TYPE_TINY:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column " + std::to_string(column));
        *value = *static_cast<char*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_SHORT:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column " + std::to_string(column));
        *value = *static_cast<short*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONG:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column " + std::to_string(column));
        *value = *static_cast<int*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_LONGLONG:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column " + std::to_string(column));
        *value = (int)*static_cast<long long*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_NEWDECIMAL:
	{
	  std::string strValue;
	  if (!getResult(column, &strValue, 0))
	    return false;

	  try {
	    *value = std::stoi(strValue);
	  } catch (std::exception&) {
	    try {
	      *value = (int)std::stod(strValue);
	    } catch (std::exception&) {
	      std::cout << "Error: MYSQL_TYPE_NEWDECIMAL " << strValue
			<< "could not be casted to int" << std::endl;
	      return false;
	    }
	  }
	}
        break;
      default:
	return false;
      }

      LOG_DEBUG(this << " result int " << column << " " << *value);

      return true;
    }

    virtual bool getResult(int column, long long *value) override
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
			     + std::to_string(column));

      if (*(out_pars_[column].is_null) == 1)
        return false;
      switch (out_pars_[column].buffer_type ){
        case MYSQL_TYPE_LONG:

          *value = *static_cast<int*>(out_pars_[column].buffer);
          break;

        case MYSQL_TYPE_LONGLONG:

          *value = *static_cast<long long*>(out_pars_[column].buffer);
          break;

        default:

	  throw MySQLException("MySQL: getResult(long long): unknown type: "
			       + std::to_string(out_pars_[column].buffer_type));
	  break;
      }

      LOG_DEBUG(this << " result long long " << column << " " << *value);

      return true;
    }

    virtual bool getResult(int column, float *value) override
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
			     + std::to_string(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

       *value = *static_cast<float*>(out_pars_[column].buffer);

      LOG_DEBUG(this << " result float " << column << " " << *value);

      return true;
    }

    virtual bool getResult(int column, double *value) override
    {

      if (*(out_pars_[column].is_null) == 1)
         return false;
      switch (out_pars_[column].buffer_type ){
      case MYSQL_TYPE_DOUBLE:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column " + std::to_string(column));
        *value = *static_cast<double*>(out_pars_[column].buffer);
        break;
      case MYSQL_TYPE_FLOAT:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column " + std::to_string(column));
        *value = *static_cast<float*>(out_pars_[column].buffer);
        break;
      case MYSQL_TYPE_NEWDECIMAL:
	{
	  std::string strValue;
	  if (!getResult(column, &strValue, 0))
	    return false;

	  try {
	    *value = std::stod(strValue);
	  } catch(std::exception& e) {
	    std::cout << "Error: MYSQL_TYPE_NEWDECIMAL " << strValue
		      << "could not be casted to double" << std::endl;
	    return false;
	  }
	}
        break;
      default:
	return false;
      }

      LOG_DEBUG(this << " result double " << column << " " << *value);

      return true;
    }

    virtual bool getResult(int column, std::chrono::system_clock::time_point *value,
                           SqlDateTimeType type) override
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
	  + std::to_string(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

      MYSQL_TIME* ts = static_cast<MYSQL_TIME*>(out_pars_[column].buffer);

      if (type == SqlDateTimeType::Date){
        std::tm tm = std::tm();
        tm.tm_year = ts->year - 1900;
        tm.tm_mon = ts->month - 1;
        tm.tm_mday = ts->day;
        std::time_t t = timegm(&tm);
        *value = std::chrono::system_clock::from_time_t(t);
      } else{
	std::tm tm = std::tm();
	tm.tm_year = ts->year - 1900;
	tm.tm_mon = ts->month - 1;
	tm.tm_mday = ts->day;
	tm.tm_hour = ts->hour;
	tm.tm_min = ts->minute;
	tm.tm_sec = ts->second;
	std::time_t t = timegm(&tm);
	*value = std::chrono::system_clock::from_time_t(t);
	*value += std::chrono::milliseconds(ts->second_part);
      }

      std::time_t t = std::chrono::system_clock::to_time_t(*value);
      LOG_DEBUG(this << " result time " << column << " " << std::ctime(&t));

      return true;
    }

    virtual bool getResult(int column, std::chrono::duration<int, std::milli>* value) override
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
	  + std::to_string(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

       MYSQL_TIME* ts = static_cast<MYSQL_TIME*>(out_pars_[column].buffer);
       auto msecs = date::floor<std::chrono::milliseconds>(
         std::chrono::microseconds(ts->second_part));
       auto absValue = std::chrono::hours(ts->hour) + std::chrono::minutes(ts->minute)
                     + std::chrono::seconds(ts->second) + msecs;
       *value = ts->neg ? -absValue : absValue;

       LOG_DEBUG(this << " result time " << column << " " << value->count() << "ms");

       return true;
    }

    virtual bool getResult(int column, std::vector<unsigned char> *value,
                           int size) override
    {
      if (*(out_pars_[column].is_null) == 1)
        return false;

      if(*(out_pars_[column].length) > 0){
	if (*(out_pars_[column].length) > out_pars_[column].buffer_length) {
	  free(out_pars_[column].buffer);
	  out_pars_[column].buffer = malloc(*(out_pars_[column].length));
	  out_pars_[column].buffer_length = *(out_pars_[column].length);
	}
        mysql_stmt_fetch_column(stmt_,  &out_pars_[column], column, 0);

      if (*out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
	  + std::to_string(column));


	std::size_t vlength = *(out_pars_[column].length);
        unsigned char *v =
            static_cast<unsigned char*>(out_pars_[column].buffer);

        value->resize(vlength);
        std::copy(v, v + vlength, value->begin());

        LOG_DEBUG(this << " result blob " << column << " (blob, size = " << static_cast<long long>(vlength) << ")");

        return true;
      }
      else
        return false;
    }

    virtual std::string sql() const override {
      return sql_;
    }

  private:
    MySQL& conn_;
    std::string sql_;
    char name_[64];
    bool has_truncation_;
    MYSQL_RES *result_;
    MYSQL_STMT* stmt_;
    MYSQL_BIND* in_pars_;
    MYSQL_BIND* out_pars_;
    int paramCount_;
    WT_MY_BOOL* errors_;
    WT_MY_BOOL* is_nulls_;
    unsigned int lastOutCount_;
    // true value to use because mysql specifies that pointer to the boolean
    // is passed in many cases....
    static const WT_MY_BOOL mysqltrue_;
    enum { NoFirstRow, NextRow, Done } state_;
    long long lastId_, row_, affectedRows_;
    int columnCount_;

    void bind_output() {
      if (!out_pars_) {
	out_pars_ =(MYSQL_BIND *)malloc(
	      mysql_num_fields(result_) * sizeof(MYSQL_BIND));
    std::memset(out_pars_, 0,
		mysql_num_fields(result_) * sizeof(MYSQL_BIND));
	errors_ = new WT_MY_BOOL[mysql_num_fields(result_)];
        is_nulls_ = new WT_MY_BOOL[mysql_num_fields(result_)];
	for(unsigned int i = 0; i < mysql_num_fields(result_); ++i){
	  MYSQL_FIELD* field = mysql_fetch_field_direct(result_, i);
	  out_pars_[i].buffer_type = field->type;
	  out_pars_[i].error = &errors_[i];
	  out_pars_[i].is_null = &is_nulls_[i];
	  switch(field->type){
	  case MYSQL_TYPE_TINY:
	    out_pars_[i].buffer = malloc(1);
	    out_pars_[i].buffer_length = 1;
	    break;

	  case MYSQL_TYPE_SHORT:
	    out_pars_[i].buffer = malloc(sizeof(short));
	    out_pars_[i].buffer_length = sizeof(short);
	    break;

	  case MYSQL_TYPE_LONG:
	    out_pars_[i].buffer = malloc(sizeof(int));
	    out_pars_[i].buffer_length = sizeof(int);
	    break;
	  case MYSQL_TYPE_FLOAT:
	    out_pars_[i].buffer = malloc(sizeof(float));
	    out_pars_[i].buffer_length = sizeof(float);
	    break;

	  case MYSQL_TYPE_LONGLONG:
	    out_pars_[i].buffer = malloc(sizeof(long long));
	    out_pars_[i].buffer_length = sizeof(long long);
	    break;
	  case MYSQL_TYPE_DOUBLE:
	    out_pars_[i].buffer = malloc(sizeof(double));
	    out_pars_[i].buffer_length = sizeof(double);
	    break;

	  case MYSQL_TYPE_TIME:
	  case MYSQL_TYPE_DATE:
	  case MYSQL_TYPE_DATETIME:
	  case MYSQL_TYPE_TIMESTAMP:
	    out_pars_[i].buffer = malloc(sizeof(MYSQL_TIME));
	    out_pars_[i].buffer_length = sizeof(MYSQL_TIME);
	    break;

	  case MYSQL_TYPE_NEWDECIMAL: // newdecimal is stored as string.
	  case MYSQL_TYPE_STRING:
	  case MYSQL_TYPE_VAR_STRING:
	  case MYSQL_TYPE_BLOB:
	    out_pars_[i].buffer = malloc(256);
	    out_pars_[i].buffer_length = 256; // Reserve 256 bytes, if the content is longer, it will be reallocated later
	    //http://dev.mysql.com/doc/refman/5.0/en/mysql-stmt-fetch.html
	    break;
	  default:
            LOG_ERROR("MySQL Backend Programming Error: unknown type " << field->type);
	  }
	  out_pars_[i].buffer_type = field->type;
	  out_pars_[i].length = (unsigned long *) malloc(sizeof(unsigned long));
	}
      }
      for (unsigned int i = 0; i < mysql_num_fields(result_); ++i) {
        // Clear error for MariaDB Connector/C (see issue #6407)
        *out_pars_[i].error = 0;
      }
      mysql_stmt_bind_result(stmt_, out_pars_);
    }

    void freeColumn(int column)
    {
      if(in_pars_[column].length != nullptr ) {
        free(in_pars_[column].length);
        in_pars_[column].length = nullptr;
      }

      if(in_pars_[column].buffer != nullptr ) {
        free(in_pars_[column].buffer);
        in_pars_[column].buffer = nullptr;
      }
    }

    void free_outpars(){

      unsigned int count;
      if(!result_){
          count = lastOutCount_;
      }else
        count = mysql_num_fields(result_);

      for (unsigned int i = 0; i < count; ++i){
       if(out_pars_[i].buffer != nullptr)free(out_pars_[i].buffer);
       if(out_pars_[i].length != nullptr)free(out_pars_[i].length);

      }
      free(out_pars_);
      out_pars_ = nullptr;
    }

};

const WT_MY_BOOL MySQLStatement::mysqltrue_ = 1;

MySQL::MySQL(const std::string &db,  const std::string &dbuser,
             const std::string &dbpasswd, const std::string dbhost,
             unsigned int dbport, const std::string &dbsocket,
             int fractionalSecondsPart)
: impl_(new MySQL_impl())
{
  setFractionalSecondsPart(fractionalSecondsPart);

  try {
    connect(db, dbuser, dbpasswd, dbhost, dbport, dbsocket);
  } catch (...) {
    delete impl_;
    throw;
  }
}

MySQL::MySQL(const MySQL& other)
  : SqlConnection(other),
    impl_(new MySQL_impl())
{
  setFractionalSecondsPart(other.fractionalSecondsPart_);

  try {
    if (!other.dbname_.empty())
      connect(other.dbname_, other.dbuser_, other.dbpasswd_, other.dbhost_, other.dbport_, other.dbsocket_);
  } catch (...) {
    delete impl_;
    throw;
  }
}

MySQL::~MySQL()
{
  clearStatementCache();
  if (impl_ && impl_->mysql)
    mysql_close(impl_->mysql);

  if (impl_){
    delete impl_;
    impl_ = nullptr;
  }
}

std::unique_ptr<SqlConnection> MySQL::clone() const
{
  return std::unique_ptr<SqlConnection>(new MySQL(*this));
}

bool MySQL::connect(const std::string &db,  const std::string &dbuser,
             const std::string &dbpasswd, const std::string &dbhost,
             unsigned int dbport, const std::string &dbsocket)
{
  if (impl_->mysql)
    throw MySQLException("MySQL : Already connected, disconnect first");

  if((impl_->mysql = mysql_init(nullptr))){
    if(mysql_real_connect(impl_->mysql, dbhost.c_str(), dbuser.c_str(),
      dbpasswd.empty() ? nullptr : dbpasswd.c_str(),
      db.c_str(), dbport,
      dbsocket.c_str(),
      CLIENT_FOUND_ROWS) != impl_->mysql) {
	std::string errtext = mysql_error(impl_->mysql);
	mysql_close(impl_->mysql);
	impl_->mysql = nullptr;
	throw MySQLException(
	  std::string("MySQL : Failed to connect to database server: ")
	  + errtext);
    } else {
      // success!
      dbname_ = db;
      dbuser_ = dbuser;
      dbpasswd_ = dbpasswd;
      dbhost_ = dbhost;
      dbsocket_ = dbsocket;
      dbport_ = dbport;
    }
  } else {
    throw MySQLException(
      std::string("MySQL : Failed to initialize database: ") + dbname_);
  }

  init();

  return true;
}

void MySQL::init()
{
  executeSql("SET sql_mode='ANSI_QUOTES,REAL_AS_FLOAT'");
  executeSql("SET default_storage_engine=INNODB;");
  executeSql("SET NAMES 'utf8mb4';");

  const std::vector<std::string>& statefulSql = getStatefulSql();
  for (std::size_t i = 0; i < statefulSql.size(); ++i)
    executeSql(statefulSql[i]);
}

void MySQL::checkConnection()
{
  /*
   * We used to rely on MySQL's ability to automatically reconnect,
   * but it's not possible to reliably detect whether a reconnect has
   * occurred, so we'll just check the connection, and manually reconnect
   * if it's lost.
   *
   * This function must be called *before* any interaction with the mysql
   * server.
   */
  int err_nb = CR_SERVER_GONE_ERROR;
  int res = 0;
  if (impl_->mysql) {
    err_nb = 0;
    res = mysql_ping(impl_->mysql);
  }
  std::string err;
  if (res != 0) {
    err_nb = mysql_errno(impl_->mysql);
    err = std::string(mysql_error(impl_->mysql));
  }
  if (err_nb == CR_SERVER_GONE_ERROR ||
      err_nb == CR_SERVER_LOST) {
    clearStatementCache();
    mysql_close(impl_->mysql);
    impl_->mysql = nullptr;
    try {
      connect(dbname_, dbuser_, dbpasswd_, dbhost_, dbport_, dbsocket_);
      return;
    } catch (MySQLException e) {
      throw MySQLException("checkConnection: Error when reconnecting: " + std::string(e.what()));
    }
  }
  if (res != 0) {
    throw MySQLException("checkConnection: MySQL ping error: " + err);
  }
}

std::unique_ptr<SqlStatement> MySQL::prepareStatement(const std::string& sql)
{
  return std::unique_ptr<SqlStatement>(new MySQLStatement(*this, sql));
}

void MySQL::executeSql(const std::string &sql)
{
  if (showQueries())
    LOG_INFO(sql);

  checkConnection();
  if( mysql_query(impl_->mysql, sql.c_str()) != 0 ){
    throw MySQLException("MySQL error performing query: '" +
			 sql + "': " + mysql_error(impl_->mysql));
  }
  //use any results up
  MYSQL_RES* res = mysql_store_result(impl_->mysql);
  if(res) mysql_free_result(res);

}

std::string MySQL::autoincrementType() const
{
  return "BIGINT";
}

std::string MySQL::autoincrementSql() const
{
  return "AUTO_INCREMENT";
}

std::string MySQL::autoincrementInsertSuffix(const std::string &id) const
{
  return std::string();
}

std::vector<std::string>
MySQL::autoincrementCreateSequenceSql(const std::string &table,
                                      const std::string &id) const{
  return std::vector<std::string>();
}

std::vector<std::string>
MySQL::autoincrementDropSequenceSql(const std::string &table,
                             const std::string &id) const{

  return std::vector<std::string>();
}

const char *MySQL::dateTimeType(SqlDateTimeType type) const
{
  switch (type) {
  case SqlDateTimeType::Date:
    return "date";
  case SqlDateTimeType::DateTime:
    return dateType_.c_str();
  case SqlDateTimeType::Time:
    return timeType_.c_str();
  }
  std::stringstream ss;
  ss << __FILE__ << ":" << __LINE__ << ": implementation error";
  throw MySQLException(ss.str());
}

const char *MySQL::blobType() const
{
  return "blob";
}

bool MySQL::supportAlterTable() const
{
  return true;
}

const char *MySQL::alterTableConstraintString() const
{
  return "foreign key";
}

int MySQL::getFractionalSecondsPart() const
{
  return fractionalSecondsPart_;
}

void MySQL::setFractionalSecondsPart(int fractionalSecondsPart)
{
  fractionalSecondsPart_ = fractionalSecondsPart;

  if (fractionalSecondsPart_ != -1) {
    dateType_ = "datetime(";
    dateType_ += std::to_string(fractionalSecondsPart_);
    dateType_ += ")";
  } else
    dateType_ = "datetime";


  //IMPL note that there is not really a "duration" type in mysql...
  if (fractionalSecondsPart_ != -1) {
    timeType_ = "time(";
    timeType_ += std::to_string(fractionalSecondsPart_);
    timeType_ += ")";
  } else
    timeType_ = "time";
}

void MySQL::startTransaction()
{
  if (showQueries())
     LOG_INFO("start transaction");

  checkConnection();
  if( mysql_query(impl_->mysql, "start transaction") != 0 ){
    throw MySQLException(std::string("MySQL error starting transaction: ") +
                         mysql_error(impl_->mysql));
  }
  //use any results up
  MYSQL_RES* res = mysql_store_result(impl_->mysql);
  if(res) mysql_free_result(res);
}

void MySQL::commitTransaction()
{
  WT_MY_BOOL status;
  if (showQueries())
     LOG_INFO("commit transaction");

  checkConnection();
  if( (status = mysql_commit(impl_->mysql)) != 0 ){
    LOG_ERROR("error committing transaction: " << mysql_error(impl_->mysql));
    throw MySQLException(std::string("MySQL error committing transaction: ") +
                         mysql_error(impl_->mysql));
  }
  //use any results up
  MYSQL_RES* res = mysql_store_result(impl_->mysql);
  if(res) mysql_free_result(res);
}

void MySQL::rollbackTransaction()
{
  WT_MY_BOOL status;
  if (showQueries())
     LOG_INFO("rollback");

  checkConnection();
  if((status =  mysql_rollback(impl_->mysql)) != 0 ){
    throw MySQLException(std::string("MySQL error rolling back transaction: ") +
                         mysql_error(impl_->mysql));
  }
  //use any results up
  MYSQL_RES* res = mysql_store_result(impl_->mysql);
  if(res) mysql_free_result(res);
}

}
}
}
