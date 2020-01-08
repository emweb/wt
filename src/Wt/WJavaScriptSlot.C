/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WException.h"
#include "Wt/WJavaScriptSlot.h"
#include "Wt/WStatelessSlot.h"
#include "Wt/WWidget.h"

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
    fid_(nextFid_++),
    nbArgs_(0)
{
  create();
}

JSlot::JSlot(const std::string& javaScript, WWidget *parent)
  : widget_(parent),
    fid_(nextFid_++),
    nbArgs_(0)
{
  create();
  setJavaScript(javaScript);
}

JSlot::JSlot(int nbArgs, WWidget *parent)
  : widget_(parent),
    fid_(nextFid_++),
    nbArgs_(nbArgs)
{
  if (nbArgs_ < 0 || nbArgs_ > 6) {
    throw WException("The number of arguments given must be between 0 and 6.");
  }
  create();
}

JSlot::JSlot(const std::string& javaScript, int nbArgs, WWidget *parent)
  : widget_(parent),
    fid_(nextFid_++),
    nbArgs_(nbArgs)
{
  if (nbArgs_ < 0 || nbArgs_ > 6) {
    throw WException("The number of arguments given must be between 0 and 6.");
  }
  create();
  setJavaScript(javaScript, nbArgs_);
}

void JSlot::create()
{
  std::stringstream ss;

  if (widget_) {
    WApplication *app = WApplication::instance();
    if (app) {
      ss << WApplication::instance()->javaScriptClass() << "."
	 << jsFunctionName() << "(o,e";
      for (int i = 1; i <= nbArgs_; ++i) {
	ss << ",a" << i;
      }
      ss << ");";
    }
  }

  imp_ = new WStatelessSlotImpl(widget_, nullptr, ss.str());
}

JSlot::~JSlot()
{
  delete imp_;
}

std::string JSlot::jsFunctionName() const
{
  return "sf" + std::to_string(fid_);
}

void JSlot::setJavaScript(const std::string& js, int nbArgs)
{
  if (nbArgs < 0 || nbArgs > 6) {
    throw WException("The number of arguments given must be between 0 and 6.");
  }
  nbArgs_ = nbArgs;
  WApplication *app = WApplication::instance();
  if (widget_ && app)
    WApplication::instance()->declareJavaScriptFunction(jsFunctionName(), js);
  else {
    std::stringstream ss;
    ss << "{var f=" << js << ";f(o,e";
    for (int i = 1; i <= nbArgs; ++i) {
      ss << ",a" << i;
    }
    ss << ");}";
    imp_->setJavaScript(ss.str());
  }
}

WStatelessSlot* JSlot::slotimp()
{
  return imp_;
}

std::string JSlot::execJs(const std::string& object,
			  const std::string& event,
			  const std::string& arg1,
			  const std::string& arg2,
			  const std::string& arg3,
			  const std::string& arg4,
			  const std::string& arg5,
			  const std::string& arg6)
{
  std::stringstream result;
  result << "{var o=" << object;
  result << ",e=" << event;
  for (int i = 0; i < nbArgs_; ++i) {
    result << ",a" << (i+1) << "=";
    switch (i) {
      case 0:
	result << arg1;
	break;
      case 1:
	result << arg2;
	break;
      case 2:
	result << arg3;
	break;
      case 3:
	result << arg4;
	break;
      case 4:
	result << arg5;
	break;
      case 5:
	result << arg6;
	break;
    }
  }
  result << ";" << imp_->javaScript() + "}";
  return result.str();
}

void JSlot::exec(const std::string& object,
		 const std::string& event,
		 const std::string& arg1,
		 const std::string& arg2,
		 const std::string& arg3,
		 const std::string& arg4,
		 const std::string& arg5,
		 const std::string& arg6)
{
  WApplication::instance()->doJavaScript(execJs(object, event, arg1, arg2, arg3, arg4, arg5, arg6));
}

#ifndef WT_TARGET_JAVA
void JSlot::disconnectFrom(EventSignalBase *signal)
{
  imp_->disconnectFrom(signal);
}
#endif

}
