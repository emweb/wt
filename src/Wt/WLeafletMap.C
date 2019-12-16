/*
 * Copyright (C) 2019 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLeafletMap.h"

#include "Wt/WApplication.h"
#include "Wt/WBrush.h"
#include "Wt/WColor.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WCssStyleSheet.h"
#include "Wt/WJavaScriptPreamble.h"
#include "Wt/WLink.h"
#include "Wt/WLogger.h"
#include "Wt/WPen.h"
#include "Wt/WStringStream.h"

#include "Wt/Json/Array.h"
#include "Wt/Json/Parser.h"
#include "Wt/Json/Serializer.h"
#include "Wt/Json/Value.h"

#include "web/DomElement.h"
#include "web/EscapeOStream.h"
#include "web/WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WLeafletMap.min.js"
#endif

namespace Wt {

LOGGER("WLeafletMap");

const std::string WLeafletMap::WIDGETMARKER_CONTAINER_RULENAME = "WLeafletMap::WidgetMarker::container";
const std::string WLeafletMap::WIDGETMARKER_CONTAINER_CHILDREN_RULENAME = "WLeafletMap::WidgetMarker::container-children";

#define WIDGETMARKER_CONTAINER_CLASS "Wt-leaflet-widgetmarker-container"

class WLeafletMap::Impl : public WWebWidget {
public:
  Impl();

  virtual DomElementType domElementType() const override;
};

WLeafletMap::Impl::Impl()
{
  setInline(false);
}

DomElementType WLeafletMap::Impl::domElementType() const
{
  return DomElementType::DIV;
}

struct WLeafletMap::Overlay {
  virtual ~Overlay();
  virtual void addJS(WStringStream &ss,
                     WLeafletMap *map) const = 0;

  Overlay(const Overlay &) = delete;
  Overlay& operator=(const Overlay &) = delete;
  Overlay(Overlay &&) = delete;
  Overlay& operator=(Overlay &&) = delete;

protected:
  Overlay();
};

struct WLeafletMap::Polyline : WLeafletMap::Overlay {
  std::vector<Coordinate> points;
  WPen pen;

  Polyline(const std::vector<Coordinate> &points, const WPen &pen);
  virtual ~Polyline() override;
  Polyline(const Polyline &) = delete;
  Polyline& operator=(const Polyline &) = delete;
  Polyline(Polyline &&) = delete;
  Polyline& operator=(Polyline &&) = delete;
  virtual void addJS(WStringStream &ss, WLeafletMap *map) const override;
};

struct WLeafletMap::Circle : WLeafletMap::Overlay {
  Coordinate center;
  double radius;
  WPen stroke;
  WBrush fill;

  Circle(const Coordinate &center, double radius, const WPen &stroke, const WBrush &fill);
  virtual ~Circle() override;

  Circle(const Circle &) = delete;
  Circle& operator=(const Circle &) = delete;
  Circle(Circle &&) = delete;
  Circle& operator=(Circle &&) = delete;

  virtual void addJS(WStringStream &ss, WLeafletMap *map) const override;
};

WLeafletMap::Overlay::Overlay()
{ }

WLeafletMap::Overlay::~Overlay()
{ }

WLeafletMap::Polyline::Polyline(const std::vector<WLeafletMap::Coordinate> &points,
                                const WPen &pen)
  : points(points),
    pen(pen)
{ }

WLeafletMap::Polyline::~Polyline()
{ }

void WLeafletMap::Polyline::addJS(WStringStream &ss,
                                  WLeafletMap *map) const
{
  if (pen.style() == PenStyle::None)
    return;

  Json::Object options;
  addPathOptions(options, pen, BrushStyle::None);
  std::string optionsStr = Json::serialize(options);

  EscapeOStream es(ss);
  es << "var o=" << map->jsRef() << ";if(o && o.wtObj){"
        "o.wtObj.addPolyline(";
  es << "[";
  for (std::size_t i = 0; i < points.size(); ++i) {
    if (i != 0)
      es << ',';
    es << "[";
    char buf[30];
    es << Utils::round_js_str(points[i].latitude(), 16, buf) << ",";
    es << Utils::round_js_str(points[i].longitude(), 16, buf);
    es << "]";
  }
  es << "],'";
  es.pushEscape(EscapeOStream::JsStringLiteralSQuote);
  es << optionsStr;
  es.popEscape();
  es << "');}";
}

WLeafletMap::Circle::Circle(const Coordinate &center,
                            double radius,
                            const WPen &stroke,
                            const WBrush &fill)
  : center(center),
    radius(radius),
    stroke(stroke),
    fill(fill)
{ }

WLeafletMap::Circle::~Circle()
{ }

void WLeafletMap::Circle::addJS(WStringStream &ss,
                                WLeafletMap *map) const
{
  Json::Object options;
  options["radius"] = Json::Value(radius);
  addPathOptions(options, stroke, fill);
  std::string optionsStr = Json::serialize(options);

  EscapeOStream es(ss);
  es << "var o=" << map->jsRef() << ";if(o && o.wtObj){"
     << "" "o.wtObj.addCircle([";
  char buf[30];
  es << Utils::round_js_str(center.latitude(), 16, buf) << ",";
  es << Utils::round_js_str(center.longitude(), 16, buf) << "],'";
  es.pushEscape(EscapeOStream::JsStringLiteralSQuote);
  es << optionsStr;
  es.popEscape();
  es << "');}";
}

WLeafletMap::Coordinate::Coordinate()
  : lat_(0.0),
    lng_(0.0)
{ }

WLeafletMap::Coordinate::Coordinate(double latitude, double longitude)
  : lat_(latitude),
    lng_(longitude)
{ }

void WLeafletMap::Coordinate::setLatitude(double latitude)
{
  lat_ = latitude;
}

void WLeafletMap::Coordinate::setLongitude(double longitude)
{
  lng_ = longitude;
}

WLeafletMap::Marker::Marker(const Coordinate &pos)
  : pos_(pos),
    map_(nullptr),
    moved_(false)
{ }

WLeafletMap::Marker::~Marker()
{ }

void WLeafletMap::Marker::move(const Coordinate &pos)
{
  pos_ = pos;
  if (map_) {
    moved_ = true;
    map_->scheduleRender();
  }
}

void WLeafletMap::Marker::setMap(WLeafletMap *map)
{
  map_ = map;
}

void WLeafletMap::Marker::unrender()
{ }

bool WLeafletMap::Marker::needsUpdate() const
{
  return false;
}

void WLeafletMap::Marker::update(WStringStream &js)
{ }

WLeafletMap::WidgetMarker::WidgetMarker(const Coordinate &pos,
                                        std::unique_ptr<WWidget> widget)
  : Marker(pos),
    container_(nullptr),
    anchorX_(-1),
    anchorY_(-1),
    anchorPointChanged_(false)
{
  createContainer();
  container_->addWidget(std::move(widget));
}

WLeafletMap::WidgetMarker::~WidgetMarker()
{
  container_.reset();
}

WWidget *WLeafletMap::WidgetMarker::widget()
{
  if (container_ && container_->count() > 0) {
    return container_->widget(0);
  }
  return 0;
}

const WWidget *WLeafletMap::WidgetMarker::widget() const
{
  if (container_ && container_->count() > 0) {
    return container_->widget(0);
  }
  return 0;
}

void WLeafletMap::WidgetMarker::setMap(WLeafletMap *map)
{
  Marker::setMap(map);

  if (container_) {
    container_->setParentWidget(map);
  }
}

void WLeafletMap::WidgetMarker::createMarkerJS(WStringStream &ss, WStringStream &postJS) const
{
  std::unique_ptr<DomElement> element(container_->createSDomElement(WApplication::instance()));

  DomElement::TimeoutList timeouts;

  char buf[30];

  if (anchorX_ >= 0 || anchorY_ >= 0) {
    updateAnchorJS(postJS);
  }

  EscapeOStream js(postJS);

  EscapeOStream es(ss);
  es << "(function(){";
  es << "var wIcon=L.divIcon({"
        "className:'',"
        "iconSize:null,"
        "iconAnchor:null,";
  es << "html:'";
  es.pushEscape(EscapeOStream::JsStringLiteralSQuote);
  element->asHTML(es, js, timeouts);
  es.popEscape();
  es << "'});";
  es << "return L.marker([";
  es << Utils::round_js_str(position().latitude(), 16, buf) << ",";
  es << Utils::round_js_str(position().longitude(), 16, buf) << "],";
  es << "{"
          "interactive:false,"
          "icon:wIcon,"
          "keyboard:false"
        "});})()";
}

void WLeafletMap::WidgetMarker::setAnchorPoint(double x, double y)
{
  anchorX_ = x;
  anchorY_ = y;

  if (map() && map()->isRendered()) {
    anchorPointChanged_ = true;
    map()->scheduleRender();
  }
}

void WLeafletMap::WidgetMarker::unrender()
{
  WWidget *w = widget();
  std::unique_ptr<WWidget> uW;
  if (w) {
    uW = container_->removeWidget(w);
  }
  container_.reset();
  createContainer();
  if (uW) {
    container_->addWidget(std::move(uW));
  }
}

void WLeafletMap::WidgetMarker::createContainer()
{
  container_.reset(new Wt::WContainerWidget());
  container_->addStyleClass(WIDGETMARKER_CONTAINER_CLASS);
  container_->setJavaScriptMember("wtReparentBarrier", "true");
}

bool WLeafletMap::WidgetMarker::needsUpdate() const
{
  return anchorPointChanged_;
}

void WLeafletMap::WidgetMarker::update(WStringStream &js)
{
  if (anchorPointChanged_) {
    updateAnchorJS(js);
    anchorPointChanged_ = false;
  }
}

void WLeafletMap::WidgetMarker::updateAnchorJS(WStringStream &js) const
{
  char buf[30];
  js << "var o=" << container_->jsRef() << ";if(o){"
        "" "o.style.transform='translate(";
  if (anchorX_ >= 0) {
    js << Utils::round_js_str(-anchorX_, 16, buf) << "px";
  } else {
    js << "-50%";
  }
  js << ',';
  if (anchorY_ >= 0) {
    js << Utils::round_js_str(-anchorY_, 16, buf) << "px";
  } else {
    js << "-50%";
  }
  js << ")';"
        "}";
}

WLeafletMap::LeafletMarker::LeafletMarker(const Coordinate &pos)
  : Marker(pos)
{ }

WLeafletMap::LeafletMarker::~LeafletMarker()
{ }

void WLeafletMap::LeafletMarker::createMarkerJS(WStringStream &ss, WStringStream &) const
{
  ss << "L.marker([";
  char buf[30];
  ss << Utils::round_js_str(position().latitude(), 16, buf) << ",";
  ss << Utils::round_js_str(position().longitude(), 16, buf) << "])";
}

WLeafletMap::WLeafletMap()
  : impl_(nullptr),
    options_(),
    zoomLevelChanged_(this, "zoomLevelChanged"),
    panChanged_(this, "panChanged"),
    zoomLevel_(13),
    nextMarkerId_(0),
    renderedTileLayersSize_(0),
    renderedOverlaysSize_(0)
{
  setup();
}

WLeafletMap::WLeafletMap(const Json::Object &options)
  : options_(options),
    zoomLevelChanged_(this, "zoomLevelChanged"),
    panChanged_(this, "panChanged"),
    zoomLevel_(13),
    nextMarkerId_(0),
    renderedTileLayersSize_(0),
    renderedOverlaysSize_(0)
{
  setup();
}

// called from constructors to reduce code duplication (not currently designed to be run again)
void WLeafletMap::setup()
{
  setImplementation(std::unique_ptr<Impl>(impl_ = new Impl()));

  zoomLevelChanged().connect(this, &WLeafletMap::handleZoomLevelChanged);
  panChanged().connect(this, &WLeafletMap::handlePanChanged);

  WApplication *app = WApplication::instance();
  if (app) {
    if (!app->styleSheet().isDefined(WIDGETMARKER_CONTAINER_RULENAME)) {
      app->styleSheet().addRule("." WIDGETMARKER_CONTAINER_CLASS, "transform: translate(-50%, -50%);", WIDGETMARKER_CONTAINER_RULENAME);
    }
    if (!app->styleSheet().isDefined(WIDGETMARKER_CONTAINER_CHILDREN_RULENAME)) {
      app->styleSheet().addRule("." WIDGETMARKER_CONTAINER_CLASS " > *", "pointer-events: auto;", WIDGETMARKER_CONTAINER_CHILDREN_RULENAME);
    }

    std::string leafletJSURL;
    std::string leafletCSSURL;
    Wt::WApplication::readConfigurationProperty("leafletJSURL", leafletJSURL);
    Wt::WApplication::readConfigurationProperty("leafletCSSURL", leafletCSSURL);
    if (!leafletJSURL.empty() &&
        !leafletCSSURL.empty()) {
      app->require(leafletJSURL);
      app->useStyleSheet(WLink(leafletCSSURL));
    } else {
      throw Wt::WException("Trying to create a WLeafletMap, but the leafletJSURL and/or leafletCSSURL properties are not configured");
    }
  } else {
    throw Wt::WException("Trying to create a WLeafletMap without an active WApplication");
  }
}

void WLeafletMap::setOptions(const Json::Object &options)
{
  options_ = options;
  flags_.set(BIT_OPTIONS_CHANGED);

  if (isRendered()) {
    for (std::size_t i = 0; i < markers_.size(); ++i) {
      if (!markers_[i].flags.test(MarkerEntry::BIT_ADDED) &&
          !markers_[i].flags.test(MarkerEntry::BIT_REMOVED)) {
        markers_[i].marker->unrender();
      }
    }
  }

  scheduleRender();
}

WLeafletMap::~WLeafletMap()
{ }

void WLeafletMap::addTileLayer(const std::string &urlTemplate,
                               const Json::Object &options)
{
  TileLayer layer;
  layer.urlTemplate = urlTemplate;
  layer.options = options;
  tileLayers_.push_back(layer);

  scheduleRender();
}

void WLeafletMap::addTileLayerJS(WStringStream &ss, const TileLayer &layer) const
{
  std::string optionsStr = Json::serialize(layer.options);

  EscapeOStream es(ss);
  es << "var o=" << jsRef() << ";if(o && o.wtObj){"
        "" "o.wtObj.addTileLayer('";
  es.pushEscape(EscapeOStream::JsStringLiteralSQuote);
  es << layer.urlTemplate;
  es.popEscape();
  es << "','";
  es.pushEscape(EscapeOStream::JsStringLiteralSQuote);
  es << optionsStr;
  es.popEscape();
  es << "');}";
}

void WLeafletMap::addMarker(std::unique_ptr<Marker> marker)
{
  marker->setMap(this);

  for (std::size_t i = 0; i < markers_.size(); ++i) {
    if (markers_[i].marker == marker.get() &&
        markers_[i].flags.test(MarkerEntry::BIT_REMOVED)) {
      markers_[i].uMarker = std::move(marker);
      markers_[i].flags.reset(MarkerEntry::BIT_REMOVED);
      return;
    }
  }

  MarkerEntry entry;
  entry.uMarker = std::move(marker);
  entry.marker = entry.uMarker.get();
  entry.flags.set(MarkerEntry::BIT_ADDED);
  entry.id = nextMarkerId_;
  ++nextMarkerId_;

  markers_.push_back(std::move(entry));

  scheduleRender();
}

void WLeafletMap::addMarkerJS(WStringStream &ss, long long id, const Marker *marker) const
{
  WStringStream js;
  ss << "var o=" << jsRef() << ";if(o && o.wtObj){"
        "" "o.wtObj.addMarker(" << id << ',';
  marker->createMarkerJS(ss, js);
  ss << ");";
  ss << js.str();
  ss << "}";
}

std::unique_ptr<WLeafletMap::Marker> WLeafletMap::removeMarker(Marker *marker)
{
  for (std::size_t i = 0; i < markers_.size(); ++i) {
    if (markers_[i].uMarker.get() == marker &&
        markers_[i].marker == marker) {
      marker->setMap(nullptr);
      std::unique_ptr<Marker> result = std::move(markers_[i].uMarker);
      if (markers_[i].flags.test(MarkerEntry::BIT_ADDED)) {
        markers_.erase(markers_.begin() + i);
        return result;
      }
      markers_[i].flags.set(MarkerEntry::BIT_REMOVED);
      scheduleRender();
      return result;
    }
  }
  return nullptr;
}

void WLeafletMap::removeMarkerJS(WStringStream &ss, long long id) const
{
  ss << "var o=" << jsRef() << ";if(o && o.wtObj){"
     << "" "o.wtObj.removeMarker(" << id << ");"
        "}";
}

void WLeafletMap::moveMarkerJS(WStringStream &ss,
                               long long id,
                               const Coordinate &position) const
{
  ss << "var o=" << jsRef() << ";if(o && o.wtObj){"
     << "" "o.wtObj.moveMarker(" << id << ",[";
  char buf[30];
  ss << Utils::round_js_str(position.latitude(), 16, buf) << ",";
  ss << Utils::round_js_str(position.longitude(), 16, buf) << "]);";
  ss << "}";
}

void WLeafletMap::addPolyline(const std::vector<Coordinate> &points,
                              const WPen &pen)
{
  overlays_.push_back(cpp14::make_unique<Polyline>(points, pen));
  scheduleRender();
}

void WLeafletMap::addCircle(const Coordinate &center,
                            double radius,
                            const WPen &stroke,
                            const WBrush &fill)
{
  overlays_.push_back(cpp14::make_unique<Circle>(center, radius, stroke, fill));
  scheduleRender();
}

void WLeafletMap::setZoomLevel(int level)
{
  zoomLevel_ = level;
  flags_.set(BIT_ZOOM_CHANGED);

  scheduleRender();
}

void WLeafletMap::zoomJS(WStringStream &ss,
                         int level) const
{
  ss << "var o=" << jsRef() << ";if(o && o.wtObj){"
        "" "o.wtObj.zoom(" << level << ");"
        "}";
}

void WLeafletMap::panTo(const Coordinate &center)
{
  position_ = center;
  flags_.set(BIT_PAN_CHANGED);

  scheduleRender();
}

void WLeafletMap::panToJS(WStringStream &ss,
                          const Coordinate &position) const
{
  ss << "var o=" << jsRef() << ";if(o && o.wtObj){"
        "" "o.wtObj.panTo(";
  char buf[30];
  ss << Utils::round_js_str(position.latitude(), 16, buf) << ",";
  ss << Utils::round_js_str(position.longitude(), 16, buf) << ");"
        "}";
}

void WLeafletMap::addPathOptions(Json::Object &options,
                                 const WPen &stroke,
                                 const WBrush &fill)
{
  using namespace Wt;
  if (stroke.style() != PenStyle::None) {
    options["stroke"] = Json::Value(true);
    options["color"] = Json::Value(WT_USTRING::fromUTF8(stroke.color().cssText(false)));
    options["opacity"] = Json::Value(stroke.color().alpha() / 255.0);
    double weight = stroke.width().toPixels();
    weight = weight == 0 ? 1.0 : weight;
    options["weight"] = Json::Value(weight);

    std::string capStyle;
    switch (stroke.capStyle()) {
    case PenCapStyle::Flat:
      capStyle = "butt";
      break;
    case PenCapStyle::Square:
      capStyle = "square";
      break;
    case PenCapStyle::Round:
      capStyle = "round";
    }

    options["lineCap"] = Json::Value(WT_USTRING::fromUTF8(capStyle));

    std::string joinStyle;
    switch (stroke.joinStyle()) {
    case PenJoinStyle::Bevel:
      joinStyle = "bevel";
      break;
    case PenJoinStyle::Miter:
      joinStyle = "miter";
      break;
    case PenJoinStyle::Round:
      joinStyle = "round";
    }

    options["lineJoin"] = Json::Value(WT_USTRING::fromUTF8(joinStyle));

    // TODO(Roel): dashArray?
  } else {
    options["stroke"] = Json::Value(false);
  }

  if (fill.style() != BrushStyle::None) {
    options["fill"] = Json::Value(true);
    options["fillColor"] = Json::Value(WT_USTRING::fromUTF8(fill.color().cssText(false)));
    options["fillOpacity"] = Json::Value(fill.color().alpha() / 255.0);
  } else {
    options["fill"] = Json::Value(false);
  }
}

void WLeafletMap::handlePanChanged(double latitude, double longitude)
{
  position_ = Coordinate(latitude, longitude);
}

void WLeafletMap::handleZoomLevelChanged(int zoomLevel)
{
  zoomLevel_ = zoomLevel;
}

void WLeafletMap::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WLeafletMap.js", "WLeafletMap", wtjs1);

  std::string optionsStr = Json::serialize(options_);

  WStringStream ss;
  EscapeOStream es(ss);
  es << "new " WT_CLASS ".WLeafletMap("
     << app->javaScriptClass() << "," << jsRef() << ",'";
  es.pushEscape(EscapeOStream::JsStringLiteralSQuote);
  es << optionsStr;
  es.popEscape();
  es << "',";
  char buf[30];
  es << Utils::round_js_str(position_.latitude(), 16, buf) << ",";
  es << Utils::round_js_str(position_.longitude(), 16, buf) << ",";
  es << Utils::round_js_str(zoomLevel_, 16, buf) << ");";

  setJavaScriptMember(" WLeafletMap", ss.str());
  setJavaScriptMember(WT_RESIZE_JS,
                      jsRef() + ".wtObj.wtResize");
}

void WLeafletMap::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full) || flags_.test(BIT_OPTIONS_CHANGED)) {
    defineJavaScript();

    // Just created, no tile layers or overlays have been rendered yet
    renderedTileLayersSize_ = 0;
    renderedOverlaysSize_ = 0;

    // pan and zoom were already set when creating new WLeafletMap
    flags_.reset(BIT_PAN_CHANGED);
    flags_.reset(BIT_ZOOM_CHANGED);
  }

  WStringStream ss;

  if (flags_.test(BIT_PAN_CHANGED)) {
    panToJS(ss, position_);
    flags_.reset(BIT_PAN_CHANGED);
  }

  if (flags_.test(BIT_ZOOM_CHANGED)) {
    zoomJS(ss, zoomLevel_);
    flags_.reset(BIT_ZOOM_CHANGED);
  }

  for (std::size_t i = renderedTileLayersSize_; i < tileLayers_.size(); ++i) {
    addTileLayerJS(ss, tileLayers_[i]);
  }
  renderedTileLayersSize_ = tileLayers_.size();

  for (std::size_t i = renderedOverlaysSize_; i < overlays_.size(); ++i) {
    overlays_[i]->addJS(ss, this);
  }
  renderedOverlaysSize_ = overlays_.size();

  for (std::size_t i = 0; i < markers_.size();) {
    if (markers_[i].flags.test(MarkerEntry::BIT_REMOVED)) {
      if (!flags_.test(BIT_OPTIONS_CHANGED)) {
        removeMarkerJS(ss, markers_[i].id);
      }
      markers_.erase(markers_.begin() + i);
    } else {
      ++i;
    }
  }

  for (std::size_t i = 0; i < markers_.size(); ++i) {
    if (flags.test(RenderFlag::Full) ||
        flags_.test(BIT_OPTIONS_CHANGED) ||
        markers_[i].flags.test(MarkerEntry::BIT_ADDED)) {
      addMarkerJS(ss, markers_[i].id, markers_[i].marker);
      markers_[i].flags.reset(MarkerEntry::BIT_ADDED);
    } else {
      if (markers_[i].marker->moved_) {
        moveMarkerJS(ss, markers_[i].id, markers_[i].marker->position());
      }
      if (markers_[i].marker->needsUpdate()) {
        markers_[i].marker->update(ss);
      }
    }
    markers_[i].marker->moved_ = false;
  }

  if (!ss.empty()) {
    doJavaScript(ss.str());
  }

  flags_.reset(BIT_OPTIONS_CHANGED);

  WCompositeWidget::render(flags);
}

std::string WLeafletMap::mapJsRef() const
{
  return "((function(){var o=" + jsRef() + ";if(o&&o.wtObj){return o.wtObj.map;}return null;})())";
}

WLeafletMap::MarkerEntry::MarkerEntry()
  : uMarker(nullptr),
    marker(nullptr),
    id(-1),
    flags()
{ }

}
