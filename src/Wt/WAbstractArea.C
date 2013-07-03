/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractArea"
#include "Wt/WApplication"
#include "Wt/WResource"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WImage"

#include "DomElement.h"

namespace Wt {

  namespace Impl {

    class AreaWidget : public WInteractWidget
    {
    public:
      AreaWidget(WAbstractArea *facade)
	: facade_(facade)
      { }

      ~AreaWidget()
      {
	if (facade_) {
	  facade_->impl_ = 0;
	  delete facade_;
	}
      }

      WAbstractArea *facade() const { return facade_; }

    private:
      WAbstractArea *facade_;

    protected:
      virtual void updateDom(DomElement& element, bool all)
      {
	bool needsUrlResolution = facade_->updateDom(element, all);

	WInteractWidget::updateDom(element, all);

	if (!element.getProperty(PropertyStyleCursor).empty()
	    && !wApp->environment().agentIsGecko()
	    && element.getAttribute("href").empty())
	  element.setAttribute("href", "javascript:void(0);");

	if (needsUrlResolution)
	  WAnchor::renderUrlResolution(this, element, all);
      }

      virtual DomElementType domElementType() const {
	return DomElement_AREA;
      }

      friend class Wt::WAbstractArea;
    };

  }

WAbstractArea::WAbstractArea()
  : impl_(new Impl::AreaWidget(this)),
    hole_(false),
    anchor_(0)
{ }

WAbstractArea::~WAbstractArea()
{
  if (impl_) {
    WImage *i = image();

    if (i)
      i->removeArea(this);

    impl_->facade_ = 0;
    delete impl_;
  }

  delete anchor_;
}

EventSignal<WKeyEvent>& WAbstractArea::keyWentDown()
{
  return impl_->keyWentDown();
}

EventSignal<WKeyEvent>& WAbstractArea::keyPressed()
{
  return impl_->keyPressed();
}

EventSignal<WKeyEvent>& WAbstractArea::keyWentUp()
{
  return impl_->keyWentUp();
}

EventSignal<>& WAbstractArea::enterPressed()
{
  return impl_->enterPressed();
}

EventSignal<>& WAbstractArea::escapePressed()
{
  return impl_->escapePressed();
}

EventSignal<WMouseEvent>& WAbstractArea::clicked()
{
  return impl_->clicked();
}

EventSignal<WMouseEvent>& WAbstractArea::doubleClicked()
{
  return impl_->doubleClicked();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentDown()
{
  return impl_->mouseWentDown();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentUp()
{
  return impl_->mouseWentUp();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentOut()
{
  return impl_->mouseWentOut();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWentOver()
{
  return impl_->mouseWentOver();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseMoved()
{
  return impl_->mouseMoved();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseDragged()
{
  return impl_->mouseDragged();
}

EventSignal<WMouseEvent>& WAbstractArea::mouseWheel()
{
  return impl_->mouseWheel();
}

void WAbstractArea::setImage(WImage *image)
{
  impl_->setParent(image);
}

WImage *WAbstractArea::image() const
{
  return dynamic_cast<WImage *>(impl_->parent());
}

void WAbstractArea::setRef(const std::string& ref)
{
  setLink(WLink(ref));
}

const std::string WAbstractArea::ref() const
{
  if (anchor_)
    switch (anchor_->linkState.link.type()) {
    case WLink::InternalPath:
      return anchor_->linkState.link.internalPath().toUTF8();
    default:
      return anchor_->linkState.link.url();
    }
  else
    return std::string();
}

void WAbstractArea::setHole(bool hole)
{
  hole_ = hole;

  repaint();
}

void WAbstractArea::setLink(const WLink& link)
{
  createAnchorImpl();

  anchor_->linkState.link = link;

  if (anchor_->linkState.link.type() == WLink::Resource)
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

void WAbstractArea::setResource(WResource *resource)
{
  setLink(WLink(resource));
}

WResource *WAbstractArea::resource() const
{
  if (anchor_)
    return anchor_->linkState.link.resource();
  else
    return 0;
}

void WAbstractArea::setTarget(AnchorTarget target)
{
  createAnchorImpl();

  anchor_->linkState.target = target;

  repaint();
}

AnchorTarget WAbstractArea::target() const
{
  if (anchor_)
    return anchor_->linkState.target;
  else
    return TargetSelf;
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
  impl_->setToolTip(text);
}

WString WAbstractArea::toolTip() const
{
  return impl_->toolTip();
}

void WAbstractArea::setStyleClass(const WT_USTRING& styleClass)
{
  impl_->setStyleClass(styleClass);
}

void WAbstractArea::setStyleClass(const char *styleClass)
{
  impl_->setStyleClass(styleClass);
}

WT_USTRING WAbstractArea::styleClass() const
{
  return impl_->styleClass();
}

void WAbstractArea::addStyleClass(const WT_USTRING& styleClass, bool force)
{
  impl_->addStyleClass(styleClass, force);
}

void WAbstractArea::removeStyleClass(const WT_USTRING& styleClass, bool force)
{
  impl_->removeStyleClass(styleClass, force);
}

void WAbstractArea::setCursor(Cursor cursor)
{
  impl_->decorationStyle().setCursor(cursor);
}

void WAbstractArea::setCursor(std::string cursorImage, Cursor fallback)
{
  impl_->decorationStyle().setCursor(cursorImage, fallback);
}

Cursor WAbstractArea::cursor() const
{
  return impl_->decorationStyle().cursor();
}

void WAbstractArea::createAnchorImpl()
{
  if (!anchor_)
    anchor_ = new AnchorImpl();
}

void WAbstractArea::resourceChanged()
{
  repaint();
}

void WAbstractArea::repaint()
{
  impl_->repaint();
}

bool WAbstractArea::updateDom(DomElement& element, bool all)
{
  bool needsUrlResolution = false;

  if (!hole_ && anchor_) {
    needsUrlResolution = WAnchor::renderHRef(impl(), anchor_->linkState,
					     element);
    WAnchor::renderHTarget(anchor_->linkState, element, all);
    element.setAttribute("alt", anchor_->altText.toUTF8());
  } else {
    element.setAttribute("alt", "");

    if (hole_)
      element.setAttribute("nohref", "nohref");
  }

  return needsUrlResolution;
}

WInteractWidget *WAbstractArea::impl()
{
  return impl_;
}

WAbstractArea *WAbstractArea::areaForImpl(WWidget *w)
{
  Impl::AreaWidget *aw = dynamic_cast<Impl::AreaWidget *>(w);

  if (!aw)
    throw WException("WAbstractArea::areaForImpl could not dynamic_cast?");

  return aw->facade();
}

}
