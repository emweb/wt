/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Paul Harrison
 */
#include "Wt/Dbo/backend/MySQL"
#include "Wt/Dbo/Exception"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef WT_WIN32
#define snprintf _snprintf
#include <winsock2.h>
#endif
#include <mysql.h>
#include <errmsg.h>

#define BYTEAOID 17

#define DEBUG(x)
//#define DEBUG(x) x

namespace Wt {
  namespace Dbo {
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
    mysql(NULL)
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
      mysql_library_init(0, NULL, NULL);
#endif
    }
  } libraryInitializer;
}

/*!
 * \brief MySQL prepared statement.
 * @todo should the getResult requests all be type checked...
 */
class MySQLStatement : public SqlStatement
{
  public:
    MySQLStatement(MySQL& conn, const std::string& sql)
    : conn_(conn),
      sql_(sql)
    {
      lastId_ = -1;
      row_ = affectedRows_ = 0;
      result_ = 0;
      out_pars_ = 0;
      errors_ = 0;
      lastOutCount_ = 0;

      conn_.checkConnection();
      stmt_ =  mysql_stmt_init(conn_.connection()->mysql);
      mysql_stmt_attr_set(stmt_, STMT_ATTR_UPDATE_MAX_LENGTH, &mysqltrue_);
      if(mysql_stmt_prepare(stmt_, sql_.c_str(), sql_.length()) != 0) {
        throw MySQLException("error creating prepared statement: '"
			      + sql + "': " + mysql_stmt_error(stmt_));
      }

      paramCount_ = mysql_stmt_param_count(stmt_);

      if (paramCount_ > 0) {
          in_pars_ =
	    (MYSQL_BIND *)malloc(sizeof(struct st_mysql_bind) * paramCount_);
          memset(in_pars_, 0, sizeof(struct st_mysql_bind) * paramCount_);
      } else {
        in_pars_ = 0;
      }

      DEBUG(std::cerr <<  " new SQLStatement for: " << sql_ << std::endl);

      state_ = Done;
    }

    virtual ~MySQLStatement()
    {
      DEBUG(std::cerr << "closing prepared stmt " << sql_ << std::endl);
      for(unsigned int i = 0;   i < mysql_stmt_param_count(stmt_) ; ++i)
          freeColumn(i);
      if (in_pars_) free(in_pars_);

      if(out_pars_) free_outpars();

      if (errors_) delete[] errors_;

      if(result_) {
        mysql_free_result(result_);
      }

      mysql_stmt_close(stmt_);
      stmt_ = 0;
    }

    virtual void reset()
    {
      state_ = Done;
      has_truncation_ = false;
    }

