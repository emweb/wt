#ifndef WLEAFLET_TEST_H_
#define WLEAFLET_TEST_H_

#include "string"

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

///////////////////////////////////////////////////////////////////////////////////////
//wmata_station_t
///////////////////////////////////////////////////////////////////////////////////////

class wmata_station_t
{
public:
  wmata_station_t(const std::string &lat_,
    const std::string &lon_,
    const std::string &name_,
    const std::string &line_code_) :
    lat(lat_),
    lon(lon_),
    name(name_),
    line_code(line_code_)
  {
  };
  std::string lat;
  std::string lon;
  std::string name;
  std::string line_code;
};

#endif