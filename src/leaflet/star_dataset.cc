#include "star_dataset.hh"
#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_dataset_t::value_at
/////////////////////////////////////////////////////////////////////////////////////////////////////

double star_dataset_t::value_at(size_t i, size_t j, size_t k)
{
  return m_data[m_shape[1] * m_shape[2] * i + m_shape[2] * j + k];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_dataset_t::value_at
/////////////////////////////////////////////////////////////////////////////////////////////////////

double star_dataset_t::value_at(size_t i, size_t j)
{
  return m_data[i * m_shape[1] + j];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_dataset_t::do_min_max
/////////////////////////////////////////////////////////////////////////////////////////////////////

void star_dataset_t::do_min_max(double &min, double &max)
{
  double v;
  max = -1E10;
  min = 1E10;
  size_t size_data = 1;
  for (size_t idx = 0; idx < m_shape.size(); idx++)
  {
    size_data *= m_shape.at(idx);
  }
  for (size_t idx = 0; idx < size_data; idx++)
  {
    v = m_data[idx];
    if (v > max) max = v;
    if (v < min) min = v;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//read_datasets
/////////////////////////////////////////////////////////////////////////////////////////////////////

int read_datasets(const char* file_name, std::vector<star_dataset_t> &vec)
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
  JsonValue root;
  JsonAllocator allocator;
  int status = jsonParse(buf, &endptr, &root, allocator);
  if (status != JSON_OK)
  {
    std::cout << "invalid JSON format for " << file_name << std::endl;
    return -1;
  }

  for (JsonNode *n_root = root.toNode(); n_root != nullptr; n_root = n_root->next)
  {
    assert(n_root->value.getTag() == JSON_OBJECT);

    if (std::string(n_root->key).compare("variables") == 0)
    {
      JsonValue variables = n_root->value;

      for (JsonNode *n_var = variables.toNode(); n_var != nullptr; n_var = n_var->next)
      {
        assert(n_var->value.getTag() == JSON_OBJECT);
        std::cout << n_var->key << std::endl; //dataset name

        JsonValue var_obj = n_var->value; //"shape", "data", or "type"

        star_dataset_t dataset;
        dataset.m_name = n_var->key;
        get_variable(var_obj, dataset);
        vec.push_back(dataset);
      } //n_var
    }//"variables"
  }//n_root

  free(buf);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//get_variable
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_variable(JsonValue var_obj, star_dataset_t &dataset)
{
  size_t arr_size;

  for (JsonNode *n_obj = var_obj.toNode(); n_obj != nullptr; n_obj = n_obj->next)
  {
    std::cout << n_obj->key << std::endl;

    if (std::string(n_obj->key).compare("type") == 0)
    {
      assert(n_obj->value.getTag() == JSON_STRING);
      dataset.m_type = n_obj->value.toString();
    }
    else if (std::string(n_obj->key).compare("shape") == 0)
    {
      assert(n_obj->value.getTag() == JSON_ARRAY);
      for (JsonNode *n_shp = n_obj->value.toNode(); n_shp != nullptr; n_shp = n_shp->next)
      {
        assert(n_shp->value.getTag() == JSON_NUMBER);
        dataset.m_shape.push_back((size_t)n_shp->value.toNumber());
      }
    }
    else if (std::string(n_obj->key).compare("data") == 0)
    {
      assert(n_obj->value.getTag() == JSON_ARRAY);

      arr_size = 0;

      /////////////////////////////////////////////////////////////////////////////////////////////////////
      //geez louise, gason is weird for parsing nested arrays
      /////////////////////////////////////////////////////////////////////////////////////////////////////

      if (dataset.m_shape.size() == 1)
      {
        for (JsonNode *n1 = n_obj->value.toNode(); n1 != nullptr; n1 = n1->next)
        {
          assert(n1->value.getTag() == JSON_NUMBER);
          dataset.m_data.push_back(n1->value.toNumber());
          arr_size++;
        }
      }
      else if (dataset.m_shape.size() == 2)
      {
        for (JsonNode *n1 = n_obj->value.toNode(); n1 != nullptr; n1 = n1->next)
        {
          assert(n1->value.getTag() == JSON_ARRAY);
          for (JsonNode *n2 = n1->value.toNode(); n2 != nullptr; n2 = n2->next)
          {
            assert(n2->value.getTag() == JSON_NUMBER);
            dataset.m_data.push_back(n2->value.toNumber());
            arr_size++;
          }
        }
      }
      else if (dataset.m_shape.size() == 3)
      {
        for (JsonNode *n1 = n_obj->value.toNode(); n1 != nullptr; n1 = n1->next)
        {
          assert(n1->value.getTag() == JSON_ARRAY);
          for (JsonNode *n2 = n1->value.toNode(); n2 != nullptr; n2 = n2->next)
          {
            assert(n2->value.getTag() == JSON_ARRAY);
            for (JsonNode *n3 = n2->value.toNode(); n3 != nullptr; n3 = n3->next)
            {
              assert(n3->value.getTag() == JSON_NUMBER);
              dataset.m_data.push_back(n3->value.toNumber());
              arr_size++;
            }
          }
        }
      }//shape 3
    } //"data"
  } //n_obj

  return 0;
}
