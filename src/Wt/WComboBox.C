/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WComboBox"
#include "Wt/WLogger"
#include "Wt/WStringListModel"

#include "DomElement.h"
#include "CgiParser.h"

namespace Wt {

WComboBox::WComboBox(WContainerWidget *parent)
  : WFormWidget(parent),
    activated(this),
    sactivated(this),
    model_(0),
    modelColumn_(0),
    currentIndex_(-1),
    itemsChanged_(false),
    selectionChanged_(false),
    currentlyConnected_(false)
{ 
  setInline(true);
  setFormObject(true);

  setModel(new WStringListModel(this));
}

void WComboBox::setModel(WAbstractItemModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  model_ = model;

  modelConnections_.push_back
    (model_->columnsInserted.connect(SLOT(this, WComboBox::itemsChanged)));
  modelConnections_.push_back
    (model_->columnsRemoved.connect(SLOT(this, WComboBox::itemsChanged)));
  modelConnections_.push_back
     (model_->rowsInserted.connect(SLOT(this, WComboBox::itemsChanged)));
  modelConnections_.push_back
     (model_->rowsRemoved.connect(SLOT(this, WComboBox::itemsChanged)));
  modelConnections_.push_back
     (model_->dataChanged.connect(SLOT(this, WComboBox::itemsChanged)));
}

void WComboBox::setModelColumn(int index)
{
  modelColumn_ = 0;
}

void WComboBox::addItem(const WString& text)
{
  insertItem(count(), text);
}

int WComboBox::count() const
{
  return model_->rowCount();
}

int WComboBox::currentIndex() const
{
  return currentIndex_;
}

const WString WComboBox::currentText() const
{
  if (currentIndex_ != -1)
    return asString(model_->data(currentIndex_, modelColumn_));
  else
    return WString();
}

void WComboBox::insertItem(int index, const WString& text)
{
  if (model_->insertRow(index))
    setItemText(index, text);
}

const WString WComboBox::itemText(int index) const
{
  return asString(model_->data(index, modelColumn_));
}

void WComboBox::removeItem(int index)
{
  model_->removeRow(index);

  setCurrentIndex(currentIndex_);
}

void WComboBox::setCurrentIndex(int index)
{
  int newIndex = std::min(index, count() - 1);

  if (currentIndex_ != newIndex) {
    currentIndex_ = newIndex;

    selectionChanged_ = true;
    repaint(RepaintPropertyIEMobile);

    // changed.emit();
  }
}

void WComboBox::setItemText(int index, const WString& text)
{
  model_->setData(index, modelColumn_, boost::any(text));
}

void WComboBox::clear()
{
  model_->removeRows(0, count());

  setCurrentIndex(currentIndex_);
}

void WComboBox::propagateChange()
{
  /*
   * copy values for when widget would be deleted from activated.emit()
   */
  int myCurrentIndex = currentIndex_;
  WString myCurrentValue;

  if (currentIndex_ != -1)
    myCurrentValue = currentText();

  /*
   * use this connection to know if the widget was killed
   */
  boost::signals::connection alive
    = sactivated.connect(SLOT(this, WComboBox::dummy));

  activated.emit(currentIndex_);

  if (alive.connected()) {
    alive.disconnect();

    if (myCurrentIndex != - 1)
      sactivated.emit(myCurrentValue);
  }
}

void WComboBox::dummy()
{ }

bool WComboBox::isSelected(int index) const
{
  return index == currentIndex_;
}

void WComboBox::updateDom(DomElement& element, bool all)
{
  if (itemsChanged_ || all) {
    if (all && count() > 0 && currentIndex_ == -1)
      currentIndex_ = 0;

    if (!all)
      element.removeAllChildren();

    for (int i = 0; i < count(); ++i) {
      DomElement *item = DomElement::createNew(DomElement_OPTION);
      item->setAttribute("value", boost::lexical_cast<std::string>(i));
      item->setProperty(Wt::PropertyInnerHTML,
			escapeText(asString(model_->data(i, modelColumn_)))
			.toUTF8());
      if (isSelected(i))
	item->setProperty(Wt::PropertySelected, "true");

      WString sc = asString(model_->data(i, modelColumn_, StyleClassRole));
      if (!sc.empty())
	item->setAttribute("class", sc.toUTF8());

      element.addChild(item);
    }

    itemsChanged_ = false;
  }

  if (selectionChanged_) {
    element.setProperty(Wt::PropertySelectedIndex,
			boost::lexical_cast<std::string>(currentIndex_));
    selectionChanged_ = false;
  }

  if (!currentlyConnected_
      && (activated.isConnected() || sactivated.isConnected())) {
    currentlyConnected_ = true;
    changed.connect(SLOT(this, WComboBox::propagateChange));
  }

  WFormWidget::updateDom(element, all);
}

DomElementType WComboBox::domElementType() const
{
  return DomElement_SELECT;
}

void WComboBox::setFormData(CgiEntry *entry)
{
  if (!entry->value().empty()) {
    try {
      currentIndex_ = boost::lexical_cast<int>(entry->value());
    } catch (boost::bad_lexical_cast&) {
      wApp->log("error") << "WComboBox received illegal form value: '"
			 << entry->value() << "'";
    }
  } else
    currentIndex_ = -1;
}

void WComboBox::refresh()
{
  itemsChanged();

  WFormWidget::refresh();
}

WValidator::State WComboBox::validate()
{
  if (validator()) {
    int pos;
    WString text = currentText();

    return validator()->validate(text, pos);
  } else
    return WValidator::Valid;
}

void WComboBox::itemsChanged()
{
  itemsChanged_ = true;
  repaint(RepaintInnerHtml);
}

int WComboBox::findText(const WString& text, MatchFlags flags)
{
  WModelIndexList list = model_->match(model_->index(0, modelColumn_),
				       DisplayRole, boost::any(text),
				       1, flags);

  if (list.empty())
    return -1;
  else
    return list.front().row();
}

}
