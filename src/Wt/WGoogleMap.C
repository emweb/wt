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

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include <cmath>

using std::string;
using std::make_pair;
using std::swap;
using std::min;
using std::max;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Wt {

WGoogleMap::Coordinate::Coordinate()
  : lat_(0), lon_(0)
{ }

WGoogleMap::Coordinate::Coordinate(double lat, double lon)
{
  setLatitude(lat);
  setLongitude(lon);
}

WGoogleMap::Coordinate::Coordinate(const std::pair<double, double>& lat_long)
{
  setLatitude(lat_long.first);
  setLongitude(lat_long.second);
}

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

// example javascript code from :
// http://code.google.com/apis/maps/documentation/

WGoogleMap::WGoogleMap(WContainerWidget *parent)
 : clicked_(this, "click"),
   doubleClicked_(this, "dblclick"),
   mouseMoved_(this, "mousemove"),
   rendered_(false)
{
  setImplementation(new WContainerWidget());

  WApplication *app = WApplication::instance();

  // if there is no google api key configured, use the one for
  // http://localhost:8080/
  static const string localhost_key
    = "ABQIAAAAWqrN5o4-ISwj0Up_depYvhTwM0brOpm-"
      "All5BF6PoaKBxRWWERS-S9gPtCri-B6BZeXV8KpT4F80DQ";

  string googlekey = localhost_key;
  Wt::WApplication::readConfigurationProperty("google_api_key", googlekey);

  // init the google javascript api
  const string gmuri = "http://www.google.com/jsapi?key=" + googlekey;
  app->require(gmuri, "google");

  if (parent)
    parent->addWidget(this);
}

WGoogleMap::~WGoogleMap()
{ }

void WGoogleMap::refresh()
{
  rendered_ = false;
}

void WGoogleMap::render()
{
  if (!rendered_) {
    // initialize the map
    std::ostringstream strm;
    strm <<
      "{ function initialize() {"
      """var self = " << jsRef() << ";"
      """var map = new google.maps.Map2(self);"
      """map.setCenter(new google.maps.LatLng(47.01887777, 8.651888), 13);"
      """self.map = map;"

      // eventhandling
      """google.maps.Event.addListener(map, \"click\", "
      ""                              "function(overlay, latlng) {"
      ""  "if (latlng) {"
      ""  << clicked_.createCall("latlng.lat() +' '+ latlng.lng()") << ";"
      ""  "}"
      """});"

      """google.maps.Event.addListener(map, \"dblclick\", "
      ""                              "function(overlay, latlng) {"
      ""  "if (latlng) {"
      ""  << doubleClicked_.createCall("latlng.lat() +' '+ latlng.lng()") << ";"
      ""  "}"
      """});"

      """google.maps.Event.addListener(map, \"mousemove\", "
      ""                              "function(latlng) {"
      ""  "if (latlng) {"
      ""  << mouseMoved_.createCall("latlng.lat() +' '+ latlng.lng()") << ";"
      ""  "}"
      """});";

    // additional things
    copy(additions_.begin(), additions_.end(),
	 std::ostream_iterator<string>(strm));

    strm <<
      "}" // function initialize()
      "google.load(\"maps\", \"2\", "
      ""          "{other_params:\"sensor=false\", callback: initialize});"
      "}"; // private scope

    additions_.clear();

    WApplication::instance()->doJavaScript(strm.str());

    rendered_ = true;
  }

  WCompositeWidget::render();
}

void WGoogleMap::clearOverlays()
{
  doGmJavaScript(jsRef() + ".map.clearOverlays();", false);
}

void WGoogleMap::doGmJavaScript(const std::string& jscode, bool sepScope)
{
  std::string js = jscode;
  // to keep the variables inside a scope where they don't interfere
  if (sepScope)
    js = "{" + js + "}";

  if (rendered_)
    WApplication::instance()->doJavaScript(js);
  else
    additions_.push_back(js);
}

