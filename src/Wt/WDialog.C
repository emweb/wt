/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WDialog.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WVBoxLayout.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"
#include "Wt/Utils.h"
#include "Wt/WGlobal.h"

#include "Resizable.h"
#include "WebController.h"
#include "WebUtils.h"

#include <boost/algorithm/string.hpp>

#ifndef WT_DEBUG_JS
#include "js/WDialog.min.js"
#endif

namespace Wt {

class DialogCover : public WContainerWidget
{
public:
  DialogCover() : WContainerWidget()
  {
    setObjectName("dialog-cover");

    hide();
  }

  void pushDialog(WDialog *dialog, const WAnimation& animation) {
    dialogs_.push_back(dialog);

    if (dialog->isModal()) {
      coverFor(dialog, animation);
    }

    scheduleRender();
  }

  void popDialog(WDialog *dialog, const WAnimation& animation) {
    Utils::erase(dialogs_, dialog);

    WDialog *topModal = nullptr;

    for (unsigned i = dialogs_.size(); i > 0; --i) {
      unsigned j = i - 1;
      if (dialogs_[j]->isModal()) {
	topModal = dialogs_[j];
	break;
      }
    }

    coverFor(topModal, animation);

    if (dialogs_.empty()) {
#ifndef WT_TARGET_JAVA
      WApplication::instance()->removeChild(this);
#else // WT_TARGET_JAVA
      delete this;
#endif // WT_TARGET_JAVA
    } else
      scheduleRender();
  }

  virtual bool isExposed(WWidget *w) final override {
    for (unsigned i = dialogs_.size(); i > 0; --i) {
      unsigned j = i - 1;

      if (dialogs_[j]->isExposed(w))
	return true;

      if (dialogs_[j]->isModal())
	return isInOtherPopup(w);
    }

    return false;
  }

  bool isTopDialogRendered(WDialog *dialog) const {
    return dialog->id() == topDialogId_;
  }

  void bringToFront(WDialog *dialog) {
    if (Utils::erase(dialogs_, dialog)) {
      dialogs_.push_back(dialog);
      scheduleRender();
    }
  }

protected:
  virtual void render(WFlags<RenderFlag> flags) final override {
    if (dialogs_.empty())
      topDialogId_.clear();
    else
      topDialogId_ = dialogs_.back()->id();
  }

private:
  std::vector<WDialog *> dialogs_;
  std::string topDialogId_; // as currently rendered

  void coverFor(WDialog *dialog, const WAnimation& animation) {
    if (dialog) {
      if (isHidden()) {

	if (!animation.empty())
	  animateShow(WAnimation(AnimationEffect::Fade,
				 TimingFunction::Linear,
				 animation.duration() * 4));
	else
	  show();

	WApplication::instance()->pushExposedConstraint(this);
      }
	
      dialog->doJSAfterLoad("setTimeout(function() {"
       + WApplication::instance()->javaScriptClass() 
       + "._p_.updateGlobal('" + dialog->layoutContainer_->id() + "') }"
       ", 0);"
       );

      setZIndex(dialog->zIndex() - 1);

      setStyleClass(userCoverClasses(dialog));

      WApplication *app = WApplication::instance();
      app->theme()->apply(app->domRoot(), this, DialogCoverWidget);
    } else {
	//call updateGlobal(null)
      WApplication::instance()->doJavaScript("setTimeout(function() {"
	    + WApplication::instance()->javaScriptClass() 
	    + "._p_.updateGlobal(null) });");
      if (!isHidden()) {
	if (!animation.empty())
	  animateHide(WAnimation(AnimationEffect::Fade,
				 TimingFunction::Linear,
				 animation.duration() * 4));
	else
	  hide();

	WApplication::instance()->popExposedConstraint(this);
      }
    }
  }

  std::string userCoverClasses(WWidget *w) {
    std::string c = w->styleClass().toUTF8();
    std::vector<std::string> classes;
    boost::split(classes, c, boost::is_any_of(" "));

    std::string result;
    for (unsigned i = 0; i < classes.size(); ++i) {
      if (!classes[i].empty() && !boost::starts_with(classes[i], "Wt-")) {
	if (!result.empty())
	  result += " ";
	result += classes[i] + "-cover";
      }
    }

    return result;
  }

