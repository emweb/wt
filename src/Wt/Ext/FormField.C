/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/FormField"
#include "Wt/WApplication"
#include "Wt/WFormWidget"
#include "Wt/WLabel"
#include "Wt/WValidator"

#include "DomElement.h"

namespace Wt {
  namespace Ext {

FormField::FormField(WContainerWidget *parent)
  : Component(parent),
    errorMessageLocation_(FancyToolTip),
    validator_(0),
    focusWhenRendered_(false)
{ }

void FormField::setErrorMessageLocation(MessageLocation location)
{
  errorMessageLocation_ = location;
}

void FormField::setValidator(WValidator *validator)
{
  validator_ = validator;

  if (!validator_->parent())
    WObject::addChild(validator_);  
}

WValidator::State FormField::validate()
{
  return WValidator::Valid;
}

void FormField::refresh()
{
  // FIXME: refresh validator messages -- is not possible now ?
  Component::refresh();
}

void FormField::applyToWidget(WWebWidget *widget,
			      std::stringstream& js, DomElement *inContainer)
{
  if (inContainer) {
    inContainer->addChild(widget->createDomElement(WApplication::instance()));
    js << elVar() << ".applyToMarkup('" << widget->id() << "');";
  }
}

void FormField::createConfig(std::ostream& config)
{
  if (errorMessageLocation_ != FancyToolTip) {
    config << ",msgTarget:";
    switch (errorMessageLocation_) {
    case FancyToolTip: config << "'qtip'"; break;
    case PlainToolTip: config << "'title'"; break;
    case Below: config << "'under'"; break;
    case Besides: config << "'side'"; break;
    default:
      config << "''"; break;
    }
  }

  if (validator_)
    validator_->createExtConfig(config);

  Component::createConfig(config);
}

WLabel *FormField::label() const
{
  return formWidget()->label();
}

void FormField::setFocus()
{
  if (isRendered())
    addUpdateJS(elVar() + ".focus(true);");
  else
    focusWhenRendered_ = true;
}

void FormField::render(WFlags<RenderFlag> flags)
{
  Component::render(flags);

  if (focusWhenRendered_) {
    WApplication::instance()->doJavaScript(elVar() + ".focus(true);");
    focusWhenRendered_ = false;
  }
}

EventSignal<>& FormField::changed()
{
  return formWidget()->changed();
}

EventSignal<>& FormField::blurred()
{
  return formWidget()->blurred();
}

EventSignal<>& FormField::focussed()
{
  return formWidget()->focussed();
}

  }

void WLabel::setBuddy(Ext::FormField *formField)
{
  setBuddy(formField->formWidget());
}

}
