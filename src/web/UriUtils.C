/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "UriUtils.h"
#include "Wt/Utils.h"

#include "Wt/WException.h"

#include <boost/algorithm/string.hpp>

namespace Wt {

  DataUri::DataUri(const std::string& uriString)
  {
    parse(uriString);
  }

  bool DataUri::isDataUri(const std::string& uriString)
  {
    return boost::starts_with(uriString, "data:");
  }

  void DataUri::parse(const std::string& uriString)
  {
    std::size_t dataEndPos = uriString.find("data:") + 5;
    std::size_t commaPos = uriString.find(",");
    if (commaPos == std::string::npos)
      commaPos = dataEndPos;
      
    mimeType = uriString.substr(dataEndPos, commaPos - dataEndPos);

    std::string d = uriString.substr(commaPos + 1);

#ifndef WT_TARGET_JAVA
    d = Utils::base64Decode(d);
    data = std::vector<unsigned char>(d.begin(), d.end());
#else
    data = Utils::base64Decode(d);
#endif // WT_TARGET_JAVA

    if (!boost::ends_with(mimeType, ";base64") || data.empty())
      throw WException("Ill formed data URI: " + uriString);
    else
      mimeType = mimeType.substr(0, mimeType.find(";"));
  }
}
