// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef URI_UTILS_H_
#define URI_UTILS_H_

#include <string>
#include <vector>

namespace Wt {

  class DataUri
  {
  public:
    DataUri(const std::string& dataUri);

    std::string mimeType;
    std::vector<unsigned char> data;

    static bool isDataUri(const std::string& uri);

  private:
    void parse(const std::string& dataUri);
  };

}

#endif // URI_UTILS_H_
