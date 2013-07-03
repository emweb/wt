/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WImage"
#include "Wt/WAbstractArea"
#include "Wt/WLogger"
#include "Wt/WResource"

#include "DomElement.h"

namespace Wt {

LOGGER("WImage");

  namespace Impl {

    class MapWidget : public WContainerWidget
    {
    public:
      MapWidget() { }

    protected:
      virtual void updateDom(DomElement& element, bool all)
      {
	if (all)
	  element.setAttribute("name", id());

	WContainerWidget::updateDom(element, all);
      }

      virtual DomElementType domElementType() const {
	return DomElement_MAP;
      }
    };

  }

const char *WImage::LOAD_SIGNAL = "load";

WImage::WImage(WContainerWidget *parent)
  : WInteractWidget(parent),
    map_(0)
{
  setLoadLaterWhenInvisible(false);
}

WImage::WImage(const WLink& link, WContainerWidget *parent)
  : WInteractWidget(parent),
    map_(0)
{ 
  setLoadLaterWhenInvisible(false);

  setImageLink(link);
}

WImage::WImage(const WLink& link, const WString& altText,
	       WContainerWidget *parent)
  : WInteractWidget(parent),
    altText_(altText),
    map_(0)
{ 
  setLoadLaterWhenInvisible(false);

  setImageLink(link);
}

#ifdef WT_TARGET_JAVA
WImage::WImage(const std::string& imageRef, WContainerWidget *parent)
  : WInteractWidget(parent),
    imageLink_(WLink(WLink::Url, imageRef)),
    map_(0)
{
  setLoadLaterWhenInvisible(false);
}

WImage::WImage(const std::string& imageRef, const WString& altText,
	       WContainerWidget *parent)
  : WInteractWidget(parent),
    imageLink_(WLink(WLink::Url, imageRef)),
    altText_(altText),
    map_(0)
{
  setLoadLaterWhenInvisible(false);
}

WImage::WImage(WResource *resource, const WString& altText,
	       WContainerWidget *parent)
  : WInteractWidget(parent),
    altText_(altText),
    map_(0)
{
  setLoadLaterWhenInvisible(false);
  setImageLink(WLink(resource));
}
#endif // WT_TARGET_JAVA

WImage::~WImage()
{ 
  delete map_;
}

EventSignal<>& WImage::imageLoaded()
{
  return *voidEventSignal(LOAD_SIGNAL, true);
}

void WImage::setImageLink(const WLink& link)
{
  if (link.type() != WLink::Resource && canOptimizeUpdates()
      && (link == imageLink_))
    return;

  imageLink_ = link;

  if (link.type() == WLink::Resource)
    link.resource()->dataChanged().connect(this, &WImage::resourceChanged);

  flags_.set(BIT_IMAGE_LINK_CHANGED);

  repaint(RepaintSizeAffected);
}

void WImage::setResource(WResource *resource)
{
  setImageLink(WLink(resource));
}

WResource *WImage::resource() const
{
  return imageLink_.resource();
}

void WImage::setImageRef(const std::string& ref)
{
  setImageLink(WLink(ref));
}

const std::string WImage::imageRef() const
{
  return imageLink_.url();
}

void WImage::resourceChanged()
{
  flags_.set(BIT_IMAGE_LINK_CHANGED);
  repaint(RepaintSizeAffected);
}

void WImage::setAlternateText(const WString& text)
{
  if (canOptimizeUpdates() && (text == altText_))
    return;

  altText_ = text;
  flags_.set(BIT_ALT_TEXT_CHANGED);

  repaint();
}

void WImage::addArea(WAbstractArea *area)
{
  insertArea(map_ ? map_->count() : 0, area);
}

void WImage::insertArea(int index, WAbstractArea *area)
{
  if (!map_) {
    addChild(map_ = new Impl::MapWidget());
    flags_.set(BIT_MAP_CREATED);
    repaint();
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
    LOG_ERROR("removeArea(): no such area");
    return;
  }

  map_->removeWidget(area->impl());
}

void WImage::updateDom(DomElement& element, bool all)
{
  DomElement *img = &element;
  if (all && element.type() == DomElement_SPAN) {
    DomElement *map = map_->createSDomElement(WApplication::instance());
    element.addChild(map);

    img = DomElement::createNew(DomElement_IMG);
    img->setId("i" + id());
  }

  if (flags_.test(BIT_IMAGE_LINK_CHANGED) || all) {
    if (!imageLink_.isNull()) {
      std::string url = resolveRelativeUrl(imageLink_.url());

      WApplication *app = WApplication::instance();
      url = app->encodeUntrustedUrl(url);

      img->setProperty(Wt::PropertySrc, url);
    } else
      img->setProperty(Wt::PropertySrc, "#");

    flags_.reset(BIT_IMAGE_LINK_CHANGED);
  }

  if (flags_.test(BIT_ALT_TEXT_CHANGED) || all) {
    img->setAttribute("alt", altText_.toUTF8());
    flags_.reset(BIT_ALT_TEXT_CHANGED);
  }

  if (flags_.test(BIT_MAP_CREATED) || (all && map_)) {
    img->setAttribute("usemap", '#' + map_->id());
    flags_.reset(BIT_MAP_CREATED);
  }

  WInteractWidget::updateDom(*img, all);

  if (&element != img)
    element.addChild(img);
}

void WImage::getDomChanges(std::vector<DomElement *>& result,
			   WApplication *app)
{
  if (map_) {
    // TODO: check if BIT_MAP_CREATED: then need to replace the whole
    // element with a <span><img /><map /></span>. Currently we document
    // this as a limitation.
    DomElement *e = DomElement::getForUpdate("i" + id(), DomElement_IMG);
    updateDom(*e, false);
    result.push_back(e);
  } else
    WInteractWidget::getDomChanges(result, app);
}

void WImage::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_IMAGE_LINK_CHANGED);
  flags_.reset(BIT_ALT_TEXT_CHANGED);

  WInteractWidget::propagateRenderOk(deep);
}

DomElementType WImage::domElementType() const
{
  return map_ ? DomElement_SPAN : DomElement_IMG;
}

}
