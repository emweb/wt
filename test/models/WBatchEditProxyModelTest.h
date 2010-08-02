// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBATCH_EDIT_PROXY_MODEL_TEST_H_
#define WBATCH_EDIT_PROXY_MODEL_TEST_H_

#include <Wt/WGlobal>
#include <Wt/WModelIndex>

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class WBatchEditProxyModelTest : public test_suite
{
public:
  WBatchEditProxyModelTest();

private:
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

  enum ModelType { SourceModel, ProxyModel };

  void geometryChanged(const Wt::WModelIndex& parent,
		       int start, int end, ModelType model, EventType type,
		       bool ended);

  void dataChanged(const Wt::WModelIndex& topLeft,
		   const Wt::WModelIndex& bottomRight, ModelType modelType);

  void headerDataChanged(Wt::Orientation orientation, int start, int end);

  void layoutAboutToBeChanged();
  void layoutChanged();

  void setup();
  void teardown();
  void connectEvents(Wt::WAbstractItemModel *model, ModelType modelType);
  void clearEvents();

  void test();
};

#endif // WBATCH_EDIT_PROXY_MODEL_TEST_H_
