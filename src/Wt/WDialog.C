/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WDialog"
#include "Wt/WException"
#include "Wt/WTemplate"
#include "Wt/WText"

#include "Resizable.h"
#include "WebController.h"
#include "WebSession.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WDialog.min.js"
#endif

namespace Wt {

WDialog::WDialog(const WString& windowTitle)
  : closeIcon_(0),
    modal_(true),
    resizable_(false),
    finished_(this),
    recursiveEventLoop_(false),
    initialized_(false)
{
  setImplementation(impl_ = new WTemplate(tr("Wt.WDialog.template")));

  const char *CSS_RULES_NAME = "Wt::WDialog";

  WApplication *app = WApplication::instance();

  /*
   * Cannot be done using the CSS stylesheet in case there are
   * contained elements with setHideWithOffsets() set
   */
  setPositionScheme(app->environment().agent() == WEnvironment::IE6
		    ? Absolute : Fixed);

  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    /* Needed for the dialog cover */
    if (app->environment().agentIsIElt(9))
      app->styleSheet().addRule("body", "height: 100%;");

    std::string position
      = app->environment().agent() == WEnvironment::IE6 ? "absolute" : "fixed";

    // we use left: 50%, top: 50%, margin hack when JavaScript is not available
    // see below for an IE workaround
    app->styleSheet().addRule("div.Wt-dialog", std::string() +
			      (app->environment().ajax() ?
			       "visibility: hidden;" : "") +
			      "position: " + position + ';'
			      + (!app->environment().ajax() ?
				 "left: 50%; top: 50%;"
				 "margin-left: -100px; margin-top: -50px;" :
				 "left: 0px; top: 0px;"),
			      CSS_RULES_NAME);

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

  setPopup(true);

  LOAD_JAVASCRIPT(app, "js/WDialog.js", "WDialog", wtjs1);

  WContainerWidget *parent = app->domRoot();
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

  hide();
}

WDialog::~WDialog()
{
  hide();
}

void WDialog::setResizable(bool resizable)
{
  if (resizable != resizable_) {
    resizable_ = resizable;
    toggleStyleClass("Wt-resizable", resizable);
    setSelectable(!resizable);

    if (resizable_) {
      setMinimumSize(WLength::Auto, WLength::Auto);
      Resizable::loadJavaScript(WApplication::instance());
      doJavaScript("(new " WT_CLASS ".Resizable("
		   WT_CLASS "," + jsRef() + ")).onresize(function(w, h) {"
		   "var obj = $('#" + id() + "').data('obj');"
		   "if (obj) obj.onresize(w, h);"
		   " });");
    }
  }
}

void WDialog::render(WFlags<RenderFlag> flags)
{
  if (!initialized_) {
    initialized_ = true;

    WApplication *app = WApplication::instance();

    bool centerX = offset(Left).isAuto() && offset(Right).isAuto(),
      centerY = offset(Top).isAuto() && offset(Bottom).isAuto();

    setJavaScriptMember("_a","0;new " WT_CLASS ".WDialog("
			+ app->javaScriptClass() + "," + jsRef()
			+ "," + (centerX ? "1" : "0")
			+ "," + (centerY ? "1" : "0") + ");");
    // so that WWidget::resize() calls it; it is set by js: WDialog()
    setJavaScriptMember(WT_RESIZE_JS, "\"dummy\"");

    app->addAutoJavaScript
      ("{var obj = $('#" + id() + "').data('obj');"
       "if (obj) obj.centerDialog();}");

    /*
     * When a dialog is shown immediately for a new session, the recentering
     * logic comes too late and causes a glitch. Thus we include directly in
     * the HTML a JavaScript block to mitigate that
     */
    if (!app->environment().agentIsIElt(9)) {
      std::string js = WString::tr("Wt.WDialog.CenterJS").toUTF8();
      Utils::replace(js, "$el", "'" + id() + "'");
      Utils::replace(js, "$centerX", centerX ? "1" : "0");
      Utils::replace(js, "$centerY", centerY ? "1" : "0");

      impl_->bindString
	("center-script", "<script>" + js + "</script>", XHTMLUnsafeText);
    } else
      impl_->bindEmpty("center-script");
  }

  WCompositeWidget::render(flags);
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

void WDialog::setClosable(bool closable)
{
  if (closable) {
    if (!closeIcon_) {
      closeIcon_ = new WText(titleBar_);
      closeIcon_->setStyleClass("closeicon");
      closeIcon_->clicked().connect(this, &WDialog::reject);
    }
  } else {
    delete closeIcon_;
    closeIcon_ = 0;
  }
}

WDialog::DialogCode WDialog::exec(const WAnimation& animation)
{
  if (recursiveEventLoop_)
    throw WException("WDialog::exec(): already being executed.");

  animateShow(animation);

#ifdef WT_TARGET_JAVA
  if (!WebController::isAsyncSupported())
     throw WException("WDialog#exec() requires a Servlet 3.0 enabled servlet " 
		      "container and an application with async-supported "
		      "enabled.");
#endif

  WApplication *app = WApplication::instance();
  recursiveEventLoop_ = true;

  if (app->environment().isTest()) {
    app->environment().dialogExecuted().emit(this);
    if (recursiveEventLoop_)
      throw WException("Test case must close dialog");
  } else {
    do {
      app->session()->doRecursiveEventLoop();
    } while (recursiveEventLoop_);
  }

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
}

void WDialog::restoreCoverState(WApplication *app, WContainerWidget *cover)
{
  cover->setHidden(coverWasHidden_);
  cover->setZIndex(coverPreviousZIndex_);
  app->popExposedConstraint(this);
}

void WDialog::setHidden(bool hidden, const WAnimation& animation)
{
  if (isHidden() != hidden) {
    if (modal_) {
      WApplication *app = WApplication::instance();
      WContainerWidget *cover = app->dialogCover();

      if (!cover)
	return; // when application is being destroyed

      if (!hidden) {
	saveCoverState(app, cover);

	if (cover->isHidden()) {
	  if (!animation.empty()) {
	    cover->animateShow(WAnimation(WAnimation::Fade, WAnimation::Linear,
					  animation.duration() * 4));
	  } else
	    cover->show();
	}

	cover->setZIndex(impl_->zIndex() - 1);
	app->pushExposedConstraint(this);

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

  WCompositeWidget::setHidden(hidden, animation);
}

void WDialog::positionAt(const WWidget *widget, Orientation orientation)
{
  setPositionScheme(Absolute);
  setOffsets(0, Left | Top);
  WCompositeWidget::positionAt(widget, orientation);
}

}
