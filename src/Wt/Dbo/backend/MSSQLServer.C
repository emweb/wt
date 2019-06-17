/*
 * Copyright (C) 2017 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Dbo/backend/MSSQLServer.h"

#include "Wt/Dbo/Exception.h"

#include "Wt/Date/date.h"

#ifdef WT_WIN32
#define NOMINMAX
#include <windows.h>
#endif // WT_WIN32
#include <sql.h>
#include <sqlext.h>
#ifndef WT_WIN32
#include <sqlucode.h>

#include <codecvt>
#include <string>
#endif // WT_WIN32

#include <cstring>
#include <iostream>
#include <vector>

// define from sqlncli.h
// See: https://docs.microsoft.com/en-us/sql/relational-databases/native-client/features/using-multiple-active-result-sets-mars
#ifndef SQL_COPT_SS_MARS_ENABLED
#define SQL_COPT_SS_MARS_ENABLED 1224
#define SQL_MARS_ENABLED_YES 1L
#endif

namespace Wt {
  namespace Dbo {
    namespace backend {

class MSSQLServerException : public Exception
{
public:
  MSSQLServerException(const std::string& msg,
                       const std::string &sqlState = std::string())
   : Exception(msg, sqlState)
  { }
};

namespace {

static void handleErr(SQLSMALLINT handleType, SQLHANDLE handle, SQLRETURN rc)
{
  if (SQL_SUCCEEDED(rc))
    return;

  SQLCHAR sqlState[6];
  SQLINTEGER nativeErr = 0;
  SQLSMALLINT msgLength = 0;
  SQLCHAR buf[SQL_MAX_MESSAGE_LENGTH];
  SQLGetDiagRec(
    handleType,
    handle,
    1,
    sqlState,
    &nativeErr,
    buf,
    sizeof(buf),
    &msgLength
  );
  throw MSSQLServerException(
          std::string((const char*)buf, msgLength),
          std::string((const char*)sqlState, 5));
}

#ifndef WT_WIN32
std::u16string toUTF16(const std::string &str)
{
  return
    std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str.data());
}
#endif // WT_WIN32

#ifdef WT_WIN32
typedef std::basic_string<SQLWCHAR> ConnectionStringType;
#else // WT_WIN32
typedef std::u16string ConnectionStringType;
#endif // WT_WIN32

}

struct MSSQLServer::Impl {
  Impl(const ConnectionStringType &str)
    : env(NULL),
      dbc(NULL),
      stmt(NULL),
      connectionString(str)
  {
    resultBuffer.buf = (char*)malloc(256);
    resultBuffer.size = 256;
  }

  Impl(const Impl &other)
    : env(NULL),
      dbc(NULL),
      stmt(NULL),
      connectionString(other.connectionString)
  {
    resultBuffer.buf = (char*)malloc(256);
    resultBuffer.size = 256;
  }

  ~Impl()
  {
    free(resultBuffer.buf);
    if (stmt)
      SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (dbc) {
      SQLDisconnect(dbc);
      SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    if (env)
      SQLFreeHandle(SQL_HANDLE_ENV, env);
  }

  void connect()
  {
    // Create SQL env handle
    SQLRETURN res = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (res == SQL_ERROR)
      throw MSSQLServerException("Failed to allocate ODBC environment handle!");
    // Set ODBC version to 3
    res = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    handleErr(SQL_HANDLE_ENV, env, res);
    // Create SQL connection handle
    res = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    handleErr(SQL_HANDLE_ENV, env, res);
    // Turn off autocommit
    res = SQLSetConnectAttrW(dbc, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);
    handleErr(SQL_HANDLE_DBC, dbc, res);
    // Turn on MARS (Multiple Active Result Sets)
    res = SQLSetConnectAttrW(dbc, SQL_COPT_SS_MARS_ENABLED, (SQLPOINTER)SQL_MARS_ENABLED_YES, SQL_IS_UINTEGER);
    handleErr(SQL_HANDLE_DBC, dbc, res);
    // Connect
    res = SQLDriverConnectW(
      dbc,
      NULL,
      (SQLWCHAR*)&(connectionString)[0],
      connectionString.size(),
      NULL,
      0,
      NULL,
      SQL_DRIVER_NOPROMPT);
    handleErr(SQL_HANDLE_DBC, dbc, res);
  }

  SQLHENV env; // Environment handle
  SQLHDBC dbc; // Connection handle
  SQLHSTMT stmt; // Statement handle for executeSql
  ConnectionStringType connectionString;
  struct ResultBuffer {
    char *buf;
    std::size_t size;
  } resultBuffer;
};

class MSSQLServerStatement : public SqlStatement {
public:
  MSSQLServerStatement(MSSQLServer &conn, const std::string &sql)
    : paramValues_(NULL),
      parameterCount_(0),
      resultColCount_(0),
      stmt_(NULL),
      conn_(conn),
      sql_(sql),
      affectedRows_(0),
      lastId_(-1)
  {
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_STMT, conn.impl_->dbc, &stmt_);
    handleErr(SQL_HANDLE_DBC, conn.impl_->dbc, rc);
#ifdef WT_WIN32
    if (sql.empty()) {
      // Empty query, should be an error, but we'll leave the reporting
      // of that error to ODBC
      SQLWCHAR wstr[] = L"";
      rc = SQLPrepareW(stmt_, wstr, 0);
    } else {
      int wstrlen = MultiByteToWideChar(CP_UTF8, 0, &sql[0], sql.size(), NULL, 0);
      assert(wstrlen != 0);
      SQLWCHAR *wstr = new SQLWCHAR[wstrlen + 1];
      wstrlen = MultiByteToWideChar(CP_UTF8, 0, &sql[0], sql.size(), wstr, wstrlen);
      assert(wstrlen != 0);
      wstr[wstrlen] = 0;
      rc = SQLPrepareW(stmt_, wstr, wstrlen);
      delete[] wstr;
    }
#else // WT_WIN32
    std::u16string wstr = toUTF16(sql);
    rc = SQLPrepareW(stmt_, (SQLWCHAR*)&wstr[0], wstr.size());
#endif // WT_WIN32
    handleErr(SQL_HANDLE_STMT, stmt_, rc);
    rc = SQLNumParams(stmt_, &parameterCount_);
    handleErr(SQL_HANDLE_STMT, stmt_, rc);
    if (parameterCount_ > 0) {
      paramValues_ = new Value[parameterCount_];
      std::memset(paramValues_, 0, parameterCount_ * sizeof(Value));
    }
  }

  virtual ~MSSQLServerStatement()
  {
    if (stmt_) {
      SQLFreeStmt(stmt_, SQL_CLOSE);
      SQLFreeHandle(SQL_HANDLE_STMT, stmt_);
    }
    for (SQLSMALLINT i = 0; i < parameterCount_; ++i)
      paramValues_[i].clear();
    delete[] paramValues_;
  }


  virtual void reset()
  {
    SQLFreeStmt(stmt_, SQL_CLOSE);
  }

  virtual void bind(int column, const std::string &value)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    bool newPtr = true;
    if (value.empty()) {
      newPtr = createOrResizeBuffer(v, 0);
      v.lengthOrInd = 0;
    } else {
#ifdef WT_WIN32
      // Convert value from UTF-8 to WCHAR (UTF-16)
      // Measure length required
      int bufsize = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), value.size(), NULL, 0);
      assert(bufsize != 0);
      newPtr = createOrResizeBuffer(v, (bufsize + 1) * sizeof(WCHAR));
      bufsize = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), value.size(), (WCHAR*)v.v.buf.p, bufsize);
      assert(bufsize != 0);
      ((WCHAR*)v.v.buf.p)[bufsize] = 0;
      v.lengthOrInd = bufsize * sizeof(WCHAR);
#else // WT_WIN32
      newPtr = createOrResizeBuffer(v, value.size() + 1);
      memcpy(v.v.buf.p, value.c_str(), value.size() + 1);
      v.lengthOrInd = value.size();
#endif // WT_WIN32
    }
    if (newPtr || v.type != SQL_C_WCHAR) {
      v.type = SQL_C_WCHAR;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
#ifdef WT_WIN32
        /*ValueType: */SQL_C_WCHAR,
