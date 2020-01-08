/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>
#include <iostream>

#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WBatchEditProxyModel.h>

using namespace Wt;

enum ModelType { SourceModel, ProxyModel };

namespace {
  std::string d(WAbstractItemModel *model, int row, int column,
		const WModelIndex& parent = WModelIndex())
  {
    return cpp17::any_cast<std::string>
      (model->data(row, column, ItemDataRole::Display, parent));
  }
}

struct BatchEditFixture {
  BatchEditFixture() {
    sourceModel_ = std::make_shared<WStandardItemModel>(0, 4);
    proxyModel_ = std::make_shared<WBatchEditProxyModel>();
    proxyModel_->setSourceModel(sourceModel_);

    proxyModel_->setNewRowData(0, std::string("New column 0"));
    proxyModel_->setNewRowData(1, std::string("New column 1"));
    proxyModel_->setNewRowData(2, std::string("New column 2"));
    proxyModel_->setNewRowData(3, std::string("New column 3"));

    connectEvents(sourceModel_, SourceModel);
    connectEvents(proxyModel_, ProxyModel);
  }

  std::shared_ptr<Wt::WStandardItemModel> sourceModel_;
  std::shared_ptr<Wt::WBatchEditProxyModel> proxyModel_;

  enum EventType { 
    RowsInserted,
    ColumnsInserted,
    RowsRemoved,
    ColumnsRemoved,
    DataChanged
  };

  struct ModelEvent {
    Wt::WModelIndex index;
    int start, end;
    bool ended;
    EventType type;
  };

  std::vector<ModelEvent> modelEvents_[2];

  void connectEvents(std::shared_ptr<WAbstractItemModel> model,
		     ModelType modelType)
  {
    typedef BatchEditFixture This;
    using namespace std::placeholders;

    model->rowsAboutToBeInserted().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsInserted, false));
    model->rowsInserted().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsInserted, true));

    model->rowsAboutToBeRemoved().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsRemoved, false));
    model->rowsRemoved().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsRemoved, true));

    model->columnsAboutToBeInserted().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, ColumnsInserted, false));
    model->columnsInserted().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, ColumnsInserted, true));

    model->columnsAboutToBeRemoved().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, ColumnsRemoved, false));
    model->columnsRemoved().connect
      (std::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, ColumnsRemoved, true));
  }


  void clearEvents()
  {
    modelEvents_[0].clear();
    modelEvents_[1].clear();
  }

  void geometryChanged(const WModelIndex& parent,
		       int start, int end,
		       ModelType model, EventType type,
		       bool ended)
  {
    if (!ended) {
      ModelEvent event;
      event.index = parent;
      event.start = start;
      event.end = end;
      event.type = type;
      event.ended = false;

      modelEvents_[model].push_back(event);
    } else {
      for (int i = (int)(modelEvents_[model].size()) - 1; i >= 0; --i) {
	ModelEvent& e = modelEvents_[model][i];
	if (e.type == type
	    && e.index == parent
	    && e.start == start
	    && e.end == end
	    && !e.ended) {
	  e.ended = true;
	  return;
	}
      }

      BOOST_FAIL("Non-matched geometry ending event");
    }
  }

};

