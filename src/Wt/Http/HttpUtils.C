/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "HttpUtils.h"

namespace Wt {
  namespace Http {
    namespace Utils {

void parseFormUrlEncoded(const Http::Message& response, 
			 Http::ParameterMap &params)
{
  Http::Request::parseFormUrlEncoded(response.body(), params);
}

const std::string *getParamValue(Http::ParameterMap &params, 
				 const std::string &name)
{
  return Http::get(params, name);
}

    }
  }
}
