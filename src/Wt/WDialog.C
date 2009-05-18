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

    app->styleSheet().addRule("div.Wt-dialog",
			      "visibility: visible;"
			      "position: fixed; left: 50%; top: 50%;"
			      "margin-left: -100px; margin-top: -50px;");

    if (app->environment().agent() == WEnvironment::IE6) {
      app->styleSheet().addRule
	("div.Wt-dialogcover",
	 "position: absolute;"
	 "left: expression("
	 "(ignoreMe2 = document.documentElement.scrollLeft) + 'px' );"
	 "top: expression("
	 "(ignoreMe = document.documentElement.scrollTop) + 'px' );");
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

    app->styleSheet().addRule("div.Wt-dialog",
			      "border: 1px solid #888888;"
			      "background: #EEEEEE none repeat scroll 0%;");
    app->styleSheet().addRule("div.Wt-dialog .titlebar",
			      "background: #888888; color: #FFFFFF;"
			      "cursor: move;"
			      "padding: 2px 6px 3px;");
    app->styleSheet().addRule("div.Wt-dialog .body",
			      "background: #EEEEEE;"
			      "padding: 4px 6px 4px;");
    app->styleSheet().addRule("div.Wt-msgbox-buttons button",
			      "padding: 1px 4px 1px;"
			      "margin: 2px;");
  }

  WContainerWidget *parent = app->domRoot();

  setPopup(true);

  app->addAutoJavaScript
    ("{"
     "" "var d=" + jsRef() + ";"
     "" "if (d && d.style.display != 'none' && !d.getAttribute('moved')) {"
     ""   "var ws=" WT_CLASS ".windowSize();"
     ""   "d.style.left=Math.round((ws.x - d.clientWidth)/2) + 'px';"
     ""   "d.style.top=Math.round((ws.y - d.clientHeight)/2) + 'px';"
     ""   "d.style.marginLeft='0px';"
     ""   "d.style.marginTop='0px';"
     "" "}"
     "}");

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

void WDialog::setCaption(const WString& caption)
{
  setWindowTitle(caption);
}

void WDialog::setWindowTitle(const WString& windowTitle)
{
  caption_->setText(windowTitle);
}

const WString& WDialog::caption() const
{
  return windowTitle();
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

  WebSession *session = WApplication::instance()->session();
  recursiveEventLoop_ = true;

  show();
  session->doRecursiveEventLoop(std::string());
  hide();

  return result_;
}
#endif // WT_TARGET_JAVA

void WDialog::done(DialogCode result)
{
  result_ = result;

  if (recursiveEventLoop_) {
    WebSession *session = WApplication::instance()->session();
    recursiveEventLoop_ = false;
    session->unlockRecursiveEventLoop();
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

void WDialog::setHidden(bool hidden)
{
  WApplication *app = WApplication::instance();

  if (modal_) {
    WContainerWidget *cover = app->dialogCover(!hidden);
    if (cover) {
      cover->setHidden(hidden);
      if (!hidden)
	cover->setAttributeValue("style", "z-index:"
	  + boost::lexical_cast<std::string>(impl_->zIndex() - 1));
    }

    app->exposeOnly(hidden ? 0 : this);
  }
}

}
