/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBootstrapTheme.h"

#include "Wt/WBootstrap2Theme.h"
#include "Wt/WBootstrap3Theme.h"
#include "Wt/WLinkedCssStyleSheet.h"

#include <cassert>
#include <memory>

namespace Wt {

WBootstrapTheme::WBootstrapTheme()
 : impl_(std::make_unique<WBootstrap2Theme>()),
   version_(BootstrapVersion::v2),
   formControlStyle_(true)
{ }

WBootstrapTheme::~WBootstrapTheme()
{ }

std::string WBootstrapTheme::name() const
{
  return impl_->name();
}

std::string WBootstrapTheme::resourcesUrl() const
{
  return impl_->resourcesUrl();
}

std::vector<WLinkedCssStyleSheet> WBootstrapTheme::styleSheets() const
{
  return impl_->styleSheets();
}

void WBootstrapTheme::init(WApplication *app) const
{
  impl_->init(app);
}

void WBootstrapTheme::apply(WWidget *widget, WWidget *child, int widgetRole)
  const
{
  impl_->apply(widget, child, widgetRole);
}

void WBootstrapTheme::apply(WWidget *widget, DomElement& element,
                            int elementRole) const
{
  impl_->apply(widget, element, elementRole);
}

std::string WBootstrapTheme::disabledClass() const
{
  return impl_->disabledClass();
}

std::string WBootstrapTheme::activeClass() const
{
  return impl_->activeClass();
}

std::string WBootstrapTheme::utilityCssClass(int utilityCssClassRole) const
{
  return impl_->utilityCssClass(utilityCssClassRole);
}

bool WBootstrapTheme::canStyleAnchorAsButton() const
{
  return impl_->canStyleAnchorAsButton();
}

void WBootstrapTheme
::applyValidationStyle(WWidget *widget,
                       const Wt::WValidator::Result& validation,
                       WFlags<ValidationStyleFlag> styles) const
{
  impl_->applyValidationStyle(widget, validation, styles);
}

bool WBootstrapTheme::canBorderBoxElement(const DomElement& element) const
{
  return impl_->canBorderBoxElement(element);
}

void WBootstrapTheme::setVersion(BootstrapVersion version)
{
  if (version_ == version)
    return;

  if (version == BootstrapVersion::v2) {
    auto bootstrap2 = std::make_unique<WBootstrap2Theme>();
    bootstrap2->setResponsive(responsive());
    impl_ = std::move(bootstrap2);
  } else {
    auto bootstrap3 = std::make_unique<WBootstrap3Theme>();
    bootstrap3->setResponsive(responsive());
    bootstrap3->setFormControlStyleEnabled(formControlStyle_);
    impl_ = std::move(bootstrap3);
  }

  version_ = version;
}

void WBootstrapTheme::setResponsive(bool enabled)
{
  if (version() == BootstrapVersion::v2) {
    auto bootstrap2 = dynamic_cast<WBootstrap2Theme*>(impl_.get());
    assert(bootstrap2);
    bootstrap2->setResponsive(enabled);
  } else {
    auto bootstrap3 = dynamic_cast<WBootstrap3Theme*>(impl_.get());
    assert(bootstrap3);
    bootstrap3->setResponsive(enabled);
  }
}

bool WBootstrapTheme::responsive() const
{
  if (version() == BootstrapVersion::v2) {
    auto bootstrap2 = dynamic_cast<const WBootstrap2Theme*>(impl_.get());
    assert(bootstrap2);
    return bootstrap2->responsive();
  } else {
    auto bootstrap3 = dynamic_cast<const WBootstrap3Theme*>(impl_.get());
    assert(bootstrap3);
    return bootstrap3->responsive();
  }
}

void WBootstrapTheme::setFormControlStyleEnabled(bool enabled)
{
  formControlStyle_ = enabled;
  if (version() == BootstrapVersion::v3) {
    auto bootstrap3 = dynamic_cast<WBootstrap3Theme*>(impl_.get());
    assert(bootstrap3);
    bootstrap3->setFormControlStyleEnabled(enabled);
  }
}

}
