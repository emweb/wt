#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topojson.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::convert
/////////////////////////////////////////////////////////////////////////////////////////////////////

int topojson_t::convert(const char* file_name)
{
  char *buf = 0;
  size_t length;
  FILE *f;

  f = fopen(file_name, "rb");
  if (!f)
  {
    std::cout << "cannot open " << file_name << std::endl;
    return -1;
  }

  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buf = (char*)malloc(length);
  if (buf)
  {
    fread(buf, 1, length, f);
  }
  fclose(f);

  char *endptr;
  JsonValue value;
  JsonAllocator allocator;
  int rc = jsonParse(buf, &endptr, &value, allocator);
  if (rc != JSON_OK)
  {
    std::cout << "invalid JSON format for " << buf << std::endl;
    return -1;
  }

  parse_root(value);
  free(buf);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::parse_root
/////////////////////////////////////////////////////////////////////////////////////////////////////

int topojson_t::parse_root(JsonValue value)
{
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    std::cout << std::string(node->key) << "\n";
    //A topology is a TopoJSON object where the type member’s value is “Topology”.
    if (std::string(node->key).compare("type") == 0)
    {
      assert(node->value.getTag() == JSON_STRING);
      std::string str = node->value.toString();
      if (str.compare("Topology"))
      {
        //define a type enumerator
      }
    }
    //A topology may have a “transform” member whose value is a transform object.
    else if (std::string(node->key).compare("transform") == 0)
    {
      assert(node->value.getTag() == JSON_OBJECT);
      parse_transform(node->value);
    }
    else if (std::string(node->key).compare("objects") == 0)
    {
      assert(node->value.getTag() == JSON_OBJECT);
      parse_topology(node->value);
    }
    else if (std::string(node->key).compare("arcs") == 0)
    {
      assert(node->value.getTag() == JSON_ARRAY);
      parse_arcs(node->value);
    }
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::parse_topology
/////////////////////////////////////////////////////////////////////////////////////////////////////

int topojson_t::parse_topology(JsonValue value)
{
  assert(value.getTag() == JSON_OBJECT);
  //A topology must have a member with the name “objects” whose value is another object. 
  //The value of each member of this object is a geometry object.
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    std::string object_name = node->key;
    std::cout << "\tobject name:\t" << object_name << "\n";
    JsonValue object = node->value;
    assert(object.getTag() == JSON_OBJECT);
    for (JsonNode *geom_obj = object.toNode(); geom_obj != nullptr; geom_obj = geom_obj->next)
    {
      //A geometry is a TopoJSON object where the type member’s value is one of the following strings: 
      //“Point”, “MultiPoint”, “LineString”, “MultiLineString”, “Polygon”, “MultiPolygon”, or “GeometryCollection”.
      if (std::string(geom_obj->key).compare("type") == 0)
      {
        assert(geom_obj->value.getTag() == JSON_STRING);
        std::string str = geom_obj->value.toString();
        std::cout << "\tgeometry type:\t" << str << "\n";
      }
      else if (std::string(geom_obj->key).compare("geometries") == 0)
      {
        assert(geom_obj->value.getTag() == JSON_ARRAY);
        parse_geometry_object(geom_obj->value);
      }//"geometries"
    }//geom_obj
  }//node
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::parse_geometry_object
/////////////////////////////////////////////////////////////////////////////////////////////////////

int topojson_t::parse_geometry_object(JsonValue value)
{
  assert(value.getTag() == JSON_ARRAY);
  //A TopoJSON geometry object of type “Point” or “MultiPoint” must have a member with the name “coordinates”. 
  //A TopoJSON geometry object of type “LineString”, “MultiLineString”, “Polygon”, or “MultiPolygon” 
  //must have a member with the name “arcs”. 
  //The value of the arcs and coordinates members is always an array. 
  //The structure for the elements in this array is determined by the type of geometry.
  //A geometry object may have a member with the name “properties”. 
  //The value of the properties member is an object (any JSON object or a JSON null value).
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    JsonValue object = node->value;
    assert(object.getTag() == JSON_OBJECT);
    Geometry_t geometry;
    for (JsonNode *obj = object.toNode(); obj != nullptr; obj = obj->next)
    {
      if (std::string(obj->key).compare("type") == 0)
      {
        assert(obj->value.getTag() == JSON_STRING);
        std::string str = obj->value.toString();
        std::cout << "\t\tgeometry type:\t" << str << "\n";
        geometry.type = str;
      }
      else if (std::string(obj->key).compare("coordinates") == 0)
      {
        assert(obj->value.getTag() == JSON_ARRAY);
      }
      else if (std::string(obj->key).compare("arcs") == 0)
      {
        assert(obj->value.getTag() == JSON_ARRAY);
      }
    }

    m_geom.push_back(geometry);
  }//node
  return 0;
}

//Transforms
//A topology may have a “transform” member whose value is a transform object.
//The purpose of the transform is to quantize positions for more efficient serialization, 
//by representing positions as integers rather than floats.
//A transform must have a member with the name “scale” whose value is a two - element array of numbers.
//A transform must have a member with the name “translate” whose value is a two - element array of numbers.
//Both the “scale” and “translate” members must be of length two.
//Every position in the topology must be quantized, with the first and second elements in each position an integer.
//To transform from a quantized position to an absolute position :
//Multiply each quantized position element by the corresponding scale element.
//Add the corresponding translate element.

std::vector<double> topojson_t::transform_point(const int position_quant[2])
{
  std::vector<double> position;
  position.push_back(position_quant[0] * scale[0] + translate[0]);
  position.push_back(position_quant[1] * scale[1] + translate[1]);
  return position;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::parse_transform
/////////////////////////////////////////////////////////////////////////////////////////////////////

int topojson_t::parse_transform(JsonValue value)
{
  assert(value.getTag() == JSON_OBJECT);
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    std::string object_name = node->key;
    std::cout << "\tobject name:\t" << object_name << "\n";
    if (std::string(node->key).compare("scale") == 0)
    {
      assert(node->value.getTag() == JSON_ARRAY);
      JsonValue arr = node->value;
      scale[0] = arr.toNode()->value.toNumber();;
      scale[1] = arr.toNode()->next->value.toNumber();
      std::cout << "\tscale:\t" << scale[0] << "," << scale[1] << "\n";
    }
    else if (std::string(node->key).compare("translate") == 0)
    {
      assert(node->value.getTag() == JSON_ARRAY);
      JsonValue arr = node->value;
      translate[0] = arr.toNode()->value.toNumber();
      translate[1] = arr.toNode()->next->value.toNumber();
      std::cout << "\ttranslate:\t" << translate[0] << "," << translate[1] << "\n";
    }

  }//node
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::parse_arcs
/////////////////////////////////////////////////////////////////////////////////////////////////////

int topojson_t::parse_arcs(JsonValue value)
{
  //A topology must have an “arcs” member whose value is an array of arrays of positions. 
  //Each arc must be an array of two or more positions.
  //If a topology is quantized, the positions of each arc in the topology which are quantized 
  //must be delta - encoded.
  assert(value.getTag() == JSON_ARRAY);
  for (JsonNode *node_arr_0 = value.toNode(); node_arr_0 != nullptr; node_arr_0 = node_arr_0->next)
  {
    assert(node_arr_0->value.getTag() == JSON_ARRAY);
    JsonValue arr_1 = node_arr_0->value;

    arc_t arc;
    for (JsonNode *node_arr_1 = arr_1.toNode(); node_arr_1 != nullptr; node_arr_1 = node_arr_1->next)
    {
      assert(node_arr_1->value.getTag() == JSON_ARRAY);
      JsonValue arr_2 = node_arr_1->value;
      std::vector<double> inner_arr;
      for (JsonNode *node_arr_2 = arr_2.toNode(); node_arr_2 != nullptr; node_arr_2 = node_arr_2->next)
      {
        assert(node_arr_2->value.getTag() == JSON_NUMBER);
        inner_arr.push_back(node_arr_2->value.toNumber());
      }//node_arr_2
      arc.vec.push_back(inner_arr);
    }//node_arr_1

    m_arcs.push_back(arc);
  }//node_arr_0

  std::cout << "arcs size: " << m_arcs.size() << "\n";
  for (size_t idx = 0; idx < m_arcs.size(); idx++)
  {
    arc_t arc = m_arcs.at(idx);
    std::cout << "\tarc size: " << arc.vec.size() << "\n";
  }
  return 0;
}

