/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WText"
#include "Wt/WDialog"
#include "Wt/WVBoxLayout"

#include "WebSession.h"
#include "WtException.h"

namespace Wt {

WDialog::WDialog(const WString& windowTitle)
  : modal_(true),
    finished_(this),
    recursiveEventLoop_(false)
{ 
  setImplementation(impl_ = new WContainerWidget);
  impl_->setStyleClass("Wt-dialog");

  const char *CSS_RULES_NAME = "Wt::WDialog";

  WApplication *app = WApplication::instance();
  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    if (app->environment().agentIsIE())
      app->styleSheet().addRule("body", "height: 100%;");


    app->doJavaScript(std::string() +
      WT_CLASS ".centerDialog = function(d){"
      "" "if (d && d.style.display != 'none' && !d.getAttribute('moved')) {"
      ""   "var ws=" WT_CLASS ".windowSize();"
      ""   "d.style.left=Math.round((ws.x - d.clientWidth)/2"
     + (app->environment().agent() == WEnvironment::IE6
	? "+ document.documentElement.scrollLeft" : "") + ") + 'px';"
      ""   "d.style.top=Math.round((ws.y - d.clientHeight)/2"
     + (app->environment().agent() == WEnvironment::IE6
	? "+ document.documentElement.scrollTop" : "") + ") + 'px';"
      ""   "d.style.marginLeft='0px';"
      ""   "d.style.marginTop='0px';"
      "" "}"
      "};", false);

    app->styleSheet().addRule("div.Wt-dialogcover", std::string() + 
			      "background: white;"
			      // IE: requres body.height=100%
			      "height: 100%; width: 100%;"
			      "top: 0px; left: 0px;"
			      "opacity: 0.5; position: fixed;" +
			      (app->environment().agentIsIE() ?
			       "filter: alpha(opacity=50);"
			       :
			       "-moz-background-clip: -moz-initial;"
			       "-moz-background-origin: -moz-initial;"
			       "-moz-background-inline-policy: -moz-initial;"
			       "-moz-opacity:0.5;"
			       "-khtml-opacity: 0.5"), CSS_RULES_NAME);

    std::string position
      = app->environment().agent() == WEnvironment::IE6 ? "absolute" : "fixed";

    // we use left: 50%, top: 50%, margin hack when JavaScript is not available
    // see below for an IE workaround
    app->styleSheet().addRule("div.Wt-dialog", std::string() +
			      "visibility: visible;"
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

  WContainerWidget *parent = app->domRoot();

  setPopup(true);

  app->addAutoJavaScript(WT_CLASS ".centerDialog(" + jsRef() + ");");

  parent->addWidget(this);

  WVBoxLayout *layout = new WVBoxLayout();
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  titleBar_ = new WContainerWidget();
  titleBar_->setStyleClass("titlebar");
  caption_ = new WText(windowTitle, titleBar_);
  layout->addWidget(titleBar_);

  contents_ = new WContainerWidget();
  contents_->setStyleClass("body");

  layout->addWidget(contents_, 1);

  impl_->setLayout(layout, AlignLeft);

  if (app->environment().agentIsIE())
    impl_->setOverflow(WContainerWidget::OverflowVisible);

  mouseDownJS_.setJavaScript
    ("function(obj, event) {"
     "  var pc = " WT_CLASS ".pageCoordinates(event);"
     "  obj.setAttribute('dsx', pc.x);"
     "  obj.setAttribute('dsy', pc.y);"
     "}");

  mouseMovedJS_.setJavaScript
    ("function(obj, event) {"
     """var WT= " WT_CLASS ";"
     """var lastx = obj.getAttribute('dsx');"
     """var lasty = obj.getAttribute('dsy');"
     """if (lastx != null && lastx != '') {"
     ""  "nowxy = WT.pageCoordinates(event);"
     ""  "var d = " + jsRef() + ";"
     ""  "d.setAttribute('moved', true);"
     ""  "d.style.left = (WT.pxself(d, 'left')+nowxy.x-lastx) + 'px';"
     ""  "d.style.top = (WT.pxself(d, 'top')+nowxy.y-lasty) + 'px';"
     ""  "obj.setAttribute('dsx', nowxy.x);"
     ""  "obj.setAttribute('dsy', nowxy.y);"
     """}"
     "}");

  mouseUpJS_.setJavaScript
    ("function(obj, event) {"
     """obj.removeAttribute('dsx');"
     "}");

  titleBar_->mouseWentDown().connect(mouseDownJS_);
  titleBar_->mouseMoved().connect(mouseMovedJS_);
  titleBar_->mouseWentUp().connect(mouseUpJS_);

  saveCoverState(app, app->dialogCover());

  hide();
}

WDialog::~WDialog()
{
  hide();
}

void WDialog::resize(const WLength& width, const WLength& height)
{
  impl_->setLayout(impl_->layout());

  WCompositeWidget::resize(width, height);
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

#ifndef WT_TARGET_JAVA
WDialog::DialogCode WDialog::exec()
{
  if (recursiveEventLoop_)
    throw WtException("WDialog::exec(): already in recursive event loop.");

  show();

  recursiveEventLoop_ = true;
  do {
    WApplication::instance()->session()->doRecursiveEventLoop();
  } while (recursiveEventLoop_);

  hide();

  return result_;
}
#endif // WT_TARGET_JAVA

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

      if (!hidden) {
	saveCoverState(app, cover);

	cover->show();
	cover->setZIndex(impl_->zIndex() - 1);
	app->constrainExposed(this);
      } else
	restoreCoverState(app, cover);
    }
  }

  WCompositeWidget::setHidden(hidden);
}

}
