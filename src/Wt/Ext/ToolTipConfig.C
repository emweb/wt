/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/Ext/ToolTipConfig"

namespace Wt {
  namespace Ext {

ToolTipConfig::ToolTipConfig(WObject *parent)
  : WObject(parent),
    autoDismiss(true),
    autoHide(true),
    dismissDelay(5000),
    hideDelay(200),
    showDelay(500),
    animate(false),
    hideOnClick(true),
    maxWidth(300),
    minWidth(40),
    trackMouse(false)
{ }

void ToolTipConfig::createConfig(std::ostream& config)
{
  if (autoDismiss != true)
    config << ",autoDismiss:false";
  if (autoHide != true)
    config << ",autoHide:false";
  if (dismissDelay != 5000)
    config << ",autoDismissDelay:" << dismissDelay;
  if (hideDelay != 200)
    config << ",hideDelay:" << hideDelay;
  if (showDelay != 500)
    config << ",showDelay:" << showDelay;
  if (animate != false)
    config << ",animate:true";
  if (hideOnClick != true)
    config << ",hideOnClick:false";
  if (maxWidth != 300)
    config << ",maxWidth:" << maxWidth;
  if (minWidth != 40)
    config << ",minWidth:" << minWidth;
  if (trackMouse != false)
    config << ",trackMouse:true";
}

  }
}
