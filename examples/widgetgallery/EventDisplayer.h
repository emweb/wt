// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EVENTDISPLAYER_H_
#define EVENTDISPLAYER_H_

#include <Wt/WContainerWidget>

namespace Wt {
  class WText;
}

class EventDisplayer: public Wt::WContainerWidget
{
public:
  EventDisplayer(WContainerWidget *parent);

  // Print a message on the displayer
  void setStatus(const Wt::WString& msg);

  void showSignal(Wt::Signal<Wt::WString>& s, const std::string& data);
  template<typename T> void showSignal(T &s, const std::string& str);
  template<typename T> void showEvent(T &s, const Wt::WString& str);

private:
  Wt::WString lastEventStr_;
  int eventCount_;
  Wt::WText *text_;

  void showWStringSignal(const std::string& str, const Wt::WString& wstr);
  void showSignalImpl(const std::string& str);
  void showEventImpl(const Wt::WString& str);
};

#ifndef WT_TARGET_JAVA
template<typename T>
void EventDisplayer::showSignal(T &s, const std::string& str)
{
  s.connect(boost::bind(&EventDisplayer::showSignalImpl, this, str));
}

template<typename T>
void EventDisplayer::showEvent(T &s, const Wt::WString& str)
{
  s.connect(boost::bind(&EventDisplayer::showEventImpl, this, str));
}
#endif

#endif
