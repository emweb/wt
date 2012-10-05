#ifndef READOBJ_H_
#define READOBJ_H_

#include <vector>
#include <string>
#include <boost/tuple/tuple.hpp>

// Function to read a very limited subset of obj files. Only reads
// files with triangles. Supports v, vn, t and f.
void readObj(const std::string &fname,
             std::vector<float> &data);

#endif

