/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "StyleLayout.h"
#include "EventDisplayer.h"

#include <Wt/WApplication.h>

#include <Wt/WText.h>
#include <Wt/WComboBox.h>
#include <Wt/WPushButton.h>

#include <Wt/WDefaultLoadingIndicator.h>
#include <Wt/WOverlayLoadingIndicator.h>

#include "EmwebLoadingIndicator.h"

#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
#include <thread>
#else
#if WT_WIN32
#include <windows.h>
#endif
#endif

StyleLayout::StyleLayout(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  this->addWidget(std::move(addText(tr("style-layout-intro"))));
}

void StyleLayout::populateSubMenu(WMenu *menu)
{
  menu->addItem("CSS", std::move(css()));
  menu->addItem("WLoadingIndicator", std::move(wLoadingIndicator()));
}

std::unique_ptr<WWidget> StyleLayout::css()
{
  return std::move(addText(tr("style-and-layout-css")));
}

std::unique_ptr<WWidget> StyleLayout::wLoadingIndicator()
{
  auto result = cpp14::make_unique<WContainerWidget>();
  topic("WLoadingIndicator", result);

  result->addWidget(std::move(addText(tr("style-WLoadingIndicator"))));

  //fix for the WOverlayLoadingIndicator
  WApplication::instance()->styleSheet().addRule("body", "margin: 0px");

  result->addWidget(std::move(addText("Select a loading indicator:  ")));
  WComboBox *cb = result->addWidget(cpp14::make_unique<WComboBox>());
  cb->addItem("WDefaultLoadingIndicator");
  cb->addItem("WOverlayLoadingIndicator");
  cb->addItem("EmwebLoadingIndicator");
  cb->setCurrentIndex(0);
  cb->sactivated().connect(this, &StyleLayout::loadingIndicatorSelected);
  result->addWidget(cpp14::make_unique<WBreak>());
  WPushButton *load =
      result->addWidget(cpp14::make_unique<WPushButton>("Load!"));
  load->clicked().connect(this, &StyleLayout::load);

  return result;
}

void StyleLayout::loadingIndicatorSelected(WString indicator)
{
  if (indicator.toUTF8() == "WDefaultLoadingIndicator") {
    WApplication::instance()
      ->setLoadingIndicator(cpp14::make_unique<WDefaultLoadingIndicator>());
  } else if (indicator.toUTF8() == "WOverlayLoadingIndicator") {
    WApplication::instance()
      ->setLoadingIndicator(cpp14::make_unique<WOverlayLoadingIndicator>());
  } else if (indicator.toUTF8() == "EmwebLoadingIndicator") {
    WApplication::instance()
      ->setLoadingIndicator(cpp14::make_unique<EmwebLoadingIndicator>());
  }
}

void StyleLayout::load(WMouseEvent) {
#if defined(WT_THREADED) || defined(WT_TARGET_JAVA)
  std::this_thread::sleep_for(std::chrono::duration(2s));
#else
#ifdef WT_WIN32
        Sleep(2000);
#else
        sleep(2);
#endif //WIN32
#endif // WT_THREADED
}
