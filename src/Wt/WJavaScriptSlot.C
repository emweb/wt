/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WJavaScriptSlot"
#include "Wt/WStatelessSlot"
#include "Wt/WWidget"
#include "Wt/WWebWidget"

namespace Wt {

int JSlot::nextFid_ = 0;

JSlot::JSlot(WWidget *parent)
  : widget_(parent),
    fid_(nextFid_++)
{
  imp_ = new WStatelessSlot
    (parent, 0, widget_
     ? WApplication::instance()->javaScriptClass()
     + '.' + jsFunctionName() + "(this, e);"
     : "");
}

JSlot::~JSlot()
{
  delete imp_;
}

std::string JSlot::jsFunctionName() const
{
  return "sf" + boost::lexical_cast<std::string>(fid_);
}

void JSlot::setJavaScript(const std::string& imp)
{
  if (widget_)
    WApplication::instance()->declareJavaScriptFunction(jsFunctionName(), imp);
  else
    imp_->setJavaScript("{var f=" + imp	+ "; f(this, e);}");
}

WStatelessSlot* JSlot::slotimp()
{
  return imp_;
}

void JSlot::exec()
{
  WApplication::instance()->doJavaScript(imp_->javaScript());
}

}
