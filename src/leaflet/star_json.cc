/////////////////////////////////////////////////////////////////////////////////////////////////////
//Center for Satellite Applications and Research (STAR)
//NOAA Center for Weather and Climate Prediction (NCWCP)
//5830 University Research Court
//College Park, MD 20740
//Purpose: Parse a HDF5 STAR JSON format file
//see star_json.html for a description of the format
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "star_json.hh"

const bool data_newline = false;
const bool object_newline = false;
const int SHIFT_WIDTH = 4;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::read
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json::read(const char* file_name)
{
  hid_t fid = -1;
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
  int status = jsonParse(buf, &endptr, &value, allocator);
  if (status != JSON_OK)
  {
    std::cout << "invalid JSON format for " << file_name << std::endl;
    return -1;
  }

  //add .h5 extension to .json file
  std::string name(file_name);
  name += ".h5";

#ifdef HAVE_HDF5
  if ((fid = H5Fcreate(name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0)
  {

  }
#endif

  do_objects_group(value, "/", fid);

#ifdef HAVE_HDF5
  if (H5Fclose(fid) < 0)
  {

  }
#endif

  free(buf);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::do_group_objects
//choose between JSON objects that represent:
//groups, attributes, variables, dimensions
//any of these JSON objects are located at each group
/////////////////////////////////////////////////////////////////////////////////////////////////////

void star_json::do_objects_group(JsonValue value, const char* grp_name, hid_t loc_id, int indent)
{
  //parameter must be JSON object 
  assert(value.getTag() == JSON_OBJECT);

  if (!value.toNode())
  {
    //empty group
    fprintf(stdout, "{}");
    fprintf(stdout, "\n");
    return;
  }

  //start JSON object
  fprintf(stdout, "{\n");

  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    assert(node->value.getTag() == JSON_OBJECT);

    fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
    dump_string(node->key);
    fprintf(stdout, ": ");

    if (std::string(node->key).compare("groups") == 0)
    {
      do_groups(node->value, grp_name, loc_id, indent + SHIFT_WIDTH);
    }
    else if (std::string(node->key).compare("attributes") == 0)
    {
      do_attributes(node->value, grp_name, loc_id, indent + SHIFT_WIDTH);
    }
    else if (std::string(node->key).compare("variables") == 0)
    {
      do_variables(node->value, grp_name, loc_id, indent + SHIFT_WIDTH);
    }

    //JSON object separator
    object_separator(node, indent);
  }

  //end JSON object
  fprintf(stdout, "%*s}\n", indent, "");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::do_groups
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json::do_groups(JsonValue value, const char* grp_name, hid_t loc_id, int indent)
{
  //parameter must be JSON object 
  assert(value.getTag() == JSON_OBJECT);

  //start JSON object
  fprintf(stdout, "{\n");

  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    assert(node->value.getTag() == JSON_OBJECT);

    fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
    dump_string(node->key);
    fprintf(stdout, ": ");

    //iterate in subgroup with name 'node->key'
    do_objects_group(node->value, node->key, loc_id, indent + SHIFT_WIDTH);
  }

  //end JSON object
  fprintf(stdout, "%*s}\n", indent, "");

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::do_variables
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json::do_variables(JsonValue value, const char* grp_name, hid_t loc_id, int indent)
{
  //parameter must be JSON object 
  assert(value.getTag() == JSON_OBJECT);

  //start JSON object
  fprintf(stdout, "{\n");

  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    assert(node->value.getTag() == JSON_OBJECT);

    fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
    dump_string(node->key);
    fprintf(stdout, ": ");

    //get objects
    get_variable_data(node->value, node->key, loc_id, indent + SHIFT_WIDTH);

    //JSON object separator
    object_separator(node, indent);
  }

  //end JSON object
  fprintf(stdout, "%*s}", indent, "");
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::do_attributes
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json::do_attributes(JsonValue value, const char* grp_name, hid_t loc_id, int indent)
{
  //parameter must be JSON object 
  assert(value.getTag() == JSON_OBJECT);

  //start JSON object
  fprintf(stdout, "{\n");

  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    //key, attribute name
    fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
    dump_string(node->key);
    fprintf(stdout, ":");

    //attribute value 
    assert(node->value.getTag() == JSON_OBJECT);
    dump_value(node->value, indent + SHIFT_WIDTH);

    //JSON object separator
    object_separator(node, indent);
  }

  //end JSON object
  fprintf(stdout, "%*s}", indent, "");
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::get_variable_data
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json::get_variable_data(JsonValue value, const char* var_name, hid_t loc_id, int indent)
{
  //parameter must be JSON object 
  assert(value.getTag() == JSON_OBJECT);

  //size of array
  size_t arr_size;

  //start JSON object
  fprintf(stdout, "{\n");

  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    //key, object name
    fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
    dump_string(node->key);
    fprintf(stdout, ":");

    if (std::string(node->key).compare("shape") == 0)
    {
      //"shape" object must be a JSON array 
      assert(node->value.getTag() == JSON_ARRAY);
      JsonValue arr_dimensions = node->value;

      arr_size = 0;
      for (JsonNode *n = arr_dimensions.toNode(); n != nullptr; n = n->next)
      {
        arr_size++;
        assert(n->value.getTag() == JSON_NUMBER);
      }

      dump_value(node->value, indent + SHIFT_WIDTH);
    }
    else if (std::string(node->key).compare("type") == 0)
    {
      //"type" object must be a JSON string 
      assert(node->value.getTag() == JSON_STRING);
      dump_value(node->value, indent + SHIFT_WIDTH);
    }
    else if (std::string(node->key).compare("data") == 0)
    {
      //"data" object must be a JSON array
      assert(node->value.getTag() == JSON_ARRAY);
      dump_value(node->value, indent + SHIFT_WIDTH);
    }
    else if (std::string(node->key).compare("attributes") == 0)
    {
      do_attributes(node->value, node->key, loc_id, indent + SHIFT_WIDTH);
    }

    //JSON object separator
    object_separator(node, indent);
  }

  //end JSON object
  fprintf(stdout, "%*s}", indent, "");
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::dump_value
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json::dump_value(JsonValue o, int indent)
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
    if (data_newline) fprintf(stdout, "\n");
    for (auto i : o)
    {
      if (data_newline) fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
      dump_value(i->value, indent + SHIFT_WIDTH);
      if (data_newline)
        fprintf(stdout, i->next ? ",\n" : "\n");
      else
        fprintf(stdout, i->next ? "," : "");
    }
    if (data_newline)
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
//star_json::dump_string
/////////////////////////////////////////////////////////////////////////////////////////////////////

void star_json::dump_string(const char *s)
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

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json::object_separator
//print  JSON object separator(comma)
/////////////////////////////////////////////////////////////////////////////////////////////////////

void star_json::object_separator(JsonNode *node, int indent)
{
  if (node->next)
  {
    if (object_newline)
    {
      fprintf(stdout, "\n");
      fprintf(stdout, "%*s,\n", indent + SHIFT_WIDTH, "");
    }
    else
    {
      fprintf(stdout, ",\n");
    }
  }
  else
  {
    fprintf(stdout, "\n");
  }
}

