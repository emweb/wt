/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ColorUtils.h"

#include "Wt/WLogger"
#include "web/WebUtils.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <string>

namespace Wt {

  LOGGER("ColorUtils");

  namespace Color {

int parseRgbArgument(const std::string& argument) 
{
  std::string arg = boost::trim_copy(argument);
  try {
    if (boost::ends_with(arg, "%"))
      return (int) (boost::lexical_cast<double>(arg.substr(0, arg.size() - 1)) 
		    * 255 / 100);
    else 
      return boost::lexical_cast<int>(arg);
  } catch (boost::bad_lexical_cast &e) {
    LOG_ERROR("invalid color component: " << arg);
    return 0;
  }
}

int replicateHex(const std::string& s)
{
  int result = Utils::hexToInt(s.c_str());

  return result | (result << 4);
}

WColor parseCssColor(const std::string &name)
{
  std::string n = name;
  boost::trim(n);
      
  int red = 0;
  int green = 0;
  int blue = 0;
  int alpha = 255;

  if (boost::starts_with(n, "#")) {
    if (n.size() - 1 == 3) {                // #rgb
      red = replicateHex(n.substr(1, 1));
      green = replicateHex(n.substr(2,1));
      blue = replicateHex(n.substr(3,1));
    } else if (n.size() - 1 == 6) {         // #rrggbb 
      red = Utils::hexToInt(n.substr(1,2).c_str());
      green = Utils::hexToInt(n.substr(3,2).c_str());
      blue = Utils::hexToInt(n.substr(5,2).c_str());
    } else {
      LOG_ERROR("could not parse rgb format: " << n);
      red = green = blue = -1;
      return WColor(red, green, blue, alpha);
    }
  } else if (boost::starts_with(n, "rgb")) { // rgb(r,g,b) or rgba(r,g,b,a)
    if (n.size() < 5) {
      LOG_ERROR("could not parse rgb format: " << n);
      return WColor(red, green, blue, alpha);
    }
	
    bool has_alpha = (n[3] == 'a');
    int start_bracket = 3 + (has_alpha ? 1 : 0);
	
    if (n[start_bracket] != '(' || n[n.size() - 1] != ')') {
      LOG_ERROR("could not parse rgb format: " << n);
      return WColor(red, green, blue, alpha);
    }
	
    std::string argumentsStr = n.substr(start_bracket + 1, 
					n.size() - 1 - (start_bracket + 1));
	
    std::vector<std::string> arguments;
    boost::split(arguments, 
		 argumentsStr,
		 boost::is_any_of(","));
	
    if (!has_alpha && arguments.size() != 3) {
      LOG_ERROR("could not parse rgb format: " << n);
      return WColor(red, green, blue, alpha);
    }
	
    if (has_alpha && arguments.size() != 4) {
      LOG_ERROR("could not parse rgb format: " << n);
      return WColor(red, green, blue, alpha);
    }
	
    red = parseRgbArgument(arguments[0]);
    green = parseRgbArgument(arguments[1]);
    blue = parseRgbArgument(arguments[2]);
	
    if (has_alpha) {
      try {
	alpha = boost::lexical_cast<int>(boost::trim_copy(arguments[3]));
      } catch (boost::bad_lexical_cast &e) {
	LOG_ERROR("could not parse rgb format: " << n);
	alpha = 255;
	return WColor(red, green, blue, alpha);
      }
    }
  }

  return WColor(red, green, blue, alpha);
}
    
  }
}
