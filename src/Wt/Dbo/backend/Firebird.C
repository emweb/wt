/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Originally contributed by Lukasz Matuszewski
 */
#include <Wt/Dbo/backend/Firebird.h>

#include "Wt/Dbo/Exception.h"
#include "Wt/Dbo/Logger.h"
#include "Wt/Dbo/StringStream.h"

#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>

#ifdef WT_WIN32
#define snprintf _snprintf
#define timegm _mkgmtime
#endif

#include <ibpp.h>

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

  static constexpr int MAX_VARCHAR_LENGTH = 32765;
}

namespace Wt
{
  namespace Dbo
  {

LOGGER("Dbo.backend.Firebird");

    namespace backend {
      class FirebirdException : public ::Wt::Dbo::Exception
      {
      public:
	FirebirdException(const std::string& msg)
          : ::Wt::Dbo::Exception(msg)
	{ }
      };

      class Firebird_impl {
      public:
	IBPP::Database        m_db;
	IBPP::Transaction     m_tra;
	
	~Firebird_impl()
	{
	  if (m_db->Connected())
	    m_db->Disconnect();
	}

	Firebird_impl()
	{ }
	
	Firebird_impl(IBPP::Database db) :
	  m_db(db)
	{ }
      };

      class FirebirdStatement final : public SqlStatement
      {
      public:
	FirebirdStatement(Firebird& conn, const std::string& sql)
	  : conn_(conn),
	    sql_(convertToNumberedPlaceholders(sql))
	{
	  lastId_ = -1;
	  row_ = affectedRows_ = 0;
          columnCount_ = 0;
	  
          snprintf(name_, 64, "SQL%p%08X", (void*)this, rand());
	  
	  LOG_DEBUG(this << " for: " << sql_);
	  
	  IBPP::Transaction tr = conn_.impl_->m_tra;
	  
	  m_stmt = IBPP::StatementFactory(conn_.impl_->m_db, tr);
	  
	  if (!m_stmt.intf()) 
	    throw FirebirdException("Could not create a IBPP::Statement");
	  
	  try {
	    m_stmt->Prepare(sql_);
	  } catch(IBPP::LogicException &e) {
	    throw FirebirdException(e.what());
	  }

          columnCount_ = m_stmt->Columns();
	}

	virtual ~FirebirdStatement()
	{ }

	virtual void reset() override
	{ }

	// The first column index is 1!
	virtual void bind(int column, const std::string& value) override
	{
          LOG_DEBUG("bind: " << column << " " << value);
	  
	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, short value) override
	{
          LOG_DEBUG("bind: " << column << " " << value);

	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, int value) override
	{
          LOG_DEBUG("bind: " << column << " " << value);
	  
	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, long long value) override
	{
          LOG_DEBUG("bind: " << column << " " << value);
	  
	  m_stmt->Set(column + 1, (int64_t) value);
	}

	virtual void bind(int column, float value) override
	{
          LOG_DEBUG("bind: " << column << " " << value);
	  
	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, double value) override
	{
          LOG_DEBUG("bind: " << column << " " << value);
	  
	  m_stmt->Set(column + 1, value);
	}

    int getMilliSeconds(const std::chrono::system_clock::time_point &tp)
    {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
        return ms.count()%1000;
    }

	virtual void bind(int column, 
              const std::chrono::duration<int, std::milli> & value) override
    {
      std::chrono::system_clock::time_point tp(value);
      std::time_t time = std::chrono::system_clock::to_time_t(tp);
      std::tm *tm = thread_local_gmtime(&time);
      char mbstr[100];
      std::strftime(mbstr, sizeof(mbstr), "%Y-%b-%d %H:%M:%S", tm);
      LOG_DEBUG("bind: " << column << " " << mbstr);

      int h = tm->tm_hour;
      int m = tm->tm_min;
      int s = tm->tm_sec;
      int ms = getMilliSeconds(std::chrono::system_clock::time_point(value));
	  IBPP::Time t(h, m, s, ms * 10);
	  
	  m_stmt->Set(column + 1, t);
	}