    virtual void bind(int column, const std::string& value)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " "
            << value << std::endl);

      unsigned long * len = (unsigned long *)malloc(sizeof(unsigned long));

      char * data;
      //memset(&in_pars_[column], 0, sizeofin_pars_[column]);// Check
      in_pars_[column].buffer_type = MYSQL_TYPE_STRING;

      unsigned long bufLen = value.length() + 1;
      *len = value.length();
      data = (char *)malloc(bufLen);
      memcpy(data, value.c_str(), value.length());
      freeColumn(column);
      in_pars_[column].buffer = data;
      in_pars_[column].buffer_length = bufLen;
      in_pars_[column].length = len;
      in_pars_[column].is_null = 0;
    }

    virtual void bind(int column, short value)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " "
            << value << std::endl);
      short * data = (short *)malloc(sizeof(short));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_SHORT;
      in_pars_[column].buffer = data;
      in_pars_[column].length = 0;
      in_pars_[column].is_null = 0;

    }

    virtual void bind(int column, int value)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " "
            << value << std::endl);
      int * data = (int *)malloc(sizeof(int));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_LONG;
      in_pars_[column].buffer = data;
      in_pars_[column].length = 0;
      in_pars_[column].is_null = 0;
    }

    virtual void bind(int column, long long value)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " "
            << value << std::endl);
      long long * data = (long long *)malloc(sizeof(long long));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_LONGLONG;
      in_pars_[column].buffer = data;
      in_pars_[column].length = 0;
      in_pars_[column].is_null = 0;
    }

    virtual void bind(int column, float value)
    {
      DEBUG(std::cerr << this << " bind " << column << " "
            << value << std::endl);
      float * data = (float *) malloc(sizeof(float));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_FLOAT;
      in_pars_[column].buffer = data;
      in_pars_[column].length = 0;
      in_pars_[column].is_null = 0;
   }

    virtual void bind(int column, double value)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " "
            << value << std::endl);
      double * data = (double *)malloc(sizeof(double));
      *data = value;
      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_DOUBLE;
      in_pars_[column].buffer = data;
      in_pars_[column].length = 0;
      in_pars_[column].is_null = 0;
    }

    virtual void bind(int column, const boost::posix_time::ptime& value,
                      SqlDateTimeType type)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " "
            << boost::posix_time::to_simple_string(value) << std::endl);

      MYSQL_TIME*  ts = (MYSQL_TIME*)malloc(sizeof(struct st_mysql_time));

      boost::posix_time::ptime::time_duration_type  tim = value.time_of_day();
      boost::posix_time::ptime::date_type dd = value.date();
      ts->year = dd.year();
      ts->month = dd.month();
      ts->day = dd.day();
      ts->neg = 0;

      if (type == SqlDate){
        in_pars_[column].buffer_type = MYSQL_TYPE_DATE;
           ts->hour = 0;
           ts->minute = 0;
           ts->second = 0;
           ts->second_part = 0;

      }
      else {
        in_pars_[column].buffer_type = MYSQL_TYPE_DATETIME;
        ts->hour = tim.hours();
        ts->minute = tim.minutes();
        ts->second = tim.seconds();
        if (conn_.getFractionalSecondsPart() > 0)
          ts->second_part = (unsigned long)tim.fractional_seconds();
        else
          ts->second_part = 0;
      }
      freeColumn(column);
      in_pars_[column].buffer = ts;
      in_pars_[column].length = 0;
      in_pars_[column].is_null = 0;

     }

    virtual void bind(int column, const boost::posix_time::time_duration& value)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " "
            << boost::posix_time::to_simple_string(value) << std::endl);

      MYSQL_TIME* ts  = (MYSQL_TIME *)malloc(sizeof(struct st_mysql_time));

      //IMPL note that there is not really a "duration" type in mysql...
      //mapping to a datetime
      in_pars_[column].buffer_type = MYSQL_TYPE_TIME;//MYSQL_TYPE_DATETIME;

      ts->year = 0;
      ts->month = 0;
      ts->day = 0;
      ts->neg = 0;
      ts->hour = value.hours();
      ts->minute = value.minutes();
      ts->second = value.seconds();
      if(conn_.getFractionalSecondsPart() > 0)
        ts->second_part = (unsigned long)value.fractional_seconds();
      else
        ts->second_part = 0;
      freeColumn(column);
      in_pars_[column].buffer = ts;
      in_pars_[column].length = 0;
      in_pars_[column].is_null = 0;
    }

    virtual void bind(int column, const std::vector<unsigned char>& value)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " (blob, size=" <<
            value.size() << ")" << std::endl);

      unsigned long * len = (unsigned long *)malloc(sizeof(unsigned long));

      char * data;
      in_pars_[column].buffer_type = MYSQL_TYPE_BLOB;

      *len = value.size();
      data = (char *)malloc(*len);
      if (value.size() > 0) // must not dereference begin() for empty vectors
	memcpy(data, &(*value.begin()), *len);

      freeColumn(column);
      in_pars_[column].buffer = data;
      in_pars_[column].buffer_length = *len;
      in_pars_[column].length = len;
      in_pars_[column].is_null = 0;

      // FIXME if first null was bound, check here and invalidate the prepared
      // statement if necessary because the type changes
    }

    virtual void bindNull(int column)
    {
      if (column >= paramCount_)
        throw MySQLException(std::string("Try to bind too much?"));

      DEBUG(std::cerr << this << " bind " << column << " null" << std::endl);

      freeColumn(column);
      in_pars_[column].buffer_type = MYSQL_TYPE_NULL;
      in_pars_[column].is_null = const_cast<my_bool*>(&mysqltrue_);
      unsigned long * len = (unsigned long *)malloc(sizeof(unsigned long));
      in_pars_[column].buffer = 0;
      in_pars_[column].buffer_length = 0;
      in_pars_[column].length = len;
    }

    virtual void execute()
    {
      if (conn_.showQueries())
        std::cerr << sql_ << std::endl;

      conn_.checkConnection();
      if(mysql_stmt_bind_param(stmt_, &in_pars_[0]) == 0){
        if (mysql_stmt_execute(stmt_) == 0) {
          if(mysql_stmt_field_count(stmt_) == 0) { // assume not select
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

    virtual long long insertedId()
    {
      return lastId_;
    }

    virtual int affectedRowCount()
    {
      return (int)affectedRows_;
    }

    virtual bool nextRow()
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
              result_ = 0;
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

    virtual bool getResult(int column, std::string *value, int size)
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
			       "column "
			       + boost::lexical_cast<std::string>(column));


	str = static_cast<char*>( out_pars_[column].buffer);
        *value = std::string(str, *out_pars_[column].length);

        DEBUG(std::cerr << this
              << " result string " << column << " " << *value << std::endl);

        return true;
      }
      else
        return false;
    }

    virtual bool getResult(int column, short *value)
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for "
			     "column " 
			     + boost::lexical_cast<std::string>(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

      *value = *static_cast<short*>(out_pars_[column].buffer);

      return true;
    }

    virtual bool getResult(int column, int *value)
    {

      if (*(out_pars_[column].is_null) == 1)
        return false;
      switch (out_pars_[column].buffer_type ){
      case MYSQL_TYPE_TINY:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column "
			       + boost::lexical_cast<std::string>(column));
        *value = *static_cast<char*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_SHORT:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column "
			       + boost::lexical_cast<std::string>(column));
        *value = *static_cast<short*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONG:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column "
			       + boost::lexical_cast<std::string>(column));
        *value = *static_cast<int*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_LONGLONG:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column "
			       + boost::lexical_cast<std::string>(column));
        *value = (int)*static_cast<long long*>(out_pars_[column].buffer);
        break;

      case MYSQL_TYPE_NEWDECIMAL:
	{
	  std::string strValue;
	  if (!getResult(column, &strValue, 0))
	    return false;

	  try{
	    *value = boost::lexical_cast<int>(strValue);
	  } catch( boost::bad_lexical_cast const& ) {
	    try{
	      *value = (int)boost::lexical_cast<double>(strValue);
	    } catch( boost::bad_lexical_cast const& ) {
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

      DEBUG(std::cerr << this
            << " result  int " << column << " " << *value << std::endl);

      return true;
    }

    virtual bool getResult(int column, long long *value)
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
			     + boost::lexical_cast<std::string>(column));

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
			       + boost::lexical_cast<std::string>
			       (out_pars_[column].buffer_type ));
	  break;
      }

      DEBUG(std::cerr << this
            << " result long long " << column << " " << *value << std::endl);

      return true;
    }

    virtual bool getResult(int column, float *value)
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
			     + boost::lexical_cast<std::string>(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

       *value = *static_cast<float*>(out_pars_[column].buffer);

      DEBUG(std::cerr << this
            << " result float " << column << " " << *value << std::endl);

      return true;
    }

    virtual bool getResult(int column, double *value)
    {

      if (*(out_pars_[column].is_null) == 1)
         return false;
      switch (out_pars_[column].buffer_type ){
      case MYSQL_TYPE_DOUBLE:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column "
			       + boost::lexical_cast<std::string>(column));
        *value = *static_cast<double*>(out_pars_[column].buffer);
        break;
      case MYSQL_TYPE_FLOAT:
        if (has_truncation_ && *out_pars_[column].error)
	  throw MySQLException("MySQL: getResult(): truncated result for "
			       "column "
			       + boost::lexical_cast<std::string>(column));
        *value = *static_cast<float*>(out_pars_[column].buffer);
        break;
      case MYSQL_TYPE_NEWDECIMAL:
	{
	  std::string strValue;
	  if (!getResult(column, &strValue, 0))
	    return false;

	  try {
	    *value = boost::lexical_cast<double>(strValue);
	  } catch( boost::bad_lexical_cast const& ) {
	    std::cout << "Error: MYSQL_TYPE_NEWDECIMAL " << strValue
		      << "could not be casted to double" << std::endl;
	    return false;
	  }
	}
        break;
      default:
	return false;
      }

      DEBUG(std::cerr << this
            << " result double " << column << " " << *value << std::endl);

      return true;
    }

    virtual bool getResult(int column, boost::posix_time::ptime *value,
                           SqlDateTimeType type)
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
	  + boost::lexical_cast<std::string>(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

      MYSQL_TIME* ts = static_cast<MYSQL_TIME*>(out_pars_[column].buffer);

      if (type == SqlDate){
        *value = boost::posix_time::ptime(
              boost::gregorian::date(ts->year, ts->month, ts->day),
                                          boost::posix_time::hours(0));
      }
      else
        *value = boost::posix_time::ptime(
            boost::gregorian::date(ts->year, ts->month, ts->day),
            boost::posix_time::time_duration(ts->hour, ts->minute, ts->second)
            + boost::posix_time::microseconds(ts->second_part));

      DEBUG(std::cerr << this
            << " result time " << column << " " << *value << std::endl);

      return true;
    }

    virtual bool getResult(int column, boost::posix_time::time_duration* value)
    {
      if (has_truncation_ && *out_pars_[column].error)
	throw MySQLException("MySQL: getResult(): truncated result for column "
	  + boost::lexical_cast<std::string>(column));

      if (*(out_pars_[column].is_null) == 1)
         return false;

       MYSQL_TIME* ts = static_cast<MYSQL_TIME*>(out_pars_[column].buffer);
       *value = boost::posix_time::time_duration(
             ts->hour, ts->minute, ts->second, ts->second_part);

       DEBUG(std::cerr << this
             << " result time " << column << " " << *value << std::endl);

       return true;
    }

    virtual bool getResult(int column, std::vector<unsigned char> *value,
                           int size)
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
	  + boost::lexical_cast<std::string>(column));


	std::size_t vlength = *(out_pars_[column].length);
        unsigned char *v =
            static_cast<unsigned char*>(out_pars_[column].buffer);

        value->resize(vlength);
        std::copy(v, v + vlength, value->begin());

        DEBUG(std::cerr << this
              << " result blob " << column << " (blob, size = "
              << vlength << ")"<< std::endl);

        return true;
      }
      else
        return false;
    }

    virtual std::string sql() const {
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
    my_bool* errors_;
    unsigned int lastOutCount_;
    // true value to use because mysql specifies that pointer to the boolean
    // is passed in many cases....
    static const my_bool mysqltrue_;
    enum { NoFirstRow, NextRow, Done } state_;
    long long lastId_, row_, affectedRows_;

    void bind_output() {
      if (!out_pars_) {
	out_pars_ =(MYSQL_BIND *)malloc(
	      mysql_num_fields(result_) * sizeof(struct st_mysql_bind));
	memset(out_pars_, 0,
		mysql_num_fields(result_) * sizeof(struct st_mysql_bind));
	errors_ = new my_bool[mysql_num_fields(result_)];
	for(unsigned int i = 0; i < mysql_num_fields(result_); ++i){
	  MYSQL_FIELD* field = mysql_fetch_field_direct(result_, i);
	  out_pars_[i].buffer_type = field->type;
	  out_pars_[i].error = &errors_[i];
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
	    out_pars_[i].buffer = malloc(sizeof(struct st_mysql_time));
	    out_pars_[i].buffer_length = sizeof(struct st_mysql_time);
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
	    std::cerr << "MySQL Backend Programming Error: unknown type "
		      << field->type << std::endl;
	  }
	  out_pars_[i].buffer_type = field->type;
	  out_pars_[i].is_null = (my_bool *)malloc(sizeof(char));
	  out_pars_[i].length =
	      (unsigned long *) malloc(sizeof(unsigned long));
	  out_pars_[i].error = (my_bool *)malloc(sizeof(char));
	}
      }
      mysql_stmt_bind_result(stmt_, out_pars_);
    }

    void freeColumn(int column)
    {
      if(in_pars_[column].length != 0 ) {
        free(in_pars_[column].length);
        in_pars_[column].length = 0;
      }

      if(in_pars_[column].buffer != 0 ) {
        free(in_pars_[column].buffer);
        in_pars_[column].buffer = 0;
      }
    }

    void free_outpars(){

      unsigned int count;
      if(!result_){
          count = lastOutCount_;
      }else
        count = mysql_num_fields(result_);

      for (unsigned int i = 0; i < count; ++i){
       if(out_pars_[i].buffer != 0)free(out_pars_[i].buffer);
       if(out_pars_[i].is_null != 0)free(out_pars_[i].is_null) ;
       if(out_pars_[i].length != 0)free(out_pars_[i].length);
       if(out_pars_[i].error != 0)free(out_pars_[i].error);

      }
      free(out_pars_);
      out_pars_ = 0;
    }

};

