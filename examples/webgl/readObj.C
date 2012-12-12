#include "readObj.h"
#include <fstream>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <stdlib.h>
#if 0
#include <boost/bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

void readObj(const std::string &fname,
             std::vector<double> &data)
{
  using qi::double_;
  using qi::int_;
  using qi::lit;
  using qi::char_;
  using qi::phrase_parse;
  using qi::_1;
  using ascii::space;
  using phoenix::push_back;
  using phoenix::ref;

  std::vector<double> points;
  std::vector<double> normals;
  std::vector<double> tex;
  std::vector<int> faces;
  std::ifstream f(fname.c_str());

  while (f) {
    std::string line;
    getline(f, line);

    bool r = phrase_parse(
      line.begin(),
      line.end(),
      (char_('v') >> double_[push_back(ref(points), _1)] >> double_[push_back(ref(points), _1)] >> double_[push_back(ref(points), _1)] |
       lit("vn") >> double_[push_back(ref(normals), _1)] >> double_[push_back(ref(normals), _1)] >> double_[push_back(ref(normals), _1)] |
       lit("vt") >> double_[push_back(ref(tex), _1)] >> double_[push_back(ref(tex), _1)] |
       char_('f') >> *(int_[push_back(ref(faces), _1)] >> char_('/') >> -(int_) >> char_('/') >> int_[push_back(ref(faces), _1)])),
      space
      );
  }

  for (unsigned i = 0; i < faces.size(); ++i) {
    data.push_back(points[(faces[i]-1)*3]);
    data.push_back(points[(faces[i]-1)*3 + 1]);
    data.push_back(points[(faces[i]-1)*3 + 2]);
    ++i;
    data.push_back(normals[(faces[i]-1)*3]);
    data.push_back(normals[(faces[i]-1)*3 + 1]);
    data.push_back(normals[(faces[i]-1)*3 + 2]);
  }
}
#else
float str2float(const std::string &s)
{
  return atof(s.c_str());
}

void readObj(const std::string &fname,
             std::vector<float> &data)
{
  std::vector<float> points;
  std::vector<float> normals;
  std::vector<float> textures;
  std::ifstream f(fname.c_str());
  
  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      std::vector<std::string> splitLine;
      boost::split(splitLine, line, boost::is_any_of(" "), boost::algorithm::token_compress_on);
      if (splitLine[0] == "v") {
        points.push_back(str2float(splitLine[1]));
        points.push_back(str2float(splitLine[2]));
        points.push_back(str2float(splitLine[3]));
      } else if (splitLine[0] == "vn") {
        normals.push_back(str2float(splitLine[1]));
        normals.push_back(str2float(splitLine[2]));
        normals.push_back(str2float(splitLine[3]));
      } else if (splitLine[0] == "vt") {
        // texture coordinates are not used at all
        textures.push_back(boost::lexical_cast<float>(splitLine[1]));
        textures.push_back(boost::lexical_cast<float>(splitLine[2]));
        if (splitLine.size()>3) textures.push_back(boost::lexical_cast<float>(splitLine[3]));
      } else if (splitLine[0] == "f") {
        //std::vector<boost::tuple<int, int, int> > face;
        //std::vector<int> face;
        for (unsigned i = 1; i < splitLine.size(); ++i) {
          std::vector<std::string> faceLine;
          boost::split(faceLine, splitLine[i], boost::is_any_of("/"), boost::algorithm::token_compress_off);
          int v, t, n;
          v = boost::lexical_cast<int>(faceLine[0]);
          if (faceLine[1] != "") {
            t = boost::lexical_cast<int>(faceLine[1]);
          } else {
            t = -1;
          }
          if (faceLine[2] != "") {
            n = boost::lexical_cast<int>(faceLine[2]);
          } else {
            n = -1;
          }
          //face.push_back(boost::make_tuple<int, int, int>(v, t, n));
          //faces.push_back(v - 1);
          data.push_back(points[(v-1)*3]);
          data.push_back(points[(v-1)*3 + 1]);
          data.push_back(points[(v-1)*3 + 2]);
          data.push_back(normals[(n-1)*3]);
          data.push_back(normals[(n-1)*3 + 1]);
          data.push_back(normals[(n-1)*3 +2]);
        }
        //faces.push_back(face);
      } else if (splitLine[0] == "#") {
        // ignore comments
      }else {
          std::cerr << "ERROR in obj file: unknown line: " << line << "\n";
          //  go on with fingers crossed
          //return;
      }
    }
  }
}
#endif
