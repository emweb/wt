/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WImage.h"
#include "Wt/WAbstractArea.h"
#include "Wt/WLogger.h"
#include "Wt/WResource.h"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WImage.min.js"
#endif

namespace Wt {

LOGGER("WImage");

namespace Impl {

class MapWidget : public WContainerWidget
{
public:
  MapWidget() { }

  void insertArea(int index, std::unique_ptr<WAbstractArea> area) {
    insertWidget(index, area->takeWidget());
    areas_.insert(areas_.begin(), std::move(area));
  }

  std::unique_ptr<WAbstractArea> removeArea(WAbstractArea *area) {
    int index = indexOf(area->widget());
    if (index != -1) {
      area->returnWidget(removeWidget(area->widget()));
      return Utils::take(areas_, area);
    } else
      return std::unique_ptr<WAbstractArea>();
  }

  WAbstractArea *area(int index) {
    return areas_[index].get();
  }

protected:
  virtual void render(WFlags<RenderFlag> flags) override
  {
    WContainerWidget::render(flags);

    WImage *parent_img = dynamic_cast<WImage *>(parent());
    if(parent_img){
      if (!parent_img->targetJS_.empty())
        parent_img->doJavaScript(parent_img->setAreaCoordsJS());
    }
  }

  virtual void updateDom(DomElement& element, bool all) override
  {
    if (all)
      element.setAttribute("name", id());

    WContainerWidget::updateDom(element, all);
  }

  virtual DomElementType domElementType() const override
  {
    return DomElementType::MAP;
  }

private:
  std::vector<std::unique_ptr<WAbstractArea> > areas_;
};

}

const char *WImage::LOAD_SIGNAL = "load";

WImage::WImage()
{
  setLoadLaterWhenInvisible(false);
}

WImage::WImage(const WLink& link)
{ 
  setLoadLaterWhenInvisible(false);

  setImageLink(link);
}

WImage::WImage(const WLink& link, const WString& altText)
  : altText_(altText)
{ 
  setLoadLaterWhenInvisible(false);

  setImageLink(link);
}

WImage::~WImage()
{
  manageWidget(map_, std::unique_ptr<Impl::MapWidget>());
}

EventSignal<>& WImage::imageLoaded()
{
  return *voidEventSignal(LOAD_SIGNAL, true);
}

void WImage::setImageLink(const WLink& link)
{
  if (link.type() != LinkType::Resource && canOptimizeUpdates()
      && (link == imageLink_))
    return;

  imageLink_ = link;

  if (link.type() == LinkType::Resource)
    link.resource()->dataChanged().connect(this, &WImage::resourceChanged);

  flags_.set(BIT_IMAGE_LINK_CHANGED);

  repaint(RepaintFlag::SizeAffected);
}

void WImage::resourceChanged()
{
  flags_.set(BIT_IMAGE_LINK_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

void WImage::setAlternateText(const WString& text)
{
  if (canOptimizeUpdates() && (text == altText_))
    return;

  altText_ = text;
  flags_.set(BIT_ALT_TEXT_CHANGED);

  repaint();
}

void WImage::addArea(std::unique_ptr<WAbstractArea> area)
{
  insertArea(map_ ? map_->count() : 0, std::move(area));
}

void WImage::insertArea(int index, std::unique_ptr<WAbstractArea> area)
{
  if (!map_) {
    auto map = cpp14::make_unique<Impl::MapWidget>();
    manageWidget(map_, std::move(map));
    flags_.set(BIT_MAP_CREATED);
    repaint();
  }

  map_->insertArea(index, std::move(area));
}

WAbstractArea *WImage::area(int index) const
{
  if (map_ && index < map_->count())
    return map_->area(index);
  else
    return nullptr;
}

const std::vector<WAbstractArea *> WImage::areas() const
{
  std::vector<WAbstractArea *> result;

  if (map_) {
    for (int i = 0; i < map_->count(); ++i)
      result.push_back(map_->area(i));
  }

  return result;
}

std::unique_ptr<WAbstractArea> WImage::removeArea(WAbstractArea *area)
{
  std::unique_ptr<WAbstractArea> result;

  if (map_)
    result = map_->removeArea(area);

  if (!result)
    LOG_ERROR("removeArea(): area was not found");

  return result;
}

void WImage::updateDom(DomElement& element, bool all)
{
  DomElement *img = &element;
  if (all && element.type() == DomElementType::SPAN) {
    DomElement *map = map_->createSDomElement(WApplication::instance());
    element.addChild(map);

    img = DomElement::createNew(DomElementType::IMG);
    img->setId("i" + id());
  }

  if (flags_.test(BIT_IMAGE_LINK_CHANGED) || all) {
    std::string url;
    WApplication *app = WApplication::instance();
    if (!imageLink_.isNull()) {
      url = resolveRelativeUrl(imageLink_.url());
      url = app->encodeUntrustedUrl(url);
    } else {
      url = app->onePixelGifUrl();
    }

    img->setProperty(Wt::Property::Src, url);

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
    DomElement *e = DomElement::getForUpdate("i" + id(), DomElementType::IMG);
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

void WImage::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WImage.js", "WImage", wtjs1);

  WStringStream ss;
  ss << "new " WT_CLASS ".WImage("
     << app->javaScriptClass() << "," << jsRef() << "," << targetJS_ << ");";
  doJavaScript(ss.str());
}

void WImage::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full)) {
    if (!targetJS_.empty())
      defineJavaScript();
  }

  WInteractWidget::render(flags);
}

DomElementType WImage::domElementType() const
{
  return map_ ? DomElementType::SPAN : DomElementType::IMG;
}

void WImage::setTargetJS(std::string targetJS)
{
  targetJS_ = targetJS;
}

std::string WImage::updateAreasJS()
{
  WStringStream ss;
  if (!targetJS_.empty()) {
    ss <<
      "(function(){"
      """var w = " << jsRef() << ";"
      """if (w && w.wtObj) { w.wtObj.updateAreas(); }"
      "})();";
  }
  return ss.str();
}

std::string WImage::setAreaCoordsJS()
{
  WStringStream ss;
  if (!targetJS_.empty()) {
    ss << jsRef() << ".wtObj.setAreaCoordsJSON("
       << updateAreaCoordsJSON() << ");";
  }
  return ss.str();
}

std::string WImage::updateAreaCoordsJSON() const
{
  WStringStream js;
  const std::vector<WAbstractArea *> &areas = this->areas();

  if (!areas.empty()) {
    for (unsigned i = 0; i < areas.size(); ++i) {
      if (areas[i]->isTransformable()) {
        if (js.empty())
          js << "[";
        else
          js << ",";
        js << areas[i]->updateAreaCoordsJS();
      }
    }
    js << "]";
  } else
    js << "null";

  return js.str();
}

}