const my_bool MySQLStatement::mysqltrue_ = 1;

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
    impl_ = 0;
  }
}

MySQL *MySQL::clone() const
{
  return new MySQL(*this);
}

bool MySQL::connect(const std::string &db,  const std::string &dbuser,
             const std::string &dbpasswd, const std::string &dbhost,
             unsigned int dbport, const std::string &dbsocket)
{
  if (impl_->mysql)
    throw MySQLException("MySQL : Already connected, disconnect first");

  if((impl_->mysql = mysql_init(NULL))){
    if(mysql_real_connect(impl_->mysql, dbhost.c_str(), dbuser.c_str(),
      dbpasswd.empty() ? 0 : dbpasswd.c_str(),
      db.c_str(), dbport,
      dbsocket.c_str(),
      CLIENT_FOUND_ROWS) != impl_->mysql) {
	std::string errtext = mysql_error(impl_->mysql);
	mysql_close(impl_->mysql);
	impl_->mysql = 0;
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
  executeSql("SET storage_engine=INNODB;");
  executeSql("SET NAMES 'utf8';");
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
  if (err_nb == CR_SERVER_GONE_ERROR) {
    clearStatementCache();
    mysql_close(impl_->mysql);
    impl_->mysql = 0;
    try {
      connect(dbname_, dbuser_, dbpasswd_, dbhost_, dbport_, dbsocket_);
    } catch (MySQLException e) {
      throw MySQLException("checkConnection: Error when reconnecting: " + std::string(e.what()));
    }
  }
  if (res != 0) {
    throw MySQLException("checkConnection: MySQL ping error: " + err);
  }
}

SqlStatement *MySQL::prepareStatement(const std::string& sql)
{
  return new MySQLStatement(*this, sql);
}

void MySQL::executeSql(const std::string &sql)
{
  if (showQueries())
    std::cerr << sql << std::endl;

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
  case SqlDate:
    return "date";
  case SqlDateTime:
    return dateType_.c_str();
  case SqlTime:
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
    dateType_ += boost::lexical_cast<std::string>(fractionalSecondsPart_);
    dateType_ += ")";
  } else
    dateType_ = "datetime";


  //IMPL note that there is not really a "duration" type in mysql...
  if (fractionalSecondsPart_ != -1) {
    timeType_ = "time(";
    timeType_ += boost::lexical_cast<std::string>(fractionalSecondsPart_);
    timeType_ += ")";
  } else
    timeType_ = "time";
}

void MySQL::startTransaction()
{
  if (showQueries())
     std::cerr << "start transaction" << std::endl;

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
  my_bool status;
  if (showQueries())
     std::cerr << "commit transaction" << std::endl;

  checkConnection();
  if( (status = mysql_commit(impl_->mysql)) != 0 ){
    std::cerr << "error committing transaction: "
              << mysql_error(impl_->mysql) << std::endl;
    throw MySQLException(std::string("MySQL error committing transaction: ") +
                         mysql_error(impl_->mysql));
  }
  //use any results up
  MYSQL_RES* res = mysql_store_result(impl_->mysql);
  if(res) mysql_free_result(res);
}

void MySQL::rollbackTransaction()
{
  my_bool status;
  if (showQueries())
     std::cerr << "rollback" << std::endl;

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
