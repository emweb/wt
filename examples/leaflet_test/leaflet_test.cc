#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout.h>
#include <Wt/WPushButton>
#include <Wt/WStringListModel>
#include <Wt/WTemplate>
#include "leaflet/WLeaflet.hh"
#include "leaflet/csv.hh"
#include "leaflet/geojson.hh"
#include "pal_rgb.h"
using namespace Wt;

//./leaflet_test.wt --http-address=0.0.0.0 --http-port=8080  --docroot=.
//-t 2 -d ../../../examples/leaflet_test/dc_311-2016.csv.s0311.csv -g ../../../examples/leaflet_test/ward-2012.geojson
//-t 3 -g ../../../examples/leaflet_test/gz_2010_us_040_00_20m.json
//38.9072, -77.0369, 14 DC
//37.0902, -95.7129, 5 US

/////////////////////////////////////////////////////////////////////////////////////////////////////
//rgb_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class rgb_t
{
public:
  rgb_t(unsigned char r, unsigned char g, unsigned char b) :
    red(r),
    green(g),
    blue(b)
  {
  }
  unsigned char red;
  unsigned char green;
  unsigned char blue;
};

///////////////////////////////////////////////////////////////////////////////////////
//dc311_data_t
///////////////////////////////////////////////////////////////////////////////////////

class dc311_data_t
{
public:
  dc311_data_t(const std::string &date_,
    const std::string &lat_,
    const std::string &lon_,
    const std::string &zip_) :
    date(date_),
    lat(lat_),
    lon(lon_),
    zip(zip_)
  {
  };
  std::string date;
  std::string lat;
  std::string lon;
  std::string zip;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////////////

int read_dc311(std::string file_name);
std::string to_hex(int n);
std::string rgb_to_hex(int r, int g, int b);

///////////////////////////////////////////////////////////////////////////////////////
//globals
///////////////////////////////////////////////////////////////////////////////////////

std::string test;
std::vector<rgb_t> rgb_256;
geojson_t geojson;
std::vector<dc311_data_t> dc311_data;
std::vector<std::string> ward_color =
{ rgb_to_hex(128, 128, 0), //olive
rgb_to_hex(255, 255, 0), //yellow 
rgb_to_hex(0, 128, 0), //green
rgb_to_hex(0, 255, 0), //lime
rgb_to_hex(0, 128, 128), //teal
rgb_to_hex(0, 255, 255), //aqua
rgb_to_hex(0, 0, 255), //blue
rgb_to_hex(128, 0, 128) //purple
};


///////////////////////////////////////////////////////////////////////////////////////
//Application_test
///////////////////////////////////////////////////////////////////////////////////////

class Application_test : public WApplication
{
public:
  Application_test(const WEnvironment& env) : WApplication(env)
  {
    useStyleSheet("boxes.css");
    auto hbox = root()->setLayout(cpp14::make_unique<WVBoxLayout>());
    std::unique_ptr<WText> text = cpp14::make_unique<WText>("item 1");
    text->setStyleClass("green-box");
    hbox->addWidget(std::move(text));
    std::unique_ptr<WLeaflet> leaflet =
      cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 38.9072, -77.0369, 13);
    leaflet->setStyleClass("blue-box");
    hbox->addWidget(std::move(leaflet));
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//Application_dc311
///////////////////////////////////////////////////////////////////////////////////////

class Application_dc311 : public WApplication
{
public:
  Application_dc311(const WEnvironment& env) : WApplication(env)
  {
    setTitle("leaflet");
    std::unique_ptr<WLeaflet> leaflet =
      cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 38.9072, -77.0369, 13);


    ///////////////////////////////////////////////////////////////////////////////////////
    //render geojson
    ///////////////////////////////////////////////////////////////////////////////////////

    size_t size_features = geojson.m_feature.size();
    for (size_t idx_fet = 0; idx_fet < size_features; idx_fet++)
    {
      feature_t feature = geojson.m_feature.at(idx_fet);

      //make each feature have a unique color
      std::string color_feature = ward_color.at(idx_fet);

      size_t size_geometry = feature.m_geometry.size();
      for (size_t idx_geo = 0; idx_geo < size_geometry; idx_geo++)
      {
        geometry_t geometry = feature.m_geometry.at(idx_geo);
        size_t size_pol = geometry.m_polygons.size();

        for (size_t idx_pol = 0; idx_pol < size_pol; idx_pol++)
        {
          polygon_t polygon = geometry.m_polygons[idx_pol];
          size_t size_crd = polygon.m_coord.size();

          if (size_crd == 0)
          {
            continue;
          }

          ///////////////////////////////////////////////////////////////////////////////////////
          //render each polygon as a vector of vertices passed to Polygon
          ///////////////////////////////////////////////////////////////////////////////////////

          std::vector<double> lat;
          std::vector<double> lon;

          for (size_t idx_crd = 0; idx_crd < size_crd; idx_crd++)
          {
            lat.push_back(polygon.m_coord[idx_crd].m_lat);
            lon.push_back(polygon.m_coord[idx_crd].m_lon);
          }

          leaflet->Polygon(lat, lon, color_feature);
        }  //idx_pol
      } //idx_geo
    } //idx_fet


    ///////////////////////////////////////////////////////////////////////////////////////
    //render CSV points from DC_311 database
    ///////////////////////////////////////////////////////////////////////////////////////

    size_t size = dc311_data.size();
    for (size_t idx = 0; idx < size; idx++)
    {
      dc311_data_t data = dc311_data.at(idx);
      leaflet->Circle(data.lat, data.lon);
    }

