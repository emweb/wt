/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WTemplate"
#include "Wt/WText"
#include "Wt/WDialog"
#include "Wt/WVBoxLayout"

#include "WebSession.h"
#include "WtException.h"
#include "WebController.h"

#include "JavaScriptLoader.h"

#ifndef WT_DEBUG_JS
#include "js/WDialog.min.js"
#endif

namespace Wt {

WDialog::WDialog(const WString& windowTitle)
  : modal_(true),
    finished_(this),
    recursiveEventLoop_(false)
{ 
  const char *TEMPLATE =
      "${shadow-x1-x2}"
      "${titlebar}"
      "${contents}";

  setImplementation(impl_ = new WTemplate(WString::fromUTF8(TEMPLATE)));

  const char *CSS_RULES_NAME = "Wt::WDialog";

  WApplication *app = WApplication::instance();
  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    if (app->environment().agentIsIElt(9))
      app->styleSheet().addRule("body", "height: 100%;");

    app->styleSheet().addRule("div.Wt-dialogcover", std::string() + 
			      // IE: requres body.height=100%
			      "height: 100%; width: 100%;"
			      "top: 0px; left: 0px;"
			      "opacity: 0.5; position: fixed;" +
			      (app->environment().agentIsIElt(9) ?
			       "filter: alpha(opacity=50);"
			       : "opacity: 0.5"), CSS_RULES_NAME);

    std::string position
      = app->environment().agent() == WEnvironment::IE6 ? "absolute" : "fixed";

    // we use left: 50%, top: 50%, margin hack when JavaScript is not available
    // see below for an IE workaround
    app->styleSheet().addRule("div.Wt-dialog", std::string() +
			      (app->environment().ajax()
			       && !app->environment().agentIsIElt(9) ?
			       "visibility: hidden;" : "") +
			      "position: " + position + ';'
			      + (!app->environment().ajax() ?
				 "left: 50%; top: 50%;"
				 "margin-left: -100px; margin-top: -50px;" :
				 "left: 0px; top: 0px;"));

    if (app->environment().agent() == WEnvironment::IE6) {
      app->styleSheet().addRule
	("div.Wt-dialogcover",
	 "position: absolute;"
	 "left: expression("
	 "(ignoreMe2 = document.documentElement.scrollLeft) + 'px' );"
	 "top: expression("
	 "(ignoreMe = document.documentElement.scrollTop) + 'px' );");

      // simulate position: fixed left: 50%; top 50%
      if (!app->environment().ajax())
	app->styleSheet().addRule
	  ("div.Wt-dialog",
	   "position: absolute;"
	   "left: expression("
	   "(ignoreMe2 = document.documentElement.scrollLeft + "
	   "document.documentElement.clientWidth/2) + 'px' );"
	   "top: expression("
	   "(ignoreMe = document.documentElement.scrollTop + "
	   "document.documentElement.clientHeight/2) + 'px' );");
    }
  }

  impl_->setStyleClass("Wt-dialog Wt-outset");

  WContainerWidget *parent = app->domRoot();

  setPopup(true);

  const char *THIS_JS = "js/WDialog.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WDialog", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

  setJavaScriptMember("_a", "0;new " WT_CLASS ".WDialog("
		    + app->javaScriptClass() + "," + jsRef() + ")");
  app->addAutoJavaScript
    ("{var obj = $('#" + id() + "').data('obj');"
     "if (obj) obj.centerDialog();}");

  parent->addWidget(this);

  titleBar_ = new WContainerWidget();
  titleBar_->setStyleClass("titlebar");
  caption_ = new WText(windowTitle, titleBar_);

  impl_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  impl_->bindWidget("titlebar", titleBar_);

  contents_ = new WContainerWidget();
  contents_->setStyleClass("body");

  impl_->bindWidget("contents", contents_);

  saveCoverState(app, app->dialogCover());

  setJavaScriptMember(WT_RESIZE_JS, "$('#" + id() + "').data('obj').wtResize");

  hide();
}

WDialog::~WDialog()
{
  hide();
}

void WDialog::rejectWhenEscapePressed()
{
  WApplication::instance()->globalEscapePressed()
    .connect(this, &WDialog::reject);

  impl_->escapePressed().connect(this, &WDialog::reject);  
}

#ifndef WT_DEPRECATED_3_0_0
void WDialog::setCaption(const WString& caption)
{
  setWindowTitle(caption);
}

const WString& WDialog::caption() const
{
  return windowTitle();
}
#endif // WT_DEPRECATED_3_0_0

void WDialog::setWindowTitle(const WString& windowTitle)
{
  caption_->setText(windowTitle);
}

const WString& WDialog::windowTitle() const
{
  return caption_->text();
}

void WDialog::setTitleBarEnabled(bool enable)
{
  titleBar_->setHidden(!enable);
}

WDialog::DialogCode WDialog::exec()
{
  if (recursiveEventLoop_)
    throw WtException("WDialog::exec(): already in recursive event loop.");

  show();

#ifdef WT_TARGET_JAVA
  if (!WebController::isAsyncSupported()) {
    throw std::runtime_error("Recursive event loop requires a Servlet 3.0 API.");
  }
#endif

  recursiveEventLoop_ = true;
  do {
    WApplication::instance()->session()->doRecursiveEventLoop();
  } while (recursiveEventLoop_);

  hide();

  return result_;
}

void WDialog::done(DialogCode result)
{
  result_ = result;

  if (recursiveEventLoop_) {
    recursiveEventLoop_ = false;
  } else
    hide();

  finished_.emit(result);
}

void WDialog::accept()
{
  done(Accepted);
}

void WDialog::reject()
{
  done(Rejected);
}

void WDialog::setModal(bool modal)
{
  modal_ = modal;
}

void WDialog::saveCoverState(WApplication *app, WContainerWidget *cover)
{
  coverWasHidden_ = cover->isHidden();
  coverPreviousZIndex_ = cover->zIndex();
  previousExposeConstraint_ = app->exposeConstraint();
}

void WDialog::restoreCoverState(WApplication *app, WContainerWidget *cover)
{
  cover->setHidden(coverWasHidden_);
  cover->setZIndex(coverPreviousZIndex_);
  app->constrainExposed(previousExposeConstraint_);
}

void WDialog::setHidden(bool hidden)
{
  if (isHidden() != hidden) {
    if (modal_) {
      WApplication *app = WApplication::instance();
      WContainerWidget *cover = app->dialogCover();

      if (!cover)
	return; // when application is being destroyed
      
      if (!hidden) {
	saveCoverState(app, cover);

	cover->show();
	cover->setZIndex(impl_->zIndex() - 1);
	app->constrainExposed(this);

	// FIXME: this should only blur if the active element is outside
	// of the dialog
	app->doJavaScript
	  ("try {"
	   """if (document.activeElement && document.activeElement.blur)"
	   ""  "document.activeElement.blur();"
	   "} catch (e) { }");
      } else
	restoreCoverState(app, cover);
    }
  }

  WCompositeWidget::setHidden(hidden);
}

}