#else // WT_WIN32
        /*ValueType: */SQL_C_CHAR, // UTF-8 to UTF-16 done by SQL Server driver
#endif // WT_WIN32
        /*ParameterType: */SQL_WVARCHAR,
        /*ColumnSize: */0, // Ignored
        /*DecimalDigits: */0, // ignored
        /*ParameterValuePtr: */v.v.buf.p,
        /*BufferLength: */v.v.buf.size,
        /*StrLen_or_IndPtr: */&v.lengthOrInd
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void bind(int column, short value)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    if (v.type == SQL_C_SSHORT) {
      v.v.s = value;
      v.lengthOrInd = 0;
    } else {
      v.clear();
      v.v.s = value;
      v.type = SQL_C_SSHORT;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
        /*ValueType: */SQL_C_SSHORT,
        /*ParameterType: */SQL_INTEGER,
        /*ColumnSize: */0, // ignored
        /*DecimalDigits: */0, // ignored
        /*ParameterValuePtr: */&v.v.s,
        /*BufferLength: */0,
        /*StrLen_or_IndPtr: */&v.lengthOrInd
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void bind(int column, int value)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    if (v.type == SQL_C_SLONG) {
      v.v.i = value;
      v.lengthOrInd = 0;
    } else {
      v.clear();
      v.v.i = value;
      v.type = SQL_C_SLONG;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
        /*ValueType: */SQL_C_SLONG,
        /*ParameterType: */SQL_INTEGER,
        /*Columnsize: */0, // seems ignored?
        /*DecimalDigits: */0, // ignored
        /*ParameterValuePtr: */&v.v.i,
        /*BufferLength: */0,
        /*StrLen_or_IndPtr: */&v.lengthOrInd
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void bind(int column, long long value)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    if (v.type == SQL_C_SBIGINT) {
      v.v.ll = value;
      v.lengthOrInd = 0;
    } else {
      v.clear();
      v.v.ll = value;
      v.type = SQL_C_SBIGINT;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
        /*ValueType: */SQL_C_SBIGINT,
        /*ParameterType: */SQL_BIGINT,
        /*Columnsize: */0, // ignored
        /*DecimalDigits: */0, // ignored
        /*ParameterValuePtr: */&v.v.ll,
        /*BufferLength: */0,
        /*StrLen_or_IndPtr: */&v.lengthOrInd
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void bind(int column, float value)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    if (v.type == SQL_C_FLOAT) {
      v.v.f = value;
      v.lengthOrInd = 0;
    } else {
      v.clear();
      v.v.f = value;
      v.type = SQL_C_FLOAT;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
        /*ValueType: */SQL_C_FLOAT,
        /*ParameterType: */SQL_REAL,
        /*Columnsize: */0, // seems ignored?
        /*DecimalDigits: */0, // ignored
        /*ParameterValuePtr: */&v.v.f,
        /*BufferLength: */0,
        /*StrLen_or_IndPtr: */&v.lengthOrInd
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void bind(int column, double value)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    if (v.type == SQL_C_DOUBLE) {
      v.v.d = value;
      v.lengthOrInd = 0;
    } else {
      v.clear();
      v.v.d = value;
      v.type = SQL_C_DOUBLE;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
        /*ValueType: */SQL_C_DOUBLE,
        /*ParameterType: */SQL_DOUBLE,
        /*Columnsize: */0, // seems ignored?
        /*DecimalDigits: */0, // ignored
        /*ParameterValuePtr: */&v.v.d,
        /*BufferLength: */0,
        /*StrLen_or_IndPtr: */&v.lengthOrInd
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void bind(
    int column,
    const std::chrono::system_clock::time_point& value,
    SqlDateTimeType type)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    if (type == SqlDateTimeType::Date) {
      if (v.type != SQL_C_TYPE_DATE)
        v.clear();
      v.lengthOrInd = 0;
      SQL_DATE_STRUCT &date = paramValues_[column].v.date;
      auto daypoint = date::floor<date::days>(value);
      auto ymd = date::year_month_day(daypoint);
      date.year = (int)ymd.year();
      date.month = (unsigned)ymd.month();
      date.day = (unsigned)ymd.day();
      if (v.type != SQL_C_TYPE_DATE) {
        v.type = SQL_C_TYPE_DATE;
        SQLRETURN rc = SQLBindParameter(
          /*StatementHandle: */stmt_,
          /*ParameterNumber: */column + 1,
          /*InputOutputType: */SQL_PARAM_INPUT,
          /*ValueType: */SQL_C_TYPE_DATE,
          /*ParameterType: */SQL_TYPE_DATE,
          /*ColumnSize: */0,
          /*DecimalDigits: */0,
          /*ParameterValuePtr: */&date,
          /*BufferLength: */0,
          /*StrLen_or_IndPtr: */&v.lengthOrInd
        );
        handleErr(SQL_HANDLE_STMT, stmt_, rc);
      }
    } else {
      if (v.type != SQL_C_TYPE_TIMESTAMP)
        v.clear();
      v.lengthOrInd = 0;
      SQL_TIMESTAMP_STRUCT &ts = paramValues_[column].v.timestamp;
      auto daypoint = date::floor<date::days>(value);
      auto ymd = date::year_month_day(daypoint);
      auto tod = date::make_time(value - daypoint);
      ts.year = (int)ymd.year();
      ts.month = (unsigned)ymd.month();
      ts.day = (unsigned)ymd.day();
      ts.hour = tod.hours().count();
      ts.minute = tod.minutes().count();
      ts.second = tod.seconds().count();
      ts.fraction = std::chrono::nanoseconds(std::chrono::duration_cast<
        std::chrono::duration<long long, std::ratio_multiply<std::ratio<100>, std::nano>>>(tod.subseconds())).count();
      if (v.type != SQL_C_TYPE_TIMESTAMP) {
        v.type = SQL_C_TYPE_TIMESTAMP;
        SQLRETURN rc = SQLBindParameter(
          /*StatementHandle: */stmt_,
          /*ParameterNumber: */column + 1,
          /*InputOutputType: */SQL_PARAM_INPUT,
          /*ValueType: */SQL_C_TYPE_TIMESTAMP,
          /*ParameterType: */SQL_TYPE_TIMESTAMP,
          /*ColumnSize: */0,
          /*DecimalDigits: */7, // SQL Server limit: max 7 decimal digits
          /*ParameterValuePtr: */&ts,
          /*BufferLength: */0,
          /*StrLen_or_IndPtr: */&v.lengthOrInd
        );
        handleErr(SQL_HANDLE_STMT, stmt_, rc);
      }
    }
  }

  virtual void bind(
    int column,
    const std::chrono::duration<int, std::milli>& value)
  {
    long long msec = value.count();
    bind(column, msec);
  }

  virtual void bind(
    int column,
    const std::vector<unsigned char>& value)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    bool newPtr = createOrResizeBuffer(v, value.size());
    v.lengthOrInd = value.size();
    memcpy(v.v.buf.p, value.data(), value.size());
    if (newPtr || v.type != SQL_C_BINARY) {
      v.type = SQL_C_BINARY;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
        /*ValueType: */SQL_C_BINARY,
        /*ParameterType: */SQL_VARBINARY,
        /*ColumnSize: */0, // ignored
        /*DecimalDigits: */0, // ignored
        /*ParameterValuePtr: */v.v.buf.p,
        /*BufferLength: */v.v.buf.size,
        /*StrLen_or_IndPtr: */&v.lengthOrInd
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void bindNull(int column)
  {
    checkColumnIndex(column);
    Value &v = paramValues_[column];
    if (v.type != 0)
      v.lengthOrInd = SQL_NULL_DATA;
    else {
      v.clear();
      v.type = SQL_C_CHAR;
      v.v.buf.p = 0;
      v.v.buf.size = SQL_NULL_DATA;
      SQLRETURN rc = SQLBindParameter(
        /*StatementHandle: */stmt_,
        /*ParameterNumber: */column + 1,
        /*InputOutputType: */SQL_PARAM_INPUT,
        /*ValueType: */SQL_C_CHAR,
        /*ParameterType: */SQL_VARCHAR,
        /*Columnsize: */0,
        /*DecimalDigits: */0,
        /*ParameterValuePtr: */NULL,
        /*BufferLength: */0,
        /*StrLen_or_IndPtr: */&v.v.buf.size
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
    }
  }

  virtual void execute()
  {
    if (conn_.showQueries())
      std::cerr << sql_ << std::endl;

    SQLRETURN rc = SQLExecute(stmt_);
    if (rc != SQL_NO_DATA) // SQL_NO_DATA can occur when no rows are affected
      handleErr(SQL_HANDLE_STMT, stmt_, rc);

    // FIXME: affectedRows_ may be -1 when doing INSERT?
    //        maybe this is because of OUTPUT Inserted.?
    rc = SQLRowCount(stmt_, &affectedRows_);
    handleErr(SQL_HANDLE_STMT, stmt_, rc);

    SQLSMALLINT numCols = 0;
    rc = SQLNumResultCols(stmt_, &numCols);
    handleErr(SQL_HANDLE_STMT, stmt_, rc);
    resultColCount_ = numCols;

    bool isInsertReturningId = false;
    const std::string returning = " OUTPUT Inserted.";
    std::size_t j = sql_.find(returning);
    if (j != std::string::npos)
      isInsertReturningId = true;

    if (isInsertReturningId) {
      bool nr = nextRow();
      if (nr) {
        getResult(0, &lastId_);
        // Set affected rows to 1, because
        // affectedRows_ seems to be -1?
        affectedRows_ = 1;
      }
    }
  }

  virtual long long insertedId()
  {
    return lastId_;
  }

  virtual int affectedRowCount()
  {
    return static_cast<int>(affectedRows_);
  }

  virtual bool nextRow()
  {
    SQLRETURN rc = SQLFetch(stmt_);
    if (rc == SQL_NO_DATA)
      return false;
    else {
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
      return true;
    }
  }

  virtual bool getResult(int column, std::string *value, int /*size*/)
  {
    MSSQLServer::Impl::ResultBuffer &resultBuffer = conn_.impl_->resultBuffer;
    std::size_t resultBufferPos = 0;
    SQLLEN strLen_or_ind = SQL_NO_TOTAL;
    while (strLen_or_ind == SQL_NO_TOTAL) {
      SQLRETURN rc = SQLGetData(
        /*StatementHandle: */stmt_,
        /*ColumnNumber: */column + 1,
#ifdef WT_WIN32
        /*TargetType: */SQL_C_WCHAR,
#else // WT_WIN32
        /*TargetType: */SQL_C_CHAR, // conversion from UTF-16 to UTF-8 done by driver
#endif // WT_WIN32
        /*TargetValue: */&resultBuffer.buf[resultBufferPos],
        /*BufferLength: */resultBuffer.size - resultBufferPos,
        /*StrLen_or_IndPtr: */&strLen_or_ind
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
#ifdef WT_WIN32
      const int charSize = 2;
#else // WT_WIN32
      const int charSize = 1;
#endif // WT_WIN32
      if (strLen_or_ind == SQL_NULL_DATA) {
        return false; // NULL
      } else if (strLen_or_ind == SQL_NO_TOTAL || strLen_or_ind + charSize > resultBuffer.size - resultBufferPos) {
        std::size_t pos = resultBuffer.size - charSize;
        resultBuffer.size *= 2;
        while (strLen_or_ind != SQL_NO_TOTAL &&
               strLen_or_ind + charSize > resultBuffer.size - resultBufferPos)
          resultBuffer.size *= 2;
        resultBufferPos = pos;
        resultBuffer.buf = (char*)realloc(resultBuffer.buf, resultBuffer.size);
        strLen_or_ind = SQL_NO_TOTAL;
      }
    }
    if (resultBufferPos == 0 && strLen_or_ind == 0) {
      value->clear();
      return true; // empty string
    }
    std::size_t totalDataSize = resultBufferPos + strLen_or_ind;
#ifdef WT_WIN32
    int strlen = WideCharToMultiByte(CP_UTF8, 0, (WCHAR*)resultBuffer.buf, totalDataSize / sizeof(WCHAR), NULL, 0, NULL, NULL);
    assert(strlen != 0);
    value->clear();
    value->resize(strlen);
    strlen = WideCharToMultiByte(CP_UTF8, 0, (WCHAR*)resultBuffer.buf, totalDataSize / sizeof(WCHAR), &(*value)[0], strlen, NULL, NULL);
    assert(strlen != 0);
#else // WT_WIN32
    *value = std::string(resultBuffer.buf, totalDataSize);
#endif // WT_WIN32
    return true;
  }

  virtual bool getResult(int column, short * value)
  {
    return getRes<SQL_C_SSHORT>(column, value);
  }

  virtual bool getResult(int column, int * value)
  {
    return getRes<SQL_C_SLONG>(column, value);
  }

  virtual bool getResult(int column, long long * value)
  {
    return getRes<SQL_C_SBIGINT>(column, value);
  }

  virtual bool getResult(int column, float * value)
  {
    return getRes<SQL_C_FLOAT>(column, value);
  }

  virtual bool getResult(int column, double * value)
  {
    return getRes<SQL_C_DOUBLE>(column, value);
  }

  virtual bool getResult(
    int column,
    std::chrono::system_clock::time_point *value,
    SqlDateTimeType type)
  {
    if (type == SqlDateTimeType::Date) {
      SQL_DATE_STRUCT date;
      bool result = getRes<SQL_C_TYPE_DATE>(column, &date);
      if (!result)
        return false; // NULL
      *value = date::sys_days{ date::year{ date.year } / date.month / date.day };
      return true;
    } else {
      SQL_TIMESTAMP_STRUCT ts;
      bool result = getRes<SQL_C_TYPE_TIMESTAMP>(column, &ts);
      if (!result)
        return false; // NULL
      *value = 
        date::sys_days{ date::year{ ts.year } / ts.month / ts.day } +
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
          std::chrono::hours{ ts.hour } +
          std::chrono::minutes{ ts.minute } +
          std::chrono::seconds{ ts.second } +
          std::chrono::nanoseconds{ ts.fraction }
        );
      return true;
    }
    assert(false);
    return false;
  }

  virtual bool getResult(
    int column,
    std::chrono::duration<int, std::milli> *value)
  {
    long long msec;
    bool res = getResult(column, &msec);
    if (!res)
      return res;

    *value = std::chrono::duration<int, std::milli>(msec);
    return true;
  }

  virtual bool getResult(
    int column,
    std::vector<unsigned char> *value,
    int size)
  {
    MSSQLServer::Impl::ResultBuffer &resultBuffer = conn_.impl_->resultBuffer;
    std::size_t resultBufferPos = 0;
    SQLLEN strLen_or_ind = SQL_NO_TOTAL;
    while (strLen_or_ind == SQL_NO_TOTAL) {
      SQLRETURN rc = SQLGetData(
        /*StatementHandle: */stmt_,
        /*ColumnNumber: */column + 1,
        /*TargetType: */SQL_C_BINARY,
        /*TargetValue: */&resultBuffer.buf[resultBufferPos],
        /*BufferLength: */resultBuffer.size - resultBufferPos,
        /*StrLen_or_IndPtr: */&strLen_or_ind
      );
      handleErr(SQL_HANDLE_STMT, stmt_, rc);
      if (strLen_or_ind == SQL_NULL_DATA) {
        return false; // NULL
      } else if (strLen_or_ind == SQL_NO_TOTAL || strLen_or_ind > resultBuffer.size - resultBufferPos) {
        std::size_t pos = resultBuffer.size;
        resultBuffer.size *= 2;
        while (strLen_or_ind != SQL_NO_TOTAL &&
               strLen_or_ind > resultBuffer.size - resultBufferPos)
          resultBuffer.size *= 2;
        resultBufferPos = pos;
        resultBuffer.buf = (char*)realloc(resultBuffer.buf, resultBuffer.size);
        strLen_or_ind = SQL_NO_TOTAL;
      }
    }
    std::size_t totalDataSize = resultBufferPos + strLen_or_ind;
    *value = std::vector<unsigned char>(resultBuffer.buf, resultBuffer.buf + totalDataSize);
    return true;
  }

  virtual std::string sql() const
  {
    return sql_;
  }

private:
  struct Value {
    union {
      struct {
        void *p; // buffer pointer 
        SQLLEN size; // buffer size
      } buf;
      short s;
      int i;
      long long ll;
      float f;
      double d;
      SQL_DATE_STRUCT date;
      SQL_TIME_STRUCT time;
      SQL_TIMESTAMP_STRUCT timestamp;
    } v;
    SQLLEN lengthOrInd; // length for binary data or string or NULL indicator
    SQLSMALLINT type; // SQL_C... type

    Value()
    {
      std::memset(&v, 0, sizeof(v));
      lengthOrInd = 0;
      type = 0;
    }

    ~Value()
    {
      if (type == SQL_C_BINARY ||
          type == SQL_C_WCHAR)
        free(v.buf.p);
    }

#ifdef WT_CXX11
    Value(const Value &other) = delete;
    Value &operator=(const Value &other) = delete;
    Value(Value &&other) = delete;
    Value &operator=(Value &&other) = delete;
#endif // WT_CXX11

    void clear()
    {
      if (type == SQL_C_BINARY ||
          type == SQL_C_WCHAR)
        free(v.buf.p);

      std::memset(&v, 0, sizeof(v));
      lengthOrInd = 0;
      type = 0;
    }

  private:
#ifndef WT_CXX11
    Value(const Value &other);
    Value& operator=(const Value &other);
#endif // WT_CXX11
  };
  Value *paramValues_;
  SQLSMALLINT parameterCount_;
  SQLSMALLINT resultColCount_;

  SQLHSTMT stmt_;
  MSSQLServer &conn_;
  std::string sql_;
  SQLLEN affectedRows_;
  long long lastId_;

  void checkColumnIndex(int column)
  {
    if (column >= parameterCount_)
      throw MSSQLServerException(
        std::string("Trying to bind too many parameters (parameter count = ") +
          std::to_string(parameterCount_) +
          ", column = " +
          std::to_string(column) +
          std::string(")"));
  }

  // bool returns whether buffer changed
  bool createOrResizeBuffer(Value &v, SQLLEN size)
  {
    if (v.type == SQL_C_WCHAR || v.type == SQL_C_BINARY) {
      // We already have a buffer
      if (v.v.buf.size >= size)
        return false;
      v.v.buf.size = size;
      v.v.buf.p = realloc(v.v.buf.p, v.v.buf.size);
      return true;
    }
    else {
      // New buffer
      v.clear();
      v.v.buf.size = size == 0 ? 1 : size;
      v.v.buf.p = malloc(v.v.buf.size);
      return true;
    }
  }

  template<SQLSMALLINT TargetType, typename ReturnType>
  bool getRes(int column, ReturnType * value)
  {
    SQLLEN strLen_or_ind = 0;
    SQLRETURN rc = SQLGetData(
      /*StatementHandle: */stmt_,
      /*ColumnNumber: */column + 1,
      /*TargetType: */TargetType,
      /*TargetValue: */value,
      /*BufferLength: */0,
      /*StrLen_or_IndPtr*/&strLen_or_ind
    );
    handleErr(SQL_HANDLE_STMT, stmt_, rc);
    return strLen_or_ind != SQL_NULL_DATA;
  }
};

MSSQLServer::MSSQLServer()
  : impl_(0)
{ }

MSSQLServer::MSSQLServer(const std::string &connectionString)
  : impl_(0)
{
  connect(connectionString);
}

MSSQLServer::MSSQLServer(const MSSQLServer &other)
  : SqlConnection(other),
    impl_(other.impl_ ? new Impl(*other.impl_) : 0)
{
  if (impl_)
    impl_->connect();
}

MSSQLServer::~MSSQLServer()
{
  clearStatementCache();
  delete impl_;
}

std::unique_ptr<SqlConnection> MSSQLServer::clone() const
{
  return std::unique_ptr<SqlConnection>(new MSSQLServer(*this));
}

bool MSSQLServer::connect(const std::string &connectionString)
{
  if (impl_)
    throw MSSQLServerException("Can't connect: already connected.");

#ifdef WT_WIN32
  ConnectionStringType connStr;
  if (!connectionString.empty()) {
    int wstrlen = MultiByteToWideChar(CP_UTF8, 0, &connectionString[0], connectionString.size(), 0, 0);
    assert(wstrlen != 0);
    connStr.resize(wstrlen);
    wstrlen = MultiByteToWideChar(CP_UTF8, 0, &connectionString[0], connectionString.size(), &connStr[0], connStr.size());
    assert(wstrlen != 0);
  }
#else // WT_WIN32
  ConnectionStringType connStr = toUTF16(connectionString);
#endif // WT_WIN32

  Impl *impl = new Impl(connStr);
  try {
    impl->connect();
  } catch (...) {
    delete impl;
    throw;
  }

  impl_ = impl;
  return true;
}

void MSSQLServer::executeSql(const std::string &sql)
{
  if (showQueries())
    std::cerr << sql << std::endl;

  SQLRETURN rc = SQL_SUCCESS;
  if (!impl_->stmt) {
    rc = SQLAllocHandle(SQL_HANDLE_STMT, impl_->dbc, &impl_->stmt);
    handleErr(SQL_HANDLE_DBC, impl_->dbc, rc);
  }
#ifdef WT_WIN32
  if (sql.empty()) {
    SQLWCHAR wstr[] = L"";
    rc = SQLExecDirectW(impl_->stmt, wstr, 0);
  } else {
    int wstrlen = MultiByteToWideChar(CP_UTF8, 0, &sql[0], sql.size(), 0, 0);
    assert(wstrlen != 0);
    SQLWCHAR *wstr = new SQLWCHAR[wstrlen + 1];
    wstrlen = MultiByteToWideChar(CP_UTF8, 0, &sql[0], sql.size(), wstr, wstrlen);
    assert(wstrlen != 0);
    wstr[wstrlen] = 0;
    rc = SQLExecDirectW(impl_->stmt, wstr, wstrlen);
    delete[] wstr;
  }
#else // WT_WIN32
  std::u16string wstr = toUTF16(sql);
  rc = SQLExecDirectW(impl_->stmt, (SQLWCHAR*)&wstr[0], wstr.size());
#endif // WT_WIN32
  try {
    handleErr(SQL_HANDLE_STMT, impl_->stmt, rc);
  } catch (...) {
    SQLFreeStmt(impl_->stmt, SQL_CLOSE);
    throw;
  }
  SQLFreeStmt(impl_->stmt, SQL_CLOSE);
}

void MSSQLServer::startTransaction()
{
  if (showQueries())
    std::cerr << "begin transaction -- implicit" << std::endl;
}

void MSSQLServer::commitTransaction()
{
  if (showQueries())
    std::cerr << "commit transaction -- using SQLEndTran" << std::endl;

  SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, impl_->dbc, SQL_COMMIT);
  handleErr(SQL_HANDLE_DBC, impl_->dbc, rc);
}

void MSSQLServer::rollbackTransaction()
{
  if (showQueries())
    std::cerr << "rollback transaction -- using SQLEndTran" << std::endl;

  SQLRETURN rc = SQLEndTran(SQL_HANDLE_DBC, impl_->dbc, SQL_ROLLBACK);
  handleErr(SQL_HANDLE_DBC, impl_->dbc, rc);
}

std::unique_ptr<SqlStatement> MSSQLServer::prepareStatement(const std::string &sql)
{
  return std::unique_ptr<SqlStatement>(new MSSQLServerStatement(*this, sql));
}

std::string MSSQLServer::autoincrementSql() const
{
  return "IDENTITY(1,1)";
}

std::vector<std::string> MSSQLServer::autoincrementCreateSequenceSql(const std::string &table, const std::string &id) const
{
  return std::vector<std::string>();
}

std::vector<std::string> MSSQLServer::autoincrementDropSequenceSql(const std::string &table, const std::string &id) const
{
  return std::vector<std::string>();
}

std::string MSSQLServer::autoincrementType() const
{
  return "bigint";
}

std::string MSSQLServer::autoincrementInsertInfix(const std::string &id) const
{
  return " OUTPUT Inserted.\"" + id + "\"";
}

std::string MSSQLServer::autoincrementInsertSuffix(const std::string &id) const
{
  return "";
}

const char *MSSQLServer::dateTimeType(SqlDateTimeType type) const
{
  if (type == SqlDateTimeType::Date)
    return "date";
  if (type == SqlDateTimeType::Time)
    return "bigint"; // SQL Server has no proper duration type, so store duration as number of milliseconds
  if (type == SqlDateTimeType::DateTime)
    return "datetime2";
  return "";
}

bool MSSQLServer::requireSubqueryAlias() const
{
  return true;
}

const char *MSSQLServer::blobType() const
{
  return "varbinary(max)";
}

const char *MSSQLServer::booleanType() const
{
  return "bit";
}

bool MSSQLServer::supportAlterTable() const
{
  return true;
}

std::string MSSQLServer::textType(int size) const
{
  if (size == -1)
    return "nvarchar(max)";
  else
    return std::string("nvarchar(") + std::to_string(size) + ")";
}

LimitQuery MSSQLServer::limitQueryMethod() const
{
  return LimitQuery::OffsetFetch;
}

    }
  }
}