	virtual void bind(int column, 
              const std::chrono::system_clock::time_point& value,
			  SqlDateTimeType type) override
	{
      std::time_t t = std::chrono::system_clock::to_time_t(value);
      std::tm *tm = thread_local_gmtime(&t);
      char mbstr[100];
      std::strftime(mbstr, sizeof(mbstr), "%Y-%b-%d %H:%M:%S", tm);
      LOG_DEBUG("bind: " << column << " " << mbstr);
	  
      if (type == SqlDateTimeType::Date) {
        IBPP::Date idate(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
	    
	    m_stmt->Set(column + 1, idate);
      } else {
        int h = tm->tm_hour;
        int m = tm->tm_min;
        int s = tm->tm_sec;
        int ms = getMilliSeconds(value);

        IBPP::Timestamp ts(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                           h, m, s, ms * 10);
	    
	    m_stmt->Set(column + 1, ts);
	  }
	}

	virtual void bind(int column, const std::vector<unsigned char>& value) override
    {

	  IBPP::Blob b = IBPP::BlobFactory(conn_.impl_->m_db, 
					   conn_.impl_->m_tra);
	  b->Create();
	  if (value.size() > 0)
	    b->Write((void *)&value.front(), value.size());
	  b->Close();
	  m_stmt->Set(column + 1, b);
	}

	virtual void bindNull(int column) override
	{
          LOG_DEBUG("bind: " << column << " null");
	  
	  m_stmt->SetNull(column + 1);
	}

	virtual void execute() override
	{
	  if (conn_.showQueries())
	    LOG_INFO(sql_);

	  try {
	    m_stmt->Execute();
	    affectedRows_ = m_stmt->AffectedRows();
	    
	    row_ = 0;
	  } catch(IBPP::LogicException &e) {
	    throw FirebirdException(e.what());
	  }

	  if (affectedRows_ == 0) {
	    const std::string returning = " returning ";
	    std::size_t j = sql_.rfind(returning);
	    if (j != std::string::npos && 
		sql_.find(' ', j + returning.length()) == std::string::npos) {
	      if (m_stmt->Columns() == 1) {
		lastId_ = -1;
		m_stmt->Get(1,  lastId_);
	      }
	    }
	  }
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
	  try {
	    return m_stmt->Fetch();
	  } catch(IBPP::Exception &e) {
	    throw FirebirdException(e.what());
	  }
	}

        virtual int columnCount() const override
        {
          return columnCount_;
        }

	void getString(int column, std::string *value, int size)
	{
	  m_stmt->Get(column, *value);
	}

	virtual bool getResult(int column, std::string *value, int size) override
	{
	  if (m_stmt->IsNull(++column))
	    return false;
	      
	  getString(column, value, size);
	  
          LOG_DEBUG("getResult " << column << " " << *value);
	  
	  return true;
	}

	virtual bool getResult(int column, short *value) override
	{
	  if (m_stmt->IsNull(++column))
	    return false;

	  m_stmt->Get(column, *value);
	
          LOG_DEBUG("getResult " << column << " " << *value);

	  return true;
	}

	virtual bool getResult(int column, int *value) override
	{
	  if (m_stmt->IsNull(++column))
	    return false;

	  m_stmt->Get(column, *value);

          LOG_DEBUG("getResult " << column << " " << *value);

	  return true;
	}

	virtual bool getResult(int column, long long *value) override
	{
	  if (m_stmt->IsNull(++column))
	    return false;

	  m_stmt->Get(column,  *((int64_t *)value));

          LOG_DEBUG("getResult " << column << " " << *value);

	  return true;
	}

	virtual bool getResult(int column, float *value) override
	{
          if (m_stmt->IsNull(++column))
            return false;

	  m_stmt->Get(column, *value);
	  
          LOG_DEBUG("getResult " << column << " " << *value);
	  	  
	  return true;
	}
	
	virtual bool getResult(int column, double *value) override
	{
	  if (m_stmt->IsNull(++column))
	    return false;
	  
	  m_stmt->Get(column, *value);

          LOG_DEBUG("getResult " << column << " " << *value);

	  return true;
	}

	virtual bool getResult(int column, 
			       std::chrono::system_clock::time_point *value,
			       SqlDateTimeType type) override
    {

	  if (m_stmt->IsNull(++column))
            return false;
	  
          switch(type) {
	  case SqlDateTimeType::Date: { 
	    IBPP::Date d;
	    m_stmt->Get(column, d);
	    std::tm tm = std::tm();
	    tm.tm_year = d.Year() - 1900;
	    tm.tm_mon = d.Month() - 1;
	    tm.tm_mday = d.Day();
	    std::time_t t = timegm(&tm);
	    *value = std::chrono::system_clock::from_time_t(t);
	    break;
	  }
	    
	  case SqlDateTimeType::DateTime: {
	    IBPP::Timestamp tm;
	    m_stmt->Get(column, tm);
	    std::tm time = std::tm();
	    time.tm_year = tm.Year() - 1900;
	    time.tm_mon = tm.Month() - 1;
	    time.tm_mday = tm.Day();
	    time.tm_hour = tm.Hours();
	    time.tm_min = tm.Minutes();
	    time.tm_sec = tm.Seconds();
	    std::time_t t = timegm(&time);
	    *value = std::chrono::system_clock::from_time_t(t);
	    *value += std::chrono::milliseconds(tm.SubSeconds() / 10);
	    break;
          }
	  case SqlDateTimeType::Time:
	    break;
	  }
          std::time_t t = std::chrono::system_clock::to_time_t(*value);
          LOG_DEBUG("getResult " << column << " " << std::ctime(&t));
	  
	  return true;
	}
	

	virtual bool getResult(int column, 
			       std::chrono::duration<int, std::milli> *value) override
    {
	  
	  if (m_stmt->IsNull(++column))
	    return false;
	  
	  IBPP::Time t;
	  m_stmt->Get(column, t);
	  *value = std::chrono::hours(t.Hours()) + std::chrono::minutes(t.Minutes()) +
              std::chrono::seconds(t.Seconds()) + std::chrono::milliseconds(t.SubSeconds() / 10);
	  
          LOG_DEBUG("getResult " << column << " " << value->count() << "ms");

	  return true;
	}

	virtual bool getResult(int column, 
			       std::vector<unsigned char> *value,
			       int size) override
	{
	  if (m_stmt->IsNull(++column))
	    return false;

	  IBPP::Blob b = IBPP::BlobFactory(conn_.impl_->m_db, 
					   conn_.impl_->m_tra);
	  m_stmt->Get(column, b);
	  b->Open();
	  int largest = -1, segments = -1;
	  b->Info(&size, &largest, &segments);
	  value->resize(size);
	  if (size > 0)
	    b->Read((void *)&value->front(), size);
	  
	  return true;
	}

	virtual std::string sql() const override
	{
	  return sql_;
	}
	
      private:
	Firebird& conn_;
	std::string sql_;
	IBPP::Statement  m_stmt;
	char name_[64];
	
	int lastId_, row_, affectedRows_;
        int columnCount_;


	void setValue(int column, const std::string& value)
	{
	  m_stmt->Set(column + 1, value);
	}

	std::string convertToNumberedPlaceholders(const std::string& sql)
	{
	  return sql;
	}
      };

