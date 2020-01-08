/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 * 
 * This is a locator builder for Selenium suitable for web applications
 * in Wt or JWt.
 * 
 * It builds an XPath expression that takes into account only:
 *  - named widgets using setObjectName()
 *  - anchors based on the referenced internal paths
 *  - numbered descendant selectors to disambiguate multiple matches
 */

LocatorBuilders.add('xpath:wt',
function(e) {
  /*
  function MyConsole() {
    var _console = Components.classes["@mozilla.org/consoleservice;1"]
      .getService(Components.interfaces.nsIConsoleService);
   
    this.log = function(string_value) {
      _console.logStringMessage(string_value);
    };
  }

  var myConsole = new MyConsole();
  */

  function objectNameSelector(e) {
    var result = e.tagName.toLowerCase();
    result += '[@data-object-name="' + e.getAttribute('data-object-name') + '"]';
    return result;
  }

  function tagSelector(e) {
    return e.tagName.toLowerCase();
  }

  function hrefSelector(e) {
    var result = e.tagName.toLowerCase();
    result += '[@href="' + e.getAttribute('href') + '"]';
    return result;
  }

  function determineIndex(child, ancestor, expr) {
    var d = ancestor.ownerDocument;
    var nsr = null;

    var iterator =
      d.evaluate(".//" + expr, ancestor,
		 nsr, XPathResult.ORDERED_NODE_ITERATOR_TYPE, null);

    // myConsole.log(ancestor + ": " + expr);

    var n = iterator.iterateNext();
    var i = 1;

    while (n) {
      // myConsole.log(n.id + "=?" + child.id);
      if (n.id == child.id) 
	return "[" + i + "]";
      ++i;
      n = iterator.iterateNext();
    }

    return "";
  }

  function addChildXPath(xpath, child_xpath, child, ancestor) {
    var index = determineIndex(child, ancestor, child_xpath);

    if (xpath.length > 0)
      xpath = "//" + child_xpath + index + xpath;
    else
      xpath = "//" + child_xpath + ")" + index;

    return xpath;
  }

  var xpath = "", child_xpath = "";

  if (e.getAttribute('data-object-name'))
    child_xpath = objectNameSelector(e);
  else
    child_xpath = tagSelector(e);

  var child = e;

  while (e.parentNode.tagName && e.parentNode.tagName.toLowerCase() != 'body') {
    var p = e.parentNode;

    if (p.getAttribute('data-object-name')) {
      xpath = addChildXPath(xpath, child_xpath, child, p);

      child_xpath = objectNameSelector(p);
      child = p;
    } else if (p.tagName.toLowerCase() == 'a' && p.href.length > 0) {
      xpath = addChildXPath(xpath, child_xpath, child, p);

      child_xpath = hrefSelector(p);
      child = p;
    }
      
    e = p;
  }

  xpath = "xpath=(" + addChildXPath(xpath, child_xpath, child, e.parentNode);

  // myConsole.log(xpath);

  return xpath;
}
);