    root()->addWidget(std::move(leaflet));
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//Application_geojson
///////////////////////////////////////////////////////////////////////////////////////

class Application_geojson : public WApplication
{
public:
  Application_geojson(const WEnvironment& env) : WApplication(env)
  {
    setTitle("leaflet");
    std::unique_ptr<WLeaflet> leaflet =
      cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 38.9072, -77.0369, 8);

    ///////////////////////////////////////////////////////////////////////////////////////
    //render geojson
    ///////////////////////////////////////////////////////////////////////////////////////

    size_t size_features = geojson.m_feature.size();
    for (size_t idx_fet = 0; idx_fet < size_features; idx_fet++)
    {
      feature_t feature = geojson.m_feature.at(idx_fet);

      //make each feature have a unique color

      size_t nbr_palette_entries = rgb_256.size();
      double value = idx_fet;
      double value_min = 0;
      double value_range = size_features - value_min;
      size_t idx_pal = (size_t)(nbr_palette_entries * ((value - value_min) / value_range));
      idx_pal = (idx_pal < 0 ? 0 : (idx_pal >= nbr_palette_entries ? nbr_palette_entries - 1 : idx_pal));
      std::string color_feature = rgb_to_hex(
        rgb_256.at(idx_pal).red,
        rgb_256.at(idx_pal).green,
        rgb_256.at(idx_pal).blue);

      size_t size_geometry = feature.m_geometry.size();
      for (size_t idx_geo = 0; idx_geo < size_geometry; idx_geo++)
      {
        geometry_t geometry = feature.m_geometry.at(idx_geo);
        size_t size_pol = geometry.m_polygons.size();

        for (size_t idx_pol = 0; idx_pol < size_pol; idx_pol++)
        {
          polygon_t polygon = geometry.m_polygons[idx_pol];
          size_t size_crd = polygon.m_coord.size();

          if (size_crd == 0)
          {
            continue;
          }

          ///////////////////////////////////////////////////////////////////////////////////////
          //render each polygon as a vector of vertices passed to Polygon
          ///////////////////////////////////////////////////////////////////////////////////////

          std::vector<double> lat;
          std::vector<double> lon;

          for (size_t idx_crd = 0; idx_crd < size_crd; idx_crd++)
          {
            lat.push_back(polygon.m_coord[idx_crd].m_lat);
            lon.push_back(polygon.m_coord[idx_crd].m_lon);
          }

          if (geometry.m_type.compare("Point") == 0)
          {
            leaflet->Circle(lat[0], lon[0], color_feature);
          }
          else if (geometry.m_type.compare("Polygon") == 0 ||
            geometry.m_type.compare("MultiPolygon") == 0)
          {
            leaflet->Polygon(lat, lon, color_feature);
          }

        }  //idx_pol
      } //idx_geo
    } //idx_fet

    root()->addWidget(std::move(leaflet));
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//create_application
///////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<WApplication> create_application(const WEnvironment& env)
{
  if (test.compare("1") == 0)
  {
    return cpp14::make_unique<Application_test>(env);
  }
  else if (test.compare("2") == 0)
  {
    return cpp14::make_unique<Application_dc311>(env);
  }
  else if (test.compare("3") == 0)
  {
    return cpp14::make_unique<Application_geojson>(env);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: ./leaflet_test.wt --http-address=0.0.0.0 --http-port=8080  --docroot=. ";
  std::cout << "-t TEST <-d DATABASE> <-g GEOJSON>";
  std::cout << std::endl;
  std::cout << "-t TEST: test number (1 to 3)" << std::endl;
  std::cout << "-d DATABASE: data file" << std::endl;
  std::cout << "-g GEOJSON: geojson file" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  std::string data_file;
  std::string geojson_file;

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 't':
        test = argv[i + 1];
        i++;
        break;
      case 'd':
        data_file = argv[i + 1];
        i++;
        break;
      case 'g':
        geojson_file = argv[i + 1];
        i++;
        break;
      }
    }
  }

  if (test.empty())
  {
    usage();
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //dc_311 test, read CSV data file with -d, geojson with -g 
  ///////////////////////////////////////////////////////////////////////////////////////

  if (test.compare("2") == 0)
  {
    if (data_file.empty())
    {
      usage();
    }
    if (read_dc311(data_file) < 0)
    {
      exit(0);
    }
    if (geojson.convert(geojson_file.c_str()) < 0)
    {
      exit(0);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //geojson test, read geojson file with -f 
  ///////////////////////////////////////////////////////////////////////////////////////

  else if (test.compare("3") == 0)
  {
    if (geojson_file.empty())
    {
      usage();
    }
    if (geojson.convert(geojson_file.c_str()) < 0)
    {
      exit(0);
    }
  }

  for (size_t idx = 0; idx < 3 * 256; idx += 3)
  {
    unsigned char r = pal_rgb[idx];
    unsigned char g = pal_rgb[idx + 1];
    unsigned char b = pal_rgb[idx + 2];
    rgb_256.push_back(rgb_t(r, g, b));
  }
  return WRun(argc, argv, &create_application);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//read_dc311
/////////////////////////////////////////////////////////////////////////////////////////////////////

int read_dc311(std::string file_name)
{
  read_csv_t csv;

  if (csv.open(file_name) < 0)
  {
    std::cout << "Cannot open file " << file_name.c_str() << std::endl;
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //iterate until an empty row is returned (end of file)
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  size_t rows = 0;
  std::vector<std::string> row;
  while (true)
  {
    row = csv.read_row();
    if (row.size() == 0)
    {
      break;
    }
    rows++;

    dc311_data_t data(row.at(0), row.at(1), row.at(2), row.at(3));
    dc311_data.push_back(data);
  }

  std::cout << "Processed " << rows << " rows" << std::endl;
  return 0;
}

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

