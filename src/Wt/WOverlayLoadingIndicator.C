/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WImage.h"
#include "Wt/WText.h"
#include "Wt/WOverlayLoadingIndicator.h"

namespace Wt {

WOverlayLoadingIndicator
::WOverlayLoadingIndicator(const WT_USTRING& styleClass,
			   const WT_USTRING& backgroundStyleClass,
			   const WT_USTRING& textStyleClass)
{
  WContainerWidget *impl;

  setImplementation(std::unique_ptr<WWidget>(impl = new WContainerWidget()));
  setInline(false);

  WApplication *app = WApplication::instance();

  impl->addWidget(std::unique_ptr<WWidget>(cover_ = new WContainerWidget()));
  impl->addWidget(std::unique_ptr<WWidget>(center_ = new WContainerWidget()));

  WImage *img;
  center_->addWidget
    (std::unique_ptr<WWidget>
     (img = new WImage(WApplication::relativeResourcesUrl()
		       + "ajax-loading.gif")));
  img->setMargin(7, Side::Top | Side::Bottom);

  center_->addWidget
    (std::unique_ptr<WWidget>
     (text_ = new WText(tr("Wt.WOverlayLoadingIndicator.Loading"))));
  text_->setInline(false);
  text_->setMargin(WLength::Auto, Side::Left | Side::Right);

  if (!styleClass.empty())
    center_->setStyleClass(styleClass);
  if (!textStyleClass.empty())
    text_->setStyleClass(textStyleClass);
  if (!backgroundStyleClass.empty())
    cover_->setStyleClass(backgroundStyleClass);

  if (app->environment().agent() == UserAgent::IE6)
    app->styleSheet().addRule("body", "height: 100%; margin: 0;");

  std::string position
    = app->environment().agent() == UserAgent::IE6 
    ? "absolute" : "fixed";

  if (backgroundStyleClass.empty())
    app->styleSheet().addRule("div#" + cover_->id(), std::string() +
			      "background: #DDDDDD;"
			      "height: 100%; width: 100%;"
			      "top: 0px; left: 0px;"
			      "position: " + position + ";"
			      "z-index: 10000;" +
			      (app->environment().agentIsIE() ?
			       "filter: alpha(opacity=50);"
			       :
			       "opacity: 0.5;"
			       ));

  if (styleClass.empty())
    app->styleSheet().addRule("div#" + center_->id(),
			      "background: white;"
			      "border: 3px solid #333333;"
			      "z-index: 10001; visibility: visible;"
			      "position: " + position + "; left: 50%; top: 50%;"
			      "margin-left: -50px; margin-top: -40px;"
			      "width: 100px; height: 80px;"
			      "font-family: arial,sans-serif;"
			      "text-align: center");
}

void WOverlayLoadingIndicator::setMessage(const WString& text)
{
  text_->setText(text);
}
  
}
