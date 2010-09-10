/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WText"
#include "Wt/WOverlayLoadingIndicator"

namespace Wt {

WOverlayLoadingIndicator::WOverlayLoadingIndicator(const WT_USTRING& styleClass, const WT_USTRING& backgroundStyleClass, const WT_USTRING& textStyleClass)
{
  setInline(false);

  WApplication *app = WApplication::instance();

  cover_ = new WContainerWidget(this);
  center_ = new WContainerWidget(this);

  WImage *img = new WImage(WApplication::resourcesUrl()
			   + "ajax-loading.gif", center_);
  img->setMargin(7, Top | Bottom);

  text_ = new WText(tr("Wt.WOverlayLoadingIndicator.Loading"), center_);
  text_->setInline(false);
  text_->setMargin(WLength::Auto, Left | Right);

  if (!styleClass.empty())
    center_->setStyleClass(styleClass);
  if (!textStyleClass.empty())
    text_->setStyleClass(textStyleClass);
  if (!backgroundStyleClass.empty())
    cover_->setStyleClass(backgroundStyleClass);

  if (app->environment().agentIsIE())
    app->styleSheet().addRule("body", "height: 100%; margin: 0;");

  if (backgroundStyleClass.empty())
    app->styleSheet().addRule("div#" + cover_->id(), std::string() +
			      "background: #DDDDDD;"
			      "height: 100%; width: 100%;"
			      "top: 0px; left: 0px;"
			      "position: absolute;"
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
			      "position: absolute; left: 50%; top: 50%;"
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
