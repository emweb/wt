#include <boost/test/unit_test.hpp>

#include <Wt/WContainerWidget.h>
#include <Wt/WModelIndex.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTreeView.h>

#include <Wt/Test/WTestEnvironment.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( treeview_test1 )
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = std::make_shared<WStandardItemModel>();

  auto root = model->invisibleRootItem();
  for (int i = 0; i < 6; ++i) {
    auto item = std::make_unique<WStandardItem>(Wt::utf8("level 1, row {1}").arg(i));
    for (int j = 0; j < 4; ++j) {
      auto subItem = std::make_unique<WStandardItem>(Wt::utf8("level 2, row {1}").arg(j));
      for (int k = 0; k < 3; ++k) {
        auto subsubItem = std::make_unique<WStandardItem>(Wt::utf8("level 3, row {1}").arg(k));
        subsubItem->appendRow(std::make_unique<WStandardItem>(Wt::utf8("level 4")));
        subItem->appendRow(std::move(subsubItem));
      }
      item->appendRow(std::move(subItem));
    }
    root->appendRow(std::move(item));
  }

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  tree->expand(model->index(2, 0));
  tree->expand(model->index(1, 0, model->index(2, 0)));
  tree->expand(model->index(0, 0, model->index(1, 0, model->index(2, 0))));
  tree->expand(model->index(3, 0));

  BOOST_REQUIRE(!tree->isExpanded(model->index(0, 0)));
  BOOST_REQUIRE(!tree->isExpanded(model->index(1, 0)));
  BOOST_REQUIRE(tree->isExpanded(model->index(2, 0)));
  BOOST_REQUIRE(tree->isExpanded(model->index(1, 0, model->index(2, 0))));
  BOOST_REQUIRE(tree->isExpanded(model->index(0, 0, model->index(1, 0, model->index(2, 0)))));
  BOOST_REQUIRE(tree->isExpanded(model->index(3, 0)));
  BOOST_REQUIRE(!tree->isExpanded(model->index(4, 0)));
  BOOST_REQUIRE(!tree->isExpanded(model->index(5, 0)));

  model->removeRows(2, 2);

  BOOST_REQUIRE(!tree->isExpanded(model->index(0, 0)));
  BOOST_REQUIRE(!tree->isExpanded(model->index(1, 0)));
  BOOST_REQUIRE(!tree->isExpanded(model->index(2, 0)));
  BOOST_REQUIRE(!tree->isExpanded(model->index(3, 0)));
  BOOST_REQUIRE(!tree->isExpanded(model->index(1, 0, model->index(2, 0))));
  BOOST_REQUIRE(!tree->isExpanded(model->index(0, 0, model->index(1, 0, model->index(2, 0)))));
}

namespace {
  std::shared_ptr<WStandardItemModel> createModel(int rows, int cols)
  {
      std::shared_ptr<WStandardItemModel> model = std::make_shared<WStandardItemModel>(rows, cols);

      for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
          model->item(row, col)->setText(WString("Row: {1} - Col: {2}").arg(row).arg(col));
        }
      }

      BOOST_REQUIRE(model->rowCount() == rows);
      BOOST_REQUIRE(model->columnCount() == cols);

      return model;
  }
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsInserted_none )
{
  // Tests whether inserting 0 columns does not change the model.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->insertColumns(1, 0);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsInserted_one )
{
  // Tests whether inserting 1 column changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->insertColumn(1);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 3);
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsInserted_multi )
{
  // Tests whether inserting multiple columns changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->insertColumns(1, 2);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 4);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsInserted_none )
{
  // Tests whether inserting 0 rows does not change the model.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->insertRows(1, 0);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsInserted_one )
{
  // Tests whether inserting 1 row changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->insertRow(1);

  BOOST_TEST(model->rowCount() == 3);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsInserted_multi )
{
  // Tests whether inserting multiple rows changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->insertRows(1, 2);

  BOOST_TEST(model->rowCount() == 4);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsRemoved_none )
{
  // Tests whether removing 0 columns does not change the model.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeColumns(1, 0);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsRemoved_none_out_of_range )
{
  // Tests whether removing a column that is out of range does not change the model.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeColumns(2, 2);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsRemoved_one )
{
  // Tests whether removing 1 column changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeColumn(1);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 1);
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsRemoved_multi )
{
  // Tests whether removing multiple columns changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeColumns(0, 2);

  BOOST_TEST(model->rowCount() == 0);
  BOOST_TEST(model->columnCount() == 0);
}

BOOST_AUTO_TEST_CASE( treeview_modelColumnsRemoved_multi_partially_out_of_range )
{
  // Tests whether removing multiple columns changes the model correctly,
  // while correcly ignoring any columns that are out of range.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeColumns(1, 2);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 1);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsRemoved_none )
{
  // Tests whether removing 0 rows does not change the model.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeRows(1, 0);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsRemoved_none_out_of_range )
{
  // Tests whether removing a row that is out of range does not change the model.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeRows(2, 2);

  BOOST_TEST(model->rowCount() == 2);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsRemoved_one )
{
  // Tests whether removing 1 row changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeRow(1);

  BOOST_TEST(model->rowCount() == 1);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsRemoved_multi )
{
  // Tests whether removing multiple columns changes the model correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeRows(0, 2);

  BOOST_TEST(model->rowCount() == 0);
  BOOST_TEST(model->columnCount() == 2);
}

BOOST_AUTO_TEST_CASE( treeview_modelRowsRemoved_multi_partially_out_of_range )
{
  // Tests whether removing multiple columns changes the model correctly,
  // while correcly ignoring any columns that are out of range.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto model = createModel(2, 2);

  auto tree = app.root()->addNew<Wt::WTreeView>();
  tree->setModel(model);

  model->removeRows(1, 2);

  BOOST_TEST(model->rowCount() == 1);
  BOOST_TEST(model->columnCount() == 2);
}
