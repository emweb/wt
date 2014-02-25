/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "StyleLayout.h"
#include "EventDisplayer.h"

#include <Wt/WApplication>

#include <Wt/WText>
#include <Wt/WComboBox>
#include <Wt/WPushButton>

#include <Wt/WDefaultLoadingIndicator>
#include <Wt/WOverlayLoadingIndicator>

#include "EmwebLoadingIndicator.h"

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
#include <boost/thread.hpp>
#else
#if WT_WIN32
#include <windows.h>
#endif
#endif

StyleLayout::StyleLayout(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  addText(tr("style-layout-intro"), this);
}

void StyleLayout::populateSubMenu(WMenu *menu)
{
  menu->addItem("CSS", css());
  menu->addItem("WLoadingIndicator", wLoadingIndicator());
}

WWidget *StyleLayout::css()
{
  return addText(tr("style-and-layout-css"));
}

WWidget *StyleLayout::wLoadingIndicator()
{
  WContainerWidget *result = new WContainerWidget();
  topic("WLoadingIndicator", result);

  addText(tr("style-WLoadingIndicator"), result);

  //fix for the WOverlayLoadingIndicator
  WApplication::instance()->styleSheet().addRule("body", "margin: 0px");

  addText("Select a loading indicator:  ", result);
  WComboBox *cb = new WComboBox(result);
  cb->addItem("WDefaultLoadingIndicator");
  cb->addItem("WOverlayLoadingIndicator");
  cb->addItem("EmwebLoadingIndicator");
  cb->setCurrentIndex(0);
  cb->sactivated().connect(this, &StyleLayout::loadingIndicatorSelected);
  new WBreak(result);
  WPushButton *load = new WPushButton("Load!", result);
  load->clicked().connect(this, &StyleLayout::load);

  return result;
}

void StyleLayout::loadingIndicatorSelected(WString indicator)
{
  if (indicator.toUTF8() == "WDefaultLoadingIndicator") {
    WApplication::instance()
      ->setLoadingIndicator(new WDefaultLoadingIndicator());
  } else if (indicator.toUTF8() == "WOverlayLoadingIndicator") {
    WApplication::instance()
      ->setLoadingIndicator(new WOverlayLoadingIndicator());
  } else if (indicator.toUTF8() == "EmwebLoadingIndicator") {
    WApplication::instance()
      ->setLoadingIndicator(new EmwebLoadingIndicator());
  }
}

void StyleLayout::load(Wt::WMouseEvent) {
#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
  boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
#else
#ifdef WT_WIN32
        Sleep(2000);
#else
        sleep(2);
#endif //WIN32
#endif // WT_THREADED
}
