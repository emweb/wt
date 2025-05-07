/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WException.h"
#include "Wt/WStackedValidator.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WStackedValidator.min.js"
#endif

namespace Wt {

WStackedValidator::WStackedValidator()
{
  setMandatory(false);
}

WStackedValidator::~WStackedValidator()
{
  doClear();
}

void WStackedValidator::addValidator(const std::shared_ptr<WValidator>& validator)
{
  if (Utils::add(validators_, validator)) {
    validator->addParentValidator(this);
    repaint();
  }
}

void WStackedValidator::insertValidator(int index, const std::shared_ptr<WValidator>& validator)
{
  if (index < 0) {
    throw WException("Cannot insert validator at negative index");
  }

  if (Utils::indexOf(validators_, validator) == -1) {
    if (index > static_cast<int>(validators_.size())) {
      index = static_cast<int>(validators_.size());
    }

    validators_.insert(validators_.begin() + index, validator);
    validator->addParentValidator(this);
    repaint();
  }
}

void WStackedValidator::removeValidator(const std::shared_ptr<WValidator>& validator)
{
  if (Utils::erase(validators_, validator)) {
    validator->removeParentValidator(this);
    repaint();
  }
}

void WStackedValidator::clear()
{
  doClear();
  repaint();
}

Wt::WValidator::Result WStackedValidator::validate(const WT_USTRING& input) const
{
  for (std::size_t i = 0; i < validators_.size(); ++i) {
    WValidator::Result result = validators_[i]->validate(input);
    if (result.state() != ValidationState::Valid) {
      return result;
    }
  }
  return WValidator::Result(ValidationState::Valid);
}

std::string WStackedValidator::javaScriptValidate() const
{
  LOAD_JAVASCRIPT(WApplication::instance(), "js/WStackedValidator.js", "WStackedValidator", wtjs1);

  WStringStream js;

  js << "new " WT_CLASS ".WStackedValidator([";
  for (std::size_t i = 0; i < validators_.size(); ++i) {
    if (i > 0) {
      js << ",";
    }
    std::string validatorJs = validators_[i]->javaScriptValidate();
    if (validatorJs[validatorJs.size() - 1] == ';') {
      validatorJs = validatorJs.substr(0, validatorJs.size() - 1);
    }
    js << validatorJs;
  }
  js << "]);";

  return js.str();
}

void WStackedValidator::doClear()
{
  for (std::size_t i = 0; i < validators_.size(); ++i) {
    validators_[i]->removeParentValidator(this);
  }
  validators_.clear();
}

}