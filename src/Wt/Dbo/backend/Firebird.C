/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Originally contributed by Lukasz Matuszewski
 */
#include <Wt/Dbo/backend/Firebird>

#include "Wt/Dbo/Exception"

#include <cstdio>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#ifdef WT_WIN32
#define snprintf _snprintf
#endif

//#define DEBUG(x) x
#define DEBUG(x)

#define bindErr(x, y)						\
  std::cerr << this << " bind " << x << " " << y << std::endl;

#define resultErr(x, y)							\
  std::cerr << this << " getResult " << x << " " << y << std::endl;

#include <ibpp.h>

namespace Wt
{
  namespace Dbo
  {
    namespace backend {
      class FirebirdException : public Wt::Dbo::Exception
      {
      public:
	FirebirdException(const std::string& msg)
	  : Wt::Dbo::Exception(msg)
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

      class FirebirdStatement : public SqlStatement
      {
      public:
	FirebirdStatement(Firebird& conn, const std::string& sql)
	  : conn_(conn),
	    sql_(convertToNumberedPlaceholders(sql))
	{
	  lastId_ = -1;
	  row_ = affectedRows_ = 0;
	  
	  snprintf(name_, 64, "SQL%p%08X", this, rand());
	  
	  DEBUG(std::cerr << this << " for: " << sql_ << std::endl);
	  
	  IBPP::Transaction tr = conn_.impl_->m_tra;
	  
	  m_stmt = IBPP::StatementFactory(conn_.impl_->m_db, tr);
	  
	  if (!m_stmt.intf()) 
	    throw FirebirdException("Could not create a IBPP::Statement");
	  
	  try {
	    m_stmt->Prepare(sql_);
	  } catch(IBPP::LogicException &e) {
	    throw FirebirdException(e.what());
	  }
	}

	virtual ~FirebirdStatement()
	{ }

	virtual void reset()
	{ }

	// The first column index is 1!
	virtual void bind(int column, const std::string& value)
	{
	  DEBUG(bindErr(column, value));
	  
	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, short value)
	{
	  DEBUG(bindErr(column, value));

	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, int value)
	{
	  DEBUG(bindErr(column, value));
	  
	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, long long value)
	{
	  DEBUG(bindErr(column, value));
	  
	  m_stmt->Set(column + 1, (int64_t) value);
	}

	virtual void bind(int column, float value)
	{
	  DEBUG(bindErr(column, value));
	  
	  m_stmt->Set(column + 1, value);
	}

	virtual void bind(int column, double value)
	{
	  DEBUG(bindErr(column, value));
	  
	  m_stmt->Set(column + 1, value);
	}

	int getMilliSeconds(const boost::posix_time::time_duration &d) 
	{
	  using namespace boost::posix_time;
	  
	  time_duration::fractional_seconds_type ticks_per_msec =
	    time_duration::ticks_per_second() / 1000;
          time_duration::fractional_seconds_type ticks =
	    d.fractional_seconds();
	  
	  return (int)(ticks / ticks_per_msec);
	}

	virtual void bind(int column, 
			  const boost::posix_time::time_duration & value)
	{
	  DEBUG(bindErr(column, boost::posix_time::to_simple_string(value)));
	
	  int h = value.hours();
	  int m = value.minutes();
	  int s = value.seconds();
	  int ms = getMilliSeconds(value); 

	  IBPP::Time t(h, m, s, ms * 10);
	  
	  m_stmt->Set(column + 1, t);
	}

	virtual void bind(int column, 
			  const boost::posix_time::ptime& value,
			  SqlDateTimeType type)
	{
	  DEBUG(bindErr(column, boost::posix_time::to_simple_string(value)));
	  
	  if (type == SqlDate) {
	    IBPP::Date idate(value.date().year(), 
			     value.date().month(), 
			     value.date().day());
	    
	    m_stmt->Set(column + 1, idate);
	  } else {
	    int h = value.time_of_day().hours();
	    int m = value.time_of_day().minutes();
	    int s = value.time_of_day().seconds();
	    int ms = getMilliSeconds(value.time_of_day()); 
	    
	    IBPP::Timestamp ts(value.date().year(), 
			       value.date().month(), 
			       value.date().day(),
			       h, m, s, ms * 10);
	    
	    m_stmt->Set(column + 1, ts);
	  }
	}

	virtual void bind(int column, const std::vector<unsigned char>& value)
	{
	  //DEBUG(bindErr(column, 
		//	std::string(" (blob, size=") + value.size() + ")"));

	  IBPP::Blob b = IBPP::BlobFactory(conn_.impl_->m_db, 
					   conn_.impl_->m_tra);
	  b->Create();
	  if (value.size() > 0)
	    b->Write((void *)&value.front(), value.size());
	  b->Close();
	  m_stmt->Set(column + 1, b);
	}

	virtual void bindNull(int column)
	{
	  DEBUG(bindErr(column, "null"));
	  
	  m_stmt->SetNull(column + 1);
	}

