/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "utilities.h"

#include <boost/test/unit_test.hpp>

#include <Wt/WAggregateProxyModel.h>
#include <Wt/WException.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStringListModel.h>

using namespace Wt;

namespace {
  void WAggregateProxyModel_addAggregate_full(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());
    int columnsCollapsed = MODEL_DIMENSION - 1;

    model->addAggregate(0, 1, columnsCollapsed);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);
  }

  void WAggregateProxyModel_addAggregate_partial(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());
    int columnsCollapsed = 1;

    model->addAggregate(0, 1, columnsCollapsed);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
    } else {
      BOOST_TEST(model->columnCount() == MODEL_DIMENSION - columnsCollapsed);
    }
  }

  void WAggregateProxyModel_addAggregate_none(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->addAggregate(MODEL_DIMENSION - 1, MODEL_DIMENSION, MODEL_DIMENSION);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
  }

  void WAggregateProxyModel_addAggregate_exception(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());

    BOOST_CHECK_THROW(model->addAggregate(0, 2, MODEL_DIMENSION), WException);
  }

  void WAggregateProxyModel_expand_one(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());
    int columnsCollapsed = 1;

    model->addAggregate(0, 1, columnsCollapsed);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
    } else {
      BOOST_TEST(model->columnCount() == MODEL_DIMENSION - columnsCollapsed);
    }

    model->expandColumn(0);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
  }

  void WAggregateProxyModel_expand_multiple(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->addAggregate(0, 1, MODEL_DIMENSION - 1);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);

    model->expandColumn(0);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
  }

  void WAggregateProxyModel_expand_none(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->addAggregate(0, 1, MODEL_DIMENSION - 1);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);

    model->expandColumn(1);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);
  }

  void WAggregateProxyModel_collapse_one(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());
    int columnsCollapsed = 1;

    model->addAggregate(0, 1, columnsCollapsed);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
    } else {
      BOOST_TEST(model->columnCount() == MODEL_DIMENSION - columnsCollapsed);
    }

    model->expandColumn(0);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());

    model->collapseColumn(0);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
    } else {
      BOOST_TEST(model->columnCount() == MODEL_DIMENSION - columnsCollapsed);
    }
  }

  void WAggregateProxyModel_collapse_multiple(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->addAggregate(0, 1, MODEL_DIMENSION - 1);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);

    model->expandColumn(0);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());

    model->collapseColumn(0);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);
  }

  void WAggregateProxyModel_collapse_none(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->addAggregate(0, 1, MODEL_DIMENSION - 1);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);

    model->expandColumn(1);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);

    model->collapseColumn(1);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == 1);
  }

  void WAggregateProxyModel_sort_no_aggregate(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());

    model->sort(1, SortOrder::Descending);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());

    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(4, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(3, 0));
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(4, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(3, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(4, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(3, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 1))) == wrapper.createValue(4, 1));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 1))) == wrapper.createValue(3, 1));

    model->sort(1, SortOrder::Ascending);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());

    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(0, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(1, 0));
    // Sorted
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(0, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(1, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(0, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(1, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 1))) == wrapper.createValue(0, 1));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 1))) == wrapper.createValue(1, 1));
  }

  void WAggregateProxyModel_sort_with_aggregate_collapsed(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());
    int columnsCollapsed = 1;

    // Collapsed by default
    model->addAggregate(0, 1, columnsCollapsed);
    model->sort(1, SortOrder::Ascending);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
    } else {
      BOOST_TEST(model->columnCount() == MODEL_DIMENSION - columnsCollapsed);
    }

    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(0, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(1, 0));
    // Sorted
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(0, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(1, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(0, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(1, 0));

    model->sort(1, SortOrder::Descending);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    if (wrapper.isListModel()) {
      BOOST_TEST(model->columnCount() == wrapper.getModelColumns());
    } else {
      BOOST_TEST(model->columnCount() == MODEL_DIMENSION - columnsCollapsed);
    }

    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(4, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(3, 0));
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(4, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(3, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(4, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(3, 0));
  }

  void WAggregateProxyModel_sort_with_aggregate_expanded(SourceModelWrapper wrapper)
  {
    std::unique_ptr<WAggregateProxyModel> model = std::make_unique<WAggregateProxyModel>();
    model->setSourceModel(wrapper.getModel());
    int columnsCollapsed = 1;

    model->addAggregate(0, 1, columnsCollapsed);
    model->expandColumn(0);
    model->sort(1, SortOrder::Ascending);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());

    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(0, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(1, 0));
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(0, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(1, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(0, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(1, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 1))) == wrapper.createValue(0, 1));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 1))) == wrapper.createValue(1, 1));

    model->sort(1, SortOrder::Descending);

    BOOST_TEST(model->rowCount() == wrapper.getModelRows());
    BOOST_TEST(model->columnCount() == wrapper.getModelColumns());

    BOOST_TEST(wrapper.get(0, 0) == wrapper.createValue(4, 0));
    BOOST_TEST(wrapper.get(1, 0) == wrapper.createValue(3, 0));
    BOOST_TEST(wrapper.get(0, 1) == wrapper.createValue(4, 1));
    BOOST_TEST(wrapper.get(1, 1) == wrapper.createValue(3, 1));

    // Via proxy
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 0))) == wrapper.createValue(4, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 0))) == wrapper.createValue(3, 0));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(0, 1))) == wrapper.createValue(4, 1));
    BOOST_TEST(cpp17::any_cast<WString>(model->data(model->index(1, 1))) == wrapper.createValue(3, 1));
  }
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_addAggregate_full_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_addAggregate_full(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_addAggregate_full(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_addAggregate_partial_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_addAggregate_partial(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_addAggregate_partial(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_addAggregate_none_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_addAggregate_none(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_addAggregate_none(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_addAggregate_exception_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_addAggregate_exception(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_addAggregate_exception(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_expand_one_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_expand_one(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_expand_one(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_expand_multiple_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_expand_multiple(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_expand_multiple(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_expand_none_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_expand_none(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_expand_none(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_collapse_one_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_collapse_one(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_collapse_one(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_collapse_multiple_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_collapse_multiple(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_collapse_multiple(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_collapse_none_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_collapse_none(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_collapse_none(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_sort_no_aggregate_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_sort_no_aggregate(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_sort_no_aggregate(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_sort_with_aggregate_collapsed_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_sort_with_aggregate_collapsed(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_sort_with_aggregate_collapsed(wrapper);
}

BOOST_AUTO_TEST_CASE( WAggregateProxyModel_sort_with_aggregate_expanded_test )
{
  SourceModelWrapper wrapper;
  wrapper.createStandardModel();
  WAggregateProxyModel_sort_with_aggregate_expanded(wrapper);

  wrapper.createListModel();
  WAggregateProxyModel_sort_with_aggregate_expanded(wrapper);
}
