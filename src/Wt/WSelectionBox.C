/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WException"
#include "Wt/WLogger"
#include "Wt/WSelectionBox"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

LOGGER("WSelectionBox");

WSelectionBox::WSelectionBox(WContainerWidget *parent)
  : WComboBox(parent),
    verticalSize_(5),
    selectionMode_(SingleSelection),
    configChanged_(false)
{ }

void WSelectionBox::setVerticalSize(int items)
{
  verticalSize_ = items;
  configChanged_ = true;
  repaint(RepaintSizeAffected);
}

void WSelectionBox::setSelectionMode(SelectionMode mode)
{
  if (mode != selectionMode_) {
    selectionMode_ = mode;
    configChanged_ = true;
    repaint();

    if (mode == ExtendedSelection) {
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
  if (selectionMode_ != ExtendedSelection)
    throw WException("WSelectionBox::setSelectedIndexes() can only be used "
		     "for an ExtendedSelection mode");

  selection_ = selection;
  selectionChanged_ = true;
  repaint();
}

void WSelectionBox::clearSelection()
{
  if (selectionMode_ == ExtendedSelection)
    setSelectedIndexes(std::set<int>());
  else
    setCurrentIndex(-1);
}

bool WSelectionBox::isSelected(int index) const
{
  if (selectionMode_ == ExtendedSelection) {
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
    element.setAttribute("size",
			 boost::lexical_cast<std::string>(verticalSize_));

    if (!all || (selectionMode_ == ExtendedSelection)) {
      element.setProperty(PropertyMultiple, selectionMode_ == ExtendedSelection
			  ? "true" : "false");
      if (!all)
	selectionChanged_ = true;
    }

    configChanged_ = false;
  }

  if (selectionMode_ == ExtendedSelection) {
    if (selectionChanged_ && !all) {
      for (int i = 0; i < count(); ++i) {
	element.callMethod("options[" + boost::lexical_cast<std::string>(i)
			+ "].selected=" + (isSelected(i) ? "true" : "false"));
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

  if (selectionMode_ == SingleSelection)
    WComboBox::setFormData(formData);
  else {
    selection_.clear();

    for (int j = 0; j < Utils::size(formData.values); ++j) {
      const std::string& v = formData.values[j];
      if (!v.empty()) {
	try {
	  int i = boost::lexical_cast<int>(v);
	  selection_.insert(i);
	} catch (boost::bad_lexical_cast& error) {
	  LOG_ERROR("received illegal form value: '" << v << "'");
	}
      }
    }
  }
}

}
