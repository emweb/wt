
#ifndef STAR_DATASET_HH
#define STAR_DATASET_HH

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
#include "gason.h"

#ifdef WT_BUILDING
#include "Wt/WDllDefs.h"
#else
#ifndef WT_API
#define WT_API
#endif
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_dataset_t
//storage for data STAR JSON data
/////////////////////////////////////////////////////////////////////////////////////////////////////

class WT_API star_dataset_t
{
public:
  star_dataset_t()
  {
  }
  std::string m_name; //name key
  std::vector<size_t> m_shape; //dimensions
  std::string m_type; //type
  std::vector<double> m_data; //data

  void do_min_max(double &min, double &max);
  double value_at(size_t i, size_t j, size_t k);
  double value_at(size_t i, size_t j);
};

int WT_API read_datasets(const char* file_name, std::vector<star_dataset_t> &datasets);
int WT_API get_variable(JsonValue value, star_dataset_t &dataset);


#endif