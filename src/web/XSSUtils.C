/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "XSSUtils.h"

#include <boost/algorithm/string.hpp>

namespace Wt {
  namespace XSS {

#ifdef WT_TARGET_JAVA	
    class XSSUtils {
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
	  || boost::iequals(name, "src"))
	return (boost::istarts_with(value, "javascript:")
		|| boost::istarts_with(value, "vbscript:")
		|| boost::istarts_with(value, "about:")
		|| boost::istarts_with(value, "chrome:")
		|| boost::istarts_with(value, "data:")
		|| boost::istarts_with(value, "disk:")
		|| boost::istarts_with(value, "hcp:")
		|| boost::istarts_with(value, "help:")
		|| boost::istarts_with(value, "livescript")
		|| boost::istarts_with(value, "lynxcgi:")
		|| boost::istarts_with(value, "lynxexec:")
		|| boost::istarts_with(value, "ms-help:")
		|| boost::istarts_with(value, "ms-its:")
		|| boost::istarts_with(value, "mhtml:")
		|| boost::istarts_with(value, "mocha:")
		|| boost::istarts_with(value, "opera:")
		|| boost::istarts_with(value, "res:")
		|| boost::istarts_with(value, "resource:")
		|| boost::istarts_with(value, "shell:")
		|| boost::istarts_with(value, "view-source:")
		|| boost::istarts_with(value, "vnd.ms.radio:")
		|| boost::istarts_with(value, "wysiwyg:"));
      else
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