BOOST_AUTO_TEST_CASE( batchedit_test1 )
{
  BatchEditFixture f;

  auto sm = f.sourceModel_.get();
  auto pm = f.proxyModel_.get();

  BOOST_REQUIRE(sm->columnCount() == 4);
  BOOST_REQUIRE(pm->columnCount() == 4);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 0);

  pm->insertRows(0, 2);

  BOOST_REQUIRE(f.modelEvents_[SourceModel].size() == 0);
  BOOST_REQUIRE(f.modelEvents_[ProxyModel].size() == 1);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 2);

  f.clearEvents();

  BOOST_REQUIRE(d(pm, 0, 0) == "New column 0");
  BOOST_REQUIRE(d(pm, 0, 1) == "New column 1");
  BOOST_REQUIRE(d(pm, 0, 2) == "New column 2");
  BOOST_REQUIRE(d(pm, 0, 3) == "New column 3");

  pm->setData(1, 0, std::string("Column 0"), ItemDataRole::Display);
  pm->setData(1, 1, std::string("Column 1"), ItemDataRole::Display);
  pm->setData(1, 2, std::string("Column 2"), ItemDataRole::Display);
  pm->setData(1, 3, std::string("Column 3"), ItemDataRole::Display);

  BOOST_REQUIRE(d(pm, 1, 0) == "Column 0");
  BOOST_REQUIRE(d(pm, 1, 1) == "Column 1");
  BOOST_REQUIRE(d(pm, 1, 2) == "Column 2");
  BOOST_REQUIRE(d(pm, 1, 3) == "Column 3");

  pm->removeRow(1);

  BOOST_REQUIRE(f.modelEvents_[SourceModel].size() == 0);
  BOOST_REQUIRE(f.modelEvents_[ProxyModel].size() == 1);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 1);

  f.clearEvents();

  f.proxyModel_->commitAll();

  BOOST_REQUIRE(f.modelEvents_[SourceModel].size() == 1);
  BOOST_REQUIRE(f.modelEvents_[ProxyModel].size() == 0);

  BOOST_REQUIRE(sm->rowCount() == 1);
  BOOST_REQUIRE(pm->rowCount() == 1);

  f.clearEvents();

  sm->insertRows(1, 3);

  BOOST_REQUIRE(f.modelEvents_[SourceModel].size() == 1);
  BOOST_REQUIRE(f.modelEvents_[ProxyModel].size() == 3);

  BOOST_REQUIRE(sm->rowCount() == 4);
  BOOST_REQUIRE(pm->rowCount() == 4);

  f.clearEvents();

  pm->removeRow(3);
  pm->removeRow(1);

  BOOST_REQUIRE(pm->rowCount() == 2);

  BOOST_REQUIRE(f.modelEvents_[SourceModel].size() == 0);
  BOOST_REQUIRE(f.modelEvents_[ProxyModel].size() == 2);

  f.clearEvents();

  pm->setData(1, 0, std::string("sm(1, 0)"), ItemDataRole::Edit);
  BOOST_REQUIRE(d(pm, 1, 0) == "sm(1, 0)");

  f.proxyModel_->commitAll();

  BOOST_REQUIRE(sm->rowCount() == 2);

  BOOST_REQUIRE(d(pm, 1, 0) == "sm(1, 0)");
  BOOST_REQUIRE(d(sm, 1, 0) == "sm(1, 0)");

  WModelIndex p = pm->index(1, 0);
  pm->insertColumns(0, 4, p);
  BOOST_REQUIRE(pm->columnCount(p) == 4);
  pm->insertRow(0, p);
  BOOST_REQUIRE(pm->rowCount(p) == 1);

  f.proxyModel_->commitAll();

  BOOST_REQUIRE(sm->rowCount(sm->index(1, 0)) == 1);
  BOOST_REQUIRE(sm->columnCount(sm->index(1, 0)) == 4);
}


BOOST_AUTO_TEST_CASE( batchedit_test2 )
{
  BatchEditFixture f;

  auto sm = f.sourceModel_.get();
  auto pm = f.proxyModel_.get();

  BOOST_REQUIRE(sm->columnCount() == 4);
  BOOST_REQUIRE(pm->columnCount() == 4);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 0);

  sm->insertRows(0, 2);

  BOOST_REQUIRE(sm->rowCount() == 2);
  BOOST_REQUIRE(pm->rowCount() == 2);

  f.clearEvents();

  pm->setData(0, 0, std::string("sm(1, 0)"), ItemDataRole::Edit);
  pm->setData(1, 0, std::string("sm(2, 0)"), ItemDataRole::Edit);
  BOOST_REQUIRE(d(pm, 0, 0) == "sm(1, 0)");
  BOOST_REQUIRE(d(pm, 1, 0) == "sm(2, 0)");

  pm->insertRows(2, 1);
  pm->setData(2, 0, std::string("sm(3, 0)"), ItemDataRole::Edit);

  BOOST_REQUIRE(sm->rowCount() == 2);
  BOOST_REQUIRE(pm->rowCount() == 3);

  pm->removeRow(0);

  BOOST_REQUIRE(sm->rowCount() == 2);
  BOOST_REQUIRE(pm->rowCount() == 2);

  BOOST_REQUIRE(d(pm, 0, 0) == "sm(2, 0)");
  BOOST_REQUIRE(d(pm, 1, 0) == "sm(3, 0)");

  f.proxyModel_->commitAll();

  BOOST_REQUIRE(sm->rowCount() == 2);

  BOOST_REQUIRE(d(sm, 0, 0) == "sm(2, 0)");
  BOOST_REQUIRE(d(sm, 1, 0) == "sm(3, 0)");

  BOOST_REQUIRE(d(pm, 0, 0) == "sm(2, 0)");
  BOOST_REQUIRE(d(pm, 1, 0) == "sm(3, 0)");
}

BOOST_AUTO_TEST_CASE( batchedit_test3 )
{
  // Test flags
  BatchEditFixture f;

  WStandardItemModel *sm = f.sourceModel_.get();
  WBatchEditProxyModel *pm = f.proxyModel_.get();

  pm->insertRows(0, 3);
  pm->commitAll();

  sm->item(0)->setFlags(ItemFlag::Selectable);
  sm->item(1)->setFlags(ItemFlag::Editable);
  sm->item(2)->setFlags(ItemFlag::UserCheckable);

  pm->setNewRowFlags(0, ItemFlag::DragEnabled);
  pm->insertRows(1, 1);

  BOOST_REQUIRE(pm->flags(pm->index(0, 0)) == ItemFlag::Selectable);
  BOOST_REQUIRE(pm->flags(pm->index(1, 0)) == ItemFlag::DragEnabled);
  BOOST_REQUIRE(pm->flags(pm->index(2, 0)) == ItemFlag::Editable);
  BOOST_REQUIRE(pm->flags(pm->index(3, 0)) == ItemFlag::UserCheckable);
}
