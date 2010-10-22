#ifndef READOBJ_H_
#define READOBJ_H_

#include <vector>
#include <string>
#include <boost/tuple/tuple.hpp>

void readObj(const std::string &fname,
             std::vector<double> &data);
             //std::vector<double> &points, std::vector<double> &normals,
             //std::vector<int> &faces);
             //std::vector<std::vector<boost::tuple<int, int, int> > > &faces);

#endif

