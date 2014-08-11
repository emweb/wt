/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WBatchEditProxyModel>

using namespace Wt;

enum ModelType { SourceModel, ProxyModel };

namespace {
  std::string d(WAbstractItemModel *model, int row, int column,
		const WModelIndex& parent = WModelIndex())
  {
    return boost::any_cast<std::string>
      (model->data(row, column, DisplayRole, parent));
  }
}

struct BatchEditFixture {
  BatchEditFixture() {
    sourceModel_ = new WStandardItemModel(0, 4);
    proxyModel_ = new WBatchEditProxyModel();
    proxyModel_->setSourceModel(sourceModel_);

    proxyModel_->setNewRowData(0, std::string("New column 0"));
    proxyModel_->setNewRowData(1, std::string("New column 1"));
    proxyModel_->setNewRowData(2, std::string("New column 2"));
    proxyModel_->setNewRowData(3, std::string("New column 3"));

    connectEvents(sourceModel_, SourceModel);
    connectEvents(proxyModel_, ProxyModel);
  }

  ~BatchEditFixture() {
    delete proxyModel_;
    delete sourceModel_;
  }

  Wt::WStandardItemModel *sourceModel_;
  Wt::WBatchEditProxyModel *proxyModel_;

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

  void connectEvents(WAbstractItemModel *model,
		     ModelType modelType)
  {
    typedef BatchEditFixture This;

    model->rowsAboutToBeInserted().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsInserted, false));
    model->rowsInserted().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsInserted, true));

    model->rowsAboutToBeRemoved().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsRemoved, false));
    model->rowsRemoved().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, RowsRemoved, true));

    model->columnsAboutToBeInserted().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, ColumnsInserted, false));
    model->columnsInserted().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, ColumnsInserted, true));

    model->columnsAboutToBeRemoved().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
		   modelType, ColumnsRemoved, false));
    model->columnsRemoved().connect
      (boost::bind(&This::geometryChanged, this, _1, _2, _3,
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

  WAbstractItemModel *sm = f.sourceModel_;
  WAbstractItemModel *pm = f.proxyModel_;

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

  pm->setData(1, 0, std::string("Column 0"), DisplayRole);
  pm->setData(1, 1, std::string("Column 1"), DisplayRole);
  pm->setData(1, 2, std::string("Column 2"), DisplayRole);
  pm->setData(1, 3, std::string("Column 3"), DisplayRole);

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

  pm->removeRow(2);
  pm->removeRow(1);

  BOOST_REQUIRE(pm->rowCount() == 2);

  BOOST_REQUIRE(f.modelEvents_[SourceModel].size() == 0);
  BOOST_REQUIRE(f.modelEvents_[ProxyModel].size() == 2);

  f.clearEvents();

  pm->setData(1, 0, std::string("sm(1, 0)"), EditRole);
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

  WAbstractItemModel *sm = f.sourceModel_;
  WAbstractItemModel *pm = f.proxyModel_;

  BOOST_REQUIRE(sm->columnCount() == 4);
  BOOST_REQUIRE(pm->columnCount() == 4);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 0);

  sm->insertRows(0, 2);

  BOOST_REQUIRE(sm->rowCount() == 2);
  BOOST_REQUIRE(pm->rowCount() == 2);

  f.clearEvents();

  pm->setData(0, 0, std::string("sm(1, 0)"), EditRole);
  pm->setData(1, 0, std::string("sm(2, 0)"), EditRole);
  BOOST_REQUIRE(d(pm, 0, 0) == "sm(1, 0)");
  BOOST_REQUIRE(d(pm, 1, 0) == "sm(2, 0)");

  pm->insertRows(2, 1);
  pm->setData(2, 0, std::string("sm(3, 0)"), EditRole);

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

  WStandardItemModel *sm = f.sourceModel_;
  WBatchEditProxyModel *pm = f.proxyModel_;

  pm->insertRows(0, 3);
  pm->commitAll();

  sm->item(0)->setFlags(ItemIsSelectable);
  sm->item(1)->setFlags(ItemIsEditable);
  sm->item(2)->setFlags(ItemIsUserCheckable);

  pm->setNewRowFlags(0, ItemIsDragEnabled);
  pm->insertRows(1, 1);

  BOOST_REQUIRE(pm->flags(pm->index(0, 0)) == ItemIsSelectable);
  BOOST_REQUIRE(pm->flags(pm->index(1, 0)) == ItemIsDragEnabled);
  BOOST_REQUIRE(pm->flags(pm->index(2, 0)) == ItemIsEditable);
  BOOST_REQUIRE(pm->flags(pm->index(3, 0)) == ItemIsUserCheckable);
}
