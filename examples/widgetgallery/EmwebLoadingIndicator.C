/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EmwebLoadingIndicator.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WImage.h"
#include "Wt/WText.h"

EmwebLoadingIndicator::EmwebLoadingIndicator()
{
  setInline(false);

  WApplication *app = WApplication::instance();

  cover_ = this->addWidget(std::make_unique<WContainerWidget>());
  center_ = this->addWidget(std::make_unique<WContainerWidget>());

  WImage *img =
      center_->addWidget(std::make_unique<WImage>(WLink("icons/emweb.jpg")));
  img->setMargin(7, Side::Top | Side::Bottom);

  text_ = center_->addWidget(std::make_unique<WText>("Loading..."));
  text_->setInline(false);
  text_->setMargin(WLength::Auto, Side::Left | Side::Right);

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
