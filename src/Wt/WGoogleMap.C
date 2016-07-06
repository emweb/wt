// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Richard Ulrich.
 */

#include <Wt/WGoogleMap>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>

#include "web/WebUtils.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <utility>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
  // if there is no google api key configured, use the one for
  // http://localhost:8080/
  static const std::string localhost_key
    = "ABQIAAAAWqrN5o4-ISwj0Up_depYvhTwM0brOpm-"
    "All5BF6PoaKBxRWWERS-S9gPtCri-B6BZeXV8KpT4F80DQ";

  void write(std::stringstream& os, const Wt::WGoogleMap::Coordinate &c)
  {
    char b1[35];
    char b2[35];
    os << "new google.maps.LatLng("
       << Wt::Utils::round_js_str(c.latitude(), 15, b1)
       << "," << Wt::Utils::round_js_str(c.longitude(), 15, b2) << ")";
  }
}

namespace Wt {

WGoogleMap::Coordinate::Coordinate()
  : lat_(0), lon_(0)
{ }

WGoogleMap::Coordinate::Coordinate(double lat, double lon)
{
  setLatitude(lat);
  setLongitude(lon);
}

#ifndef WT_TARGET_JAVA
WGoogleMap::Coordinate::Coordinate(const std::pair<double, double>& lat_long)
{
  setLatitude(lat_long.first);
  setLongitude(lat_long.second);
}
#endif 

void WGoogleMap::Coordinate::setLatitude(double latitude)
{
  if (latitude < -90.0 || latitude > 90.0)
    throw std::out_of_range("invalid latitude: "
			    + boost::lexical_cast<std::string>(latitude));

  lat_ = latitude;
}

void WGoogleMap::Coordinate::setLongitude(double longitude)
{
  if (longitude < -180.0 || longitude > 180.0)
    throw std::out_of_range("invalid longitude: "
			    + boost::lexical_cast<std::string>(longitude));

  lon_ = longitude;
}

double WGoogleMap::Coordinate::distanceTo(const Coordinate &rhs) const
{
  const double lat1 = lat_ * M_PI / 180.0;
  const double lat2 = rhs.latitude() * M_PI / 180.0;
  const double deltaLong = (rhs.longitude() - lon_) * M_PI / 180.0;
  const double angle = std::sin(lat1) * std::sin(lat2)
    + std::cos(lat1) * std::cos(lat2) * std::cos(deltaLong);
  const double earthRadius = 6371.0; // km
  const double dist = earthRadius * std::acos(angle);

  return dist;
}

#ifndef WT_TARGET_JAVA
std::pair<double, double> WGoogleMap::Coordinate::operator ()() const
{
  return std::make_pair(lat_, lon_);
}

std::istream& operator>> (std::istream& i, WGoogleMap::Coordinate& c)
{
  double lat, lon;
  i >> lat >> std::ws >> lon;
  c.setLatitude(lat);
  c.setLongitude(lon);

  return i;
}
#endif

// example javascript code from :
// http://code.google.com/apis/maps/documentation/

WGoogleMap::WGoogleMap(ApiVersion version, WContainerWidget *parent)
 : clicked_(this, "click"),
   doubleClicked_(this, "dblclick"),
   mouseMoved_(0),
   apiVersion_(version)
{
  setImplementation(new WContainerWidget());

  if (parent)
    parent->addWidget(this);
}

WGoogleMap::WGoogleMap(WContainerWidget *parent)
 : clicked_(this, "click"),
   doubleClicked_(this, "dblclick"),
   mouseMoved_(0),
   apiVersion_(Version2)
{
  setImplementation(new WContainerWidget());

  if (parent)
    parent->addWidget(this);
}

WGoogleMap::~WGoogleMap()
{ 
  delete mouseMoved_;
}

void WGoogleMap::streamJSListener(const JSignal<Coordinate> &signal, 
				  std::string signalName,
				  Wt::WStringStream &strm) 
{
  if (apiVersion_ == Version2) {
    strm <<
      """google.maps.Event.addListener(map, \"" << signalName << "\", "
      ""                              "function(overlay, latlng) {"
      ""  "if (latlng) {"
	<< signal.createCall("latlng.lat() +' '+ latlng.lng()") << ";"
      ""  "}"
      """});";
  } else {
    strm << 
      """google.maps.event.addListener(map, \"" << signalName << "\", "
      ""                              "function(event) {"
      ""  "if (event && event.latLng) {"
	 << signal.createCall("event.latLng.lat() +' '+ event.latLng.lng()") 
	 << ";"
      ""  "}"
      """});";
  }
}

