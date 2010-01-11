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

class WStatelessSlotImpl : public WStatelessSlot {
public:
  WStatelessSlotImpl(WObject *target, WObjectMethod method,
		     const std::string& javaScript) :
    WStatelessSlot(target, method, javaScript) { }
};

JSlot::JSlot(WWidget *parent)
  : widget_(parent),
    fid_(nextFid_++)
{
  create();
}

JSlot::JSlot(const std::string& javaScript, WWidget *parent)
  : widget_(parent),
    fid_(nextFid_++)
{
  create();
  setJavaScript(javaScript);
}

void JSlot::create()
{
  imp_ = new WStatelessSlotImpl
    (widget_, 0, widget_
     ? WApplication::instance()->javaScriptClass()
     + '.' + jsFunctionName() + "(o,e);"
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

void JSlot::setJavaScript(const std::string& js)
{
  if (widget_)
    WApplication::instance()->declareJavaScriptFunction(jsFunctionName(), js);
  else
    imp_->setJavaScript("{var f=" + js + ";f(o,e);}");
}

WStatelessSlot* JSlot::slotimp()
{
  return imp_;
}

std::string JSlot::execJs(const std::string& object, const std::string& event)
{
  return "{var o=" + object + ", e=" + event + ";" + imp_->javaScript() + "}";
}

void JSlot::exec(const std::string& object, const std::string& event)
{
  WApplication::instance()->doJavaScript(execJs(object, event));
}

}
