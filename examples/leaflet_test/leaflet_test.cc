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
#include "leaflet/star_json.hh"
#include "leaflet/star_dataset.hh"
#include "pal_rgb.h"
using namespace Wt;

//./leaflet_test.wt --http-address=0.0.0.0 --http-port=8080  --docroot=.

/////////////////////////////////////////////////////////////////////////////////////////////////////
//example 2
//DC311 rodent complaints and DC wards geojson
/////////////////////////////////////////////////////////////////////////////////////////////////////

//-t 2 -d ../../../examples/leaflet_test/dc_311-2016.csv.s0311.csv -g ../../../examples/leaflet_test/ward-2012.geojson

/////////////////////////////////////////////////////////////////////////////////////////////////////
//example 3
//US states geojson
/////////////////////////////////////////////////////////////////////////////////////////////////////

//-t 3 -g ../../../examples/leaflet_test/gz_2010_us_040_00_20m.json

/////////////////////////////////////////////////////////////////////////////////////////////////////
//example 4
//NOAA ATMS data
/////////////////////////////////////////////////////////////////////////////////////////////////////

//-t 4 -d ../../../examples/leaflet_test/TATMS_npp_d20141130_t1817273_e1817589_b16023_c20141201005810987954_noaa_ops.h5.star.json

std::vector<star_dataset_t> datasets;
star_dataset_t find_dataset(std::string name);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//example 5
//ep
/////////////////////////////////////////////////////////////////////////////////////////////////////

//-t 5 
//-d ../../../examples/leaflet_test/us_states_epilepsy_2015.csv 
//-g ../../../examples/leaflet_test/gz_2010_us_040_00_20m.json 
//-u ../../../examples/leaflet_test/us_states_population_2015.csv

/////////////////////////////////////////////////////////////////////////////////////////////////////
//example 6
//schools
/////////////////////////////////////////////////////////////////////////////////////////////////////

//-t 6
//-d ../../../examples/leaflet_test/montgomery_county_schools.csv
//-g ../../../examples/leaflet_test/board_education_districts.json 


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

///////////////////////////////////////////////////////////////////////////////////////
//ep_data_t
///////////////////////////////////////////////////////////////////////////////////////

class ep_data_t
{
public:
  ep_data_t(std::string state_, size_t ep_) :
    state(state_),
    ep(ep_)
  {
  };
  std::string state;
  size_t ep;
};

///////////////////////////////////////////////////////////////////////////////////////
//us_state_pop_t
///////////////////////////////////////////////////////////////////////////////////////

class us_state_pop_t
{
public:
  us_state_pop_t(std::string state_, size_t population_) :
    state(state_),
    population(population_)
  {
  };
  std::string state;
  size_t population;
};


///////////////////////////////////////////////////////////////////////////////////////
//school_t
///////////////////////////////////////////////////////////////////////////////////////

class school_t
{
public:
  school_t(std::string name_, std::string lat_, std::string lon_, int rating_) :
    name(name_),
    lat(lat_),
    lon(lon_),
    rating(rating_)
  {
  };
  std::string name;
  std::string lat;
  std::string lon;
  int rating;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//forward declarations
/////////////////////////////////////////////////////////////////////////////////////////////////////

int read_dc311(std::string file_name);
int read_ep_pop(const std::string &file_name_ep, const std::string &file_name_pop);
int read_schools(const std::string &file_name);
size_t find_ep(std::string state_name);
size_t find_population(std::string state_name);
std::string to_hex(int n);
std::string rgb_to_hex(int r, int g, int b);

///////////////////////////////////////////////////////////////////////////////////////
//globals
///////////////////////////////////////////////////////////////////////////////////////

std::string test;
std::vector<rgb_t> rgb_256;
geojson_t geojson;
std::vector<dc311_data_t> dc311_data;
std::vector<ep_data_t> ep_data;
std::vector<us_state_pop_t> us_state_pop;
std::vector<school_t> schools_list;
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
//38.9072, -77.0369, 14 DC
//37.0902, -95.7129, 5 US
///////////////////////////////////////////////////////////////////////////////////////

class Application_test : public WApplication
{
public:
  Application_test(const WEnvironment& env) : WApplication(env)
  {
    setTitle("test");
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
    setTitle("dc311");
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
      std::string color = ward_color.at(idx_fet);

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

          leaflet->Polygon(lat, lon, color);
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
//Application_us_states
///////////////////////////////////////////////////////////////////////////////////////

class Application_us_states : public WApplication
{
public:
  Application_us_states(const WEnvironment& env) : WApplication(env)
  {
    setTitle("geojson");
    std::unique_ptr<WLeaflet> leaflet =
      cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 38.9072, -77.0369, 8);

    ///////////////////////////////////////////////////////////////////////////////////////
    //render geojson
    ///////////////////////////////////////////////////////////////////////////////////////

    size_t size_features = geojson.m_feature.size();
    for (size_t idx_fet = 0; idx_fet < size_features; idx_fet++)
    {
      feature_t feature = geojson.m_feature.at(idx_fet);
      size_t nbr_palette_entries = rgb_256.size();
      double value = idx_fet;
      double value_min = 0;
      double value_range = size_features - value_min;
      size_t idx_pal = (size_t)(nbr_palette_entries * ((value - value_min) / value_range));
      idx_pal = (idx_pal < 0 ? 0 : (idx_pal >= nbr_palette_entries ? nbr_palette_entries - 1 : idx_pal));
      std::string color = rgb_to_hex(
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
            leaflet->Circle(lat[0], lon[0], color);
          }
          else if (geometry.m_type.compare("Polygon") == 0 ||
            geometry.m_type.compare("MultiPolygon") == 0)
          {
            leaflet->Polygon(lat, lon, color);
          }

        }  //idx_pol
      } //idx_geo
    } //idx_fet