  bool isInOtherPopup(WWidget *w) {
    WApplication *app = WApplication::instance();

    /*
     * Not sure if the following is entirely correct for popup widgets
     * and popup menus?
     */
    for (WWidget *p = w; p; p = p->parent()) {
      if (dynamic_cast<WDialog *>(p))
	return false;

      if (p == app->domRoot())
	return w != app->root();

      w = p;
    }

    return false;
  }
};

WDialog::WDialog()
  : WPopupWidget(std::unique_ptr<WWidget>
		 (new WTemplate(tr("Wt.WDialog.template")))),
    moved_(this, "moved"),
    resized_(this, "resized"),
    zIndexChanged_(this, "zIndexChanged")
{
  create();
}

WDialog::WDialog(const WString& windowTitle)
  : WPopupWidget(std::unique_ptr<WWidget>
		 (new WTemplate(tr("Wt.WDialog.template")))),
    moved_(this, "moved"),
    resized_(this, "resized"),
    zIndexChanged_(this, "zIndexChanged")
{
  create();
  setWindowTitle(windowTitle);
}

void WDialog::create()
{
  closeIcon_ = nullptr;
  footer_ = nullptr;
  modal_ = true;
  resizable_ = false;
  recursiveEventLoop_ = false;
  escapeIsReject_ = false;
  autoFocus_ = true;
  impl_ = dynamic_cast<WTemplate *>(implementation());

  const char *CSS_RULES_NAME = "Wt::WDialog";

  WApplication *app = WApplication::instance();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    /* Needed for the dialog cover */
    if (app->environment().agentIsIElt(9))
      app->styleSheet().addRule("body", "height: 100%;");

    std::string position
      = app->environment().agent() 
      == UserAgent::IE6 ? "absolute" : "fixed";

    // we use left: 50%, top: 50%, margin hack when JavaScript is not available
    // see below for an IE workaround
    app->styleSheet().addRule("div.Wt-dialog", std::string()
			      //"position: " + position + ';'
			      + (!app->environment().ajax() ?
				 "left: 50%; top: 50%;"
				 "margin-left: -100px; margin-top: -50px;" :
				 "left: 0px; top: 0px;"),
			      CSS_RULES_NAME);

    if (app->environment().agent() == UserAgent::IE6) {
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

  LOAD_JAVASCRIPT(app, "js/WDialog.js", "WDialog", wtjs1);

  std::unique_ptr<WContainerWidget> layoutContainer(new WContainerWidget());
  layoutContainer_ = layoutContainer.get();
  layoutContainer->setGlobalUnfocused(true);
  wApp->theme()->apply(this, layoutContainer.get(), DialogContent);
  layoutContainer->addStyleClass("dialog-layout");
  std::unique_ptr<WVBoxLayout> layoutPtr(new WVBoxLayout());
  WVBoxLayout *layout = layoutPtr.get();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layoutContainer->setLayout(std::move(layoutPtr));
  impl_->bindWidget("layout", std::move(layoutContainer));

  titleBar_ = new WContainerWidget();
  app->theme()->apply(this, titleBar_, DialogTitleBar);

  caption_ = new WText();
  caption_->setInline(false);
  titleBar_->addWidget(std::unique_ptr<WText>(caption_));
  
  contents_ = new WContainerWidget();
  app->theme()->apply(this, contents_, DialogBody);

  layout->addWidget(std::unique_ptr<WWidget>(titleBar_));
  layout->addWidget(std::unique_ptr<WWidget>(contents_), 1);

  /*
   * Cannot be done using the CSS stylesheet in case there are
   * contained elements with setHideWithOffsets() set
   *
   * For IE, we cannot set it yet since it will confuse width measurements
   * to become minimum size instead of (unconstrained) preferred size
   */
  if (app->environment().ajax()) {
    impl_->setMargin(0);

    /*
     * This is needed for animations only, but setting absolute or
     * fixed positioning confuses layout measurement in IE browsers
     */
    if (!app->environment().agentIsIElt(9))
      setPositionScheme(PositionScheme::Fixed);
  } else
    setPositionScheme(app->environment().agent() == UserAgent::IE6
		      ? PositionScheme::Absolute : PositionScheme::Fixed);

  setMovable(true);

  zIndexChanged_.connect(this, &WDialog::zIndexChanged);
}

WDialog::~WDialog()
{
  hide();
}

WContainerWidget *WDialog::footer() const
{
  if (!footer_) {
    std::unique_ptr<WContainerWidget> footer(footer_ = new WContainerWidget());
    WApplication::instance()->theme()->apply
      (const_cast<WDialog *>(this), footer_, DialogFooter);

    WContainerWidget *layoutContainer
      = impl_->resolve<WContainerWidget *>("layout");
    layoutContainer->layout()->addWidget(std::move(footer));
  }

  return footer_;
}

void WDialog::setResizable(bool resizable)
{
  if (resizable != resizable_) {
    resizable_ = resizable;
    toggleStyleClass("Wt-resizable", resizable);
    setSelectable(!resizable);

    if (resizable)
      contents_->setSelectable(true);

    if (resizable_) {
      Resizable::loadJavaScript(WApplication::instance());
      setJavaScriptMember
	(" Resizable",
	 "(new " WT_CLASS ".Resizable("
	 WT_CLASS "," + jsRef() + ")).onresize(function(w, h, done) {"
	 "var obj = " + jsRef() + ".wtObj;"
	 "if (obj) obj.onresize(w, h, done);"
	 " });");
    }
  }
}

void WDialog::setMovable(bool movable)
{
  movable_ = movable;

  layoutContainer_->toggleStyleClass("movable", movable_);
}

void WDialog::setMaximumSize(const WLength& width, const WLength& height)
{
  WPopupWidget::setMaximumSize(width, height);

  WLength w = width.unit() != LengthUnit::Percentage ? width : WLength::Auto;
  WLength h = height.unit() != LengthUnit::Percentage ? height : WLength::Auto;

  impl_->resolveWidget("layout")->setMaximumSize(w, h);
}

void WDialog::setMinimumSize(const WLength& width, const WLength& height)
{
  WPopupWidget::setMinimumSize(width, height);

  impl_->resolveWidget("layout")->setMinimumSize(width, height);
}

void WDialog::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full)) {
    WApplication *app = WApplication::instance();

    bool centerX = offset(Side::Left).isAuto() && offset(Side::Right).isAuto(),
      centerY = offset(Side::Top).isAuto() && offset(Side::Bottom).isAuto();

    /*
     * Make sure layout adjusts to contents preferred width, especially
     * important for IE workaround which uses static position scheme
     */
    if (app->environment().ajax())
      if (width().isAuto())
	if (maximumWidth().unit() == LengthUnit::Percentage ||
	    maximumWidth().toPixels() == 0)
	  impl_->resolveWidget("layout")->setMaximumSize(999999,
							 maximumHeight());

    doJavaScript("new " WT_CLASS ".WDialog("
		 + app->javaScriptClass() + "," + jsRef()
		 + "," + titleBar_->jsRef()
		 + "," + (movable_ ? "1" : "0")
		 + "," + (centerX ? "1" : "0")
		 + "," + (centerY ? "1" : "0") 
		 + "," + (moved_.isConnected()
			  ? '"' + moved_.name() + '"' 
			  : "null")
		 + "," + (resized_.isConnected()
			  ? '"' + resized_.name() + '"' 
			  : "null")
                 + ",\"" + zIndexChanged_.name() + '"'
		 + ");");

    for (std::size_t i = 0; i < delayedJs_.size(); ++i) {
      doJavaScript(delayedJs_[i]);
    }
    delayedJs_.clear();

    /*
     * When a dialog is shown immediately for a new session, the recentering
     * logic comes too late and causes a glitch. Thus we include directly in
     * the HTML a JavaScript block to mitigate that
     */
    if (!app->environment().agentIsIElt(9) &&
        !app->environment().ajax()) {
      std::string js = WString::tr("Wt.WDialog.CenterJS").toUTF8();
      Utils::replace(js, "$el", "'" + id() + "'");
      Utils::replace(js, "$centerX", centerX ? "1" : "0");
      Utils::replace(js, "$centerY", centerY ? "1" : "0");

      impl_->bindString
	("center-script", "<script>" + Utils::htmlEncode(js)
	 + "</script>", TextFormat::UnsafeXHTML);
    } else
      impl_->bindEmpty("center-script");
  }

  if (!isModal())
    impl_->mouseWentDown().connect(this, &WDialog::bringToFront);

  if ( flags.test(RenderFlag::Full) && autoFocus_) {
    if (!impl_->findById(Wt::WApplication::instance()->focus()))
      impl_->setFirstFocus();
  }

  WPopupWidget::render(flags);
}

