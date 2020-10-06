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
