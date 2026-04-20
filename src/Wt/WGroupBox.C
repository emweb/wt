/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLogger.h"
#include "Wt/WGroupBox.h"
#include "Wt/WVBoxLayout.h"

#include "DomElement.h"
#include "StdWidgetItemImpl.h"

namespace Wt {

  LOGGER("WGroupBox");

WGroupBox::WGroupBox()
{
  init();
}

WGroupBox::WGroupBox(const WString& title)
  : title_(title)
{
  init();
}

void WGroupBox::init()
{
  setJavaScriptMember(WT_GETPS_JS, StdWidgetItemImpl::secondGetPSJS());

  /* This makes the JS layout take the legend into account when
   * computing the minimum size of the fieldset.
   */
  setJavaScriptMember(WT_GETEXTRAMS_JS, StdWidgetItemImpl::secondGetPSJS());
  setLogicalLayout(nullptr);
}

void WGroupBox::setTitle(const WString& title)
{
  title_ = title;
  titleChanged_ = true;
  repaint(RepaintFlag::SizeAffected);
}

void WGroupBox::updateDom(DomElement& element, bool all)
{
  if (all || titleChanged_) {
    DomElement *legend;
    if (all) {
      legend = DomElement::createNew(DomElementType::LEGEND);
      legend->setId(id() + "l");
    } else
      legend = DomElement::getForUpdate(id() + "l", DomElementType::LEGEND);

    legend->setProperty(Wt::Property::InnerHTML, escapeText(title_).toUTF8());

    /* Set size to fit-content to ensure it does not depends on the
     * fieldset size. Otherwise, it will stop the fieldset shrinking
     * when using JS layout.
     */
    legend->setProperty(Property::StyleWidth, "fit-content");
    legend->setProperty(Property::StyleHeight, "fit-content");

    element.addChild(legend);

    titleChanged_ = false;
  }

  WContainerWidget::updateDom(element, all);
}

void WGroupBox::propagateRenderOk(bool deep)
{
  titleChanged_ = false;

  WContainerWidget::propagateRenderOk(deep);
}

DomElementType WGroupBox::domElementType() const
{
  return DomElementType::FIELDSET;
}

void WGroupBox::refresh()
{
  if (title_.refresh()) {
    titleChanged_ = true;
    repaint(RepaintFlag::SizeAffected);
  }

  WContainerWidget::refresh();
}

void WGroupBox::addWidget(std::unique_ptr<WWidget> widget)
{
  if (!logicalLayout_) {
    realLayout()->addWidget(std::move(widget));
  }
}

void WGroupBox::insertBefore(std::unique_ptr<WWidget> widget,
                             WWidget *before)
{
  int index = indexOf(before);

  if (index == -1) {
    LOG_WARN("insertBefore(): before is not in container, appending at back");
    // We don't want to use an override here, to behave like a WContainerWidget.
    index = WGroupBox::count();
  }

  insertWidget(index, std::move(widget));
}

void WGroupBox::insertWidget(int index, std::unique_ptr<WWidget> widget)
{
  if (!logicalLayout_) {
    WBoxLayout *boxLayout = dynamic_cast<WBoxLayout *>(realLayout());
    if (boxLayout) {
      boxLayout->insertWidget(index, std::move(widget));
    } else {
      // should never happen
      LOG_ERROR("Could not insert widget. Logical layout is not a box layout");
    }
  }
}

int WGroupBox::indexOf(WWidget* w) const
{
  if (!logicalLayout_) {
    for (int i = 0; i < realLayout()->count(); ++i) {
      WWidget* candidate = widget(i);
      if (candidate == w) {
        return i;
      }
    }
  }

  return -1;
}

WWidget* WGroupBox::widget(int index) const
{
  if (logicalLayout_) {
    return nullptr;
  }

  WLayoutItem *item = realLayout()->itemAt(index);
  return item ? item->widget() : nullptr;
}

int WGroupBox::count() const
{
  return logicalLayout_ ? 0 : realLayout()->count();
}

int WGroupBox::firstChildIndex() const
{
  return 1; // Legend is before
}

void WGroupBox::setLogicalLayout(std::unique_ptr<WLayout> layout)
{
  std::unique_ptr<WVBoxLayout> newLayout = std::make_unique<WVBoxLayout>();
  logicalLayout_ = layout.get();

  if (layout) {
    newLayout->addLayout(std::move(layout));
  }

  WContainerWidget::setLogicalLayout(std::move(newLayout));
}

}
