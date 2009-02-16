/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WDefaultLoadingIndicator"

namespace Wt {

WDefaultLoadingIndicator::WDefaultLoadingIndicator()
  : WText("Loading...")
{
  setInline(false);

  WApplication *app = WApplication::instance();

  app->styleSheet().addRule("div#" + id(),
			    "background-color: red; color: white;"
			    "font-family: Arial,Helvetica,sans-serif;"
			    "font-size: small;"
			    "position: absolute; right: 0px; top: 0px;");
  app->styleSheet().addRule("body > div#" + id(),
			    "position: fixed;");

  if (app->environment().userAgent().find("MSIE 5.5") != std::string::npos
      || app->environment().userAgent().find("MSIE 6") != std::string::npos)
    app->styleSheet().addRule
      ("div#" + id(),
       "right: expression((("
       "ignoreMe2 = document.documentElement.scrollLeft ? "
       "document.documentElement.scrollLeft : "
       "document.body.scrollLeft )) + 'px' );"
       "top: expression((("
       "ignoreMe = document.documentElement.scrollTop ? "
       "document.documentElement.scrollTop : "
       "document.body.scrollTop)) + 'px' );");
}

void WDefaultLoadingIndicator::setMessage(const WString& text)
{
  setText(text);
}
  
}
