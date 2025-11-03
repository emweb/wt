/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "utilities.h"

#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
#include <Wt/WIdentityProxyModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStringListModel.h>

namespace {
  void WIdentityProxyModel_insertColumns_append(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int addedColumns = 2;
    model->insertColumns(wrapper.getModelRows(), addedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->columnCount() == 1);
    } else {
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns() + addedColumns);
    }

    // No columns can be added to it
    if (wrapper.isListModel()) {
      addedColumns = 0;
    }

    for (int i = 0; i < model->rowCount(); ++i) {
      for (int j = 0; j < model->columnCount() - addedColumns; ++j) {
        WString value = wrapper.createValue(i, j);
        BOOST_TEST(wrapper.get(i, j) == value);
        BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(i, j))) == value);
      }
    }

    if (!wrapper.isListModel()) {
      for (int i = 0; i < model->rowCount(); ++i) {
        for (int j = 0; j < addedColumns; ++j) {
          BOOST_TEST(wrapper.get(i, MODEL_DIMENSION + j).empty());
          // Empty WStandardItem -> empty any
          BOOST_TEST(!cpp17::any_has_value(model->data(model->index(i, MODEL_DIMENSION + j))));
        }
      }
    }
  }

  void WIdentityProxyModel_insertColumns_insert(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = 1;
    int addedColumns = 2;
    model->insertColumns(editOffset, addedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->columnCount() == 1);
    } else {
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns() + addedColumns);
    }

    for (int i = 0; i < model->rowCount(); ++i) {
      for (int j = 0; j < model->columnCount(); ++j) {
        if (j >= editOffset && j < editOffset + addedColumns) {
          BOOST_TEST(wrapper.get(i, j).empty());
          // Empty WStandardItem -> empty any
          BOOST_TEST(!cpp17::any_has_value(model->data(model->index(i, j))));
        } else {
          int column = j;
          if (j >= editOffset + addedColumns) {
            column -= addedColumns;
          }

          WString value = wrapper.createValue(i, column);
          BOOST_TEST(wrapper.get(i, j) == value);
          BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(i, j))) == value);
        }
      }
    }
  }

  void WIdentityProxyModel_insertColumns_nonexistent(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = wrapper.getModelColumns() + 1;
    int addedColumns = 2;
    if (!wrapper.isListModel()) {
      BOOST_CHECK_THROW(model->insertColumns(editOffset, addedColumns), Wt::WException);
    } else {
      model->insertColumns(editOffset, addedColumns);
    }

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->columnCount() == 1);
    } else {
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());
    }

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WIdentityProxyModel_insertRows_append(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int addedRows = 2;
    model->insertRows(wrapper.getModelRows(), addedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows() + addedRows);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    for (int i = 0; i < model->rowCount() - addedRows; ++i) {
      for (int j = 0; j < model->columnCount(); ++j) {
        WString value = wrapper.createValue(i, j);
        BOOST_TEST(wrapper.get(i, j) == value);
        BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(i, j))) == value);
      }
    }

    for (int i = 0; i < addedRows; ++i) {
      for (int j = 0; j < model->columnCount(); ++j) {
        BOOST_TEST(wrapper.get(wrapper.getModelRows() + i, j).empty());
        if (!wrapper.isListModel()) {
          // Empty WStandardItem -> empty any
          BOOST_TEST(!cpp17::any_has_value(model->data(model->index(wrapper.getModelRows() + i, j))));
        }
      }
    }
  }

  void WIdentityProxyModel_insertRows_insert(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = 1;
    int addedRows = 2;
    model->insertRows(editOffset, addedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows() + addedRows);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    for (int i = 0; i < model->rowCount(); ++i) {
      for (int j = 0; j < model->columnCount(); ++j) {
        if (i >= editOffset && i < editOffset + addedRows) {
          BOOST_TEST(wrapper.get(i, j).empty());
          if (wrapper.isListModel()) {
            BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(i, j))).empty());
          } else {
            // Empty WStandardItem -> empty any
            BOOST_TEST(!cpp17::any_has_value(model->data(model->index(i, j))));
          }
        } else {
          int row = i;
          if (i >= editOffset + addedRows) {
            row -= addedRows;
          }

          WString value = wrapper.createValue(row, j);
          BOOST_TEST(wrapper.get(i, j) == value);
          BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(i, j))) == value);
        }
      }
    }
  }

  void WIdentityProxyModel_insertRows_nonexistent(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = wrapper.getModelRows() + 1;
    int addedRows = 2;
    BOOST_CHECK_THROW(model->insertRows(editOffset, addedRows), Wt::WException);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WIdentityProxyModel_removeColumns_none(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int removedColumns = 2;
    model->removeColumns(wrapper.getModelRows(), removedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WIdentityProxyModel_removeColumns_partial(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = wrapper.getModelColumns() - 1;
    int removedColumns = 2;
    model->removeColumns(editOffset, removedColumns);

    // String list doesn't implement removeColumns
    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

      return;
    }

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns() - 1);

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());

    for (int i = 0; i < model->rowCount(); ++i) {
      BOOST_CHECK_THROW(wrapper.get(i, editOffset), std::out_of_range);
      // WIdentity cannot resolve model's own out of bounds
      BOOST_TEST(!cpp17::any_has_value(model->data(wrapper.getModel()->index(i, editOffset))));
    }
  }

  void WIdentityProxyModel_removeColumns_full(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->removeColumns(0, wrapper.getModelColumns());

    // String list doesn't implement removeColumns
    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

      return;
    }

    BOOST_REQUIRE(model->rowCount() == 0);
    BOOST_REQUIRE(model->columnCount() == 0);

    BOOST_TEST(!cpp17::any_has_value(wrapper.getModel()->data(0, 0)));
    BOOST_TEST(!cpp17::any_has_value(model->data(model->index(0, 0))));
  }

  void WIdentityProxyModel_removeRows_none(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int removedRows = 2;
    model->removeRows(wrapper.getModelRows(), removedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WIdentityProxyModel_removeRows_partial(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = wrapper.getModelRows() - 1;
    int removedRows = 2;
    model->removeRows(editOffset, removedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows() - 1);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());

    for (int j = 0; j < model->columnCount(); ++j) {
      BOOST_CHECK_THROW(wrapper.get(editOffset, j), std::out_of_range);
    }
  }

  void WIdentityProxyModel_removeRows_full(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WIdentityProxyModel> model = std::make_unique<WIdentityProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->removeRows(0, wrapper.getModelRows());

    BOOST_REQUIRE(model->rowCount() == 0);
    BOOST_REQUIRE(model->columnCount() == 0);

    BOOST_CHECK_THROW(wrapper.get(0, 0), std::out_of_range);
  }
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_insertColumns_append_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_insertColumns_append(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_insertColumns_append(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_insertColumns_insert_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_insertColumns_insert(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_insertColumns_insert(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_insertColumns_nonexistent_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_insertColumns_nonexistent(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_insertColumns_nonexistent(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_insertRows_append_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_insertRows_append(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_insertRows_append(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_insertRows_insert_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_insertRows_insert(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_insertRows_insert(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_insertRows_nonexistent_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_insertRows_nonexistent(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_insertRows_nonexistent(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_removeColumns_none_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_removeColumns_none(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_removeColumns_none(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_removeColumns_partial_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_removeColumns_partial(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_removeColumns_partial(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_removeColumns_full_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_removeColumns_full(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_removeColumns_full(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_removeRows_none_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_removeRows_none(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_removeRows_none(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_removeRows_partial_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_removeRows_partial(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_removeRows_partial(wrapper);
}

BOOST_AUTO_TEST_CASE( WIdentityProxyModel_removeRows_full_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WIdentityProxyModel_removeRows_full(wrapper);

  wrapper.createListModel();
  WIdentityProxyModel_removeRows_full(wrapper);
}
