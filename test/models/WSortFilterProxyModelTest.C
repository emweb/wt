/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "utilities.h"

#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
#include <Wt/WSortFilterProxyModel.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStringListModel.h>
#include <Wt/WLogger.h>

using namespace Wt;

namespace {
  void WSortFilterProxyModel_insertColumns_append(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int addedColumns = 2;
    model->insertColumns(wrapper.getModelColumns(), addedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());
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
          BOOST_TEST(wrapper.get(i, wrapper.getModelColumns() + j).empty());
          // Empty WStandardItem -> empty any
          BOOST_TEST(!cpp17::any_has_value(model->data(model->index(i, wrapper.getModelColumns() + j))));
        }
      }
    }
  }

  void WSortFilterProxyModel_insertColumns_insert(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = 1;
    int addedColumns = 2;
    model->insertColumns(editOffset, addedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());
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

  void WSortFilterProxyModel_insertColumns_nonexistent(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = wrapper.getModelColumns() + 1;
    int addedColumns = 2;
    if (!wrapper.isListModel()) {
      BOOST_CHECK_THROW(model->insertColumns(editOffset, addedColumns), Wt::WException);
    } else {
      model->insertColumns(editOffset, addedColumns);
    }

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(wrapper.getModelRows(), wrapper.getModelColumns(), model.get());
  }

  void WSortFilterProxyModel_insertRows_append(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int addedRows = 2;
    model->insertRows(wrapper.getModelRows(), addedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows() + addedRows);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount() - addedRows, model->columnCount(), model.get());

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

  void WSortFilterProxyModel_insertRows_insert(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
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
          if (!wrapper.isListModel()) {
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

  void WSortFilterProxyModel_insertRows_nonexistent(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = wrapper.getModelRows() + 1;
    int addedRows = 2;
    model->insertRows(editOffset, addedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows() + addedRows);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(wrapper.getModelRows(), wrapper.getModelColumns(), model.get());
  }

  void WSortFilterProxyModel_removeColumns_none(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int removedColumns = 2;
    model->removeColumns(wrapper.getModelColumns(), removedColumns);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WSortFilterProxyModel_removeColumns_partial(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
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
      BOOST_TEST(!cpp17::any_has_value(model->data(model->index(i, editOffset))));
    }
  }

  void WSortFilterProxyModel_removeColumns_full(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
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

    BOOST_CHECK_THROW(wrapper.get(0, 0), std::out_of_range);
    BOOST_TEST(!cpp17::any_has_value(model->data(model->index(0, 0))));
  }

  void WSortFilterProxyModel_removeRows_none(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int removedRows = 2;
    model->removeRows(wrapper.getModelRows(), removedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());
  }

  void WSortFilterProxyModel_removeRows_partial(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    int editOffset = wrapper.getModelRows() - 1;
    int removedRows = 2;
    model->removeRows(editOffset, removedRows);

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows() - 1);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.testUnchangedRange(model->rowCount(), model->columnCount(), model.get());

    for (int j = 0; j < model->columnCount(); ++j) {
      BOOST_CHECK_THROW(wrapper.get(editOffset, j), std::out_of_range);
      if (!wrapper.isListModel()) {
        // Empty WStandardItem -> empty any
        BOOST_TEST(!cpp17::any_has_value(model->data(model->index(editOffset, j))));
      }
    }
  }

  void WSortFilterProxyModel_removeRows_full(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->removeRows(0, wrapper.getModelRows());

    BOOST_REQUIRE(model->rowCount() == 0);
    BOOST_REQUIRE(model->columnCount() == 0);

    BOOST_CHECK_THROW(wrapper.get(0, 0), std::out_of_range);
  }

  class FilterModel : public WSortFilterProxyModel
  {
  public:
    FilterModel()
      : WSortFilterProxyModel()
    {
    }

    bool filterAcceptRow(int row, const WModelIndex& index) const override
    {
      return WSortFilterProxyModel::filterAcceptRow(row, index);
    }
  };

  void WSortFilterProxyModel_setFilter(SourceModelWrapper wrapper)
  {
    std::unique_ptr<FilterModel> model = std::make_unique<FilterModel>();
    model->setSourceModel(wrapper.getModel());

    model->setFilterKeyColumn(1);
    model->setFilterRole(ItemDataRole::Display);
    model->setFilterRegExp(std::make_unique<std::regex>(".*4.*"));

    BOOST_REQUIRE(model->rowCount() == 1);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    for (int i = 0; i < wrapper.getModelRows(); ++i) {
      if (i == 4) {
        BOOST_TEST(model->filterAcceptRow(i, WModelIndex()));
      } else {
        BOOST_TEST(!model->filterAcceptRow(i, WModelIndex()));
      }
    }
  }

  void WSortFilterProxyModel_setFilter_non_dynamic(SourceModelWrapper wrapper)
  {
    std::unique_ptr<FilterModel> model = std::make_unique<FilterModel>();
    model->setSourceModel(wrapper.getModel());

    model->setFilterKeyColumn(1);
    model->setFilterRole(ItemDataRole::Display);
    model->setFilterRegExp(std::make_unique<std::regex>(".*4.*"));

    BOOST_REQUIRE(model->rowCount() == 1);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    if (wrapper.isListModel()) {
      std::shared_ptr<WStringListModel> model = std::static_pointer_cast<WStringListModel>(wrapper.getModel());
      model->addString("Part of filter -> 4");
    } else {
      std::shared_ptr<WStandardItemModel> model = std::static_pointer_cast<WStandardItemModel>(wrapper.getModel());
      std::vector<std::unique_ptr<WStandardItem>> items;
      items.push_back(std::make_unique<WStandardItem>("Not filtered"));
      items.push_back(std::make_unique<WStandardItem>("Part of filter -> 4"));
      items.push_back(std::make_unique<WStandardItem>("Not filtered"));
      items.push_back(std::make_unique<WStandardItem>("Not filtered"));
      items.push_back(std::make_unique<WStandardItem>("Not filtered"));
      model->insertRow(1, std::move(items));
    }

    BOOST_REQUIRE(model->rowCount() == 1);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());
  }

  void WSortFilterProxyModel_setFilter_dynamic_row(SourceModelWrapper wrapper)
  {
    std::unique_ptr<FilterModel> model = std::make_unique<FilterModel>();
    model->setSourceModel(wrapper.getModel());

    model->setDynamicSortFilter(true);
    model->setFilterKeyColumn(1);
    model->setFilterRole(ItemDataRole::Display);
    model->setFilterRegExp(std::make_unique<std::regex>(".*4.*"));

    BOOST_REQUIRE(model->rowCount() == 1);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.getModel()->insertRow(wrapper.getModelRows());
    // This sets the (5, 1) value, which resolves to (5, 0) in the
    // WStringListModel.
    wrapper.getModel()->setData(wrapper.getModelRows(), 1, "Part of filter -> 4", ItemDataRole::Display);

    BOOST_REQUIRE(model->rowCount() == 2);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    for (int i = 0; i < wrapper.getModelRows() + 1; ++i) {
      if (i == 4 || i == 5) {
        BOOST_TEST(model->filterAcceptRow(i, WModelIndex()));
      } else {
        BOOST_TEST(!model->filterAcceptRow(i, WModelIndex()));
      }
    }
  }

  void WSortFilterProxyModel_setFilter_dynamic_column(SourceModelWrapper wrapper)
  {
    std::unique_ptr<FilterModel> model = std::make_unique<FilterModel>();
    model->setSourceModel(wrapper.getModel());

    model->setDynamicSortFilter(true);
    model->setFilterKeyColumn(1);
    model->setFilterRole(ItemDataRole::Display);
    model->setFilterRegExp(std::make_unique<std::regex>(".*4.*"));

    BOOST_REQUIRE(model->rowCount() == 1);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    wrapper.getModel()->insertColumn(1);
    // This sets the (0, 1) value, which resolves to (0, 0) in the
    // WStringListModel.
    wrapper.getModel()->setData(0, 1, "Part of filter -> 4", ItemDataRole::Display);

    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->rowCount() == 2);
    } else {
      BOOST_REQUIRE(model->rowCount() == 1);
    }

    if (wrapper.isListModel()) {
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());
    } else {
      BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns() + 1);
    }

    for (int i = 0; i < wrapper.getModelRows(); ++i) {
      // WStringListModel doesn't "guard" against too big columns, as
      // this would throw too deep down the chain.
      // It instead makes e.g. (0, 1)  resolve to (0, 0).
      if ((wrapper.isListModel() && (i == 0 || i == 4)) ||
          (!wrapper.isListModel() && i == 0)) {
        BOOST_TEST(model->filterAcceptRow(i, WModelIndex()));
      } else {
        BOOST_TEST(!model->filterAcceptRow(i, WModelIndex()));
      }
    }
  }

  void WSortFilterProxyModel_sort(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->sort(1, SortOrder::Descending);

    BOOST_REQUIRE(model->rowCount() == 5);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    // Unaltered
    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(0, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(1, 0));
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(0, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(1, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(4, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(3, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 1))) == wrapper.createValue(4, 1));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 1))) == wrapper.createValue(3, 1));

    model->sort(1, SortOrder::Ascending);

    BOOST_REQUIRE(model->rowCount() == 5);
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());

    // Unaltered
    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(0, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(1, 0));
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(0, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(1, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(0, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(1, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 1))) == wrapper.createValue(0, 1));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 1))) == wrapper.createValue(1, 1));
  }

  void WSortFilterProxyModel_invalidate(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WSortFilterProxyModel> model = std::make_unique<WSortFilterProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->invalidate();

    BOOST_REQUIRE(model->rowCount() == wrapper.getModelRows());
    BOOST_REQUIRE(model->columnCount() == wrapper.getModelColumns());
  }
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_insertColumns_append_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_insertColumns_append(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_insertColumns_append(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_insertColumns_insert_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_insertColumns_insert(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_insertColumns_insert(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_insertColumns_nonexistent_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_insertColumns_nonexistent(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_insertColumns_nonexistent(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_insertRows_append_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_insertRows_append(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_insertRows_append(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_insertRows_insert_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_insertRows_insert(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_insertRows_insert(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_insertRows_nonexistent_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_insertRows_nonexistent(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_insertRows_nonexistent(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_removeColumns_none_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_removeColumns_none(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_removeColumns_none(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_removeColumns_partial_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_removeColumns_partial(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_removeColumns_partial(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_removeColumns_full_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_removeColumns_full(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_removeColumns_full(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_removeRows_none_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_removeRows_none(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_removeRows_none(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_removeRows_partial_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_removeRows_partial(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_removeRows_partial(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_removeRows_full_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_removeRows_full(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_removeRows_full(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_setFilter_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_setFilter(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_setFilter(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_setFilter_non_dynamic_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_setFilter_non_dynamic(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_setFilter_non_dynamic(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_setFilter_dynamic_row_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_setFilter_dynamic_row(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_setFilter_dynamic_row(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_setFilter_dynamic_column_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_setFilter_dynamic_column(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_setFilter_dynamic_column(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_sort_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_sort(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_sort(wrapper);
}

BOOST_AUTO_TEST_CASE( WSortFilterProxyModel_invalidate_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WSortFilterProxyModel_invalidate(wrapper);

  wrapper.createListModel();
  WSortFilterProxyModel_invalidate(wrapper);
}