void WDialog::onPathChange()
{
}

void WDialog::rejectWhenEscapePressed(bool enable)
{
  escapeIsReject_ = enable;
}

void WDialog::setWindowTitle(const WString& windowTitle)
{
  caption_->setText
    (WString::fromUTF8("<h4>" + Utils::htmlEncode(windowTitle.toUTF8())
		       + "</h4>"));
}

WString WDialog::windowTitle() const
{
  std::string text = caption_->text().toUTF8();
  if (text.length() > 9)
    return WString::fromUTF8(text.substr(4, text.length() - 9));
  else
    return WString::Empty;
}

void WDialog::setTitleBarEnabled(bool enable)
{
  titleBar_->setHidden(!enable);
}

void WDialog::setClosable(bool closable)
{
  if (closable) {
    if (!closeIcon_) {
      std::unique_ptr<WText> closeIcon(closeIcon_ = new WText());
      titleBar_->insertWidget(0, std::move(closeIcon));
      WApplication::instance()->theme()->apply
	(this, closeIcon_, DialogCloseIcon);
      closeIcon_->clicked().connect(this, &WDialog::reject);
    }
  } else {
    titleBar_->removeWidget(closeIcon_);
    closeIcon_ = nullptr;
  }
}

DialogCode WDialog::exec(const WAnimation& animation)
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
      app->waitForEvent();
    } while (recursiveEventLoop_);
  }

  hide();

  return result_;
}

