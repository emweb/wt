/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractArea.h"
#include "Wt/WApplication.h"
#include "Wt/WResource.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WImage.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

  namespace Impl {

    class AreaWidget : public WInteractWidget
    {
    public:
      AreaWidget(WAbstractArea *facade)
	: facade_(facade)
      { }

      ~AreaWidget()
      { }

      WAbstractArea *facade() const { return facade_; }

    private:
      WAbstractArea *facade_;

    protected:
      virtual void updateDom(DomElement& element, bool all) override
      {
	bool needsUrlResolution = facade_->updateDom(element, all);

	WInteractWidget::updateDom(element, all);

	if (!element.getProperty(Property::StyleCursor).empty()
	    && !wApp->environment().agentIsGecko()
	    && element.getAttribute("href").empty())
	  element.setAttribute("href", "javascript:void(0);");

	if (needsUrlResolution)
	  WAnchor::renderUrlResolution(this, element, all);
      }

      virtual DomElementType domElementType() const override
      {
	return DomElementType::AREA;
      }

      friend class Wt::WAbstractArea;
    };

  }

WAbstractArea::WAbstractArea()
  : uWidget_(new Impl::AreaWidget(this)),
    widget_(uWidget_.get()),
    hole_(false),
    transformable_(true)
{ }

WAbstractArea::~WAbstractArea()
{ }

EventSignal<WKeyEvent>& WAbstractArea::keyWentDown()
{
  return widget_->keyWentDown();
}

EventSignal<WKeyEvent>& WAbstractArea::keyPressed()
{
  return widget_->keyPressed();
}

EventSignal<WKeyEvent>& WAbstractArea::keyWentUp()
{
  return widget_->keyWentUp();
}

EventSignal<>& WAbstractArea::enterPressed()
{
  return widget_->enterPressed();
}

EventSignal<>& WAbstractArea::escapePressed()
{
  return widget_->escapePressed();
}

EventSignal<WMouseEvent>& WAbstractArea::clicked()
{
  return widget_->clicked();
}

EventSignal<WMouseEvent>& WAbstractArea::doubleClicked()
{
  return widget_->doubleClicked();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentDown()
{
  return widget_->mouseWentDown();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentUp()
{
  return widget_->mouseWentUp();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentOut()
{
  return widget_->mouseWentOut();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentOver()
{
  return widget_->mouseWentOver();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseMoved()
{
  return widget_->mouseMoved();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseDragged()
{
  return widget_->mouseDragged();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWheel()
{
  return widget_->mouseWheel();
}

void WAbstractArea::setImage(WImage *image)
{
  image_ = image;
}

WImage *WAbstractArea::image() const
{
  return image_.get();
}

void WAbstractArea::setHole(bool hole)
{
  hole_ = hole;

  repaint();
}

void WAbstractArea::setTransformable(bool transformable)
{
  transformable_ = transformable;

  repaint();
}

void WAbstractArea::setLink(const WLink& link)
{
  createAnchorImpl();

  anchor_->linkState.link = link;

  if (anchor_->linkState.link.type() == LinkType::Resource)
    anchor_->linkState.link.resource()->dataChanged().connect
      (this, &WAbstractArea::resourceChanged);

  repaint();
}

WLink WAbstractArea::link() const
{
  if (!anchor_)
    return WLink();
  else
    return anchor_->linkState.link;
}

void WAbstractArea::setAlternateText(const WString& text)
{
  createAnchorImpl();

  anchor_->altText = text;

  repaint();
}

const WString WAbstractArea::alternateText() const
{
  if (anchor_)
    return anchor_->altText;
  else
    return WString();
}

void WAbstractArea::setToolTip(const WString& text)
{
  widget_->setToolTip(text);
}

WString WAbstractArea::toolTip() const
{
  return widget_->toolTip();
}

void WAbstractArea::setStyleClass(const WT_USTRING& styleClass)
{
  widget_->setStyleClass(styleClass);
}

void WAbstractArea::setStyleClass(const char *styleClass)
{
  widget_->setStyleClass(styleClass);
}

WT_USTRING WAbstractArea::styleClass() const
{
  return widget_->styleClass();
}

void WAbstractArea::addStyleClass(const WT_USTRING& styleClass, bool force)
{
  widget_->addStyleClass(styleClass, force);
}

void WAbstractArea::removeStyleClass(const WT_USTRING& styleClass, bool force)
{
  widget_->removeStyleClass(styleClass, force);
}

void WAbstractArea::setCursor(Cursor cursor)
{
  widget_->decorationStyle().setCursor(cursor);
}

void WAbstractArea::setCursor(std::string cursorImage, Cursor fallback)
{
  widget_->decorationStyle().setCursor(cursorImage, fallback);
}

Cursor WAbstractArea::cursor() const
{
  return widget_->decorationStyle().cursor();
}

void WAbstractArea::createAnchorImpl()
{
  if (!anchor_)
    anchor_.reset(new AnchorImpl());
}

void WAbstractArea::resourceChanged()
{
  repaint();
}

void WAbstractArea::repaint()
{
  widget_->repaint();
}

bool WAbstractArea::updateDom(DomElement& element, bool all)
{
  bool needsUrlResolution = false;

  if (!hole_ && anchor_) {
    needsUrlResolution
      = WAnchor::renderHRef(widget_, anchor_->linkState, element);
    WAnchor::renderHTarget(anchor_->linkState, element, all);
    element.setAttribute("alt", anchor_->altText.toUTF8());
  } else {
    element.setAttribute("alt", "");

    if (hole_)
      element.setAttribute("nohref", "nohref");
  }

  return needsUrlResolution;
}

WInteractWidget *WAbstractArea::widget()
{
  return widget_;
}

std::unique_ptr<WWidget> WAbstractArea::takeWidget()
{
  return std::move(uWidget_);
}

void WAbstractArea::returnWidget(std::unique_ptr<WWidget> w)
{
  uWidget_ = Utils::dynamic_unique_ptr_cast<Impl::AreaWidget>(std::move(w));
}

std::string WAbstractArea::jsRef() const
{
  return widget_->jsRef();
}

}