      Firebird::Firebird()
	: m_writableTransaction(true)
      {
	impl_ = new Firebird_impl();
      }

      Firebird::Firebird(const std::string& ServerName,
			 const std::string& DatabaseName, 
			 const std::string& UserName,
			 const std::string& UserPassword, 
			 const std::string& RoleName,
			 const std::string& CharSet, 
			 const std::string& CreateParams)
	: m_writableTransaction(true)
      {
	IBPP::Database db = IBPP::DatabaseFactory(ServerName, 
						  DatabaseName, 
						  UserName, UserPassword, 
						  RoleName, 
						  CharSet, CreateParams);
	db->Connect();
	
	impl_ = new Firebird_impl(db);
      }

      Firebird::Firebird(IBPP::Database db)
      {
	impl_ = new Firebird_impl(db);
      }

      Firebird::Firebird(const Firebird& other)
	: SqlConnection(other),
	  m_writableTransaction(other.m_writableTransaction)
      {
	IBPP::Database db = 
	  IBPP::DatabaseFactory(other.impl_->m_db->ServerName(),
				other.impl_->m_db->DatabaseName(),
				other.impl_->m_db->Username(),
				other.impl_->m_db->UserPassword(),
				other.impl_->m_db->RoleName(),
				other.impl_->m_db->CharSet(),
				other.impl_->m_db->CreateParams());
	db->Connect();

	impl_ = new Firebird_impl(db);
      }

      bool Firebird::connect(const std::string& ServerName,
			     const std::string& DatabaseName, 
			     const std::string& UserName,
			     const std::string& UserPassword, 
			     const std::string& RoleName,
			     const std::string& CharSet, 
			     const std::string& CreateParams)
      {
	IBPP::Database db = IBPP::DatabaseFactory(ServerName, 
						  DatabaseName, 
						  UserName, UserPassword, 
						  RoleName, 
						  CharSet, CreateParams);
	db->Connect();

	impl_->m_db = db;

	return true;
      }
      
