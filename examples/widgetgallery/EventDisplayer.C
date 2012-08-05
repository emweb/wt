#include "EventDisplayer.h"
#include <Wt/WText>

using namespace Wt;

EventDisplayer::EventDisplayer(WContainerWidget *parent):
  WContainerWidget(parent),
  eventCount_(0),
  text_(new WText("Events will be shown here.", this))
{
  setStyleClass("events");
}

void EventDisplayer::showSignal(Signal<WString>& signal,
				const std::string& data)
{
  signal.connect(boost::bind(&EventDisplayer::showWStringSignal,
			     this, data, _1));
}

void EventDisplayer::showSignalImpl(const std::string& str)
{
  showEventImpl("Last activated signal: " + str);
}

void EventDisplayer::showWStringSignal(const std::string& str,
				       const WString& wstr)
{
  showEventImpl("Last activated signal: " + str + wstr);
}

void EventDisplayer::setStatus(const WString &msg)
{
  showEventImpl("Last status message: " + msg);
}

void EventDisplayer::showEventImpl(const WString& str)
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
