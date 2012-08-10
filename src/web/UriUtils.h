// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef URI_UTILS_H_
#define URI_UTILS_H_

#include <string>

namespace Wt {
  namespace Uri {
    bool isDataUri(const std::string& uri);

    struct Uri {
      std::string mimeType;
      std::string data;
    };

    Uri parseDataUri(const std::string& uri);
  }
}

#endif // URI_UTILS_H_
