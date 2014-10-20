
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WComboBox"
#include "Wt/WLogger"
#include "Wt/WStringListModel"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

LOGGER("WComboBox");

WComboBox::WComboBox(WContainerWidget *parent)
  : WFormWidget(parent),
    model_(0),
    modelColumn_(0),
    currentIndex_(-1),
    currentIndexRaw_(0),
    itemsChanged_(false),
    selectionChanged_(true),
    currentlyConnected_(false),
    activated_(this),
    sactivated_(this)
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
    (model_->columnsInserted().connect(this, &WComboBox::itemsChanged));
  modelConnections_.push_back
    (model_->columnsRemoved().connect(this, &WComboBox::itemsChanged));
  modelConnections_.push_back
    (model_->rowsInserted().connect(this, &WComboBox::rowsInserted));
  modelConnections_.push_back
    (model_->rowsRemoved().connect(this, &WComboBox::rowsRemoved));
  modelConnections_.push_back
    (model_->dataChanged().connect(this, &WComboBox::itemsChanged));
  modelConnections_.push_back
    (model_->modelReset().connect(this, &WComboBox::itemsChanged));
  modelConnections_.push_back
    (model_->layoutAboutToBeChanged().connect(this,
					      &WComboBox::saveSelection));
  modelConnections_.push_back
    (model_->layoutChanged().connect(this, &WComboBox::layoutChanged));

  /* Redraw contents of the combo box to match the contents of the new model.
   */
  refresh();
}

void WComboBox::rowsRemoved(const WModelIndex &index, int from, int to)
{
  itemsChanged_ = true;
  repaint(RepaintSizeAffected);

  if (currentIndex_ < from) // selection is not affected
    return;

  int count = to - from + 1;
  
  if (currentIndex_ > to) // shift up the selection by amount of removed rows
    currentIndex_ -= count; 
  else if (currentIndex_ >= from) {
    if (supportsNoSelection())
      currentIndex_ = -1;
    else
      currentIndex_ = model_->rowCount() > 0 ? 0 : -1;
  }
}

void WComboBox::rowsInserted(const WModelIndex &index, int from, int to)
{
  itemsChanged_ = true;
  repaint(RepaintSizeAffected);

  int count = to - from + 1;

  if (currentIndex_ == -1) {
    if (model_->rowCount() == count && !supportsNoSelection())
      setCurrentIndex(0);
  } else if (currentIndex_ >= from)
    currentIndex_ += count;
}

void WComboBox::setModelColumn(int index)
{
  modelColumn_ = index;
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
  if (model_->insertRow(index)) {
    setItemText(index, text);
    if (model_->rowCount() == 1 && 
	currentIndex_ == -1 && 
	!supportsNoSelection())
      setCurrentIndex(0);
  }
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

    validate();

    selectionChanged_ = true;
    repaint();
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
   * copy values for when widget would be deleted from activated_.emit()
   */
  int myCurrentIndex = currentIndex_;
  WString myCurrentValue;

  if (currentIndex_ != -1)
    myCurrentValue = currentText();

  DeletionTracker guard(this);

  activated_.emit(currentIndex_);

  if (!guard.deleted()) {

    if (myCurrentIndex != - 1)
      sactivated_.emit(myCurrentValue);
  }

}

bool WComboBox::isSelected(int index) const
{
  return index == currentIndex_;
}

bool WComboBox::supportsNoSelection() const
{
  /*
   * Actually, these days, all browsers support 'no selection' for
   * combo-boxes, but we keep it like this to avoid breaking our
   * behavior
   *
   * See http://stackoverflow.com/questions/6223865/blank-html-select-without-blank-item-in-dropdown-list
   */

  return false;
}

