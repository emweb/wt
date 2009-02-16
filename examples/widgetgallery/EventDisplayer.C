#include "EventDisplayer.h"
#include <Wt/WText>

using namespace Wt;

EventDisplayer::EventDisplayer(WContainerWidget *parent):
  WContainerWidget(parent),
  text_(new WText("Events will be shown here.", this)),
  map_(new WSignalMapper<std::string>(this)),
  mapWString_(new WSignalMapper<std::string, WString>(this))
{
  setStyleClass("events");

  map_->mapped().connect(SLOT(this, EventDisplayer::showSignal));
  mapWString_->mapped().connect(SLOT(this, EventDisplayer::showWStringSignal));
}

void EventDisplayer::mapConnect(Wt::SignalBase &signal, const std::string& data)
{
  map_->mapConnect(signal, data);
}

void EventDisplayer::mapConnectWString(Signal<WString> &signal,
				       const std::string& data)
{
  mapWString_->mapConnect1(signal, data);
}

void EventDisplayer::showSignal(const std::string& str)
{
  showEvent("Last activated signal: " + str);
}

void EventDisplayer::showWStringSignal(const std::string& str,
				       const WString& wstr)
{
  showEvent("Last activated signal: " + str + wstr);
}

void EventDisplayer::setStatus(const WString &msg)
{
  showEvent("Last status message: " + msg);
}

void EventDisplayer::showEvent(const WString& str)
{
  if (str == lastEventStr_) {
    ++eventCount_;
    text_->setText(str + " (" + boost::lexical_cast<std::string>(eventCount_)
		   + " times)");
  } else {
    lastEventStr_ = str;
    eventCount_ = 1;
    text_->setText(str);
  }
}
