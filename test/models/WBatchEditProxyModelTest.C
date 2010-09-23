// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WBatchEditProxyModel>

#include "WBatchEditProxyModelTest.h"

using namespace Wt;

namespace {
  std::string d(WAbstractItemModel *model, int row, int column,
		const WModelIndex& parent = WModelIndex())
  {
    return boost::any_cast<std::string>
      (model->data(row, column, DisplayRole, parent));
  }
}


void WBatchEditProxyModelTest::test()
{
  setup();

  WAbstractItemModel *sm = sourceModel_;
  WAbstractItemModel *pm = proxyModel_;

  BOOST_REQUIRE(sm->columnCount() == 4);
  BOOST_REQUIRE(pm->columnCount() == 4);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 0);

  pm->insertRows(0, 2);

  BOOST_REQUIRE(modelEvents_[SourceModel].size() == 0);
  BOOST_REQUIRE(modelEvents_[ProxyModel].size() == 1);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 2);

  clearEvents();

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

  BOOST_REQUIRE(modelEvents_[SourceModel].size() == 0);
  BOOST_REQUIRE(modelEvents_[ProxyModel].size() == 1);

  BOOST_REQUIRE(sm->rowCount() == 0);
  BOOST_REQUIRE(pm->rowCount() == 1);

  clearEvents();

  proxyModel_->commitAll();

  BOOST_REQUIRE(modelEvents_[SourceModel].size() == 1);
  BOOST_REQUIRE(modelEvents_[ProxyModel].size() == 0);

  BOOST_REQUIRE(sm->rowCount() == 1);
  BOOST_REQUIRE(pm->rowCount() == 1);

  clearEvents();

  sm->insertRows(1, 3);

  BOOST_REQUIRE(modelEvents_[SourceModel].size() == 1);
  BOOST_REQUIRE(modelEvents_[ProxyModel].size() == 3);

  BOOST_REQUIRE(sm->rowCount() == 4);
  BOOST_REQUIRE(pm->rowCount() == 4);

  clearEvents();

  pm->removeRow(2);
  pm->removeRow(1);

  BOOST_REQUIRE(pm->rowCount() == 2);

  BOOST_REQUIRE(modelEvents_[SourceModel].size() == 0);
  BOOST_REQUIRE(modelEvents_[ProxyModel].size() == 2);

  clearEvents();

  pm->setData(1, 0, std::string("sm(1, 0)"), EditRole);
  BOOST_REQUIRE(d(pm, 1, 0) == "sm(1, 0)");

  proxyModel_->commitAll();

  BOOST_REQUIRE(sm->rowCount() == 2);

  BOOST_REQUIRE(d(pm, 1, 0) == "sm(1, 0)");
  BOOST_REQUIRE(d(sm, 1, 0) == "sm(1, 0)");

  WModelIndex p = pm->index(1, 0);
  pm->insertColumns(0, 4, p);
  BOOST_REQUIRE(pm->columnCount(p) == 4);
  pm->insertRow(0, p);
  BOOST_REQUIRE(pm->rowCount(p) == 1);

  proxyModel_->commitAll();

  BOOST_REQUIRE(sm->rowCount(sm->index(1, 0)) == 1);
  BOOST_REQUIRE(sm->columnCount(sm->index(1, 0)) == 4);

  teardown();
}

void WBatchEditProxyModelTest::setup()
{
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

void WBatchEditProxyModelTest::connectEvents(WAbstractItemModel *model,
					     ModelType modelType)
{
  typedef WBatchEditProxyModelTest This;

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

void WBatchEditProxyModelTest::clearEvents()
{
  modelEvents_[0].clear();
  modelEvents_[1].clear();
}

void WBatchEditProxyModelTest::geometryChanged(const WModelIndex& parent,
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

void WBatchEditProxyModelTest::teardown()
{
  delete proxyModel_;
  delete sourceModel_;
}
  
WBatchEditProxyModelTest::WBatchEditProxyModelTest()
  : test_suite("WBatchEditProxyModel")
{
  add(BOOST_TEST_CASE(boost::bind(&WBatchEditProxyModelTest::test, this)));
}
