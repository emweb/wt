#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "geojson.hh"

const int SHIFT_WIDTH = 4;
const bool DATA_NEWLINE = false;
const bool OBJECT_NEWLINE = false;


/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t::convert
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_t::convert(const char* file_name)
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

  if (0) dump_value(value);
  parse_root(value);
  free(buf);
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t::parse_root
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_t::parse_root(JsonValue value)
{
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    //JSON organized in hierarchical levels
    //level 0, root with objects: "type", "features"
    //FeatureCollection is not much more than an object that has "type": "FeatureCollection" 
    //and then an array of Feature objects under the key "features". 

    if (std::string(node->key).compare("type") == 0)
    {
      assert(node->value.getTag() == JSON_STRING);
    }
    if (std::string(node->key).compare("features") == 0)
    {
      assert(node->value.getTag() == JSON_ARRAY);
      parse_features(node->value);
    }
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t::parse_features
//array of Feature objects under the key "features". 
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_t::parse_features(JsonValue value)
{
  assert(value.getTag() == JSON_ARRAY);

  size_t arr_size = 0; //size of array
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    arr_size++;
  }

  std::cout << "features: " << arr_size << std::endl;

  for (JsonNode *n_feat = value.toNode(); n_feat != nullptr; n_feat = n_feat->next)
  {
    JsonValue object = n_feat->value;
    assert(object.getTag() == JSON_OBJECT);

    feature_t feature;

    //3 objects with keys: 
    // "type", 
    // "properties", 
    // "geometry"
    //"type" has a string value "Feature"
    //"properties" has a list of objects
    //"geometry" has 2 objects: 
    //key "type" with value string geometry type (e.g."Polygon") and
    //key "coordinates" an array

    for (JsonNode *obj = object.toNode(); obj != nullptr; obj = obj->next)
    {
      if (std::string(obj->key).compare("type") == 0)
      {
        assert(obj->value.getTag() == JSON_STRING);
      }
      else if (std::string(obj->key).compare("properties") == 0)
      {
        assert(obj->value.getTag() == JSON_OBJECT);
        //parse properties
        for (JsonNode *prp = obj->value.toNode(); prp != nullptr; prp = prp->next)
        {
          //get name
          if (std::string(prp->key).compare("NAME") == 0 || std::string(prp->key).compare("name") == 0)
          {
            assert(prp->value.getTag() == JSON_STRING);
            feature.m_name = prp->value.toString();
            std::cout << "NAME: " << feature.m_name << std::endl;
          }
        }
      }
      else if (std::string(obj->key).compare("geometry") == 0)
      {
        assert(obj->value.getTag() == JSON_OBJECT);
        parse_geometry(obj->value, feature);
      }
    }

    m_feature.push_back(feature);
  } //n_feat


  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t::parse_geometry
//"geometry" has 2 objects: 
//key "type" with value string geometry type (e.g."Polygon") and
//key "coordinates" an array
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_t::parse_geometry(JsonValue value, feature_t &feature)
{
  assert(value.getTag() == JSON_OBJECT);
  std::string str_geometry_type; //"Polygon", "MultiPolygon", "Point"

  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    if (std::string(node->key).compare("type") == 0)
    {
      assert(node->value.getTag() == JSON_STRING);
      str_geometry_type = node->value.toString();
    }
    else if (std::string(node->key).compare("coordinates") == 0)
    {
      assert(node->value.getTag() == JSON_ARRAY);

      if (str_geometry_type.compare("Point") == 0)
      {
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        //store geometry locally for points
        /////////////////////////////////////////////////////////////////////////////////////////////////////

        geometry_t geometry;
        geometry.m_type = str_geometry_type;
        coord_t coord;
        polygon_t polygon;
        JsonValue arr_coord = node->value;
        coord.m_lon = arr_coord.toNode()->value.toNumber();;
        coord.m_lat = arr_coord.toNode()->next->value.toNumber();
        polygon.m_coord.push_back(coord);
        geometry.m_polygons.push_back(polygon);
       
        feature.m_geometry.push_back(geometry);
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////////
      //store geometry in parse_coordinates() for polygons
      /////////////////////////////////////////////////////////////////////////////////////////////////////

      if (str_geometry_type.compare("Polygon") == 0)
      {
        assert(node->value.getTag() == JSON_ARRAY);
        parse_coordinates(node->value, str_geometry_type, feature);
      }
      if (str_geometry_type.compare("MultiPolygon") == 0)
      {
        assert(node->value.getTag() == JSON_ARRAY);
        parse_coordinates(node->value, str_geometry_type, feature);
      }
    }

  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t::parse_coordinates
//"parse_coordinates" 
//for "Polygon"
//is an array of size 1 that contains another array and then an array of 2 numbers (lat, lon)
//for "MultiPolygon"
//is an array that contains another array of size 1, that contains another array,
//and then an array of 2 numbers (lat, lon)
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_t::parse_coordinates(JsonValue value,
  const std::string &type,
  feature_t &feature)
{
  assert(value.getTag() == JSON_ARRAY);

  geometry_t geometry;
  geometry.m_type = type;

  if (type.compare("Polygon") == 0)
  {
    for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
    {
      JsonValue arr = node->value;
      assert(arr.getTag() == JSON_ARRAY);

      polygon_t polygon;
      for (JsonNode *n = arr.toNode(); n != nullptr; n = n->next)
      {
        JsonValue crd = n->value;
        assert(crd.getTag() == JSON_ARRAY);
        coord_t coord;
        coord.m_lon = crd.toNode()->value.toNumber();;
        coord.m_lat = crd.toNode()->next->value.toNumber();
        polygon.m_coord.push_back(coord);
      }

      geometry.m_polygons.push_back(polygon);
    }
  }

  if (type.compare("MultiPolygon") == 0)
  {
    for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
    {
      JsonValue arr = node->value;
      assert(arr.getTag() == JSON_ARRAY);

      for (JsonNode *n = arr.toNode(); n != nullptr; n = n->next) //array of size 1
      {
        JsonValue arr_crd = n->value;
        assert(arr_crd.getTag() == JSON_ARRAY);

        polygon_t polygon;
        for (JsonNode *m = arr_crd.toNode(); m != nullptr; m = m->next)
        {
          JsonValue crd = m->value;
          assert(crd.getTag() == JSON_ARRAY);
          coord_t coord;
          coord.m_lon = crd.toNode()->value.toNumber();;
          coord.m_lat = crd.toNode()->next->value.toNumber();
          polygon.m_coord.push_back(coord);
        }

        geometry.m_polygons.push_back(polygon);
      }
    }
  }

  //store
  feature.m_geometry.push_back(geometry);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t::dump_value
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_t::dump_value(JsonValue o, int indent)
{
  switch (o.getTag())
  {
  case JSON_NUMBER:
    fprintf(stdout, "%f", o.toNumber());
    break;
  case JSON_STRING:
    dump_string(o.toString());
    break;
  case JSON_ARRAY:
    if (!o.toNode())
    {
      fprintf(stdout, "[]");
      break;
    }
    fprintf(stdout, "[");
    if (DATA_NEWLINE) fprintf(stdout, "\n");
    for (auto i : o)
    {
      if (DATA_NEWLINE) fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
      dump_value(i->value, indent + SHIFT_WIDTH);
      if (DATA_NEWLINE)
        fprintf(stdout, i->next ? ",\n" : "\n");
      else
        fprintf(stdout, i->next ? "," : "");
    }
    if (DATA_NEWLINE)
      fprintf(stdout, "%*s]", indent, "");
    else
      fprintf(stdout, "]");
    break;
  case JSON_OBJECT:
    if (!o.toNode())
    {
      fprintf(stdout, "{}");
      break;
    }
    fprintf(stdout, "{\n");
    for (auto i : o)
    {
      fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
      dump_string(i->key);
      fprintf(stdout, ": ");
      dump_value(i->value, indent + SHIFT_WIDTH);
      fprintf(stdout, i->next ? ",\n" : "\n");
    }
    fprintf(stdout, "%*s}", indent, "");
    break;
  case JSON_TRUE:
    fprintf(stdout, "true");
    break;
  case JSON_FALSE:
    fprintf(stdout, "false");
    break;
  case JSON_NULL:
    fprintf(stdout, "null");
    break;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t::dump_string
/////////////////////////////////////////////////////////////////////////////////////////////////////

void geojson_t::dump_string(const char *s)
{
  fputc('"', stdout);
  while (*s)
  {
    int c = *s++;
    switch (c)
    {
    case '\b':
      fprintf(stdout, "\\b");
      break;
    case '\f':
      fprintf(stdout, "\\f");
      break;
    case '\n':
      fprintf(stdout, "\\n");
      break;
    case '\r':
      fprintf(stdout, "\\r");
      break;
    case '\t':
      fprintf(stdout, "\\t");
      break;
    case '\\':
      fprintf(stdout, "\\\\");
      break;
    case '"':
      fprintf(stdout, "\\\"");
      break;
    default:
      fputc(c, stdout);
    }
  }
  fprintf(stdout, "%s\"", s);
}