    root()->addWidget(std::move(leaflet));
  }
};

///////////////////////////////////////////////////////////////////////////////////////
//Application_atms
///////////////////////////////////////////////////////////////////////////////////////

class Application_atms : public WApplication
{
public:
  Application_atms(const WEnvironment& env) : WApplication(env)
  {
    star_dataset_t temperature = find_dataset("AntennaTemperature");
    star_dataset_t latitude = find_dataset("latitude");
    star_dataset_t longitude = find_dataset("longitude");
    size_t nbr_rows = temperature.m_shape[0];
    size_t nbr_cols = temperature.m_shape[1];
    size_t nbr_lev = temperature.m_shape[2];
    double temp_min;
    double temp_max;
    double lat_min;
    double lat_max;
    double lon_min;
    double lon_max;
    temperature.do_min_max(temp_min, temp_max);
    latitude.do_min_max(lat_min, lat_max);
    longitude.do_min_max(lon_min, lon_max);
    double lat = (lat_max + lat_min) / 2;
    double lon = (lon_max + lon_min) / 2;

    setTitle("atms");
    std::unique_ptr<WLeaflet> leaflet =
      cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, lat, lon, 5);

    ///////////////////////////////////////////////////////////////////////////////////////
    //render border
    ///////////////////////////////////////////////////////////////////////////////////////

    std::vector<double> blat(4);
    std::vector<double> blon(4);
    blat[0] = lat_min;
    blon[0] = lon_min;
    blat[1] = lat_max;
    blon[1] = lon_min;
    blat[2] = lat_max;
    blon[2] = lon_max;
    blat[3] = lat_min;
    blon[3] = lon_max;
    leaflet->Polygon(blat, blon, "#76d7c4");

    ///////////////////////////////////////////////////////////////////////////////////////
    //render data
    ///////////////////////////////////////////////////////////////////////////////////////

    size_t idx_channel = 0;

    for (size_t idx_row = 0; idx_row < nbr_rows - 1; idx_row++)
    {
      for (size_t idx_col = 0; idx_col < nbr_cols - 1; idx_col++)
      {
        ///////////////////////////////////////////////////////////////////////////
        //polygon coordinates for a granule
        ///////////////////////////////////////////////////////////////////////////

        std::vector<double> vlat(4);
        std::vector<double> vlon(4);
        vlat[0] = latitude.value_at(idx_row, idx_col);
        vlon[0] = longitude.value_at(idx_row, idx_col);
        vlat[1] = latitude.value_at(idx_row, idx_col + 1);
        vlon[1] = longitude.value_at(idx_row, idx_col + 1);
        vlat[2] = latitude.value_at(idx_row + 1, idx_col + 1);
        vlon[2] = longitude.value_at(idx_row + 1, idx_col + 1);
        vlat[3] = latitude.value_at(idx_row + 1, idx_col);
        vlon[3] = longitude.value_at(idx_row + 1, idx_col);

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        //color
        //The Vizualization Toolkit, pg.156
        /////////////////////////////////////////////////////////////////////////////////////////////////////

        size_t nbr_palette_entries = rgb_256.size();
        double value_range = temp_max - temp_min;
        double value_min = temp_min;
        double value = temperature.value_at(idx_row, idx_col, idx_channel);
        size_t idx_pal = (size_t)(nbr_palette_entries * ((value - value_min) / value_range));
        idx_pal = (idx_pal < 0 ? 0 : (idx_pal >= nbr_palette_entries ? nbr_palette_entries - 1 : idx_pal));
        std::string color = rgb_to_hex(
          rgb_256.at(idx_pal).red,
          rgb_256.at(idx_pal).green,
          rgb_256.at(idx_pal).blue);

        leaflet->Polygon(vlat, vlon, color);
      }
    }

