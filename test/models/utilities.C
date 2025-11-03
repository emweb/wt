#include "utilities.h"

#include <boost/test/unit_test.hpp>

#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStringListModel.h>

namespace {
  std::shared_ptr<WStandardItemModel> createStandardModel(int rows, int cols)
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

  std::shared_ptr<WStringListModel> createListModel(int rows)
  {
    std::shared_ptr<WStringListModel> model = std::make_shared<WStringListModel>();

    for (int row = 0; row < rows; ++row) {
      model->insertString(row, WString("Row: {1}").arg(row));
    }

    BOOST_REQUIRE(model->rowCount() == rows);
    BOOST_REQUIRE(model->columnCount() == 1);
    BOOST_REQUIRE(static_cast<int>(model->stringList().size()) == rows);

    return model;
  }
}

void SourceModelWrapper::createStandardModel()
{
  stringListModel_.reset();
  standardItemModel_ = ::createStandardModel(MODEL_DIMENSION, MODEL_DIMENSION);
}

void SourceModelWrapper::createListModel()
{
  standardItemModel_.reset();
  stringListModel_ = ::createListModel(MODEL_DIMENSION);
}

int SourceModelWrapper::getModelRows()
{
  return MODEL_DIMENSION;
}

int SourceModelWrapper::getModelColumns()
{
  return standardItemModel_ ? MODEL_DIMENSION : 1;
}

std::shared_ptr<WAbstractItemModel> SourceModelWrapper::getModel()
{
  if (standardItemModel_) {
    return standardItemModel_;
  }

  if (stringListModel_) {
    return stringListModel_;
  }

  return nullptr;
}

bool SourceModelWrapper::isListModel()
{
  return stringListModel_.get();
}

WString SourceModelWrapper::get(int i, int j)
{
  if (standardItemModel_) {
    WStandardItem* item = standardItemModel_->item(i, j);
    if (item) {
      return item->text();
    }

    throw std::out_of_range("No item defined");
  }

  if (stringListModel_) {
    if (static_cast<int>(stringListModel_->stringList().size()) > i) {
      return stringListModel_->stringList()[i];
    }

    throw std::out_of_range("No string defined");
  }

  return "";
}

WString SourceModelWrapper::createValue(int i, int j)
{
  if (standardItemModel_) {
    return WString("Row: {1} - Col: {2}").arg(i).arg(j);
  }

  if (stringListModel_) {
    return WString("Row: {1}").arg(i);
  }

  return "";
}
