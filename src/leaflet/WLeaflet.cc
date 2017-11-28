#include "Wt/WLogger"
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include "web/WebUtils.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <utility>
#include <iostream>
#include <cmath>

#include "leaflet/WLeaflet.hh"

namespace Wt
{
  LOGGER("WLeaflet");

  ///////////////////////////////////////////////////////////////////////////////////////
  //WLeaflet::WLeaflet
  ///////////////////////////////////////////////////////////////////////////////////////

  WLeaflet::WLeaflet(tile_provider_t tile, double lat, double lon, int zoom) :
    m_tile(tile),
    m_lat(lat),
    m_lon(lon),
    m_zoom(zoom)
  {
    setImplementation(std::unique_ptr<WWidget>(new WContainerWidget()));
    this->addCssRule("html", "height: 100%");
    this->addCssRule("body", "height: 100%");
    this->addCssRule("#" + id(), "position:relative; top:0; bottom:0; height: 100%");
    WApplication *app = WApplication::instance();
    app->useStyleSheet("https://unpkg.com/leaflet@1.0.3/dist/leaflet.css");
    const std::string leaflet = "https://unpkg.com/leaflet@1.0.3/dist/leaflet.js";
    app->require(leaflet, "leaflet");
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //WLeaflet::render
  ///////////////////////////////////////////////////////////////////////////////////////

  void WLeaflet::render(WFlags<RenderFlag> flags)
  {
    if (flags.test(RenderFlag::Full))
    {
      Wt::WApplication * app = Wt::WApplication::instance();
      Wt::WString initFunction = app->javaScriptClass() + ".init_leaflet_" + id();
      Wt::WStringStream strm;

      strm
        << "{ " << initFunction.toUTF8() << " = function() {\n"
        << "  var self = " << jsRef() << ";\n"
        << "  if (!self) {\n"
        << "    setTimeout(" << initFunction.toUTF8() << ", 0);\n"
        << "  }\n";

      if (m_tile == tile_provider_t::RRZE)
      {
        strm
          << "  var layer_base = L.tileLayer(\n"
          << "  'http://{s}.osm.rrze.fau.de/osmhd/{z}/{x}/{y}.png',{\n"
          << "  attribution: '<a href=http://www.openstreetmap.org/copyright>(C) Openstreetmap Contributors</a>'\n"
          << "  });\n";
      }
      else if (m_tile == tile_provider_t::CARTODB)
      {
        strm
          << "  var layer_base = L.tileLayer(\n"
          << "  'http://cartodb-basemaps-{s}.global.ssl.fastly.net/light_all/{z}/{x}/{y}@2x.png',{\n"
          << "  opacity: 1,\n"
          << "  attribution: '&copy; <a href=http://www.openstreetmap.org/copyright>OpenStreetMap</a>, &copy; <a href=https://carto.com/attribution>CARTO</a>'\n"
          << "  });\n";
      }

      std::string str_z = std::to_string((int)m_zoom);
      std::string str_ll;
      str_ll = std::to_string((long double)m_lat);
      str_ll += ",";
      str_ll += std::to_string((long double)m_lon);

      strm
        << "  var map = new L.Map(self, {\n"
        << "  center: new L.LatLng(" << str_ll << "), \n"
        << "  zoom: " << str_z << ",\n"
        << "  scrollWheelZoom: false,\n"
        << "  layers: [layer_base]\n"
        << "  });\n";

      strm << "self.map = map;";

      for (size_t idx = 0; idx < m_additions.size(); idx++)
      {
        strm << m_additions[idx];
      }

      strm
        << "  setTimeout(function(){ delete " << initFunction.toUTF8() << ";}, 0)};\n"
        << "}\n"
        << initFunction.toUTF8() << "();\n";

      if (0) LOG_INFO(strm.str());

      m_additions.clear();
      app->doJavaScript(strm.str(), true);
    }

    Wt::WCompositeWidget::render(flags);
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //WLeaflet::Circle
  ///////////////////////////////////////////////////////////////////////////////////////

  void WLeaflet::Circle(const std::string &lat, const std::string &lon)
  {
    Wt::WStringStream strm;

    strm
      << " var opt = {radius: 100, stroke: false, color: '#ff0000'}\n";

    std::string str_ll;
    str_ll = lat;
    str_ll += ",";
    str_ll += lon;

    strm
      << " var c = new L.circle([" << str_ll << "], opt).addTo(map);\n";

    m_additions.push_back(strm.str());
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //WLeaflet::Circle
  ///////////////////////////////////////////////////////////////////////////////////////

  void WLeaflet::Circle(const double lat, const double lon, const std::string &color)
  {
    Wt::WStringStream strm;

    strm
      << " var opt = {radius: 100, stroke: false, color: '"
      << color
      << "'}\n";

    std::string str_ll;
    str_ll = std::to_string((long double)lat);
    str_ll += ",";
    str_ll += std::to_string((long double)lon);

    strm
      << " var c = new L.circle([" << str_ll << "], opt).addTo(map);\n";

    m_additions.push_back(strm.str());

  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //WLeaflet::Polygon
  ///////////////////////////////////////////////////////////////////////////////////////

  void WLeaflet::Polygon(const std::vector<double> &lat, const std::vector<double> &lon,
    const std::string &color)
  {
    Wt::WStringStream strm;

    strm
      << "var vert = [ [";

    for (size_t idx = 0; idx < lat.size() - 1; idx++)
    {
      strm
        << std::to_string((long double)lat.at(idx))
        << ","
        << std::to_string((long double)lon.at(idx))
        << "], [";
    }

    //last
    size_t idx = lat.size() - 1;
    strm
      << std::to_string((long double)lat.at(idx))
      << ","
      << std::to_string((long double)lon.at(idx))
      << "] ];";

    strm
      << "L.polygon(vert,{color:'"
      << color
      << "',opacity:.1}).addTo(map);";

    m_additions.push_back(strm.str());
  }
}
