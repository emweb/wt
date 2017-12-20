Leaflet map for Wt
=============

[Wt](https://www.webtoolkit.eu/wt) is a C++ library for developing web applications. 
This fork adds a [Leaflet](http://leafletjs.com/) class.


Building
=============

Switch to branch leaflet
------------
<pre>
git checkout leaflet
</pre>

Install dependencies
------------
[cmake](https://cmake.org/)

[boost](http://www.boost.org/)

Linux Ubuntu, install packages with

<pre>
sudo apt-get install cmake
sudo apt-get install build-essential
sudo apt-get install python-dev
sudo apt-get install libboost-all-dev
</pre>

Mac OSX

Install Homebrew

<pre>
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
</pre>

Install packages with

<pre>
brew install cmake 
brew install boost 
</pre>


Build with
--------------
<pre>
git clone https://github.com/pedro-vicente/wt.git
git checkout leaflet
cd build
cmake .. 
make
</pre>

Build with custom boost location
------------
<pre>
cmake .. -G "Visual Studio 14" -DBOOST_PREFIX=E:\wt\boost_1_64_0 -DBOOST_DYNAMIC=ON -DSHARED_LIBS=ON
</pre>

Run with
------------
<pre>
cd examples/leaflet_dc311
make
./leaflet_dc311.wt --http-address=0.0.0.0 --http-port=8080  --docroot=. -f ../../../examples/leaflet_dc311/dc_311-2016.csv.s0311.csv
</pre>

Open a browser
------------
<pre>
http://127.0.0.1:8080/
</pre>

Source code examples
===========

Examples are in

DC311
District of Columbia, Office of Unified Communications complaints

[/examples/leaflet_dc311/leaflet_dc311.cc](https://github.com/pedro-vicente/wt/blob/leaflet/examples/leaflet_dc311/leaflet_dc311.cc)

US states
Topology of US states

[/examples/leaflet_states/leaflet_states.cc](https://github.com/pedro-vicente/wt/blob/leaflet/examples/leaflet_states/leaflet_states.cc)


Leaflet class source is in [/src/leaflet/WLeaflet.cc](https://github.com/pedro-vicente/wt/blob/leaflet/src/leaflet/WLeaflet.cc)

Usage of the Leaflet Wt class
==========

The API supports 2 map tile providers: CartoDB and RRZE Openstreetmap-Server

[cartoDB](https://carto.com/)

[RRZE Openstreetmap-Server](https://osm.rrze.fau.de/)

```c++
enum class tile_provider_t
{
  CARTODB, RRZE
};
```

Example of a map of Washington DC using CartoDB tiles

```c++
class MapApplication : public WApplication
{
public:
  MapApplication(const WEnvironment& env) : WApplication(env)
  {
    std::unique_ptr<WLeaflet> leaflet = cpp14::make_unique<WLeaflet>(tile_provider_t::CARTODB, 38.9072 -77.0369, 13);
    root()->addWidget(std::move(leaflet));
  }
};
```

API
------------
```c++
void Circle(const std::string &lat, const std::string &lon);
void Polygon(const std::vector<double> &lat, const std::vector<double> &lon);
```

Output
------------
Washington DC S0311 code (rodent complaints) [DC311](https://311.dc.gov/) occurrences for year 2016. The circle has a radius of 100 meters

![image](https://user-images.githubusercontent.com/6119070/31053476-03da5ff8-a66c-11e7-9ad9-487aef6e062c.png)


Topology of US states example
---------
Data source

https://www.census.gov/geo/maps-data/data/cbf/cbf_state.html
Converted to GeoJSON
https://tools.ietf.org/html/rfc7946

Run with
------------
<pre>
cd examples/leaflet_states
make
./leaflet_states.wt --http-address=0.0.0.0 --http-port=8080  --docroot=. -f ../../../examples/leaflet_states/gz_2010_us_040_00_20m.json
</pre>

Open browser
------------
<pre>
http://127.0.0.1:8080/
</pre>

Output
------------
![image](https://user-images.githubusercontent.com/6119070/31628950-25fbfe14-b280-11e7-880f-b3784ca3ceba.png)