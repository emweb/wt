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
#include <Wt/WEnvironment>
#include <Wt/WContainerWidget>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <sstream>
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
   mouseMoved_(this, "mousemove"),
   apiVersion_(version)
{
  setImplementation(new WContainerWidget());

  if (parent)
    parent->addWidget(this);
}

WGoogleMap::WGoogleMap(WContainerWidget *parent)
 : clicked_(this, "click"),
   doubleClicked_(this, "dblclick"),
   mouseMoved_(this, "mousemove"),
   apiVersion_(Version2)
{
  setImplementation(new WContainerWidget());

  if (parent)
    parent->addWidget(this);
}

WGoogleMap::~WGoogleMap()
{ }

void WGoogleMap::streamJSListener(const JSignal<Coordinate> &signal, 
				  std::string signalName,
				  std::ostream &strm) 
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

void WGoogleMap::render(WFlags<RenderFlag> flags)
{
  //

  if (flags & RenderFull) {
    WApplication *app = WApplication::instance();

    if (apiVersion_ == Version2) {
      std::string googlekey = localhost_key;
      Wt::WApplication::readConfigurationProperty("google_api_key", googlekey);
      
      // init the google javascript api
      const std::string gmuri = "http://www.google.com/jsapi?key=" + googlekey;
      app->require(gmuri, "google");
    }

    std::string initFunction = 
      app->javaScriptClass() + ".init_google_maps_" + id();

    // initialize the map
    std::stringstream strm;
    strm <<
      "{ " << initFunction
	       << " = function() {"
      """var self = " << jsRef() << ";"
      """if (!self) { "
      ""   "setTimeout(" << initFunction << ", 0);"
      ""   "return;"
      """}";

    if (apiVersion_ == Version2) {
      //TODO 
      //calling this function more than once in the same request seems to
      //be impossible
      strm << 
	"""var map = new google.maps.Map(self);"
	"""map.setCenter(new google.maps.LatLng(47.01887777, 8.651888), 13);";
      setJavaScriptMember("wtResize",
                          """function(self, w, h) {"
                          """if (self.map)"
			  """  self.map.checkResize();"
                          """}");
    } else {
      strm << 
	"""var latlng = new google.maps.LatLng(47.01887777, 8.651888);"
	"""var myOptions = {"
	""   "zoom: 13,"
	""   "center: latlng,"
	""   "mapTypeId: google.maps.MapTypeId.ROADMAP"
	"""};"
	"""var map = new google.maps.Map(self, myOptions);"
	"""map.overlays = [];"
	"""map.infowindows = [];";

      setJavaScriptMember("wtResize",
                          """function(self, w, h) {"
                          """if (self.map)"
			  """ google.maps.event.trigger(self.map, 'resize');"
                          """}");
    }
    strm << """self.map = map;";


    // eventhandling
    streamJSListener(clicked_, "click", strm);
    streamJSListener(doubleClicked_, "dblclick", strm);
    streamJSListener(mouseMoved_, "mousemove", strm);

    // additional things
    for (unsigned int i = 0; i < additions_.size(); i++)
      strm << additions_[i];

    strm 
      << "setTimeout(function(){ delete " << initFunction << ";}, 0)"
      << "};"; // function initialize()

    if (apiVersion_ == Version2) {
      strm << "google.load(\"maps\", \"2\", "
	""          "{other_params:\"sensor=false\", callback: "
	   << app->javaScriptClass() + ".init_google_maps_" + id()
	   << "});";
    }
    strm << "}"; // private scope

    additions_.clear();

    app->doJavaScript(strm.str(), apiVersion_ == Version2);

    if (apiVersion_ == Version3) {
      std::string uri;
      if (app->environment().ajax()) {
	uri = "http://maps.google.com/maps/api/js?sensor=false&callback=";
	uri += app->javaScriptClass() + ".init_google_maps_" + id();
      } else {
	uri = "http://maps.google.com/maps/api/js?sensor=false";
      }
    
      app->require(uri);
    }
  }

  WCompositeWidget::render(flags);
}

void WGoogleMap::clearOverlays()
{
  if (apiVersion_ == Version2) {
    doGmJavaScript(jsRef() + ".map.clearOverlays();", false);
  } else {
    std::stringstream strm;
    strm 
      << """var mapLocal = " << jsRef() + ".map;\n"
      << """if (mapLocal.overlays) {\n"
      << """  for (i in mapLocal.overlays) {\n"
      << """    mapLocal.overlays[i].setMap(null);\n"
      << """  }\n"
      << """  mapLocal.overlays.length = 0;\n"
      << """}\n"
      << """if (mapLocal.infowindows) {\n"
      << """  for (i in mapLocal.infowindows) {\n"
      << """    mapLocal.infowindows[i].close();\n"
      << """  }\n"
      << """  mapLocal.infowindows.length = 0;\n"
      << """}\n";
    doGmJavaScript(strm.str(), false);
  }
}

void WGoogleMap::doGmJavaScript(const std::string& jscode, bool sepScope)
{
  std::string js = jscode;
  // to keep the variables inside a scope where they don't interfere
  if (sepScope)
    js = "{" + js + "}";

  if (isRendered())
    WApplication::instance()->doJavaScript(js);
  else
    additions_.push_back(js);
}

void WGoogleMap::addMarker(const Coordinate& pos)
{
  std::stringstream strm;

  if (apiVersion_ == Version2) {
    strm << "var marker = new google.maps.Marker(new google.maps.LatLng("
	 << pos.latitude() << ", " << pos.longitude() << "));"
	 << jsRef() << ".map.addOverlay(marker);";
  } else {
    strm << "var position = new google.maps.LatLng("
	 << pos.latitude() << ", " << pos.longitude() << ");"
	 << "var marker = new google.maps.Marker({"
	 << "position: position,"
	 << "map: " << jsRef() << ".map"
	 << "});"
	 << jsRef() << ".map.overlays.push(marker);";
  }

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::addPolyline(const std::vector<Coordinate>& points,
			     const WColor& color, int width, double opacity)
{
  // opacity has to be between 0.0 and 1.0
  opacity = std::max(std::min(opacity, 1.0), 0.0);

  std::stringstream strm;
  strm << "var waypoints = [];";
  for (size_t i = 0; i < points.size(); ++i)
    strm << "waypoints[" << i << "] = new google.maps.LatLng("
	 << points[i].latitude() << ", " << points[i].longitude() << ");";



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

  doGmJavaScript(strm.str(), true);
}

void WGoogleMap::openInfoWindow(const Coordinate& pos,
				const WString& myHtml)
{
  std::stringstream strm;
  strm << "var pos = new google.maps.LatLng(" 
       << pos.latitude() << ", " << pos.longitude() << ");";
  
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

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::setCenter(const Coordinate& center)
{
  std::stringstream strm;
  strm << jsRef() << ".map.setCenter(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << "));";

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::setCenter(const Coordinate& center, int zoom)
{
  std::stringstream strm;
  strm << jsRef() << ".map.setCenter(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << ")); "
       << jsRef() << ".map.setZoom(" << zoom << ");";

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::panTo(const Coordinate& center)
{
  std::stringstream strm;
  strm << jsRef() << ".map.panTo(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << "));";

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::setZoom(int level)
{
  doGmJavaScript(jsRef() + ".map.setZoom("
		 + boost::lexical_cast<std::string>(level) + ");", false);
}

void WGoogleMap::zoomIn()
{
  std::stringstream strm;
  strm 
    << "var zoom = " << jsRef() << ".map.getZoom();"
    << jsRef() << ".map.setZoom(zoom + 1);";
  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::zoomOut()
{
  std::stringstream strm;
  strm 
    << "var zoom = " << jsRef() << ".map.getZoom();"
    << jsRef() << ".map.setZoom(zoom - 1);";
  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::savePosition()
{
  if (apiVersion_ == Version2) {
    doGmJavaScript(jsRef() + ".map.savePosition();", false);
  } else {
    std::stringstream strm;
    strm
      << jsRef() << ".map.savedZoom = " << jsRef() << ".map.getZoom();"
      << jsRef() << ".map.savedPosition = " << jsRef() << ".map.getCenter();";
    doGmJavaScript(strm.str(), false);
  } 
}

void WGoogleMap::returnToSavedPosition()
{
  if (apiVersion_ == Version2) {
    doGmJavaScript(jsRef() + ".map.returnToSavedPosition();", false);
  } else {
    std::stringstream strm;
    strm
      << jsRef() << ".map.setZoom(" << jsRef() << ".map.savedZoom);"
      << jsRef() << ".map.setCenter(" << jsRef() << ".map.savedPosition);";
    doGmJavaScript(strm.str(), false);
  }
}

void WGoogleMap::checkResize()
{
  doGmJavaScript(jsRef() + ".map.checkResize();", false);
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

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::enableDragging()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableDragging();", false);
  else
    setMapOption("draggable", "true");
}

void WGoogleMap::disableDragging()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableDragging();", false);
  else
    setMapOption("draggable", "false");
}

void WGoogleMap::enableDoubleClickZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableDoubleClickZoom();", false);
  else
    setMapOption("disableDoubleClickZoom", "false");
}

void WGoogleMap::disableDoubleClickZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableDoubleClickZoom();", false);
  else
    setMapOption("disableDoubleClickZoom", "true");
}

void WGoogleMap::enableGoogleBar()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableGoogleBar();", false);
  else
    throw std::logic_error("WGoogleMap::enableGoogleBar is not supported " 
			   "in the Google Maps API v3.");
}

void WGoogleMap::disableGoogleBar()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableGoogleBar();", false);
  else
    throw std::logic_error("WGoogleMap::disableGoogleBar is not supported " 
			   "in the Google Maps API v3.");
}

void WGoogleMap::enableScrollWheelZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.enableScrollWheelZoom();", false);
  else 
    setMapOption("scrollwheel", "true");
}

void WGoogleMap::disableScrollWheelZoom()
{
  if (apiVersion_ == Version2)
    doGmJavaScript(jsRef() + ".map.disableScrollWheelZoom();", false);
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
  Coordinate topLeftC = topLeft;
  Coordinate rightBottomC = rightBottom;

  const Coordinate center
    ((topLeftC.latitude() + rightBottomC.latitude()) / 2.0,
     (topLeftC.longitude() + rightBottomC.longitude()) / 2.0);

  topLeftC = 
    Coordinate(std::min(topLeftC.latitude(), rightBottomC.latitude()),
	       std::min(topLeftC.longitude(), rightBottomC.longitude()));
  rightBottomC = 
    Coordinate(std::max(topLeftC.latitude(), rightBottomC.latitude()),
	       std::max(topLeftC.longitude(), rightBottomC.longitude()));
  std::stringstream strm;
  strm << "var bbox = new google.maps.LatLngBounds(new google.maps.LatLng("
       << topLeftC.latitude()  << ", " << topLeftC.longitude() << "), "
       << "new google.maps.LatLng("
       << rightBottomC.latitude() << ", " << rightBottomC.longitude() << "));";

  if (apiVersion_ == Version2) {
    strm 
      << "var zooml = " << jsRef() << ".map.getBoundsZoomLevel(bbox);"
      << jsRef() << ".map.setCenter(new google.maps.LatLng("
      << center.latitude() << ", " << center.longitude() << "), zooml);";
  } else {
    strm 
      << jsRef() << ".map.fitBounds(bbox);";
  }

  doGmJavaScript(strm.str(), true);
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
      << """var options = {"
      << "disableDefaultUI: " << (control == "" ? "true" : "false") << ","
      << """  mapTypeControlOptions: {";
    if (control != "")
      strm << "style: google.maps.MapTypeControlStyle." << control;
    strm 
      << """  }"
      << """};"
      << jsRef() << ".map.setOptions(options);";
  }
  
  doGmJavaScript(strm.str(), false);
}

}
