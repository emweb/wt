/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WException.h"
#include "Wt/WLogger.h"
#include "Wt/WSelectionBox.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

LOGGER("WSelectionBox");

WSelectionBox::WSelectionBox()
  : verticalSize_(5),
    selectionMode_(SelectionMode::Single),
    configChanged_(false)
{ 
  noSelectionEnabled_ = true;
}

void WSelectionBox::setVerticalSize(int items)
{
  verticalSize_ = items;
  configChanged_ = true;
  repaint(RepaintFlag::SizeAffected);
}

void WSelectionBox::setSelectionMode(SelectionMode mode)
{
  if (mode != selectionMode_) {
    selectionMode_ = mode;
    configChanged_ = true;
    repaint();

    if (mode == SelectionMode::Extended) {
      selection_.clear();
      if (currentIndex() != -1)
	selection_.insert(currentIndex());
    } else {
      if (selection_.size() == 1)
	setCurrentIndex(*selection_.begin());
      else
	setCurrentIndex(-1);
      selection_.clear();
    }
  }
}

void WSelectionBox::setSelectedIndexes(const std::set<int>& selection)
{
  if (selectionMode_ != SelectionMode::Extended)
    throw WException("WSelectionBox::setSelectedIndexes() can only be used "
		     "for an SelectionMode::Extended mode");

  selection_ = selection;
  selectionChanged_ = true;
  repaint();
}

const std::set<int>& WSelectionBox::selectedIndexes() const
{
  if (selectionMode_ != SelectionMode::Extended)
    throw WException("WSelectionBox::setSelectedIndexes() can only be used "
		     "for an SelectionMode::Extended mode");

  return selection_;
}

void WSelectionBox::clearSelection()
{
  if (selectionMode_ == SelectionMode::Extended)
    setSelectedIndexes(std::set<int>());
  else
    setCurrentIndex(-1);
}

bool WSelectionBox::isSelected(int index) const
{
  if (selectionMode_ == SelectionMode::Extended) {
    std::set<int>::const_iterator i = selection_.find(index);
    return i != selection_.end();
  } else
    return WComboBox::isSelected(index);
}

bool WSelectionBox::supportsNoSelection() const
{
  return true;
}

void WSelectionBox::updateDom(DomElement& element, bool all)
{
  if (configChanged_ || all) {
    element.setAttribute("size", std::to_string(verticalSize_));

    if (!all || (selectionMode_ == SelectionMode::Extended)) {
      element.setProperty(Property::Multiple,
			  selectionMode_ == SelectionMode::Extended
			  ? "true" : "false");
      if (!all)
	selectionChanged_ = true;
    }

    configChanged_ = false;
  }

  if (selectionMode_ == SelectionMode::Extended) {
    if (selectionChanged_ && !all) {
      for (int i = 0; i < count(); ++i) {
	element.callMethod("options[" + std::to_string(i) + "].selected="
			   + (isSelected(i) ? "true" : "false"));
      }
    }
    selectionChanged_ = false;
  }

  WComboBox::updateDom(element, all);
}

void WSelectionBox::propagateRenderOk(bool deep)
{
  configChanged_ = false;
  selectionChanged_ = false;

  WComboBox::propagateRenderOk(deep);
}

void WSelectionBox::setFormData(const FormData& formData)
{
  if (selectionChanged_)
    return;

  if (selectionMode_ == SelectionMode::Single)
    WComboBox::setFormData(formData);
  else {
    selection_.clear();

    for (int j = 0; j < Utils::size(formData.values); ++j) {
      const std::string& v = formData.values[j];
      if (!v.empty()) {
	try {
	  int i = Utils::stoi(v);
	  selection_.insert(i);
	} catch (std::exception& e) {
	  LOG_ERROR("received illegal form value: '" << v << "'");
	}
      }
    }
  }
}

}
