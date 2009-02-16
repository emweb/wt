// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EVENTDISPLAYER_H_
#define EVENTDISPLAYER_H_

#include <Wt/WContainerWidget>
#include <Wt/WSignalMapper>

namespace Wt {
  class WText;
}

class EventDisplayer: public Wt::WContainerWidget
{
public:
  EventDisplayer(WContainerWidget *parent);

  // connects the invocation of the given signal to the display of the
  // string
  void mapConnect(Wt::SignalBase &signal, const std::string& data);
  // connects the invocation of the given signal to the display of the
  // given string, concatenated with the WText parameter of the signal.
  void mapConnectWString(Wt::Signal<Wt::WString> &signal,
			 const std::string& data);

  // Print a message on the displayer
  void setStatus(const Wt::WString &msg);

private:
  Wt::WString lastEventStr_;
  int eventCount_;
  Wt::WText *text_;

  Wt::WSignalMapper<std::string> *map_;
  Wt::WSignalMapper<std::string, Wt::WString> *mapWString_;
  void showSignal(const std::string& str);
  void showWStringSignal(const std::string& str, const Wt::WString& wstr);

  void showEvent(const Wt::WString& str);
};

#endif