JSignal<WGoogleMap::Coordinate>& WGoogleMap::mouseMoved()
{
  if (!mouseMoved_)
    mouseMoved_ = new JSignal<Coordinate>(this, "mousemove");

  return *mouseMoved_;
}

void WGoogleMap::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull) {
    WApplication *app = WApplication::instance();

    std::string googlekey = localhost_key;
    Wt::WApplication::readConfigurationProperty("google_api_key", googlekey);
      
    // init the google javascript api
    const std::string gmuri = "//www.google.com/jsapi?key=" + googlekey;
    app->require(gmuri, "google");

    std::string initFunction = 
      app->javaScriptClass() + ".init_google_maps_" + id();

    // initialize the map
    WStringStream strm;
    strm <<
      "{ " << initFunction << " = function() {"
      """var self = " << jsRef() << ";"
      """if (!self) { "
      ""   "setTimeout(" << initFunction << ", 0);"
      ""   "return;"
      "}";

    if (apiVersion_ == Version2) {
      strm << 
	"var map = new google.maps.Map(self);"
	"map.setCenter(new google.maps.LatLng(47.01887777, 8.651888), 13);";
      setJavaScriptMember(WT_RESIZE_JS,
                          "function(self, w, h) {"
			  """if (w >= 0) "
			  ""  "self.style.width=w + 'px';"
			  """if (h >= 0) "
			  ""  "self.style.height=h + 'px';"
                          """if (self.map) "
			  ""  "self.map.checkResize();"
                          "}");
    } else {
      strm << 
	"var latlng = new google.maps.LatLng(47.01887777, 8.651888);"
	"var myOptions = {"
	"" "zoom: 13,"
	"" "center: latlng,"
	"" "mapTypeId: google.maps.MapTypeId.ROADMAP"
	"};"
	"var map = new google.maps.Map(self, myOptions);"
	"map.overlays = [];"
	"map.infowindows = [];";
      setJavaScriptMember(WT_RESIZE_JS,
                          "function(self, w, h) {"
			  """if (w >= 0) "
			  ""  "self.style.width=w + 'px';"
			  """if (h >= 0) "
			  ""  "self.style.height=h + 'px';"
                          """if (self.map)"
			  """ google.maps.event.trigger(self.map, 'resize');"
                          "}");
    }
    strm << "self.map = map;";

    // eventhandling
    streamJSListener(clicked_, "click", strm);
    streamJSListener(doubleClicked_, "dblclick", strm);
    if (mouseMoved_)
      streamJSListener(*mouseMoved_, "mousemove", strm);

    // additional things
    for (unsigned int i = 0; i < additions_.size(); i++)
      strm << additions_[i];

    strm << "setTimeout(function(){ delete " << initFunction << ";}, 0)};"
	 << "google.load(\"maps\", \"" << (apiVersion_ == Version2 ? '2' : '3')
	 << "\", {other_params:\"sensor=false\", callback: "
	 << initFunction << "});"
	 << "}"; // private scope

    additions_.clear();

    app->doJavaScript(strm.str(), true);
  }

  WCompositeWidget::render(flags);
}

void WGoogleMap::clearOverlays()
{
  if (apiVersion_ == Version2) {
    doGmJavaScript(jsRef() + ".map.clearOverlays();");
  } else {
    std::stringstream strm;
    strm 
      << "var mapLocal = " << jsRef() + ".map, i;\n"
      << "if (mapLocal.overlays) {\n"
      << """for (i in mapLocal.overlays) {\n"
      << """mapLocal.overlays[i].setMap(null);\n"
      << "}\n"
      << "mapLocal.overlays.length = 0;\n"
      << "}\n"
      << "if (mapLocal.infowindows) {\n"
      << """for (i in mapLocal.infowindows) {\n"
      << ""  "mapLocal.infowindows[i].close();\n"
      << ""  "}\n"
      << """mapLocal.infowindows.length = 0;\n"
      << "}\n";

    doGmJavaScript(strm.str());
  }
}

void WGoogleMap::doGmJavaScript(const std::string& jscode)
{
  if (isRendered())
    doJavaScript(jscode);
  else
    additions_.push_back(jscode);
}

void WGoogleMap::addMarker(const Coordinate& pos)
{
  std::stringstream strm;

  if (apiVersion_ == Version2) {
    strm << "var marker = ";
    write(strm, pos);
    strm << ";"
	 << jsRef() << ".map.addOverlay(marker);";
  } else {
    strm << "var position = ";
    write(strm, pos);
    strm << ";"
	 << "var marker = new google.maps.Marker({"
	 << "position: position,"
	 << "map: " << jsRef() << ".map"
	 << "});"
	 << jsRef() << ".map.overlays.push(marker);";
  }

  doGmJavaScript(strm.str());
}

void WGoogleMap::addIconMarker(const Coordinate &pos,
                               const std::string& iconURL)
{
  std::stringstream strm;
  
  if (apiVersion_ == Version2) {
    throw std::logic_error("WGoogleMap::addIconMarker is not supported "
                           "in the Google Maps API v2.");
  } else {
    strm << "var position = ";
    write(strm, pos);
    strm << ";"
         << "var marker = new google.maps.Marker({"
	 << "position: position,"
	 << "icon: \"" <<  iconURL << "\","
         << "map: " << jsRef() << ".map"
	 << "});"
      
         << jsRef() << ".map.overlays.push(marker);";
  }
 
  doGmJavaScript(strm.str());
}

void WGoogleMap::addCircle(const Coordinate& center, double radius, 
			   const WColor& strokeColor, int strokeWidth,
			   const WColor& fillColor)
{
  if ( apiVersion_ == Version2 ) {
    throw std::logic_error("WGoogleMap::addCircle is not supported " 
			   "in the Google Maps API v2.");
    //we could support this by rendering the circle by rendering 
    //a set of lines (see maps.forum.nu)
    //when doing this we can implement a drawPolygon function at the same time
  } else {
    std::stringstream strm;

    double strokeOpacity = strokeColor.alpha() / 255.0;
    double fillOpacity = fillColor.alpha() / 255.0;
    
    strm << "var mapLocal = " << jsRef() + ".map;"
	 << "var latLng = ";
    write(strm, center);
    strm << ";"
	 << "var circle = new google.maps.Circle( "
            "{ "
            "  map: mapLocal, "
            "  radius: " << radius << ", "
            "  center:  latLng  ,"
            "  fillOpacity: \"" << fillOpacity << "\","
            "  fillColor: \"" << fillColor.cssText() << "\","
            "  strokeWeight: " << strokeWidth << ","
            "  strokeColor:\"" << strokeColor.cssText() << "\","
            "  strokeOpacity: " << strokeOpacity <<
            "} "
            ");"
            << jsRef() << ".map.overlays.push(circle);";

    doGmJavaScript(strm.str());
  }
}

void WGoogleMap::addPolyline(const std::vector<Coordinate>& points,
			     const WColor& color, int width, double opacity)
{
  if (opacity == 1.0)
    opacity = color.alpha() / 255.0;

  // opacity has to be between 0.0 and 1.0
  opacity = std::max(std::min(opacity, 1.0), 0.0);

  std::stringstream strm;
  strm << "var waypoints = [];";
  for (size_t i = 0; i < points.size(); ++i) {
    strm << "waypoints[" << i << "] = ";
    write(strm, points[i]);
    strm << ";";
  }

  if (apiVersion_ == Version2) {
    strm << "var poly = new google.maps.Polyline(waypoints, \""
	 << color.cssText() << "\", " << width << ", " << opacity << ");"
	 << jsRef() << ".map.addOverlay(poly);";
  } else {
    strm << 
      "var poly = new google.maps.Polyline({"
      "path: waypoints,"
      "strokeColor: \"" << color.cssText() << "\"," <<
      "strokeOpacity: " << opacity << "," << 
      "strokeWeight: " << width <<
      "});" <<
      "poly.setMap(" << jsRef() << ".map);" <<
      jsRef() << ".map.overlays.push(poly);";
  }

  doGmJavaScript(strm.str());
}

void WGoogleMap::openInfoWindow(const Coordinate& pos,
				const WString& myHtml)
{
  std::stringstream strm;
  strm << "var pos = ";
  write(strm, pos);
  strm << ";";
  
  if (apiVersion_ == Version2) {
    strm << jsRef() << ".map.openInfoWindow(pos, "
	 << WWebWidget::jsStringLiteral(myHtml) << ");";
  } else {
    strm << "var infowindow = new google.maps.InfoWindow({"
      "content: " << WWebWidget::jsStringLiteral(myHtml) << "," <<
      "position: pos"
      "});"
      "infowindow.open(" << jsRef() << ".map);" <<
      jsRef() << ".map.infowindows.push(infowindow);";
  }

  doGmJavaScript(strm.str());
}

void WGoogleMap::setCenter(const Coordinate& center)
{
  std::stringstream strm;
  strm << jsRef() << ".map.setCenter(";
  write(strm, center);
  strm << ");";
  doGmJavaScript(strm.str());
}

void WGoogleMap::setCenter(const Coordinate& center, int zoom)
{
  std::stringstream strm;
  strm << jsRef() << ".map.setCenter(";
  write(strm, center);
  strm << "); "
       << jsRef() << ".map.setZoom(" << zoom << ");";

  doGmJavaScript(strm.str());
}

void WGoogleMap::panTo(const Coordinate& center)
{
  std::stringstream strm;
  strm << jsRef() << ".map.panTo(";
  write (strm, center);
  strm << ");";

  doGmJavaScript(strm.str());
}

void WGoogleMap::setZoom(int level)
{
  doGmJavaScript(jsRef() + ".map.setZoom("
		 + boost::lexical_cast<std::string>(level) + ");");
}

void WGoogleMap::zoomIn()
{
  std::stringstream strm;
  strm 
    << "var zoom = " << jsRef() << ".map.getZoom();"
    << jsRef() << ".map.setZoom(zoom + 1);";
  doGmJavaScript(strm.str());
}

void WGoogleMap::zoomOut()
{
  std::stringstream strm;
  strm 
    << "var zoom = " << jsRef() << ".map.getZoom();"
    << jsRef() << ".map.setZoom(zoom - 1);";
  doGmJavaScript(strm.str());
}

void WGoogleMap::savePosition()
{
  if (apiVersion_ == Version2) {
    doGmJavaScript(jsRef() + ".map.savePosition();");
  } else {
    std::stringstream strm;
    strm
      << jsRef() << ".map.savedZoom = " << jsRef() << ".map.getZoom();"
      << jsRef() << ".map.savedPosition = " << jsRef() << ".map.getCenter();";
    doGmJavaScript(strm.str());
  } 
}

void WGoogleMap::returnToSavedPosition()
{
  if (apiVersion_ == Version2) {
    doGmJavaScript(jsRef() + ".map.returnToSavedPosition();");
  } else {
    std::stringstream strm;
    strm
      << jsRef() << ".map.setZoom(" << jsRef() << ".map.savedZoom);"
      << jsRef() << ".map.setCenter(" << jsRef() << ".map.savedPosition);";
    doGmJavaScript(strm.str());
  }
}

void WGoogleMap::checkResize()
{
  doGmJavaScript(jsRef() + ".map.checkResize();");
}

void WGoogleMap::setMapOption(const std::string &option, 
			      const std::string &value)
{
  std::stringstream strm;
  strm
    << "var option = {"
    << option << " :" << value
    << "};"
    << jsRef() << ".map.setOptions(option);";

  doGmJavaScript(strm.str());
}

void WGoogleMap::enableDragging()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableDragging();");
  else
    setMapOption("draggable", "true");
}

void WGoogleMap::disableDragging()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableDragging();");
  else
    setMapOption("draggable", "false");
}

void WGoogleMap::enableDoubleClickZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableDoubleClickZoom();");
  else
    setMapOption("disableDoubleClickZoom", "false");
}

void WGoogleMap::disableDoubleClickZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableDoubleClickZoom();");
  else
    setMapOption("disableDoubleClickZoom", "true");
}

void WGoogleMap::enableGoogleBar()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableGoogleBar();");
  else
    throw std::logic_error("WGoogleMap::enableGoogleBar is not supported " 
			   "in the Google Maps API v3.");
}

void WGoogleMap::disableGoogleBar()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableGoogleBar();");
  else
    throw std::logic_error("WGoogleMap::disableGoogleBar is not supported " 
			   "in the Google Maps API v3.");
}

void WGoogleMap::enableScrollWheelZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableScrollWheelZoom();");
  else 
    setMapOption("scrollwheel", "true");
}

void WGoogleMap::disableScrollWheelZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableScrollWheelZoom();");
  else 
    setMapOption("scrollwheel", "false");
}

#ifndef WT_TARGET_JAVA
void WGoogleMap::zoomWindow(const std::pair<Coordinate, Coordinate>& bbox)
{
  zoomWindow(bbox.first, bbox.second);
}
#endif

void WGoogleMap::zoomWindow(const Coordinate& topLeft, 
			    const Coordinate& rightBottom)
{
  const Coordinate center
    ((topLeft.latitude() + rightBottom.latitude()) / 2.0,
     (topLeft.longitude() + rightBottom.longitude()) / 2.0);

  Coordinate topLeftC = 
    Coordinate(std::min(topLeft.latitude(), rightBottom.latitude()),
	       std::min(topLeft.longitude(), rightBottom.longitude()));
  Coordinate rightBottomC = 
    Coordinate(std::max(topLeft.latitude(), rightBottom.latitude()),
	       std::max(topLeft.longitude(), rightBottom.longitude()));
  std::stringstream strm;
  strm << "var bbox = new google.maps.LatLngBounds(";
  write(strm, topLeftC);
  strm << ", ";
  write(strm, rightBottomC);
  strm << ");";

  if (apiVersion_ == Version2) {
    strm 
      << "var zooml = " << jsRef() << ".map.getBoundsZoomLevel(bbox);"
      << jsRef() << ".map.setCenter(";
    write (strm, center);
    strm << ", zooml);";
  } else {
    strm 
      << jsRef() << ".map.fitBounds(bbox);";
  }

  doGmJavaScript(strm.str());
}

void WGoogleMap::setMapTypeControl(MapTypeControl type)
{
  std::stringstream strm;

  if (apiVersion_ == Version2) {
    std::string control;
    switch (type) {
    case DefaultControl:
      control = "google.maps.MapTypeControl";
      break;
    case MenuControl:
      control = "google.maps.MenuMapTypeControl";
      break;
    case HierarchicalControl:
      control = "google.maps.HierarchicalMapTypeControl";
      break;
    case HorizontalBarControl:
      throw std::logic_error("WGoogleMap::setMapTypeControl: "
			     "HorizontalBarControl is not supported when using "
			     "Google Maps API v2.");
    default:
      control = "";
    }

    strm << jsRef() << ".map.removeControl(" << jsRef() << ".mtc);";
       
    if(control != "")
      strm << "var mtc = new " << control << "();"
	   << jsRef() << ".mtc = mtc;"
	   << jsRef() << ".map.addControl(mtc);";
  } else {
    std::string control;
    switch (type) {
    case DefaultControl:
      control = "DEFAULT";
      break;
    case MenuControl:
      control = "DROPDOWN_MENU";
      break;
    case HorizontalBarControl:
      control = "HORIZONTAL_BAR";
      break;
    case HierarchicalControl:
      throw std::logic_error("WGoogleMap::setMapTypeControl: "
			     "HierarchicalControl is not supported when using "
			     "Google Maps API v3.");
    default:
      control = "";
    }

    strm 
      << "var options = {"
      << """disableDefaultUI: " << (control == "" ? "true" : "false") << ","
      << ""  "mapTypeControlOptions: {";

    if (control != "")
      strm << "style: google.maps.MapTypeControlStyle." << control;

    strm 
      << """}"
      << "};"
      << jsRef() << ".map.setOptions(options);";
  }
  
  doGmJavaScript(strm.str());
}

}
