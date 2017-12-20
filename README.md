Leaflet map for Wt
=============

[Wt](https://www.webtoolkit.eu/wt) is a C++ library for developing web applications. 
<br>
This fork adds a [Leaflet](http://leafletjs.com/) class.


Live demos
=============

Washington DC 311 database
--------

Washington DC S0311 code (rodent complaints) [DC311](https://311.dc.gov/) occurrences for year 2016. The circle has a radius of 100 meters

http://www.eden-earth.org:8081/

![image](https://user-images.githubusercontent.com/6119070/31053476-03da5ff8-a66c-11e7-9ad9-487aef6e062c.png)


US states topology
---------


http://www.eden-earth.org:8082/

![image](https://user-images.githubusercontent.com/6119070/31628950-25fbfe14-b280-11e7-880f-b3784ca3ceba.png)


NOAA ATMS satellite data 
--------


http://www.eden-earth.org:8083/

![image](https://user-images.githubusercontent.com/6119070/34190182-5986aaf8-e50d-11e7-847c-f68d6fa2310d.png)


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
cd examples/leaflet_test
make
./leaflet_test.wt --http-address=0.0.0.0 --http-port=8080  --docroot=.

-t 2 -d ../../../examples/leaflet_test/dc_311-2016.csv.s0311.csv

 -g ../../../examples/leaflet_test/ward-2012.geojson

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

Topology of US states example
---------
Data source

https://www.census.gov/geo/maps-data/data/cbf/cbf_state.html
Converted to GeoJSON
https://tools.ietf.org/html/rfc7946

Run with
<pre>
./leaflet_test.wt --http-address=0.0.0.0 --http-port=8080  --docroot=.

-t 3 -g ../../../examples/leaflet_test/gz_2010_us_040_00_20m.json

</pre>


NOAA ATMS example
----------

Run with

<pre>
./leaflet_test.wt --http-address=0.0.0.0 --http-port=8080  --docroot=.

-t 4 -d ../../../examples/leaflet_test/TATMS_npp_d20141130_t1817273_e1817589_b16023_c20141201005810987954_noaa_ops.h5.star.json

</pre>