    root()->addWidget(std::move(leaflet));
  }
};



///////////////////////////////////////////////////////////////////////////////////////
//Application_ep
///////////////////////////////////////////////////////////////////////////////////////

class Application_ep : public WApplication
{
public:
  Application_ep(const WEnvironment& env) : WApplication(env)
  {
    setTitle("geojson");
    std::unique_ptr<WLeaflet> leaflet =
      cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 37.0902, -95.7129, 5);

    ///////////////////////////////////////////////////////////////////////////////////////
    //render geojson
    ///////////////////////////////////////////////////////////////////////////////////////

    size_t size_features = geojson.m_feature.size();
    for (size_t idx_fet = 0; idx_fet < size_features; idx_fet++)
    {
      feature_t feature = geojson.m_feature.at(idx_fet);

      //find ep by state name
      size_t ep = find_ep(feature.m_name);
      //find population by state name
      size_t population = find_population(feature.m_name);

      if (ep == 0 || population == 0)
      {
        continue;
      }

      double percentage_state_epilepsy = (double(ep) / double(population)) / 100.0;

      percentage_state_epilepsy *= 100000;
      std::string color;
      if (percentage_state_epilepsy < 10)
      {
        color = rgb_to_hex(0, 255, 0);
      }
      else if (percentage_state_epilepsy < 10.4)
      {
        color = rgb_to_hex(0, 128, 0);
      }
      else if (percentage_state_epilepsy < 10.8)
      {
        color = rgb_to_hex(0, 255, 255);
      }
      else if (percentage_state_epilepsy < 11.2)
      {
        color = rgb_to_hex(255, 0, 0);
      }
      else
      {
        color = rgb_to_hex(128, 0, 0);
      }

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
            leaflet->Circle(lat[0], lon[0], color);
          }
          else if (geometry.m_type.compare("Polygon") == 0 ||
            geometry.m_type.compare("MultiPolygon") == 0)
          {
            leaflet->Polygon(lat, lon, color);
          }

        }  //idx_pol
      } //idx_geo
    } //idx_fet

    root()->addWidget(std::move(leaflet));
  }
};


///////////////////////////////////////////////////////////////////////////////////////
//Application_schools
///////////////////////////////////////////////////////////////////////////////////////

class Application_schools : public WApplication
{
public:
  Application_schools(const WEnvironment& env) : WApplication(env)
  {
    setTitle("geojson");
    std::unique_ptr<WLeaflet> leaflet =
      cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 39.0443047898, -77.1731281364, 12);

    ///////////////////////////////////////////////////////////////////////////////////////
    //render schools
    ///////////////////////////////////////////////////////////////////////////////////////

