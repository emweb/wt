/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WDefaultLoadingIndicator.h"

namespace Wt {

WDefaultLoadingIndicator::WDefaultLoadingIndicator()
{
  setImplementation(std::unique_ptr<WWidget>
		    (new WText(tr("Wt.WDefaultLoadingIndicator.Loading"))));
  setInline(false);
  setStyleClass("Wt-loading");

  WApplication *app = WApplication::instance();

  app->styleSheet().addRule("div.Wt-loading",
			    "background-color: red; color: white;"
			    "font-family: Arial,Helvetica,sans-serif;"
			    "font-size: small;"
			    "position: absolute; right: 0px; top: 0px;");
  app->styleSheet().addRule("body div > div.Wt-loading",
			    "position: fixed;");

  if (app->environment().userAgent().find("MSIE 5.5") != std::string::npos
      || app->environment().userAgent().find("MSIE 6") != std::string::npos)
    app->styleSheet().addRule
      ("div.Wt-loading",
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
  dynamic_cast<WText *>(implementation())->setText(text);
}

}
