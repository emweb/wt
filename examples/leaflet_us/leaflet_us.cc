#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WPushButton.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTemplate.h>
#include "leaflet_us.hh"
#include "gason.h"
#include "leaflet/WLeaflet.hh"
#include "leaflet/WPlotly.hh"
#include "leaflet/WCelsium.hh"
#include "leaflet/csv.hh"
#include "leaflet/geojson.hh"
#include "leaflet/topojson.hh"
using namespace Wt;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//example 8
//topojson US counties
/////////////////////////////////////////////////////////////////////////////////////////////////////

//-t 8 -g ../../../examples/leaflet_test/topojson/us.topojson

///////////////////////////////////////////////////////////////////////////////////////
//globals
///////////////////////////////////////////////////////////////////////////////////////

std::string rgb_to_hex(int r, int g, int b);
topojson_t topojson;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//to_hex
//convert int to hex string, apply zero padding
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string to_hex(int n)
{
  std::stringstream ss;
  ss << std::hex << n;
  std::string str(ss.str());
  return str.size() == 1 ? "0" + str : str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//rgb_to_hex
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string rgb_to_hex(int r, int g, int b)
{
  std::string str("#");
  str += to_hex(r);
  str += to_hex(g);
  str += to_hex(b);
  return str;
}

///////////////////////////////////////////////////////////////////////////////////////
//Application_us_counties
///////////////////////////////////////////////////////////////////////////////////////

class Application_us_counties : public WApplication
{
public:
  Application_us_counties(const WEnvironment& env) : WApplication(env)
  {
    setTitle("US counties");
    std::unique_ptr<WLeaflet> leaflet = cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 37.0902, -95.7129, 5);

    ///////////////////////////////////////////////////////////////////////////////////////
    //render topojson
    ///////////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    //must make the coordinates for the topology first
    //3 objects: counties (0), states (1), land (2) 
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t topology_index = 1;
    topojson.make_coordinates(topology_index);
    topology_object_t topology = topojson.m_topology.at(topology_index);

    size_t size_geom = topology.m_geom.size();
    for (size_t idx_geom = 0; idx_geom < size_geom; idx_geom++)
    {
      Geometry_t geometry = topology.m_geom.at(idx_geom);
      if (geometry.type.compare("Polygon") == 0 || geometry.type.compare("MultiPolygon") == 0)
      {
        size_t size_pol = geometry.m_polygon.size();
        for (size_t idx_pol = 0; idx_pol < size_pol; idx_pol++)
        {
          Polygon_topojson_t polygon = geometry.m_polygon.at(idx_pol);
          size_t size_arcs = polygon.arcs.size();

          ///////////////////////////////////////////////////////////////////////////////////////
          //render each polygon as a vector of vertices passed to Polygon
          ///////////////////////////////////////////////////////////////////////////////////////

          std::vector<double> lat;
          std::vector<double> lon;
          size_t size_points = geometry.m_polygon.at(idx_pol).m_y.size();
          for (size_t idx_crd = 0; idx_crd < size_points; idx_crd++)
          {
            lat.push_back(geometry.m_polygon.at(idx_pol).m_y.at(idx_crd));
            lon.push_back(geometry.m_polygon.at(idx_pol).m_x.at(idx_crd));
          }
          std::string color = rgb_to_hex(128, 0, 0);
          leaflet->Polygon(lat, lon, color);
        }//size_pol
      }//"Polygon"
    }//size_geom

    root()->addWidget(std::move(leaflet));
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//Application_plotly
///////////////////////////////////////////////////////////////////////////////////////

class Application_plotly : public WApplication
{
public:
  Application_plotly(const WEnvironment& env) : WApplication(env)
  {
    setTitle("Chart");
    std::string js;

    js += "var trace1 = {";
    js += "x: [1, 2, 3, 4],";
    js += "y: [10, 15, 13, 17],";
    js += "type: 'scatter'";
    js += "};";
    js += "var data = [trace1];";

    js += "var layout = {};";

    std::unique_ptr<WPlotly> plotly = cpp14::make_unique<WPlotly>(js);
    root()->addWidget(Wt::cpp14::make_unique<Wt::WText>("Plot"));
    root()->addWidget(std::move(plotly));
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//Application_celsium
///////////////////////////////////////////////////////////////////////////////////////

class Application_celsium : public WApplication
{
public:
  Application_celsium(const WEnvironment& env) : WApplication(env)
  {
    setTitle("Celsium");
    std::string js;

    std::unique_ptr<WCelsium> celsium = cpp14::make_unique<WCelsium>(js);
    root()->addWidget(std::move(celsium));
  }
};


///////////////////////////////////////////////////////////////////////////////////////
//create_application
///////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<WApplication> create_application(const WEnvironment& env)
{
  return cpp14::make_unique<Application_celsium>(env);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: ./leaflet_us.wt --http-address=0.0.0.0 --http-port=8080  --docroot=. ";
  std::cout << "-g GEOJSON: geojson file" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  std::string geojson_file;
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 'g':
        geojson_file = argv[i + 1];
        i++;
        break;
      }
    }
  }
  if (geojson_file.empty())
  {
    usage();
  }
  std::cout << geojson_file << std::endl;
  if (topojson.convert(geojson_file.c_str()) < 0)
  {
    assert(0);
  }
  return WRun(argc, argv, &create_application);
}








