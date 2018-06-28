#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topojson.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//is_topojson
/////////////////////////////////////////////////////////////////////////////////////////////////////

int is_topojson(const char* file_name)
{
  char *buf = 0;
  size_t length;
  FILE *f;
  int json_type = -1;

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

  //parse
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    //A topology is a TopoJSON object where the type member’s value is “Topology”.
    if (std::string(node->key).compare("type") == 0)
    {
      assert(node->value.getTag() == JSON_STRING);
      std::string str = node->value.toString();
      if (str.compare("Topology") == 0)
      {
        json_type++;
      }
    }
    else if (std::string(node->key).compare("arcs") == 0)
    {
      assert(node->value.getTag() == JSON_ARRAY);
      json_type++;
    }
  }

  free(buf);
  return json_type;
}

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
  make_coordinates();
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
    for (JsonNode *obj_geometry = object.toNode(); obj_geometry != nullptr; obj_geometry = obj_geometry->next)
    {
      if (std::string(obj_geometry->key).compare("type") == 0)
      {
        assert(obj_geometry->value.getTag() == JSON_STRING);
        geometry.type = obj_geometry->value.toString();
        std::cout << "\t\tgeometry type:\t" << geometry.type << "\n";
      }
    }//obj_geometry

    m_geom.push_back(geometry);
  }//node

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //second traverse to define geometry (geometry "type" could be defined after "arcs" )
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    JsonValue object = node->value;
    assert(object.getTag() == JSON_OBJECT);

    Geometry_t geometry = m_geom.at(idx_geom);

    for (JsonNode *obj_geometry = object.toNode(); obj_geometry != nullptr; obj_geometry = obj_geometry->next)
    {
      if (std::string(obj_geometry->key).compare("type") == 0)
      {
        assert(obj_geometry->value.getTag() == JSON_STRING);
        std::string str = obj_geometry->value.toString();
        std::cout << "\t\tgeometry type:\t" << str << "\n";
        assert(m_geom.at(idx_geom).type == str);
      }
      else if (std::string(obj_geometry->key).compare("coordinates") == 0)
      {
        assert(obj_geometry->value.getTag() == JSON_ARRAY);
      }
      else if (std::string(obj_geometry->key).compare("arcs") == 0)
      {
        assert(obj_geometry->value.getTag() == JSON_ARRAY);
        JsonValue arcs = obj_geometry->value;
        //get size of "arcs" array
        size_t size_arr_arcs = 0;
        for (JsonNode *arr_arcs = arcs.toNode(); arr_arcs != nullptr; arr_arcs = arr_arcs->next)
        {
          size_arr_arcs++;
        }
        for (JsonNode *arr_arcs = arcs.toNode(); arr_arcs != nullptr; arr_arcs = arr_arcs->next)
        {
          if (m_geom.at(idx_geom).type.compare("LineString") == 0)
          {
            assert(arr_arcs->value.getTag() == JSON_NUMBER);
          }
          //For type “Polygon”, the “arcs” member must be an array of LinearRing arc indexes. 
          //For Polygons with multiple rings, the first must be the exterior ring and 
          //any others must be interior rings or holes.
          else if (m_geom.at(idx_geom).type.compare("Polygon") == 0)
          {
            JsonValue arr_pol = arr_arcs->value;
            assert(arr_pol.getTag() == JSON_ARRAY);
            //indices into arc vector
            Polygon_topojson_t polygon;
            for (JsonNode *arr_values = arr_pol.toNode(); arr_values != nullptr; arr_values = arr_values->next)
            {
              assert(arr_values->value.getTag() == JSON_NUMBER);
              polygon.arcs.push_back((int)arr_values->value.toNumber());
            }//arr_values
            m_geom.at(idx_geom).m_polygon.push_back(polygon);
          }//"Polygon"
          //For type “MultiPolygon”, the “arcs” member must be an array of Polygon arc indexes.
          else if (geometry.type.compare("MultiPolygon") == 0)
          {
            JsonValue arr_multi = arr_arcs->value;
            assert(arr_multi.getTag() == JSON_ARRAY);
            for (JsonNode *arr_m_values = arr_multi.toNode(); arr_m_values != nullptr; arr_m_values = arr_m_values->next)
            {
              JsonValue arr_pol = arr_m_values->value;
              assert(arr_pol.getTag() == JSON_ARRAY);
              //indices into arc vector
              Polygon_topojson_t polygon;
              for (JsonNode *arr_values = arr_pol.toNode(); arr_values != nullptr; arr_values = arr_values->next)
              {
                assert(arr_values->value.getTag() == JSON_NUMBER);
                polygon.arcs.push_back((int)arr_values->value.toNumber());
              }//arr_values
              m_geom.at(idx_geom).m_polygon.push_back(polygon);
            }//arr_m_values
          }//"MultiPolygon"
        }//arr_obj
      }//"arcs"
    }//obj_geometry

    idx_geom++; //go to next geometry
    std::cout << idx_geom << "\t";

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

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::get_first
//get first arc coordinates(center, render)
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<double> topojson_t::get_first()
{
  std::vector<double> first;
  size_t size_geom = m_geom.size();
  for (idx_geom = 0; idx_geom < size_geom; idx_geom++)
  {
    Geometry_t geometry = m_geom.at(idx_geom);
    if (geometry.type.compare("Polygon") == 0)
    {
      size_t size_pol = geometry.m_polygon.size();
      for (size_t idx_pol = 0; idx_pol < size_pol; idx_pol++)
      {
        Polygon_topojson_t polygon = geometry.m_polygon.at(idx_pol);
        size_t size_arcs = polygon.arcs.size();
        for (size_t idx_arc = 0; idx_arc < size_arcs; idx_arc++)
        {
          int index = polygon.arcs.at(idx_arc);
          int idx = index < 0 ? ~index : index;
          arc_t arc = m_arcs.at(idx);
          first = arc.vec.at(0);
        }
      }
    }
  }
  return first;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topojson_t::make_coordinates
/////////////////////////////////////////////////////////////////////////////////////////////////////

void topojson_t::make_coordinates()
{
  size_t size_geom = m_geom.size();
  for (idx_geom = 0; idx_geom < size_geom; idx_geom++)
  {
    Geometry_t geometry = m_geom.at(idx_geom);
    if (geometry.type.compare("Polygon") == 0 || geometry.type.compare("MultiPolygon") == 0)
    {
      size_t size_pol = geometry.m_polygon.size();
      for (size_t idx_pol = 0; idx_pol < size_pol; idx_pol++)
      {
        Polygon_topojson_t polygon = geometry.m_polygon.at(idx_pol);
        size_t size_arcs = polygon.arcs.size();

        for (size_t idx_arc = 0; idx_arc < size_arcs; idx_arc++)
        {
          int index = polygon.arcs.at(idx_arc);
          int index_q = index < 0 ? ~index : index;
          arc_t arc = m_arcs.at(index_q);
          size_t size_vec_arcs = arc.vec.size();
          //if a topology is quantized, the positions of each arc in the topology which are quantized 
          //must be delta-encoded. The first position of the arc is a normal position [x1, y1]. 
          //The second position [x2, y2] is encoded as [dx2, dy2], where 
          //x2 = x1 + dx2 and 
          //y2 = y1 + dx2.
          //The third position [x3, y3] is encoded as [dx3, dy3], where 
          //x3 = x2 + dx3 = x1 + dx2 + dx3 and
          //y3 = y2 + dy3 = y1 + dy2 + dy3 and so on.
          double x = 0;
          double y = 0;
          //temporary positions
          std::vector<double> xp;
          std::vector<double> yp;
          for (size_t idx = 0; idx < size_vec_arcs; idx++)
          {
            double position[2];
            position[0] = arc.vec.at(idx).at(0);
            position[1] = arc.vec.at(idx).at(1);
            position[0] = (x += position[0]) * scale[0] + translate[0];
            position[1] = (y += position[1]) * scale[1] + translate[1];
            xp.push_back(position[0]);
            yp.push_back(position[1]);
          }//size_vec_arcs

          //@mbostock
          //TopoJSON allows you to reference a reversed arc in a geometry so that when geometries share an arc, 
          //but some geometries need the arc in the opposite direction, the geometries can reference the same arc.
          //This occurs very commonly when you have neighboring geometries.For example, California and Nevada 
          //share a border, but given that both would typically have the same winding order, 
          //the shared border must be reversed between the two polygons if you want to share the arc.
          //A reversed arc means that rather than the arc’s points going p_0, p_1, … p_n, 
          //the points go p_n, p_{ n - 1 }, … p_0.

           //reverse the subsequences of points represented by the negative arc indexes
          if (index < 0)
          {
            for (size_t idx = 0; idx < size_vec_arcs; idx++)
            {
              size_t jdx = size_vec_arcs - idx - 1;
              m_geom.at(idx_geom).m_polygon.at(idx_pol).m_x.push_back(xp[jdx]);
              m_geom.at(idx_geom).m_polygon.at(idx_pol).m_y.push_back(yp[jdx]);
            }
          }
          //do not reverse
          else
          {
            for (size_t idx = 0; idx < size_vec_arcs; idx++)
            {
              size_t jdx = idx;
              m_geom.at(idx_geom).m_polygon.at(idx_pol).m_x.push_back(xp[jdx]);
              m_geom.at(idx_geom).m_polygon.at(idx_pol).m_y.push_back(yp[jdx]);
            }
          }
          assert(xp.size() == size_vec_arcs);

        }//size_arcs
      }//size_pol
    }//"Polygon"
  }//size_geom
}
