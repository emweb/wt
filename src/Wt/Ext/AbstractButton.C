/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/AbstractButton"
#include "Wt/Ext/Menu"
#include "Wt/Ext/ToolTipConfig"

namespace Wt {
  namespace Ext {

AbstractButton::AbstractButton(WContainerWidget *parent)
  : Component(parent),
    activated_(this, "activated", true),
    toggled_(this, "toggled", true),
    checkable_(false),
    checked_(false),
    menu_(0),
    toolTipConfig_(0)
{ }

void AbstractButton::setChecked(bool how)
{
  if (checkable_) {
    if (checked_ != how) {
      checked_ = how;

      if (isRendered())
	addUpdateJS(elVar() + "." + checkMethodJS()
		    + "(" + (checked_ ? "true" : "false") + ");");
    }
  }
}

void AbstractButton::setCheckable(bool how)
{
  checkable_ = how;

  if (checkable_) {
    toggled_.connect(this, &AbstractButton::wasToggled);
  }
}

void AbstractButton::wasToggled(bool how)
{
  checked_ = how;
}

void AbstractButton::setText(const WString& text)
{
  text_ = text;

  if (isRendered())
    addUpdateJS(elVar() + ".setText(" + text_.jsStringLiteral() + ");");
}

void AbstractButton::setIcon(const std::string& iconPath)
{
  icon_ = iconPath;

  if (isRendered())
    addUpdateJS(elVar() + ".icon = " + WWebWidget::jsStringLiteral(icon_) + ";"
		+ elVar() + ".setText(" + elVar() + ".text);");
}

void AbstractButton::refresh()
{
  if (text_.refresh()) {
    setText(text_);
  }

  Component::refresh();
}

void AbstractButton::setMenu(Menu *menu)
{
  if (menu_)
    removeOrphan(menu_);

  menu_ = menu;

  if (menu_)
    addOrphan(menu_);
}

void AbstractButton::configureToolTip(ToolTipConfig *config)
{
  toolTipConfig_ = config;

  if (!config->parent())
    WObject::addChild(config);
}

void AbstractButton::updateExt()
{
  updateWtSignal(&activated_, activated_.name(), "", "");
  updateWtSignal(&toggled_, toggled_.name(), "i,c", "c");
  Component::updateExt();
}

std::string AbstractButton::createJSHelper(const std::string& extClassName,
					   bool intoElement)
{
  std::stringstream result;
  std::string menuvar;

  std::stringstream buf;

  buf << "{a:0";

  if (checkable_ != false) {
    if (extClassName != "Ext.menu.CheckItem")
      buf << ",enableToggle:true";

    if (checked_ != false) {
      buf << "," << checkInitialState() << ":true";
    }
  }

  if (menu_) {
    menuvar = menu_->createExtElement(result, 0);
    buf << ",menu:" << menuvar;
  }

  createConfig(buf);
  buf << "}";

  result <<
    elVar() << "=new " << extClassName << "(" << buf.str() << ");";

  if (intoElement)
    result << elVar() << ".render('" << id() << "');";

  bindEventHandler(checkEventJS(), "toggleH", result);

  return result.str();
}

void AbstractButton::createConfig(std::ostream& config)
{
  if (!text_.empty())
    config << ",text:" << text_.jsStringLiteral();

  if (!icon_.empty())
    config << ",icon:" << WWebWidget::jsStringLiteral(icon_);

  // default cls is 'x-btn-text'
  if (!icon_.empty()) {
    if (text_.empty())
      config << ",cls:'x-btn-icon'";
    else
      config << ",cls:'x-btn-text-icon'";
  }

  addWtSignalConfig("handler", &activated_, activated_.name(),
		    "", "", config);
  addWtSignalConfig("toggleH", &toggled_, toggled_.name(),
		    "i,c", "c", config);

  if (!toolTip().empty()) {
    config << ",tooltip:{"
      "text:" << toolTip().jsStringLiteral();

    if (toolTipConfig_)
      toolTipConfig_->createConfig(config);

    config << "}";
  }

  Component::createConfig(config);
}


  }
}
