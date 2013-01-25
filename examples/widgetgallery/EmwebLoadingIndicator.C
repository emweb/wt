/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EmwebLoadingIndicator.h"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WText"

EmwebLoadingIndicator::EmwebLoadingIndicator()
{
  setInline(false);

  WApplication *app = WApplication::instance();

  cover_ = new WContainerWidget(this);
  center_ = new WContainerWidget(this);

  WImage *img = new WImage(WLink("icons/emweb.jpg"), center_);
  img->setMargin(7, Top | Bottom);

  text_ = new WText("Loading...", center_);
  text_->setInline(false);
  text_->setMargin(WLength::Auto, Left | Right);

  if (app->environment().agentIsIE())
    app->styleSheet().addRule("body", "height: 100%; margin: 0;");

    app->styleSheet().addRule("div#" + cover_->id(), std::string() +
			      "background: #DDDDDD;"
			      "height: 100%; width: 100%;"
			      "top: 0px; left: 0px;"
			      "opacity: 0.5; position: absolute;"
			      "-khtml-opacity: 0.5;"
			      "z-index: 10000;" +
			      (app->environment().agentIsIE() ?
			       "filter: alpha(opacity=50);"
			       :
			       "-moz-opacity:0.5;"
			       "-moz-background-clip: -moz-initial;"
			       "-moz-background-origin: -moz-initial;"
			       "-moz-background-inline-policy: -moz-initial;"
			       ));

    app->styleSheet().addRule("div#" + center_->id(),
			      "background: white;"
			      "border: 3px solid #333333;"
			      "z-index: 10001; visibility: visible;"
			      "position: absolute; left: 50%; top: 50%;"
			      "margin-left: -120px; margin-top: -60px;"
			      "width: 240px; height: 120px;"
			      "font-family: arial,sans-serif;"
			      "text-align: center");
}

void EmwebLoadingIndicator::setMessage(const WString& text)
{
  text_->setText(text);
}
