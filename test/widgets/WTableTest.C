/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WTable.h>
#include <Wt/Test/WTestEnvironment.h>

using namespace Wt;

namespace {
Wt::Test::WTestEnvironment environment;
Wt::WApplication testApp(environment);

void CheckRowNumbers(WTable* table)
{
  for (auto i = 0; i < table->rowCount(); ++i) {
    BOOST_REQUIRE_EQUAL(table->rowAt(i)->rowNum(), i);
  }
}
void CheckColumnNumbers(WTable* table)
{
  for (auto i = 0; i < table->columnCount(); ++i) {
    BOOST_REQUIRE_EQUAL(table->columnAt(i)->columnNum(), i);
  }
}
}

BOOST_AUTO_TEST_SUITE( WTableTest )

BOOST_AUTO_TEST_CASE( elementAt1 )
{
  auto table = cpp14::make_unique<WTable>();
  BOOST_REQUIRE_NO_THROW(table->elementAt(5, 1));

  auto rowCount = table->rowCount();

  BOOST_REQUIRE_EQUAL(rowCount, 6);
  CheckRowNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( elementAt2 )
{
  auto table = cpp14::make_unique<WTable>();
  BOOST_REQUIRE_NO_THROW(table->elementAt(1, 5));

  auto columnCount = table->columnCount();

  BOOST_REQUIRE_EQUAL(columnCount, 6);
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( elementAt3 )
{
  auto table = cpp14::make_unique<WTable>();
  BOOST_REQUIRE_NO_THROW(table->elementAt(5, 4));

  auto rowCount = table->rowCount();
  auto columnCount = table->columnCount();

  BOOST_REQUIRE_EQUAL(rowCount, 6);
  BOOST_REQUIRE_EQUAL(columnCount, 5);
  CheckRowNumbers(table.get());
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( rowAt )
{
  auto table = cpp14::make_unique<WTable>();
  BOOST_REQUIRE_NO_THROW(table->rowAt(5));

  auto rowCount = table->rowCount();

  BOOST_REQUIRE_EQUAL(rowCount, 6);
  CheckRowNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( columnAt )
{
  auto table = cpp14::make_unique<WTable>();
  BOOST_REQUIRE_NO_THROW(table->columnAt(5));

  auto columnCount = table->columnCount();

  BOOST_REQUIRE_EQUAL(columnCount, 6);
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( insertRow1 )
{
  auto table = cpp14::make_unique<WTable>();
  table->rowAt(2);
  BOOST_REQUIRE_NO_THROW(table->insertRow(1));

  auto rowCount = table->rowCount();

  BOOST_REQUIRE_EQUAL(rowCount, 4);
  CheckRowNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( insertRow2 )
{
  auto table = cpp14::make_unique<WTable>();
  table->rowAt(2);
  BOOST_REQUIRE_NO_THROW(table->insertRow(3));

  auto rowCount = table->rowCount();

  BOOST_REQUIRE_EQUAL(rowCount, 4);
  CheckRowNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( insertRow3 )
{
  auto table = cpp14::make_unique<WTable>();
  BOOST_REQUIRE_NO_THROW(table->insertRow(0));
}

BOOST_AUTO_TEST_CASE( insertColumn1 )
{
  auto table = cpp14::make_unique<WTable>();
  table->columnAt(2);
  BOOST_REQUIRE_NO_THROW(table->insertColumn(1));

  auto columnCount = table->columnCount();

  BOOST_REQUIRE_EQUAL(columnCount, 4);
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( insertColumn2 )
{
  auto table = cpp14::make_unique<WTable>();
  table->columnAt(2);
  BOOST_REQUIRE_NO_THROW(table->insertColumn(3));

  auto columnCount = table->columnCount();

  BOOST_REQUIRE_EQUAL(columnCount, 4);
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( insertColumn3 )
{
  auto table = cpp14::make_unique<WTable>();
  BOOST_REQUIRE_NO_THROW(table->insertColumn(0));
}

BOOST_AUTO_TEST_CASE( removeRow1 )
{
  auto table = cpp14::make_unique<WTable>();
  table->rowAt(5);
  BOOST_REQUIRE_NO_THROW(table->removeRow(2));

  auto rowCount = table->rowCount();

  BOOST_REQUIRE_EQUAL(rowCount, 5);
  CheckRowNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( removeRow2 )
{
  auto table = cpp14::make_unique<WTable>();
  table->rowAt(0);
  BOOST_REQUIRE_NO_THROW(table->removeRow(0));
}

BOOST_AUTO_TEST_CASE( removeColumn1 )
{
  auto table = cpp14::make_unique<WTable>();
  table->columnAt(5);
  BOOST_REQUIRE_NO_THROW(table->removeColumn(2));

  auto columnCount = table->columnCount();

  BOOST_REQUIRE_EQUAL(columnCount, 5);
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( removeColumn2 )
{
  auto table = cpp14::make_unique<WTable>();
  table->columnAt(0);
  BOOST_REQUIRE_NO_THROW(table->removeColumn(0));
}

BOOST_AUTO_TEST_CASE( moveRow1 )
{
  auto table = cpp14::make_unique<WTable>();
  table->rowAt(5);

  WTableCell* cell2 = table->elementAt(2, 0);
  WTableCell* cell0 = table->elementAt(0, 0);

  BOOST_REQUIRE_NO_THROW(table->moveRow(2, 0));

  BOOST_REQUIRE_EQUAL(table->elementAt(0, 0), cell2);
  BOOST_REQUIRE_EQUAL(table->elementAt(1, 0), cell0);
  BOOST_REQUIRE_NE(table->elementAt(2, 0), cell2);

  BOOST_REQUIRE_EQUAL(table->rowCount(), 6);
  CheckRowNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( moveRow2, * boost::unit_test::disabled() )
{
  auto table = cpp14::make_unique<WTable>();
  table->rowAt(5);

  WTableCell* cell2 = table->elementAt(2, 0);

  BOOST_REQUIRE_NO_THROW(table->moveRow(2, 7));

  BOOST_REQUIRE_NE(table->elementAt(2, 0), cell2);
  BOOST_REQUIRE_EQUAL(table->elementAt(7, 0), cell2);

  BOOST_REQUIRE_EQUAL(table->rowCount(), 8);
  CheckRowNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( moveColumn1 )
{
  auto table = cpp14::make_unique<WTable>();
  table->columnAt(5);

  WTableCell* cell2 = table->elementAt(0, 2);
  WTableCell* cell0 = table->elementAt(0, 0);

  BOOST_REQUIRE_NO_THROW(table->moveColumn(2, 0));

  BOOST_REQUIRE_EQUAL(table->elementAt(0, 0), cell2);
  BOOST_REQUIRE_EQUAL(table->elementAt(0, 1), cell0);
  BOOST_REQUIRE_NE(table->elementAt(0, 2), cell2);

  BOOST_REQUIRE_EQUAL(table->columnCount(), 6);
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( moveColumn2, * boost::unit_test::disabled() )
{
  auto table = cpp14::make_unique<WTable>();
  table->columnAt(5);

  WTableCell* cell2 = table->elementAt(0, 2);

  BOOST_REQUIRE_NO_THROW(table->moveColumn(2, 7));

  BOOST_REQUIRE_EQUAL(table->elementAt(0, 7), cell2);
  BOOST_REQUIRE_NE(table->elementAt(0, 2), cell2);

  BOOST_REQUIRE_EQUAL(table->columnCount(), 8);
  CheckColumnNumbers(table.get());
}

BOOST_AUTO_TEST_CASE( clear )
{
  auto table = cpp14::make_unique<WTable>();
  table->elementAt(4, 4);

  BOOST_REQUIRE_NO_THROW(table->clear());

  BOOST_REQUIRE_EQUAL(table->rowCount(), 0);
  BOOST_REQUIRE_EQUAL(table->columnCount(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