void WDialog::done(DialogCode result)
{
  if (isHidden())
    return;

  result_ = result;

  if (recursiveEventLoop_) {
    recursiveEventLoop_ = false;
  } else
    hide();

  finished_.emit(result);
}

void WDialog::accept()
{
  done(DialogCode::Accepted);
}

void WDialog::reject()
{
  done(DialogCode::Rejected);
}

void WDialog::setModal(bool modal)
{
  modal_ = modal;
}

void WDialog::onDefaultPressed()
{
  DialogCover *c = cover();
  if (footer_ && c && c->isTopDialogRendered(this)) {
    for (int i = 0; i < footer()->count(); ++i) {
      WPushButton *b = dynamic_cast<WPushButton *>(footer()->widget(i));
      if (b && b->isDefault()) {
	if (b->isEnabled())
	  b->clicked().emit(WMouseEvent());
        break;
      }
    }
  }
}

void WDialog::onEscapePressed()
{
  DialogCover *c = cover();
  if (c && c->isTopDialogRendered(this))
    reject();
}

void WDialog::setHidden(bool hidden, const WAnimation& animation)
{
  /* For JWt: setHidden() is called from WPopupWidget constructor. */
  if (contents_ && isHidden() != hidden) {
    if (!hidden) {
      if (footer_) {
	for (int i = 0; i < footer()->count(); ++i) {
	  WPushButton *b = dynamic_cast<WPushButton *>(footer()->widget(i));
	  if (b && b->isDefault()) {
            enterConnection1_ = enterPressed()
              .connect(this, &WDialog::onDefaultPressed);

	    enterConnection2_ = impl_->enterPressed()
	      .connect(this, &WDialog::onDefaultPressed);
	    break;
	  }
	}
      }

      if (escapeIsReject_) {
        if (isModal()) {
          escapeConnection1_ = escapePressed()
            .connect(this, &WDialog::onEscapePressed);
        } else {
          escapeConnection1_ = WApplication::instance()->globalEscapePressed()
            .connect(this, &WDialog::onEscapePressed);
        }

	escapeConnection2_ = impl_->escapePressed()
	  .connect(this, &WDialog::onEscapePressed);
      }
    } else {
      escapeConnection1_.disconnect();
      escapeConnection2_.disconnect();
      enterConnection1_.disconnect();
      enterConnection2_.disconnect();
    }

    DialogCover *c = cover();
    if (!hidden) {
      if (c)
	c->pushDialog(this, animation);
    
      if (modal_) {
        doJSAfterLoad
	  ("try {"
	   """var ae=document.activeElement;"
	   // On IE when a dialog is shown on startup, activeElement is the
	   // body. Bluring the body sends the window to the background if
	   // it is the only tab.
	   // http://redmine.emweb.be/boards/2/topics/6415
	   """if (ae && ae.blur && ae.nodeName != 'BODY') {"
	   ""  "document.activeElement.blur();"
	   "}"
	   "} catch (e) { }");
      }
    } else {
      if (c)
	c->popDialog(this, animation);
    }
  }

  WPopupWidget::setHidden(hidden, animation);
}

void WDialog::positionAt(const WWidget *widget, Orientation orientation)
{
  setPositionScheme(PositionScheme::Absolute);
  if (wApp->environment().javaScript())
    setOffsets(0, Side::Left | Side::Top);
  WPopupWidget::positionAt(widget, orientation);
}

void WDialog::positionAt(const Wt::WMouseEvent& ev)
{
  setPositionScheme(PositionScheme::Fixed);
  if (wApp->environment().javaScript()) {
	setOffsets(ev.window().x, Side::Left);
	setOffsets(ev.window().y, Side::Top);
  }
}

DialogCover *WDialog::cover() 
{
  WApplication *app = WApplication::instance();

  if (app->domRoot()) {
    WWidget *w = app->findWidget("dialog-cover");

    if (w)
      return dynamic_cast<DialogCover *>(w);
    else {
      std::unique_ptr<DialogCover> d(new DialogCover());
      auto result = d.get();
      app->addGlobalWidget(result);
#ifndef WT_TARGET_JAVA
      app->addChild(std::move(d));
#endif // WT_TARGET_JAVA
      return result;
    }
  } else
    return 0;
}

void WDialog::bringToFront(const WMouseEvent &e)
{
  if (e.button() == MouseButton::Left &&
      e.modifiers() == KeyboardModifier::None) {
    raiseToFront();
  }
}

void WDialog::raiseToFront()
{
  doJSAfterLoad(jsRef() + ".wtObj.bringToFront()");
  DialogCover *c = cover();
  c->bringToFront(this);  
}

void WDialog::zIndexChanged(int zIndex)
{
  impl_->layoutImpl_->zIndex_ = zIndex;
}

EventSignal<WKeyEvent>& WDialog::keyWentDown()
{
  return layoutContainer_->keyWentDown();
}

EventSignal<WKeyEvent>& WDialog::keyWentUp()
{
  return layoutContainer_->keyWentUp();
}

EventSignal<WKeyEvent>& WDialog::keyPressed()
{
  return layoutContainer_->keyPressed();
}

EventSignal<>& WDialog::enterPressed()
{
  return layoutContainer_->enterPressed();
}

EventSignal<>& WDialog::escapePressed()
{
  return layoutContainer_->escapePressed();
}

EventSignal<WTouchEvent>& WDialog::touchStarted()
{
  return layoutContainer_->touchStarted();
}

EventSignal<WTouchEvent>& WDialog::touchEnded()
{
  return layoutContainer_->touchEnded();
}

EventSignal<WTouchEvent>& WDialog::touchMoved()
{
  return layoutContainer_->touchMoved();
}

void WDialog::doJSAfterLoad(std::string js)
{
  if (isRendered())
    doJavaScript(js);
  else
    delayedJs_.push_back(js);
}

}
