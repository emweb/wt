/*
 * Copyright (C) 2019 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLeafletMap"

#include "Wt/WApplication"
#include "Wt/WBrush"
#include "Wt/WColor"
#include "Wt/WContainerWidget"
#include "Wt/WJavaScriptPreamble"
#include "Wt/WLink"
#include "Wt/WLogger"
#include "Wt/WPen"
#include "Wt/WStringStream"

#include "Wt/Json/Array"
#include "Wt/Json/Parser"
#include "Wt/Json/Serializer"
#include "Wt/Json/Value"

#include "web/DomElement.h"
#include "web/EscapeOStream.h"
#include "web/WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WLeafletMap.min.js"
#endif

namespace Wt {

LOGGER("WLeafletMap");

class WLeafletMap::Impl : public WWebWidget {
public:
  Impl();

  virtual DomElementType domElementType() const WT_CXX11ONLY(override);
};

WLeafletMap::Impl::Impl()
{
  setInline(false);
  setIgnoreChildRemoves(true);
}

DomElementType WLeafletMap::Impl::domElementType() const
{
  return DomElement_DIV;
}

struct WLeafletMap::Overlay {
  virtual ~Overlay();
  virtual void addJS(WStringStream &ss,
                     WLeafletMap *map) const = 0;

protected:
  Overlay();

#ifdef WT_CXX11
public:
#else // WT_CXX11
private:
#endif // WT_CXX11
  Overlay(const Overlay &) WT_CXX11ONLY(= delete);
  Overlay& operator=(const Overlay &) WT_CXX11ONLY(= delete);
#ifdef WT_CXX11
  Overlay(Overlay &&) = delete;
  Overlay& operator=(Overlay &&) = delete;
#endif // WT_CXX11
};

struct WLeafletMap::Polyline : WLeafletMap::Overlay {
  std::vector<Coordinate> points;
  WPen pen;

  Polyline(const std::vector<Coordinate> &points, const WPen &pen);
  virtual ~Polyline() WT_CXX11ONLY(override);
  virtual void addJS(WStringStream &ss, WLeafletMap *map) const WT_CXX11ONLY(override);

#ifndef WT_CXX11
private:
#endif // WT_CXX11
  Polyline(const Polyline &) WT_CXX11ONLY(= delete);
  Polyline& operator=(const Polyline &) WT_CXX11ONLY(= delete);
#ifdef WT_CXX11
  Polyline(Polyline &&) = delete;
  Polyline& operator=(Polyline &&) = delete;
#endif // WT_CXX11
};

struct WLeafletMap::Circle : WLeafletMap::Overlay {
  Coordinate center;
  double radius;
  WPen stroke;
  WBrush fill;

  Circle(const Coordinate &center, double radius, const WPen &stroke, const WBrush &fill);
  virtual ~Circle() WT_CXX11ONLY(override);
  virtual void addJS(WStringStream &ss, WLeafletMap *map) const WT_CXX11ONLY(override);

#ifndef WT_CXX11
private:
#endif // WT_CXX11
  Circle(const Circle &) WT_CXX11ONLY(= delete);
  Circle& operator=(const Circle &) WT_CXX11ONLY(= delete);
#ifdef WT_CXX11
  Circle(Circle &&) = delete;
  Circle& operator=(Circle &&) = delete;
#endif // WT_CXX11
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
  if (pen.style() == NoPen)
    return;

  Json::Object options;
  addPathOptions(options, pen, NoBrush);
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
    map_(0),
    moved_(false)
{ }

WLeafletMap::Marker::~Marker()
{
  if (map_) {
    map_->removeMarker(this);
  }
}

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

WLeafletMap::WidgetMarker::WidgetMarker(const Coordinate &pos,
                                        WWidget *widget)
  : Marker(pos),
    container_(new WContainerWidget())
{
  container_->setJavaScriptMember("wtNoReparent", "true");
  container_->addWidget(widget);
#ifndef WT_TARGET_JAVA
  widget->destroyed().connect(this, &WidgetMarker::widgetDestroyed);
#endif // WT_TARGET_JAVA
}

WLeafletMap::WidgetMarker::~WidgetMarker()
{
  WContainerWidget *c = container_;
  container_ = 0;
  delete c;
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

#ifndef WT_TARGET_JAVA
void WLeafletMap::WidgetMarker::widgetDestroyed()
{
  WContainerWidget *c = container_;
  container_ = 0;
  delete c;
  if (map_) {
    map_->removeMarker(this);
  }
}
#endif // WT_TARGET_JAVA

void WLeafletMap::WidgetMarker::setMap(WLeafletMap *map)
{
  Marker::setMap(map);

  if (container_) {
    container_->setParentWidget(map);
  }
}

void WLeafletMap::WidgetMarker::createMarkerJS(WStringStream &ss, WStringStream &postJS) const
{
  DomElement *element = container_->createSDomElement(WApplication::instance());

  DomElement::TimeoutList timeouts;

  EscapeOStream js(postJS);
  // FIXME: allow control over disabling/enabling pointerdown propagation?
  js << "var o=" << container_->jsRef() << ";if(o){"
        "" "o.addEventListener('pointerdown',function(e){e.stopPropagation();},false);"
        "}";

  EscapeOStream es(ss);
  char buf[30];
  es << "(function(){";
  es << "var wIcon=L.divIcon({"
        "className:'',";
  if (!widget()->width().isAuto() &&
      !widget()->height().isAuto()) {
    es << "iconSize:[";
    es << Utils::round_js_str(widget()->width().toPixels(), 16, buf) << ',';
    es << Utils::round_js_str(widget()->height().toPixels(), 16, buf) << "],";
  }
  es << "html:'";
  es.pushEscape(EscapeOStream::JsStringLiteralSQuote);
  element->asHTML(es, js, timeouts);
  es.popEscape();
  es << "'});";
  es << "return L.marker([";
  es << Utils::round_js_str(position().latitude(), 16, buf) << ",";
  es << Utils::round_js_str(position().longitude(), 16, buf) << "],";
  es << "{"
          "icon:wIcon,"
          "keyboard:false"
        "});})()";

  delete element;
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

WLeafletMap::WLeafletMap(WContainerWidget *parent)
  : WCompositeWidget(parent),
    impl_(new Impl()),
    zoomLevelChanged_(this, "zoomLevelChanged"),
    panChanged_(this, "panChanged"),
    zoomLevel_(13),
    nextMarkerId_(0),
    renderedTileLayersSize_(0),
    renderedOverlaysSize_(0)
{
  setImplementation(impl_);

  zoomLevelChanged().connect(this, &WLeafletMap::handleZoomLevelChanged);
  panChanged().connect(this, &WLeafletMap::handlePanChanged);

  WApplication *app = WApplication::instance();
  if (app) {
    // TODO(Roel): custom URL?
    app->require("https://unpkg.com/leaflet@1.5.1/dist/leaflet.js");
    app->useStyleSheet(WLink("https://unpkg.com/leaflet@1.5.1/dist/leaflet.css"));
  }
}

WLeafletMap::~WLeafletMap()
{
  for (std::size_t i = 0; i < overlays_.size(); ++i) {
    delete overlays_[i];
  }
  overlays_.clear();
  for (std::size_t i = 0; i < markers_.size(); ++i) {
    if (!markers_[i].flags.test(MarkerEntry::BIT_REMOVED)) {
      delete markers_[i].marker;
    }
  }
  markers_.clear();
}

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

void WLeafletMap::addMarker(Marker *marker)
{
  if (marker->map_ == this)
    return;

  if (marker->map_) {
    marker->map_->removeMarker(marker);
  }

  marker->setMap(this);

  for (std::size_t i = 0; i < markers_.size(); ++i) {
    if (markers_[i].marker == marker &&
        markers_[i].flags.test(MarkerEntry::BIT_REMOVED)) {
      markers_[i].flags.reset(MarkerEntry::BIT_REMOVED);
      return;
    }
  }

  MarkerEntry entry;
  entry.marker = marker;
  entry.flags.set(MarkerEntry::BIT_ADDED);
  entry.id = nextMarkerId_;
  ++nextMarkerId_;

  markers_.push_back(entry);

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

void WLeafletMap::removeMarker(Marker *marker)
{
  for (std::size_t i = 0; i < markers_.size(); ++i) {
    if (markers_[i].marker == marker) {
      marker->setMap(0);
      if (markers_[i].flags.test(MarkerEntry::BIT_ADDED)) {
        markers_.erase(markers_.begin() + i);
        return;
      }
      markers_[i].flags.set(MarkerEntry::BIT_REMOVED);
      scheduleRender();
      return;
    }
  }
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
  Polyline *polyline = new Polyline(points, pen);
  overlays_.push_back(polyline);
  scheduleRender();
}

void WLeafletMap::addCircle(const Coordinate &center,
                            double radius,
                            const WPen &stroke,
                            const WBrush &fill)
{
  Circle *circle = new Circle(center, radius, stroke, fill);
  overlays_.push_back(circle);
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
  if (stroke.style() != NoPen) {
    options["stroke"] = Json::Value(true);
    options["color"] = Json::Value(WT_USTRING::fromUTF8(stroke.color().cssText(false)));
    options["opacity"] = Json::Value(stroke.color().alpha() / 255.0);
    double weight = stroke.width().toPixels();
    weight = weight == 0 ? 1.0 : weight;
    options["weight"] = Json::Value(weight);

    std::string capStyle;
    switch (stroke.capStyle()) {
    case FlatCap:
      capStyle = "butt";
      break;
    case SquareCap:
      capStyle = "square";
      break;
    case RoundCap:
      capStyle = "round";
    }

    options["lineCap"] = Json::Value(WT_USTRING::fromUTF8(capStyle));

    std::string joinStyle;
    switch (stroke.joinStyle()) {
    case BevelJoin:
      joinStyle = "bevel";
      break;
    case MiterJoin:
      joinStyle = "miter";
      break;
    case RoundJoin:
      joinStyle = "round";
    }

    options["lineJoin"] = Json::Value(WT_USTRING::fromUTF8(joinStyle));

    // TODO(Roel): dashArray?
  } else {
    options["stroke"] = Json::Value(false);
  }

  if (fill.style() != NoBrush) {
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

  WStringStream ss;
  ss << "new " WT_CLASS ".WLeafletMap("
     << app->javaScriptClass() << "," << jsRef() << ",";
  char buf[30];
  ss << Utils::round_js_str(position_.latitude(), 16, buf) << ",";
  ss << Utils::round_js_str(position_.longitude(), 16, buf) << ",";
  ss << Utils::round_js_str(zoomLevel_, 16, buf) << ");";

  setJavaScriptMember(" WLeafletMap", ss.str());
  setJavaScriptMember(WT_RESIZE_JS,
                      jsRef() + ".wtObj.wtResize");
}

void WLeafletMap::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull) {
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
      removeMarkerJS(ss, markers_[i].id);
      markers_.erase(markers_.begin() + i);
    } else {
      ++i;
    }
  }

  for (std::size_t i = 0; i < markers_.size(); ++i) {
    if (flags & RenderFull || markers_[i].flags.test(MarkerEntry::BIT_ADDED)) {
      addMarkerJS(ss, markers_[i].id, markers_[i].marker);
      markers_[i].flags.reset(MarkerEntry::BIT_ADDED);
    } else if (markers_[i].marker->moved_) {
      moveMarkerJS(ss, markers_[i].id, markers_[i].marker->position());
    }
    markers_[i].marker->moved_ = false;
  }

  if (!ss.empty()) {
    doJavaScript(ss.str());
  }

  WCompositeWidget::render(flags);
}

}
