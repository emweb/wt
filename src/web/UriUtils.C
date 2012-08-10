/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "UriUtils.h"
#include "Wt/Utils"

#include "Wt/WException"

#include <boost/algorithm/string.hpp>

namespace Wt {
  namespace Uri {

#ifdef WT_TARGET_JAVA	
    class UriUtils {
    };
#endif //WT_TARGET_JAVA

    bool isDataUri(const std::string& uriString)
    {
      return boost::starts_with(uriString, "data:");
    }
    
    Uri parseDataUri(const std::string& uriString)
    {
      Uri uri;

      size_t dataEndPos = uriString.find("data:") + 5;
      size_t commaPos = uriString.find(",");
      if (commaPos == std::string::npos)
	commaPos = dataEndPos;
      
      uri.mimeType = uriString.substr(dataEndPos, commaPos - dataEndPos);
      uri.data = uriString.substr(commaPos + 1);

      uri.data = Utils::base64Decode(uri.data);
      
      if (!boost::ends_with(uri.mimeType, ";base64") || uri.data.size() == 0)
	throw WException("Ill formed data URI: " + uriString);
      else {
	uri.mimeType = uri.mimeType.substr(0, uri.mimeType.find(";"));
	return uri;
      }
    }
  }
}