void WGoogleMap::addMarker(const Coordinate& pos)
{
  std::ostringstream strm;
  strm << "var marker = new google.maps.Marker(new google.maps.LatLng("
       << pos.latitude() << ", " << pos.longitude() << "));"
       << jsRef() << ".map.addOverlay(marker);";

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::addPolyline(const std::vector<Coordinate>& points,
			     const WColor& color, int width, double opacity)
{
  // opacity has to be between 0.0 and 1.0
  opacity = std::max(std::min(opacity, 1.0), 0.0);

  std::ostringstream strm;
  strm << "var waypoints = [];";
  for (size_t i = 0; i < points.size(); ++i)
    strm << "waypoints[" << i << "] = new google.maps.LatLng("
	 << points[i].latitude() << ", " << points[i].longitude() << ");";

  strm << "var poly = new google.maps.Polyline(waypoints, \""
       << color.cssText() << "\", " << width << ", " << opacity << ");"
       << jsRef() << ".map.addOverlay(poly);";

  doGmJavaScript(strm.str(), true);
}

void WGoogleMap::openInfoWindow(const Coordinate& pos,
				const std::string& myHtml)
{
  std::ostringstream strm;
  strm << jsRef() << ".map.openInfoWindow(new google.maps.LatLng("
       << pos.latitude() << ", " << pos.longitude() << "), "
       << myHtml << ");";

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::setCenter(const Coordinate& center, int zoom)
{
  std::ostringstream strm;
  strm << jsRef() << ".map.setCenter(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << "), "
       << zoom << ");";

  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::panTo(const Coordinate& center)
{
  std::ostringstream strm;
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
  doGmJavaScript(jsRef() + ".map.zoomIn();", false);
}

void WGoogleMap::zoomOut()
{
  doGmJavaScript(jsRef() + ".map.zoomOut();", false);
}

void WGoogleMap::savePosition()
{
  doGmJavaScript(jsRef() + ".map.savePosition();", false);
}

void WGoogleMap::returnToSavedPosition()
{
  doGmJavaScript(jsRef() + ".map.returnToSavedPosition();", false);
}

void WGoogleMap::checkResize()
{
  doGmJavaScript(jsRef() + ".map.checkResize();", false);
}

void WGoogleMap::enableDragging()
{
  doGmJavaScript(jsRef() + ".map.enableDragging();", false);
}

void WGoogleMap::disableDragging()
{
  doGmJavaScript(jsRef() + ".map.disableDragging();", false);
}

void WGoogleMap::enableDoubleClickZoom()
{
  doGmJavaScript(jsRef() + ".map.enableDoubleClickZoom();", false);
}

void WGoogleMap::disableDoubleClickZoom()
{
  doGmJavaScript(jsRef() + ".map.disableDoubleClickZoom();", false);
}

void WGoogleMap::enableGoogleBar()
{
  doGmJavaScript(jsRef() + ".map.enableGoogleBar();", false);
}

void WGoogleMap::disableGoogleBar()
{
  doGmJavaScript(jsRef() + ".map.disableGoogleBar();", false);
}

void WGoogleMap::enableScrollWheelZoom()
{
  doGmJavaScript(jsRef() + ".map.enableScrollWheelZoom();", false);
}

void WGoogleMap::disableScrollWheelZoom()
{
  doGmJavaScript(jsRef() + ".map.disableScrollWheelZoom();", false);
}

void WGoogleMap::zoomWindow(const std::pair<Coordinate, Coordinate>& bbox)
{
  std::pair<Coordinate, Coordinate> bb = bbox;

  const Coordinate center
    ((bb.first.latitude() + bb.second.latitude()) / 2.0,
     (bb.first.longitude() + bb.second.longitude()) / 2.0);

  bb = std::make_pair
    (Coordinate(min(bb.first.latitude(), bb.second.latitude()),
		min(bb.first.longitude(), bb.second.longitude())),
     Coordinate(max(bb.first.latitude(), bb.second.latitude()),
		max(bb.first.longitude(), bb.second.longitude())));

  std::ostringstream strm;
  strm << "var bbox = new google.maps.LatLngBounds(new google.maps.LatLng("
       << bb.first.latitude()  << ", " << bb.first.longitude() << "), "
       << "new google.maps.LatLng("
       << bb.second.latitude() << ", " << bb.second.longitude() << "));"
       << "var zooml = " << jsRef() << ".map.getBoundsZoomLevel(bbox);"
       << jsRef() << ".map.setCenter(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << "), zooml);";

  doGmJavaScript(strm.str(), true);
}

void WGoogleMap::addMapTypeControl()
{
  std::ostringstream strm;
  strm << "var mtc = new google.maps.MapTypeControl();"
       << jsRef() << ".map.removeControl(" << jsRef() << ".mtc);"
       << jsRef() << ".mtc = mtc;"
       << jsRef() << ".map.addControl(mtc);";
  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::addHierarchicalMapTypeControl()
{
  std::ostringstream strm;
  strm << "var mtc = new google.maps.HierarchicalMapTypeControl();"
       << jsRef() << ".map.removeControl(" << jsRef() << ".mtc);"
       << jsRef() << ".mtc = mtc;"
       << jsRef() << ".map.addControl(mtc);";
  doGmJavaScript(strm.str(), false);
}

void WGoogleMap::addMenuMapTypeControl()
{
  std::ostringstream strm;
  strm << "var mtc = new google.maps.MenuMapTypeControl();"
       << jsRef() << ".map.removeControl(" << jsRef() << ".mtc);"
       << jsRef() << ".mtc = mtc;"
       << jsRef() << ".map.addControl(mtc);";
  doGmJavaScript(strm.str(), false);
}

}