      Firebird::~Firebird()
      {
	clearStatementCache();
	delete impl_;
      }

      std::unique_ptr<SqlConnection> Firebird::clone() const
      {
	return std::unique_ptr<SqlConnection>(new Firebird(*this));
      }
      
      std::unique_ptr<SqlStatement> Firebird::prepareStatement(const std::string& sql)
      {
	return std::unique_ptr<SqlStatement>(
	    new FirebirdStatement(*this, sql));
      }

      std::string Firebird::autoincrementType() const
      {
	return "bigint";
      }

      std::string Firebird::autoincrementSql() const
      {
	return std::string();
      }

      std::vector<std::string> 
      Firebird::autoincrementCreateSequenceSql(const std::string &table,
					       const std::string &id) const
      {
	std::vector<std::string> sql;
	
	std::string sequenceId = "seq_" + table + "_" + id;
	
	sql.push_back(std::string("create generator ") + sequenceId);
	sql.push_back(std::string("set generator ") + sequenceId + " to 0");
	
	std::stringstream trigger;
	trigger << "CREATE TRIGGER seq_trig_" << table
		<< " FOR \"" << table << "\""
		<< " ACTIVE BEFORE INSERT POSITION 0" 
		<< " AS"
		<< " BEGIN"
		<< " if (NEW.\"" << id << "\" is NULL)"
		<< "   then NEW.\"" << id << "\" "
		<< "     = GEN_ID(" << sequenceId << ", 1);"
		<< " END ";
	
	sql.push_back(trigger.str());
	
	return sql;
      }

      std::vector<std::string> 
      Firebird::autoincrementDropSequenceSql(const std::string &table,
					     const std::string &id) const
      {
	std::vector<std::string> sql;
	
	sql.push_back(std::string("drop trigger seq_trig_") + table); 
	sql.push_back(std::string("drop generator seq_") 
		      + table + "_" + id);

	return sql;
      }
      
      std::string Firebird::autoincrementInsertSuffix(const std::string& id) const
      {
	return " returning \"" + id + "\"";
      }

      const char *Firebird::dateTimeType(SqlDateTimeType type) const
      {
	switch (type) {
	case SqlDateTimeType::Date:
	  return "date";
        case SqlDateTimeType::DateTime:
          return "timestamp";
        case SqlDateTimeType::Time:
          return "time";
	}
	
	std::stringstream ss;
	ss << __FILE__ << ":" << __LINE__ << ": implementation error";
	throw FirebirdException(ss.str());
      }

      const char *Firebird::blobType() const
      {
	return "blob";
      }
      
      void Firebird::startTransaction()
      {
	if (!impl_->m_tra.intf()) {
	  if (m_writableTransaction) {
	    impl_->m_tra = IBPP::TransactionFactory(impl_->m_db, 
						    IBPP::amWrite, 
						    IBPP::ilReadCommitted, 
						    IBPP::lrWait);
	  } else {
	    impl_->m_tra = IBPP::TransactionFactory(impl_->m_db, 
						    IBPP::amRead, 
						    IBPP::ilReadCommitted, 
						    IBPP::lrWait);
	  }
	}

	if (!impl_->m_tra.intf())
	  throw FirebirdException("Could not start transaction!");

	impl_->m_tra->Start();
      }

      void Firebird::commitTransaction()
      {
	if (!impl_->m_tra.intf())
	  throw FirebirdException("Transaction was not started yet!");

	impl_->m_tra->Commit();
      }

      void Firebird::rollbackTransaction()
      {	
	if (!impl_->m_tra.intf())
	  throw FirebirdException("Transaction was not started yet!");

	impl_->m_tra->Rollback();
      }
      
      std::string Firebird::textType(int size) const
      {
        if (size != -1 && size <= MAX_VARCHAR_LENGTH)
          return std::string("varchar (") + std::to_string(size) + ")";
        else
          return std::string("blob sub_type text");
      }

      const char *Firebird::booleanType() const
      {
	return "smallint";
      }
      
      void Firebird::prepareForDropTables()
      { 
	clearStatementCache();
      }
      
      LimitQuery Firebird::limitQueryMethod() const
      {
        return LimitQuery::RowsFromTo;
      }

      bool Firebird::supportAlterTable() const
      {
        return true;
      }

      IBPP::Database Firebird::connection()
      {
	return impl_->m_db;
      }
    }
  }
}
