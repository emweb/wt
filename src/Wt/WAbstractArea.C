/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WAbstractArea"

#include "DomElement.h"
#include "WtException.h"

namespace Wt {

  namespace Impl {

    class AreaWidget : public WInteractWidget
    {
    public:
      AreaWidget(WAbstractArea *facade)
	: facade_(facade)
      { }

      void repaint() {
	WInteractWidget::repaint(RepaintPropertyAttribute);
      }

      WAbstractArea *facade() const { return facade_; }

    private:
      WAbstractArea *facade_;

    protected:
      virtual void updateDom(DomElement& element, bool all)
      {
	facade_->updateDom(element, all);

	WInteractWidget::updateDom(element, all);

	if (!element.getProperty(PropertyStyleCursor).empty()
	    && !wApp->environment().agentGecko()
	    && element.getAttribute("href").empty())
	  element.setAttribute("href", "#");
      }

      virtual DomElementType domElementType() const {
	return DomElement_AREA;
      }
    };

  }

WAbstractArea::WAbstractArea()
  : impl_(new Impl::AreaWidget(this)),
    keyWentDown(impl_->keyWentDown),
    keyPressed(impl_->keyPressed),
    keyWentUp(impl_->keyWentUp),
    enterPressed(impl_->enterPressed),
    escapePressed(impl_->escapePressed),
    clicked(impl_->clicked),
    doubleClicked(impl_->doubleClicked),
    mouseWentDown(impl_->mouseWentDown),
    mouseWentUp(impl_->mouseWentUp),
    mouseWentOut(impl_->mouseWentOut),
    mouseWentOver(impl_->mouseWentOver),
    mouseMoved(impl_->mouseMoved),
    hole_(false),
    anchor_(0)
{ }

WAbstractArea::~WAbstractArea()
{
  WImage *i = image();

  if (i)
    i->removeArea(this);

  delete anchor_;
  delete impl_;
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
  createAnchorImpl();

  anchor_->ref_ = ref;

  repaint();
}

const std::string WAbstractArea::ref() const
{
  if (anchor_)
    return anchor_->ref_;
  else
    return std::string();
}

void WAbstractArea::setHole(bool hole)
{
  hole_ = hole;

  repaint();
}

void WAbstractArea::setResource(WResource *resource)
{
  createAnchorImpl();

  anchor_->resource_ = resource;
  anchor_->resource_->dataChanged.connect(SLOT(this,
					       WAbstractArea::resourceChanged));

  setRef(resource->generateUrl());
}

WResource *WAbstractArea::resource() const
{
  if (anchor_)
    return anchor_->resource_;
  else
    return 0;
}

void WAbstractArea::setTarget(AnchorTarget target)
{
  createAnchorImpl();

  anchor_->target_ = target;

  repaint();
}

AnchorTarget WAbstractArea::target() const
{
  if (anchor_)
    return anchor_->target_;
  else
    return TargetSelf;
}

void WAbstractArea::setAlternateText(const WString& text)
{
  createAnchorImpl();

  anchor_->altText_ = text;

  repaint();
}

const WString WAbstractArea::alternateText() const
{
  if (anchor_)
    return anchor_->altText_;
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

void WAbstractArea::setStyleClass(const WString& styleClass)
{
  impl_->setStyleClass(styleClass);
}

void WAbstractArea::setStyleClass(const char *styleClass)
{
  impl_->setStyleClass(styleClass);
}

WString WAbstractArea::styleClass() const
{
  return impl_->styleClass();
}

void WAbstractArea::setCursor(Cursor cursor)
{
  impl_->decorationStyle().setCursor(cursor);
}

Cursor WAbstractArea::cursor() const
{
  return impl_->decorationStyle().cursor();
}

void WAbstractArea::createAnchorImpl()
{
  if (!anchor_) {
    anchor_ = new AnchorImpl();
    anchor_->resource_ = 0;
    anchor_->target_ = TargetSelf;
  }
}

void WAbstractArea::resourceChanged()
{
  setRef(anchor_->resource_->generateUrl());
}

void WAbstractArea::repaint()
{
  impl_->repaint();
}

void WAbstractArea::updateDom(DomElement& element, bool all)
{
  if (!hole_ && anchor_) {
    element.setAttribute("href", WWebWidget::fixRelativeUrl(anchor_->ref_));

    switch (anchor_->target_) {
    case TargetSelf:
      break;
    case TargetThisWindow:
      element.setAttribute("target", "_top");
      break;
    case TargetNewWindow:
      element.setAttribute("target", "_blank");
    }
    element.setAttribute("alt", anchor_->altText_.toUTF8());
  } else {
    element.setAttribute("alt", "");
    if (hole_)
      element.setAttribute("nohref", "nohref");
  }
}

WInteractWidget *WAbstractArea::impl()
{
  return impl_;
}

WAbstractArea *WAbstractArea::areaForImpl(WWidget *w)
{
  Impl::AreaWidget *aw = dynamic_cast<Impl::AreaWidget *>(w);

  if (!aw)
    throw WtException("WAbstractArea::areaForImpl could not dynamic_cast?");

  return aw->facade();
}

}