	virtual void execute()
	{
	  if (conn_.showQueries())
	    std::cerr << sql_ << std::endl;

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
	  try {
	    return m_stmt->Fetch();
	  } catch(IBPP::Exception &e) {
	    throw FirebirdException(e.what());
	  }
	}

	void getString(int column, std::string *value, int size)
	{
	  m_stmt->Get(column, *value);
	}

	virtual bool getResult(int column, std::string *value, int size)
	{
	  if (m_stmt->IsNull(++column))
	    return false;
	      
	  getString(column, value, size);
	  
	  DEBUG(resultErr(column, *value));
	  
	  return true;
	}

	virtual bool getResult(int column, short *value)
	{
	  if (m_stmt->IsNull(++column))
	    return false;

	  m_stmt->Get(column, *value);
	
	  DEBUG(resultErr(column, *value));

	  return true;
	}

	virtual bool getResult(int column, int *value)
	{
	  if (m_stmt->IsNull(++column))
	    return false;

	  m_stmt->Get(column, *value);

	  DEBUG(resultErr(column, *value));

	  return true;
	}

	virtual bool getResult(int column, long long *value)
	{
	  if (m_stmt->IsNull(++column))
	    return false;

	  m_stmt->Get(column,  *((int64_t *)value));

	  DEBUG(resultErr(column, *value));

	  return true;
	}

	virtual bool getResult(int column, float *value)
	{
          if (m_stmt->IsNull(++column))
            return false;

          m_stmt->Get(column, *value);
	  
	  DEBUG(resultErr(column, *value));
	  	  
	  return true;
	}
	
	virtual bool getResult(int column, double *value)
	{
	  if (m_stmt->IsNull(++column))
	    return false;
	  
	  m_stmt->Get(column, *value);

	  DEBUG(resultErr(column, *value));

	  return true;
	}

	virtual bool getResult(int column, 
			       boost::posix_time::ptime *value,
			       SqlDateTimeType type)
	{
	  using namespace boost::posix_time;
	  using namespace boost::gregorian;

	  if (m_stmt->IsNull(++column))
            return false;
	  
          switch(type) {
	  case SqlDate: { 
	    IBPP::Date d;
	    m_stmt->Get(column, d);
	    *value = ptime(date(d.Year(), d.Month(), d.Day()));
	    break;
	  }
	    
	  case SqlDateTime: {
	    IBPP::Timestamp tm;
	    m_stmt->Get(column, tm);
	    *value = ptime(date(tm.Year(), tm.Month(), tm.Day()), 
			   hours(tm.Hours()) + 
			   minutes(tm.Minutes()) + 
			   seconds(tm.Seconds()) + 
			   microseconds(tm.SubSeconds() * 100));
	    break;
          }
	  case SqlTime:
	    break;
	  }
	  
	  DEBUG(resultErr(column, to_simple_string(*value)));
	  
	  return true;
	}
	

	virtual bool getResult(int column, 
			       boost::posix_time::time_duration *value)
	{
	  using namespace boost::posix_time;
	  
	  if (m_stmt->IsNull(++column))
	    return false;
	  
	  IBPP::Time t;
	  m_stmt->Get(column, t);
	  *value = boost::posix_time::hours(t.Hours()) +
	    boost::posix_time::minutes(t.Minutes()) +
	    boost::posix_time::seconds(t.Seconds()) +
	    boost::posix_time::microseconds(t.SubSeconds() * 100);
	  
	  DEBUG(resultErr(column, to_simple_string(*value)));

	  return true;
	}

	virtual bool getResult(int column, 
			       std::vector<unsigned char> *value,
			       int size)
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

	  //DEBUG(resultErr(column, std::string("blob (size=") + value->size()));
	  
	  return true;
	}

	virtual std::string sql() const 
	{
	  return sql_;
	}
	
      private:
	Firebird& conn_;
	std::string sql_;
	IBPP::Statement  m_stmt;
	char name_[64];
	
	int lastId_, row_, affectedRows_;


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

      Firebird *Firebird::clone() const
      {
	return new Firebird(*this);
      }
      
      SqlStatement *Firebird::prepareStatement(const std::string& sql)
      {
	return new FirebirdStatement(*this, sql);
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
	case SqlDate:
	  return "date";
        case SqlDateTime:
          return "timestamp";
        case SqlTime:
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
	  throw new FirebirdException("Could not start transaction!");

	impl_->m_tra->Start();
      }

      void Firebird::commitTransaction()
      {
	if (!impl_->m_tra.intf())
	  throw new FirebirdException("Transaction was not started yet!");

	impl_->m_tra->Commit();
      }

      void Firebird::rollbackTransaction()
      {	
	if (!impl_->m_tra.intf())
	  throw new FirebirdException("Transaction was not started yet!");

	impl_->m_tra->Rollback();
      }
      
      std::string Firebird::textType(int size) const
      {
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
        return RowsFromTo;
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