    size_t size = schools_list.size();
    for (size_t idx = 0; idx < size; idx++)
    {
      school_t data = schools_list.at(idx);
      leaflet->Marker(data.lat, data.lon, data.name);
    }

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
    return cpp14::make_unique<Application_us_states>(env);
  }
  else if (test.compare("4") == 0)
  {
    return cpp14::make_unique<Application_atms>(env);
  }
  else if (test.compare("5") == 0)
  {
    return cpp14::make_unique<Application_ep>(env);
  }
  else if (test.compare("6") == 0)
  {
    return cpp14::make_unique<Application_schools>(env);
  }
  assert(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: ./leaflet_test.wt --http-address=0.0.0.0 --http-port=8080  --docroot=. ";
  std::cout << "-t TEST <-d DATABASE> <-u DATABASE> <-g GEOJSON> ";
  std::cout << std::endl;
  std::cout << "-t TEST: test number (1 to 6)" << std::endl;
  std::cout << "-d DATABASE: data file" << std::endl;
  std::cout << "-g GEOJSON: geojson file" << std::endl;
  std::cout << "-u DATABASE: data file" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  std::string data_file;
  std::string data_file_us_states_pop;
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
      case 'u':
        data_file_us_states_pop = argv[i + 1];
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

  ///////////////////////////////////////////////////////////////////////////////////////
  //NOAA ATMS star json file
  ///////////////////////////////////////////////////////////////////////////////////////

  else if (test.compare("4") == 0)
  {
    if (data_file.empty())
    {
      usage();
    }
    if (read_datasets(data_file.c_str(), datasets) < 0)
    {
      assert(0);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //ep
  ///////////////////////////////////////////////////////////////////////////////////////

  else if (test.compare("5") == 0)
  {
    if (data_file.empty() || data_file_us_states_pop.empty() || geojson_file.empty())
    {
      usage();
    }

    std::cout << data_file << std::endl;
    std::cout << data_file_us_states_pop << std::endl;
    std::cout << geojson_file << std::endl;

    if (data_file.empty() || geojson_file.empty())
    {
      usage();
    }
    if (read_ep_pop(data_file, data_file_us_states_pop) < 0)
    {
      exit(0);
    }
    if (geojson.convert(geojson_file.c_str()) < 0)
    {
      exit(0);
    }
  }
  ///////////////////////////////////////////////////////////////////////////////////////
  //schools
  ///////////////////////////////////////////////////////////////////////////////////////

  else if (test.compare("6") == 0)
  {
    std::cout << data_file << std::endl;
    std::cout << geojson_file << std::endl;
    if (read_schools(data_file) < 0)
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
//read_ep
/////////////////////////////////////////////////////////////////////////////////////////////////////

int read_ep_pop(const std::string &file_name_ep, const std::string &file_name_pop)
{
  read_csv_t csv;

  if (csv.open(file_name_ep) < 0)
  {
    std::cout << "Cannot open file " << file_name_ep.c_str() << std::endl;
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

    std::string eps = row.at(1);
    size_t pos = eps.find("(");
    eps = eps.substr(0, pos);
    pos = eps.find(",");
    std::string eps1 = eps.substr(0, pos);
    std::string eps2 = eps.substr(pos + 1);
    eps = eps1 + eps2;
    ep_data_t data(row.at(0), std::stoul(eps));
    ep_data.push_back(data);
    std::cout << ep_data.back().state << " " << ep_data.back().ep << "\t";
  }

  std::cout << "\n";
  std::cout << "Read " << rows << " rows" << "\n";

  csv.m_ifs.close();

  if (csv.open(file_name_pop) < 0)
  {
    std::cout << "Cannot open file " << file_name_pop.c_str() << std::endl;
    return -1;
  }

  rows = 0;
  while (true)
  {
    row = csv.read_row();
    if (row.size() == 0)
    {
      break;
    }
    rows++;

    std::string state = row.at(0);
    std::string pop = row.at(1);
    us_state_pop_t state_pop(state, std::stoul(pop));
    us_state_pop.push_back(state_pop);
    std::cout << us_state_pop.back().state << " " << us_state_pop.back().population << "\t";

  }
  std::cout << "\n";
  std::cout << "Read " << rows << " rows" << "\n";

  csv.m_ifs.close();
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


/////////////////////////////////////////////////////////////////////////////////////////////////////
//find_dataset
/////////////////////////////////////////////////////////////////////////////////////////////////////

star_dataset_t find_dataset(std::string name)
{
  for (size_t idx = 0; idx < datasets.size(); idx++)
  {
    if (datasets.at(idx).m_name.compare(name) == 0)
    {
      return datasets.at(idx);
    }
  }
  assert(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//find_ep
//by state name
/////////////////////////////////////////////////////////////////////////////////////////////////////

size_t find_ep(std::string state_name)
{
  for (size_t idx = 0; idx < ep_data.size(); idx++)
  {
    std::string state = ep_data.at(idx).state;
    if (state.find(state_name) == 0)
    {
      return ep_data.at(idx).ep;
    }
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//find_population
//by state name
/////////////////////////////////////////////////////////////////////////////////////////////////////

size_t find_population(std::string state_name)
{
  for (size_t idx = 0; idx < us_state_pop.size(); idx++)
  {
    std::string state = us_state_pop.at(idx).state;
    if (state.find(state_name) == 0)
    {
      return us_state_pop.at(idx).population;
    }
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//read_schools
//CATEGORY,SCHOOL NAME,ADDRESS,CITY,ZIP CODE,PHONE,URL,LONGITUDE,LATITUDE,LOCATION
/////////////////////////////////////////////////////////////////////////////////////////////////////

int read_schools(const std::string &file_name)
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

  std::vector<std::string> row;

  //read header
  row = csv.read_row();
  size_t rows = 0;
  while (true)
  {
    row = csv.read_row();
    if (row.size() == 0)
    {
      break;
    }
    rows++;

    std::string name = row.at(1);
    std::string lon = row.at(7);
    std::string lat = row.at(8);
    std::string rating = row.at(10);
    schools_list.push_back(school_t(name, lat, lon, std::stoi(rating)));
  }

  std::cout << "\n";
  std::cout << "Read " << rows << " rows" << "\n";
  csv.m_ifs.close();
  return 0;
}
