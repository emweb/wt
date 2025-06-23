/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace dbo = Wt::Dbo;

namespace {
	class TestStatement : public dbo::SqlStatement
	{
	public:
		explicit TestStatement(std::string sql) : sql_(std::move(sql)) {}

		void reset() override {}
		void bind(int, const std::string&) override {}
		void bind(int, short) override {}
		void bind(int, int) override {}
		void bind(int, long long) override {}
		void bind(int, float) override {}
		void bind(int, double) override {}
		void bind(int, const std::chrono::system_clock::time_point&, dbo::SqlDateTimeType) override {}
		void bind(int, const std::chrono::duration<int, std::milli>&) override {}
		void bind(int, const std::vector<unsigned char>&) override {}
		void bindNull(int) override {}
		void execute() override {}
		long long insertedId() override { return 0; }
		int affectedRowCount() override { return 0; }
		bool nextRow() override { return false; }
		int columnCount() const override { return 0; }
		bool getResult(int, std::string*, int) override { return false; }
		bool getResult(int, short*) override { return false; }
		bool getResult(int, int*) override { return false; }
		bool getResult(int, long long*) override { return false; }
		bool getResult(int, float*) override { return false; }
		bool getResult(int, double*) override { return false; }
		bool getResult(int, std::chrono::system_clock::time_point*, dbo::SqlDateTimeType) override { return false; }
		bool getResult(int, std::chrono::duration<int, std::milli>*) override { return false; }
		bool getResult(int, std::vector<unsigned char>*, int) override { return false; }
		std::string sql() const override { return sql_; }

	private:
		std::string sql_;
	};

	class TestConnection : public dbo::SqlConnection
	{
	public:
		~TestConnection()
		{
			clearStatementCache();
		}

		std::unique_ptr<dbo::SqlConnection> clone() const override
		{
			return std::make_unique<TestConnection>();
		}

		void startTransaction() override {}
		void commitTransaction() override {}
		void rollbackTransaction() override {}
		std::unique_ptr<dbo::SqlStatement> prepareStatement(const std::string& sql) override
		{
			return std::make_unique<TestStatement>(sql);
		}

		std::string autoincrementSql() const override { return {}; }
		std::vector<std::string> autoincrementCreateSequenceSql(const std::string&, const std::string&) const override
		{ return {}; }
		std::vector<std::string> autoincrementDropSequenceSql(const std::string&, const std::string&) const override
		{ return {}; }
		std::string autoincrementType() const override { return {}; }
		std::string autoincrementInsertSuffix(const std::string&) const override { return {}; }
		const char *dateTimeType(dbo::SqlDateTimeType) const override { return "timestamp"; }
		const char *blobType() const override { return "blob"; }
	};
}

BOOST_AUTO_TEST_CASE( SqlConnection_statement_cache_reuses_idle_statement )
{
	TestConnection connection;
	auto statement = std::make_unique<TestStatement>("select 1");
	TestStatement* saved = statement.get();

	connection.saveStatement("query", std::move(statement));

	BOOST_CHECK(connection.getStatement("query") == saved);
	BOOST_CHECK(connection.getStatement("query") == nullptr);
	saved->done();
	BOOST_CHECK(connection.getStatement("query") == saved);
}

BOOST_AUTO_TEST_CASE( SqlConnection_statement_cache_uses_an_idle_duplicate )
{
	TestConnection connection;
	auto first = std::make_unique<TestStatement>("select 1");
	auto second = std::make_unique<TestStatement>("select 1");
	TestStatement* secondSaved = second.get();

	first->use();
	connection.saveStatement("query", std::move(first));
	connection.saveStatement("query", std::move(second));

	BOOST_CHECK(connection.getStatement("query") == secondSaved);
}

BOOST_AUTO_TEST_CASE( SqlConnection_statement_cache_prunes_least_recently_used )
{
	TestConnection connection;
	connection.setMaxSavedStatement(3);

	auto first = std::make_unique<TestStatement>("select 1");
	auto second = std::make_unique<TestStatement>("select 2");
	auto third = std::make_unique<TestStatement>("select 3");
	TestStatement* firstSaved = first.get();
	TestStatement* secondSaved = second.get();
	TestStatement* thirdSaved = third.get();

	connection.saveStatement("first", std::move(first));
	connection.saveStatement("second", std::move(second));
	connection.saveStatement("third", std::move(third));

	BOOST_REQUIRE(connection.getStatement("second") == secondSaved);
  secondSaved->done();
	BOOST_REQUIRE(connection.getStatement("third") == thirdSaved);
  thirdSaved->done();
  BOOST_REQUIRE(connection.getStatement("first") == firstSaved);
  firstSaved->done();

  connection.setMaxSavedStatement(2);

  BOOST_CHECK(connection.getStatement("first") == firstSaved);
  BOOST_CHECK(connection.getStatement("second") == nullptr);
  BOOST_CHECK(connection.getStatement("third") == thirdSaved);
}

BOOST_AUTO_TEST_CASE( SqlConnection_statement_cache_keeps_statements_in_use )
{
	TestConnection connection;
	connection.setMaxSavedStatement(1);

	auto first = std::make_unique<TestStatement>("select 1");
	auto second = std::make_unique<TestStatement>("select 2");
	TestStatement* firstSaved = first.get();
	TestStatement* secondSaved = second.get();

	first->use();
	connection.saveStatement("first", std::move(first));
	second->use();
	connection.saveStatement("second", std::move(second));

	BOOST_CHECK(connection.getStatement("first") == nullptr);
	BOOST_CHECK(connection.getStatement("second") == nullptr);

	secondSaved->done();
  BOOST_CHECK(connection.getStatement("first") == nullptr);
	BOOST_CHECK(connection.getStatement("second") == nullptr);

  firstSaved->done();
	BOOST_CHECK(connection.getStatement("first") == firstSaved);
	BOOST_CHECK(connection.getStatement("second") == nullptr);
}
