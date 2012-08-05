/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/ComboBox"
#include "Wt/WLineEdit"
#include "Wt/WStringListModel"
#include "DataStore"

#include "DomElement.h"

namespace Wt {
  namespace Ext {

ComboBox::ComboBox(WContainerWidget *parent)
  : LineEdit(parent),
    activated_(this, "activated", true),
    dataLocation_(ClientSide),
    editable_(false),
    queryDelay_(-1),
    minQueryLength_(4),
    pageSize_(0),
    model_(0),
    modelColumn_(0),
    dataStore_(0)
{
  //extjs: combobox doesn't stand a parent with display: none
  setHideWithOffsets(true);

  setModel(new WStringListModel(this));
}

void ComboBox::setDataLocation(DataLocation dataLocation)
{
  dataLocation_ = dataLocation;
}

void ComboBox::setEditable(bool how)
{
  editable_ = how;
}

void ComboBox::setQueryDelay(int milliSeconds)
{
  queryDelay_ = milliSeconds;
}

void ComboBox::setPageSize(int pageSize)
{
  pageSize_ = pageSize;
}

void ComboBox::setMinQueryLength(int numChars)
{
  minQueryLength_ = numChars;
}

void ComboBox::setModel(WAbstractItemModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  model_ = model;

  modelConnections_.push_back
    (model_->columnsInserted().connect(this, &ComboBox::modelColumnsInserted));
  modelConnections_.push_back
    (model_->columnsRemoved().connect(this, &ComboBox::modelColumnsRemoved));
  modelConnections_.push_back
    (model_->rowsInserted().connect(this, &ComboBox::modelRowsInserted));
  modelConnections_.push_back
    (model_->rowsRemoved().connect(this, &ComboBox::modelRowsRemoved));
  modelConnections_.push_back
    (model_->dataChanged().connect(this, &ComboBox::modelDataChanged));
  modelConnections_.push_back
    (model_->layoutChanged().connect(this, &ComboBox::modelLayoutChanged));
  modelConnections_.push_back
    (model_->modelReset().connect(this, &ComboBox::modelLayoutChanged));

  modelLayoutChanged();
}

void ComboBox::modelLayoutChanged()
{
  if (dataStore_) {
    dataStore_->setModel(model_);
    repaint();
  }
}

void ComboBox::setModelColumn(int index)
{
  modelColumn_ = index;
}

void ComboBox::addItem(const WString& text)
{
  insertItem(count(), text);
}

int ComboBox::count() const
{
  return model_->rowCount();
}

int ComboBox::currentIndex() const
{
  for (int i = 0; i < count(); ++i) {
    if (itemText(i) == lineEdit()->text())
      return i;
  }

  return -1;
}

void ComboBox::insertItem(int index, const WString& text)
{
  if (model_->insertRow(index))
    setItemText(index, text);
}

void ComboBox::removeItem(int index)
{
  model_->removeRow(index);
}

void ComboBox::setCurrentIndex(int index)
{
  int newIndex = std::min(index, count() - 1);

  if (newIndex == -1) {
    if (isRendered())
      addUpdateJS(elVar() + ".setValue('');");

    lineEdit()->setText(WString());
  } else {
    WModelIndex modelIndex = model_->index(index, modelColumn_);
    const boost::any& v = model_->data(modelIndex);

    if (isRendered()) {
      TextFormat tf = 
	model_->flags(modelIndex) & ItemIsXHTMLText ? XHTMLText : PlainText;
      addUpdateJS(elVar() + ".setValue("
		  + Wt::Impl::asJSLiteral(v, tf)
		  + ");");
    }

    lineEdit()->setText(asString(v));
  }
}

void ComboBox::setItemText(int index, const WString& text)
{
  model_->setData(index, modelColumn_, boost::any(text));
}

const WString ComboBox::currentText() const
{
  return lineEdit()->text();
}

const WString ComboBox::itemText(int index) const
{
  return asString(model_->data(index, modelColumn_));
}

void ComboBox::setLoadingText(const WString& text)
{
  //NYI
}

void ComboBox::clear()
{
  model_->removeRows(0, count());
}

void ComboBox::modelColumnsInserted(const WModelIndex& parent,
				    int start, int end)
{
  //TODO: refill
}

void ComboBox::modelColumnsRemoved(const WModelIndex& parent,
				   int start, int end)
{
  //TODO: refill
}

void ComboBox::modelRowsInserted(const WModelIndex& parent, int start, int end)
{
  if (dataStore_) {
    dataStore_->modelRowsInserted(start, end);
    repaint();
  }
}

void ComboBox::modelRowsRemoved(const WModelIndex& parent, int start, int end)
{
  if (dataStore_) {
    dataStore_->modelRowsRemoved(start, end);
    repaint();
  }
}

void ComboBox::modelDataChanged(const WModelIndex& topLeft,
				const WModelIndex& bottomRight)
{
  if (dataStore_) {
    dataStore_->modelDataChanged(topLeft, bottomRight);
    repaint();
  }
}

void ComboBox::refresh()
{
  if (model_->rowCount() > 0)
    modelDataChanged
      (model_->index(0, 0),
       model_->index(model_->columnCount() - 1, model_->rowCount() - 1));

  LineEdit::refresh();
}

void ComboBox::updateExt()
{
  addUpdateJS(elVar() + ".clearValue();");
  addUpdateJS(dataStore_->jsGetUpdates(elVar() + ".store"));
  updateWtSignal(&activated_, activated_.name(), "",
		 elVar() + ".selectedIndex");
  setCurrentIndex(currentIndex());
}

std::string ComboBox::createJS(DomElement *inContainer)
{
  std::stringstream result;
  result << elVar() << " = new Ext.form.ComboBox(" << configStruct() << ");";
  result << dataStore_->jsCreateRecordDef(elVar() + ".store");

  bindEventHandler("select", "selectH", result);

  applyToWidget(lineEdit(), result, inContainer);

  return result.str();
}

void ComboBox::createConfig(std::ostream& config)
{
  if (!dataStore_) {
    dataStore_ = new DataStore(model_, dataLocation_, this);
    dataStore_->addColumn(modelColumn_, "c0");
    dataStore_->setFilterColumn(modelColumn_);
  }

  std::string store = dataStore_->jsCreateStore();
  std::string mode = dataLocation_ == ClientSide ? "'local'" : "'remote'";

  config << ",store: " << store
	 << ",displayField:'c0',typeAhead:true,triggerAction:'all',mode:"
	 << mode;

  if (!editable_)
    config << ",forceSelection:true";
  if (minQueryLength_ != 4)
    config << ",minChars:" << minQueryLength_;
  if (queryDelay_ != -1)
    config << ",queryDelay:" << queryDelay_ + 1;
  if (pageSize_ != 0)
    config << ",pageSize:" << pageSize_;

  addWtSignalConfig("selectH", &activated_, activated_.name(),
		    "", elVar() + ".selectedIndex", config);  

  LineEdit::createConfig(config);
}

  }
}