void WComboBox::updateDom(DomElement& element, bool all)
{
  if (itemsChanged_ || all) {
    if (!all)
      element.removeAllChildren();

    DomElement *currentGroup = 0;
    bool groupDisabled = true;
    for (int i = 0; i < count(); ++i) {
      // Make new option item
      DomElement *item = DomElement::createNew(DomElement_OPTION);
      item->setProperty(PropertyValue, boost::lexical_cast<std::string>(i));
      item->setProperty(PropertyInnerHTML,
			escapeText(asString(model_->data(i, modelColumn_)))
			.toUTF8());

      if (!(model_->flags(model_->index(i, modelColumn_)) & ItemIsSelectable))
	item->setProperty(PropertyDisabled, "true");

      if (isSelected(i))
	item->setProperty(PropertySelected, "true");

      WString sc = asString(model_->data(i, modelColumn_, StyleClassRole));
      if (!sc.empty())
	item->setProperty(PropertyClass, sc.toUTF8());


      // Read out opt-group
      WString groupname = Wt::asString(model_->data(i, modelColumn_,
						    LevelRole));

      bool isSoloItem = false;
      if (groupname.empty()) { // no group
	isSoloItem = true;

	if (currentGroup) { // possibly close off an active group
	  if (groupDisabled)
	    currentGroup->setProperty(PropertyDisabled, "true");
	  element.addChild(currentGroup);
	  currentGroup = 0;
	}
      } else {
	isSoloItem = false;

	// not same as current group
	if (!currentGroup ||
	    currentGroup->getProperty(PropertyLabel) != groupname.toUTF8()) {
	  if (currentGroup) { // possibly close off an active group
	    if (groupDisabled)
	      currentGroup->setProperty(PropertyDisabled, "true");
	    element.addChild(currentGroup);
	    currentGroup = 0;
	  }

	  // make group
	  currentGroup = DomElement::createNew(DomElement_OPTGROUP);
	  currentGroup->setProperty(PropertyLabel, groupname.toUTF8());
	  groupDisabled = !(model_->flags(model_->index(i, modelColumn_))
			    & ItemIsSelectable);
	} else {
	  if (model_->flags(model_->index(i, modelColumn_)) & ItemIsSelectable)
	    groupDisabled = false;
	}
      }
      
      if (isSoloItem)
	element.addChild(item);
      else
	currentGroup->addChild(item);

      // last loop and there's still an open group
      if (i == count() - 1 && currentGroup) {
	if (groupDisabled)
	  currentGroup->setProperty(PropertyDisabled, "true");
	element.addChild(currentGroup);
	currentGroup = 0;
      }
    }

    itemsChanged_ = false;
  }

  if (selectionChanged_) {
    element.setProperty(PropertySelectedIndex,
			boost::lexical_cast<std::string>(currentIndex_));
    selectionChanged_ = false;
  }

  if (!currentlyConnected_
      && (activated_.isConnected() || sactivated_.isConnected())) {
    currentlyConnected_ = true;
    changed().connect(this, &WComboBox::propagateChange);
  }

  WFormWidget::updateDom(element, all);
}

void WComboBox::propagateRenderOk(bool deep)
{
  itemsChanged_ = false;
  selectionChanged_ = false;

  WFormWidget::propagateRenderOk(deep);
}

DomElementType WComboBox::domElementType() const
{
  return DomElement_SELECT;
}

void WComboBox::setFormData(const FormData& formData)
{
  if (selectionChanged_ || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];

    if (!value.empty()) {
      try {
	currentIndex_ = boost::lexical_cast<int>(value);
      } catch (boost::bad_lexical_cast& e) {
	LOG_ERROR("received illegal form value: '" << value << "'");
      }
    } else
      currentIndex_ = -1;
  }
}

void WComboBox::refresh()
{
  itemsChanged();

  WFormWidget::refresh();
}

WT_USTRING WComboBox::valueText() const
{
  return currentText();
}

void WComboBox::setValueText(const WT_USTRING& value)
{
#ifndef WT_TARGET_JAVA
  int i = findText(value, MatchExactly);
  setCurrentIndex(i);
#else
  for (int i = 0; i < count(); ++i) {
    if (Wt::asString(model_->index(i, modelColumn_).data(DisplayRole))
	== value) {
      setCurrentIndex(i);
      return;
    }
  }

  setCurrentIndex(-1);
#endif
}

void WComboBox::itemsChanged()
{
  itemsChanged_ = true;
  repaint(RepaintSizeAffected);

  if (currentIndex_ > count() - 1)
    currentIndex_ = count() - 1;
}

void WComboBox::saveSelection()
{
  if (currentIndex_ >= 0)
    currentIndexRaw_ = 
      model_->toRawIndex(model_->index(currentIndex_, modelColumn_));
  else
    currentIndexRaw_ = 0;
}

void WComboBox::restoreSelection()
{
  if (currentIndexRaw_) {
    WModelIndex m = model_->fromRawIndex(currentIndexRaw_);
    if (m.isValid())
      currentIndex_ = m.row();
    else
      currentIndex_ = -1;
  } else
    currentIndex_ = -1;

  currentIndexRaw_ = 0;
}

void WComboBox::layoutChanged()
{
  itemsChanged_ = true;
  repaint(RepaintSizeAffected);

  restoreSelection();
}

int WComboBox::findText(const WString& text, WFlags<MatchFlag> flags)
{
  WModelIndexList list = model_->match(model_->index(0, modelColumn_),
				       DisplayRole, boost::any(text),
				       1, flags);

  if (list.empty())
    return -1;
  else
    return list[0].row();
}

}
