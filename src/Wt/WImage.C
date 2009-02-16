/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WImage"
#include "Wt/WAbstractArea"
#include "Wt/WLogger"

#include "DomElement.h"

namespace Wt {

  namespace Impl {

    class MapWidget : public WContainerWidget
    {
    public:
      MapWidget() { }

    protected:
      virtual void updateDom(DomElement& element, bool all)
      {
	if (all)
	  element.setAttribute("name", formName());

	WContainerWidget::updateDom(element, all);
      }

      virtual DomElementType domElementType() const {
	return DomElement_MAP;
      }
    };

  }


WImage::WImage(WContainerWidget *parent)
  : WInteractWidget(parent),
    loaded(this),
    resource_(0),
    map_(0)
{
  setLoadLaterWhenInvisible(false);
}

WImage::WImage(const std::string& imageRef, WContainerWidget *parent)
  : WInteractWidget(parent),
    loaded(this),
    imageRef_(imageRef),
    resource_(0),
    map_(0)
{
  setLoadLaterWhenInvisible(false);
}

WImage::WImage(const std::string& imageRef, const WString& altText,
	       WContainerWidget *parent)
  : WInteractWidget(parent),
    loaded(this),
    altText_(altText),
    imageRef_(imageRef),
    resource_(0),
    map_(0)
{
  setLoadLaterWhenInvisible(false);
}

WImage::WImage(WResource *resource, const WString& altText,
	       WContainerWidget *parent)
  : WInteractWidget(parent),
    loaded(this),
    altText_(altText),
    resource_(resource),
    map_(0)
{
  resource_->dataChanged.connect(SLOT(this, WImage::resourceChanged));
  imageRef_ = resource_->generateUrl();

  setLoadLaterWhenInvisible(false);
}

WImage::~WImage()
{ 
  delete map_;
}

void WImage::setResource(WResource *resource)
{
  resource_ = resource;
  resource_->dataChanged.connect(SLOT(this, WImage::resourceChanged));
  setImageRef(resource_->generateUrl());
}

void WImage::setAlternateText(const WString& text)
{
  if (canOptimizeUpdates() && (text == altText_))
    return;

  altText_ = text;
  flags_.set(BIT_ALT_TEXT_CHANGED);

  repaint(RepaintPropertyAttribute);
}

void WImage::setImageRef(const std::string& ref)
{
  if (canOptimizeUpdates() && (ref == imageRef_))
    return;

  imageRef_ = ref;
  flags_.set(BIT_IMAGE_REF_CHANGED);

  repaint(RepaintPropertyIEMobile);
}

const std::string WImage::imageRef() const
{
  return imageRef_;
}

void WImage::resourceChanged()
{
  setImageRef(resource_->generateUrl());
}

void WImage::addArea(WAbstractArea *area)
{
  insertArea(map_ ? map_->count() : 0, area);
}

void WImage::insertArea(int index, WAbstractArea *area)
{
  if (!map_) {
    wApp->domRoot()->addWidget(map_ = new Impl::MapWidget());
    flags_.set(BIT_MAP_CREATED);
    repaint(RepaintPropertyAttribute);
  }

  map_->insertWidget(index, area->impl());
}

WAbstractArea *WImage::area(int index) const
{
  if (map_ && index < map_->count())
    return WAbstractArea::areaForImpl(map_->widget(index));
  else
    return 0;
}

const std::vector<WAbstractArea *> WImage::areas() const
{
  std::vector<WAbstractArea *> result;

  if (map_) {
    for (int i = 0; i < map_->count(); ++i)
      result.push_back(WAbstractArea::areaForImpl(map_->widget(i)));
  }

  return result;
}

void WImage::removeArea(WAbstractArea *area)
{
  if (!map_) {
    wApp->log("error") << "WImage::removeArea(): no such area";
    return;
  }

  map_->removeWidget(area->impl());
}

void WImage::updateDom(DomElement& element, bool all)
{
  if (flags_.test(BIT_IMAGE_REF_CHANGED) || all) {
    if (!imageRef_.empty())
      element.setProperty(Wt::PropertySrc, fixRelativeUrl(imageRef_));
    flags_.reset(BIT_IMAGE_REF_CHANGED);
  }

  updateSignalConnection(element, loaded, "load", all);

  if (flags_.test(BIT_ALT_TEXT_CHANGED) || all) {
    element.setAttribute("alt", altText_.toUTF8());
    flags_.reset(BIT_ALT_TEXT_CHANGED);
  }

  if (flags_.test(BIT_MAP_CREATED) || (all && map_)) {
    element.setAttribute("usemap", '#' + map_->formName());
    flags_.reset(BIT_MAP_CREATED);
  }

  WInteractWidget::updateDom(element, all);
}

DomElementType WImage::domElementType() const
{
  return DomElement_IMG;
}

}
