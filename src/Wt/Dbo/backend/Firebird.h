// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Originally contributed by Lukasz Matuszewski
 */
#ifndef _WT_DBO_BACKEND_FIREBIRD_
#define _WT_DBO_BACKEND_FIREBIRD_

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>
#include <Wt/Dbo/backend/WDboFirebirdDllDefs.h>

namespace IBPP {
  class IDatabase;
  template<class T>
  class Ptr;
  typedef Ptr<IDatabase> Database;
}

namespace Wt {
  namespace Dbo {
    namespace backend {
      class Firebird_impl;

      /*! \class Firebird Wt/Dbo/backend/Firebird.h
       *  \brief A %Firebird connection
       *
       * This class provides the backend implementation for %Firebird
       * databases. It supports %Firebird databases with version 2.1
       * or higher.
       *
       * \warning
       * The %Firebird backend is **UNMAINTAINED**, and may not work at all
       * with recent versions of %Firebird. This backend is based on
       * IBPP, a project which has not seen any activity in a very
       * long time. Emweb has no plans to continue supporting the
       * %Firebird backend.
       *
       * \ingroup dbo
       */
      class WTDBOFIREBIRD_API Firebird : public SqlConnection
      {
      public:
        /*! \brief Creates a %Firebird backend connection.
         *
         * The connection is not yet open, and requires a connect() before it
         * can be used.
         */
        Firebird();

        /*! \brief Creates and opens a %Firebird backend connection.
         */
        Firebird(const std::string& ServerName,
		 const std::string& DatabaseName, 
		 const std::string& UserName,
		 const std::string& UserPassword, 
		 const std::string& RoleName,
		 const std::string& CharSet = std::string(), 
		 const std::string& CreateParams = std::string());

	/*! \brief Creates and opens a %Firebird backend connection.
	 */
        Firebird(IBPP::Database db);

        /*! \brief Copy constructor.
	 *
	 * This creates a new backend connection with the same settings
	 * as another connection.
         */
        Firebird(const Firebird& other);
	
        /*! \brief Destructor.
         *
         * Closes the connection.
         */
        virtual ~Firebird();

	/*! \brief Tries to connect.
         *
         * Throws an exception if there was a problem, otherwise
         * returns \c true.
         */
	bool connect(const std::string& ServerName,
		     const std::string& DatabaseName, 
		     const std::string& UserName,
		     const std::string& UserPassword, 
		     const std::string& RoleName,
		     const std::string& CharSet = std::string(), 
		     const std::string& CreateParams = std::string());

	virtual std::unique_ptr<SqlConnection> clone() const override;

        /*! \brief Returns the underlying connection handle.
         */
        IBPP::Database connection();

        virtual void startTransaction() override;
        virtual void commitTransaction() override;
        virtual void rollbackTransaction() override;
	
        virtual std::unique_ptr<SqlStatement> prepareStatement(const std::string& sql) override;

        /** @name Methods that return dialect information
         */
        //!@{
        virtual std::string autoincrementSql() const override;
        virtual std::vector<std::string>
	  autoincrementCreateSequenceSql(const std::string &table,
                                         const std::string &id) const override;
	virtual std::vector<std::string> 
	  autoincrementDropSequenceSql(const std::string &table,
                                       const std::string &id) const override;
        virtual std::string autoincrementType() const override;
        virtual std::string autoincrementInsertSuffix(const std::string& id) const override;
        virtual const char *dateTimeType(SqlDateTimeType type) const override;
        virtual const char *blobType() const override;
        virtual std::string textType(int size) const override;
        virtual const char *booleanType() const override;
        virtual LimitQuery limitQueryMethod() const override;
        virtual bool supportAlterTable() const override;
        virtual bool usesRowsFromTo() const override {return false;}
	//!@}
 
        virtual void prepareForDropTables() override;
	
      private:
	Firebird_impl          *impl_;
        bool                    m_writableTransaction;

	friend class FirebirdStatement;
      };
    }
  }
}

#endif /* _WT_DBO_BACKEND_FIREBIRD_ */

