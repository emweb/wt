/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "XSSUtils.h"

#include <boost/algorithm/string.hpp>

namespace Wt {
  namespace XSS {

#ifdef WT_TARGET_JAVA	
    class XSSUtils {
    private:
      XSSUtils() { }
    };
#endif //WT_TARGET_JAVA

    bool isBadTag(const std::string& name)
    {
      return (boost::iequals(name, "script")
	      || boost::iequals(name, "applet")
	      || boost::iequals(name, "object")
	      || boost::iequals(name, "iframe")
	      || boost::iequals(name, "frame")
	      || boost::iequals(name, "layer")
	      || boost::iequals(name, "ilayer")
	      || boost::iequals(name, "frameset")
	      || boost::iequals(name, "link")
	      || boost::iequals(name, "meta")
	      || boost::iequals(name, "title")
	      || boost::iequals(name, "base")
	      || boost::iequals(name, "basefont")
	      || boost::iequals(name, "bgsound")
	      || boost::iequals(name, "head")
	      || boost::iequals(name, "body")
	      || boost::iequals(name, "embed")
	      || boost::iequals(name, "style")
              || boost::iequals(name, "comment")
	      || boost::iequals(name, "blink"));
    }
  
    bool isBadAttribute(const std::string& name)
    {
      return (boost::istarts_with(name, "on")
	      || boost::istarts_with(name, "data")
	      || boost::iequals(name, "dynsrc")
	      || boost::iequals(name, "id")
	      || boost::iequals(name, "autofocus")
              || boost::iequals(name, "name")
              // avoid repeat-based client DoS
              || boost::iequals(name, "repeat-start")
              || boost::iequals(name, "repeat-end")
              || boost::iequals(name, "repeat")
              // Some opera crashes on bad patterns
              || boost::iequals(name, "pattern")
	      );
    }

    bool isBadAttributeValue(const std::string& name, const std::string& value)
    {
      if (boost::iequals(name, "action")
	  || boost::iequals(name, "background")
	  || boost::iequals(name, "codebase")
	  || boost::iequals(name, "dynsrc")
	  || boost::iequals(name, "href")
	  || boost::iequals(name, "formaction")
	  || boost::iequals(name, "poster")
	  || boost::iequals(name, "src")) {
	std::string v = boost::trim_copy(value);

	return (boost::istarts_with(v, "javascript:")
		|| boost::istarts_with(v, "vbscript:")
		|| boost::istarts_with(v, "about:")
		|| boost::istarts_with(v, "chrome:")
		|| boost::istarts_with(v, "data:")
		|| boost::istarts_with(v, "disk:")
		|| boost::istarts_with(v, "hcp:")
		|| boost::istarts_with(v, "help:")
		|| boost::istarts_with(v, "livescript")
		|| boost::istarts_with(v, "lynxcgi:")
		|| boost::istarts_with(v, "lynxexec:")
		|| boost::istarts_with(v, "ms-help:")
		|| boost::istarts_with(v, "ms-its:")
		|| boost::istarts_with(v, "mhtml:")
		|| boost::istarts_with(v, "mocha:")
		|| boost::istarts_with(v, "opera:")
		|| boost::istarts_with(v, "res:")
		|| boost::istarts_with(v, "resource:")
		|| boost::istarts_with(v, "shell:")
		|| boost::istarts_with(v, "view-source:")
		|| boost::istarts_with(v, "vnd.ms.radio:")
		|| boost::istarts_with(v, "wysiwyg:"));
      } else
	if (boost::iequals(name, "style")) {
	  /*
	   * FIXME: implement CSS 2.1 backslash decoding before doing
	   * the following checks
	   * http://www.w3.org/TR/CSS21/syndata.html#characters
	   */
	  return boost::icontains(value, "absolute")
	    || boost::icontains(value, "behaviour")
	    || boost::icontains(value, "behavior")
	    || boost::icontains(value, "content")
	    || boost::icontains(value, "expression")
	    || boost::icontains(value, "fixed")
	    || boost::icontains(value, "include-source")
	    || boost::icontains(value, "moz-binding")
	    || boost::icontains(value, "javascript");
	} else
	  return false;
    } 
  }
}
