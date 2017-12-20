#ifndef STAR_JSON_HH
#define STAR_JSON_HH

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Center for Satellite Applications and Research (STAR)
//NOAA Center for Weather and Climate Prediction (NCWCP)
//5830 University Research Court
//College Park, MD 20740
//Purpose: Parse a HDF5 STAR JSON format file
//see star_json.html for a description of the format
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gason.h"
#ifdef HAVE_HDF5
#include <hdf5.h>
#else
typedef int hid_t;
#endif

#ifdef WT_BUILDING
#include "Wt/WDllDefs.h"
#else
#ifndef WT_API
#define WT_API
#endif
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json
/////////////////////////////////////////////////////////////////////////////////////////////////////

class WT_API star_json
{
public:
  star_json()
  {
  }
  int read(const char* file_name);

private:
  void do_objects_group(JsonValue value, const char* grp_name, hid_t loc_id, int indent = 0);
  int do_groups(JsonValue value, const char* grp_name, hid_t loc_id, int indent = 0);
  int do_variables(JsonValue value, const char* grp_name, hid_t loc_id, int indent = 0);
  int do_attributes(JsonValue value, const char* grp_name, hid_t loc_id, int indent = 0);
  int get_variable_data(JsonValue value, const char* var_name, hid_t loc_id, int indent = 0);
  int dump_value(JsonValue value, int indent = 0);
  void dump_string(const char *s);
  void object_separator(JsonNode *node, int indent);
};

#endif



