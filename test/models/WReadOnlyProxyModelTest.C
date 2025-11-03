/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "utilities.h"

#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
#include <Wt/WReadOnlyProxyModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStringListModel.h>
#include <Wt/WLogger.h>

namespace {
  void WReadOnlyProxyModel_insertColumns(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WReadOnlyProxyModel> model = std::make_unique<WReadOnlyProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int addedColumns = 2;
    model->insertColumns(wrapper.getModelColumns(), addedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WReadOnlyProxyModel_insertRows(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WReadOnlyProxyModel> model = std::make_unique<WReadOnlyProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int addedRows = 2;
    model->insertRows(wrapper.getModelRows(), addedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WReadOnlyProxyModel_removeColumns(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WReadOnlyProxyModel> model = std::make_unique<WReadOnlyProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = 1;
    int removedColumns = 2;
    model->removeColumns(editOffset, removedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WReadOnlyProxyModel_removeRows(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WReadOnlyProxyModel> model = std::make_unique<WReadOnlyProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = 1;
    int removedRows = 2;
    model->removeRows(editOffset, removedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }
}

BOOST_AUTO_TEST_CASE( WReadOnlyProxyModel_insertColumns_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WReadOnlyProxyModel_insertColumns(wrapper);

  wrapper.createListModel();
  WReadOnlyProxyModel_insertColumns(wrapper);
}

BOOST_AUTO_TEST_CASE( WReadOnlyProxyModel_insertRows_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WReadOnlyProxyModel_insertRows(wrapper);

  wrapper.createListModel();
  WReadOnlyProxyModel_insertColumns(wrapper);
}

BOOST_AUTO_TEST_CASE( WReadOnlyProxyModel_removeColumns_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WReadOnlyProxyModel_removeColumns(wrapper);

  wrapper.createListModel();
  WReadOnlyProxyModel_insertColumns(wrapper);
}

BOOST_AUTO_TEST_CASE( WReadOnlyProxyModel_removeRows_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WReadOnlyProxyModel_removeRows(wrapper);

  wrapper.createListModel();
  WReadOnlyProxyModel_insertColumns(wrapper);
}
