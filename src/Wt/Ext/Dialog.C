/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Ext/Button"
#include "Wt/Ext/Dialog"

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WException"
#include "Wt/WFitLayout"

#include "DomElement.h"
#include "WebSession.h"

namespace Wt {
  namespace Ext {

Dialog::Dialog(const WString& windowTitle)
  : Panel(WApplication::instance()->domRoot()),
    finished_(this),
    contents_(0),
    sizeGripEnabled_(true),
    recursiveEventLoop_(false),
    hiddenS_(this, "hidden", false),
    hidden_(true)
{
  setInline(false);

  setTitle(windowTitle);
  setStyleClass("x-hidden");
}

Dialog::Dialog(Bla)
  : Panel(),
    finished_(this),
    contents_(0),
    sizeGripEnabled_(false),
    recursiveEventLoop_(false),
    hiddenS_(this, "hidden", false),
    hidden_(true)
{ }

Dialog::~Dialog()
{ 
  hide();
}

WContainerWidget *Dialog::contents() const
{
  if (!contents_) {
    Dialog *self = const_cast<Dialog *>(this);

    self->contents_ = new WContainerWidget();
    self->setLayout(new WFitLayout());
    self->layout()->addWidget(self->contents_);
  }

  return contents_;
}

void Dialog::addButton(Button *button)
{
  Panel::addFooterButton(button);
}

void Dialog::removeButton(Button *button)
{
  Panel::removeFooterButton(button);
}

void Dialog::setDefaultButton(Button *button)
{
  Panel::setDefaultButton(button);
}

Button *Dialog::defaultButton() const
{
  return Panel::defaultButton();
}

void Dialog::setWindowTitle(const WString& windowTitle)
{
  Panel::setTitle(windowTitle);
}

void Dialog::setSizeGripEnabled(bool enabled)
{
  sizeGripEnabled_ = enabled;
}

Dialog::DialogCode Dialog::exec()
{
  if (recursiveEventLoop_)
    throw WException("Dialog::exec(): already in recursive event loop.");

  show();

  recursiveEventLoop_ = true;
  do {
    WApplication::instance()->session()->doRecursiveEventLoop();
  } while (recursiveEventLoop_);

  hide();

  return result_;
}

void Dialog::done(DialogCode result)
{
  result_ = result;
  if (recursiveEventLoop_) {
    recursiveEventLoop_ = false;
  } else
    hide();

  finished_.emit(result);
}

void Dialog::accept()
{
  done(Accepted);
}

void Dialog::reject()
{
  done(Rejected);
}

void Dialog::wasHidden()
{
  // hidden through the 'close' button in the right upper corner
  hidden_ = true;

  WApplication::instance()->popExposedConstraint(this);

  reject();
}

void Dialog::setHidden(bool hidden, const WAnimation& animation)
{
  if (hidden_ != hidden) {
    hidden_ = hidden;

    Panel::setHidden(hidden, animation);

    WApplication *app = WApplication::instance();
    if (!app->environment().agentIsIE() && !hidden_)
      app->doJavaScript
	(WT_CLASS ".getElement('" + elVar() + "').style.position='fixed';");

    if (!hidden)
      setExposeMask(app);
    else
      restoreExposeMask(app);
  }
}

void Dialog::setExposeMask(WApplication *app)
{
  app->pushExposedConstraint(this);
}

void Dialog::restoreExposeMask(WApplication *app)
{
  app->popExposedConstraint(this);
}

std::string Dialog::extClassName() const
{
  return "Ext.Window";
}

std::string Dialog::createJS(DomElement *inContainer)
{
  if (!hiddenS_.isConnected())
    hiddenS_.connect(this, &Dialog::wasHidden);

  std::string result = Panel::createJS(inContainer);

  if (!hidden_) {
    result += elVar() + ".show();";
    result += "{var xy=" + elVar() + ".getPosition();"
      "if (xy[0]< 0 || xy[1]<0) " + elVar()
      + ".setPosition(xy[0]>0?xy[0]:0, xy[1]>0?xy[1]:0);}";
    WApplication *app = WApplication::instance();
    if (!app->environment().agentIsIE())
      result += WT_CLASS ".getElement('" + elVar()
	+ "').style.position='fixed';";
  }

  return result;
}

void Dialog::createConfig(std::ostream& config)
{
  Panel::createConfig(config);

  config << ",modal:true,shadow:true,closable:false";

  if (defaultButton())
    config << ",defaultButton:" << defaultButton()->elRef();

  if (sizeGripEnabled_)
    config << ",resizeHandles:'se'";
  else
    config << ",resizable:false";
}


  }
}
