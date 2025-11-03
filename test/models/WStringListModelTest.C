/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
#include <Wt/WStringListModel.h>

using namespace Wt;

namespace {
  std::unique_ptr<WStringListModel> createPopulatedModel(int rows)
  {
    std::unique_ptr<WStringListModel> model = std::make_unique<WStringListModel>();

    for (int row = 0; row < rows; ++row) {
      model->insertString(row, WString("Row: {1}").arg(row));
    }

    BOOST_REQUIRE(model->rowCount() == rows);
    BOOST_REQUIRE(model->columnCount() == (rows > 0) ? 1 : 0);
    BOOST_REQUIRE(static_cast<int>(model->stringList().size()) == rows);

    return model;
  }
}

BOOST_AUTO_TEST_CASE( WStringListModel_index_test )
{
  // Tests whether the model correctly retrieves indices,
  // and "corrects" out-of-range columns

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  WModelIndex i1 = model->index(0, 0);
  WModelIndex i2 = model->index(1, 0);
  WModelIndex i3 = model->index(0, 1);
  WModelIndex i4 = model->index(2, 0);

  BOOST_TEST(i1.row() == 0);
  BOOST_TEST(i1.column() == 0);
  BOOST_TEST(i2.row() == 1);
  BOOST_TEST(i2.column() == 0);
  BOOST_TEST(i3.row() == 0);
  BOOST_TEST(i3.column() == 0);
  BOOST_TEST(i4.row() == 0);
  BOOST_TEST(i4.column() == 0);
}

BOOST_AUTO_TEST_CASE( WStringListModel_setStringList_test )
{
  // Tests whether the model correctly resets it content

  std::unique_ptr<WStringListModel> model = createPopulatedModel(0);
  std::vector<WString> items;
  items.emplace_back();
  items.emplace_back();
  items.emplace_back();

  model->setStringList(items);

  BOOST_REQUIRE(model->rowCount() == 3);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 3);
  BOOST_TEST(model->stringList()[0].empty());
  BOOST_TEST(model->stringList()[1].empty());
  BOOST_TEST(model->stringList()[2].empty());
}

BOOST_AUTO_TEST_CASE( WStringListModel_addString_test )
{
  // Tests whether the model correctly adds/appends items

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  model->addString("");

  BOOST_REQUIRE(model->rowCount() == 3);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 3);
  BOOST_TEST(model->stringList()[0] == "Row: 0");
  BOOST_TEST(model->stringList()[1] == "Row: 1");
  BOOST_TEST(model->stringList()[2].empty());
}

BOOST_AUTO_TEST_CASE( WStringListModel_insertString_test )
{
  // Tests whether the model correctly adds/inserts items

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  model->insertString(1, "");

  BOOST_REQUIRE(model->rowCount() == 3);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 3);
  BOOST_TEST(model->stringList()[0] == "Row: 0");
  BOOST_TEST(model->stringList()[1].empty());
  BOOST_TEST(model->stringList()[2] == "Row: 1");
}

BOOST_AUTO_TEST_CASE( WStringListModel_insertRows_insert_test )
{
  // Tests whether the model correctly inserts multiple rows

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  bool inserted = model->insertRows(1, 2);

  BOOST_REQUIRE(inserted);
  BOOST_REQUIRE(model->rowCount() == 4);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 4);
  BOOST_TEST(model->stringList()[0] == "Row: 0");
  BOOST_TEST(model->stringList()[1].empty());
  BOOST_TEST(model->stringList()[2].empty());
  BOOST_TEST(model->stringList()[3] == "Row: 1");
}

BOOST_AUTO_TEST_CASE( WStringListModel_insertRows_append_test )
{
  // Tests whether the model correctly appends multiple rows

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  bool inserted = model->insertRows(2, 2);

  BOOST_REQUIRE(inserted);
  BOOST_REQUIRE(model->rowCount() == 4);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 4);
  BOOST_TEST(model->stringList()[0] == "Row: 0");
  BOOST_TEST(model->stringList()[1] == "Row: 1");
  BOOST_TEST(model->stringList()[2].empty());
  BOOST_TEST(model->stringList()[3].empty());
}

BOOST_AUTO_TEST_CASE( WStringListModel_insertRows_nonexisting_test )
{
  // Tests whether the model correctly appends multiple rows, when the
  // index of the first insertion exceeds the current model's bounds.

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  BOOST_CHECK_THROW(model->insertRows(3, 2), Wt::WException);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 2);
  BOOST_TEST(model->stringList()[0] == "Row: 0");
  BOOST_TEST(model->stringList()[1] == "Row: 1");
}

BOOST_AUTO_TEST_CASE( WStringListModel_removeRows_full_test )
{
  // Tests whether the model correctly removes multiple rows

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  bool removed = model->removeRows(0, 2);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 0);
  BOOST_REQUIRE(model->columnCount() == 0);

  BOOST_REQUIRE(model->stringList().empty());
}

BOOST_AUTO_TEST_CASE( WStringListModel_removeRows_partial_test )
{
  // Tests whether the model correctly removes a single row at the end

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  bool removed = model->removeRows(1, 2);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 1);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 1);
  BOOST_TEST(model->stringList()[0] == "Row: 0");
}

BOOST_AUTO_TEST_CASE( WStringListModel_removeRows_none_test )
{
  // Tests whether the model correctly ignores removing out-of-range indices

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  bool removed = model->removeRows(3, 1);

  BOOST_REQUIRE(!removed);
  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_REQUIRE(model->stringList().size() == 2);
  BOOST_TEST(model->stringList()[0] == "Row: 0");
  BOOST_TEST(model->stringList()[1] == "Row: 1");
}

BOOST_AUTO_TEST_CASE( WStringListModel_sort_test )
{
  // Tests whether the model correctly sorts items

  std::unique_ptr<WStringListModel> model = createPopulatedModel(2);

  // Column is not used
  model->sort(1, SortOrder::Ascending);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_TEST(model->stringList()[0] == "Row: 0");
  BOOST_TEST(model->stringList()[1] == "Row: 1");

  // Column is not used
  model->sort(5, SortOrder::Descending);

  BOOST_TEST(model->stringList()[0] == "Row: 1");
  BOOST_TEST(model->stringList()[1] == "Row: 0");
}
