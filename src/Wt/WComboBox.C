
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WComboBox.h"
#include "Wt/WLogger.h"
#include "Wt/WStringListModel.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

LOGGER("WComboBox");

WComboBox::WComboBox()
  : modelColumn_(0),
    currentIndex_(-1),
    currentIndexRaw_(nullptr),
    itemsChanged_(false),
    selectionChanged_(true),
    currentlyConnected_(false),
    noSelectionEnabled_(false)
{ 
  setInline(true);
  setFormObject(true);
  setModel(std::make_shared<WStringListModel>());
}

void WComboBox::setModel(const std::shared_ptr<WAbstractItemModel> model)
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
  repaint(RepaintFlag::SizeAffected);

  if (currentIndex_ < from) // selection is not affected
    return;

  int count = to - from + 1;
  
  if (currentIndex_ > to) // shift up the selection by amount of removed rows
    currentIndex_ -= count; 
  else if (currentIndex_ >= from) {
    currentIndex_ = -1;
    makeCurrentIndexValid();
  }
}

void WComboBox::rowsInserted(const WModelIndex &index, int from, int to)
{
  itemsChanged_ = true;
  repaint(RepaintFlag::SizeAffected);

  int count = to - from + 1;

  if (currentIndex_ == -1)
    makeCurrentIndexValid();
  else if (currentIndex_ >= from)
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
    makeCurrentIndexValid();
  }
}

const WString WComboBox::itemText(int index) const
{
  return asString(model_->data(index, modelColumn_));
}

void WComboBox::removeItem(int index)
{
  model_->removeRow(index);

  makeCurrentIndexValid();
}

void WComboBox::setCurrentIndex(int index)
{
  int newIndex = std::min(index, count() - 1);

  if (currentIndex_ != newIndex) {
    currentIndex_ = newIndex;
    makeCurrentIndexValid();

    validate();

    selectionChanged_ = true;
    repaint();
  }
}

void WComboBox::setItemText(int index, const WString& text)
{
  model_->setData(index, modelColumn_, cpp17::any(text));
}

void WComboBox::clear()
{
  model_->removeRows(0, count());

  makeCurrentIndexValid();
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

  observing_ptr<WComboBox> guard(this);

  activated_.emit(currentIndex_);

  if (guard) {
    if (myCurrentIndex != - 1)
      sactivated_.emit(myCurrentValue);
  }
}

bool WComboBox::isSelected(int index) const
{
  return index == currentIndex_;
}

void WComboBox::setNoSelectionEnabled(bool enabled)
{
  if (noSelectionEnabled_ != enabled) {
    noSelectionEnabled_ = enabled;

    makeCurrentIndexValid();
  }
}

void WComboBox::makeCurrentIndexValid()
{
  int c = count();

  if (currentIndex_ > c - 1)
    setCurrentIndex(c - 1);
  else if (c > 0 && currentIndex_ == -1 && !supportsNoSelection())
    setCurrentIndex(0);
}

bool WComboBox::supportsNoSelection() const
{
  return noSelectionEnabled_;
}

void WComboBox::updateDom(DomElement& element, bool all)
{
  if (itemsChanged_ || all) {
    if (!all)
      element.removeAllChildren();

    DomElement *currentGroup = nullptr;
    bool groupDisabled = true;

    int size = count();
    for (int i = 0; i < size; ++i) {
      // Make new option item
      DomElement *item = DomElement::createNew(DomElementType::OPTION);
      item->setProperty(Property::Value, std::to_string(i));
      item->setProperty(Property::InnerHTML,
			escapeText(asString(model_->data(i, modelColumn_)))
			.toUTF8());

      if (!(model_->flags(model_->index(i, modelColumn_)) &
	    ItemFlag::Selectable))
	item->setProperty(Property::Disabled, "true");

      if (isSelected(i))
	item->setProperty(Property::Selected, "true");

      WString sc = asString(model_->data(i, modelColumn_, 
					 ItemDataRole::StyleClass));
      if (!sc.empty())
	item->setProperty(Property::Class, sc.toUTF8());


      // Read out opt-group
      WString groupname = Wt::asString(model_->data(i, modelColumn_,
						    ItemDataRole::Level));

      bool isSoloItem = false;
      if (groupname.empty()) { // no group
	isSoloItem = true;

	if (currentGroup) { // possibly close off an active group
	  if (groupDisabled)
	    currentGroup->setProperty(Property::Disabled, "true");
	  element.addChild(currentGroup);
	  currentGroup = nullptr;
	}
      } else {
	isSoloItem = false;

	// not same as current group
	if (!currentGroup ||
	    currentGroup->getProperty(Property::Label) != groupname.toUTF8()) {
	  if (currentGroup) { // possibly close off an active group
	    if (groupDisabled)
	      currentGroup->setProperty(Property::Disabled, "true");
	    element.addChild(currentGroup);
	    currentGroup = nullptr;
	  }

	  // make group
	  currentGroup = DomElement::createNew(DomElementType::OPTGROUP);
	  currentGroup->setProperty(Property::Label, groupname.toUTF8());
	  groupDisabled = !(model_->flags(model_->index(i, modelColumn_)) &
			    ItemFlag::Selectable);
	} else {
	  if (model_->flags(model_->index(i, modelColumn_)).test(
	      ItemFlag::Selectable))
	    groupDisabled = false;
	}
      }
      
      if (isSoloItem)
	element.addChild(item);
      else
	currentGroup->addChild(item);

      // last loop and there's still an open group
      if (i == size - 1 && currentGroup) {
	if (groupDisabled)
	  currentGroup->setProperty(Property::Disabled, "true");
	element.addChild(currentGroup);
	currentGroup = nullptr;
      }
    }

    itemsChanged_ = false;
  }

  if (selectionChanged_ ||
      (all && (selectionMode() == SelectionMode::Single))) {
    element.setProperty(Property::SelectedIndex, std::to_string(currentIndex_));
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
  return DomElementType::SELECT;
}

void WComboBox::setFormData(const FormData& formData)
{
  if (selectionChanged_ || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];

    if (!value.empty()) {
      try {
	currentIndex_ = Utils::stoi(value);
      } catch (std::exception& e) {
	LOG_ERROR("received illegal form value: '" << value << "'");
      }
    } else
      currentIndex_ = -1;

    makeCurrentIndexValid();
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
  int i = findText(value, MatchFlag::Exactly);
  setCurrentIndex(i);
#else
  int size = count();
  for (int i = 0; i < size; ++i) {
    if (Wt::asString(model_->index(i, modelColumn_).data(ItemDataRole::Display))
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
  repaint(RepaintFlag::SizeAffected);

  makeCurrentIndexValid();
}

void WComboBox::saveSelection()
{
  if (currentIndex_ >= 0)
    currentIndexRaw_ = 
      model_->toRawIndex(model_->index(currentIndex_, modelColumn_));
  else
    currentIndexRaw_ = nullptr;
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

  makeCurrentIndexValid();

  currentIndexRaw_ = nullptr;
}

void WComboBox::layoutChanged()
{
  itemsChanged_ = true;
  repaint(RepaintFlag::SizeAffected);

  restoreSelection();
}

int WComboBox::findText(const WString& text, WFlags<MatchFlag> flags) const
{
  WModelIndexList list = model_->match(model_->index(0, modelColumn_),
				       ItemDataRole::Display, cpp17::any(text),
				       1, flags);

  if (list.empty())
    return -1;
  else
    return list[0].row();
}

}
