/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WCssStyleSheet.h>
#include <Wt/WText.h>

using namespace Wt;
using namespace Selenium;

namespace {
  class WCssStyleSheetTestApp : public WApplication
  {
  public:
    WCssStyleSheetTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      doJavaScript(
        "(function() {"
        "const wtStyle = document.getElementById('Wt-inline-css');"
        "const foreignStyle = document.createElement('style');"
        "foreignStyle.id = 'not-wt';"
        "foreignStyle.media = 'not all';"
        "wtStyle.parentNode.insertBefore(foreignStyle, wtStyle);"
        "}());",
        false);

      auto target = root()->addWidget(std::make_unique<WText>("Target"));
      target->setId("style-sheet-target");
      styleSheet().addRule("#style-sheet-target", "font-size: 37px;");
    }
  };
}

BOOST_AUTO_TEST_SUITE(selenium_css_style_sheet)

SELENIUM_TEST(dynamic_rule_uses_wt_inline_style_sheet, WCssStyleSheetTestApp)
  auto target = api.getElement(SeleniumAPI::FindBy::ID, "style-sheet-target");
  BOOST_REQUIRE(target.has_value());
  BOOST_TEST(target->getCssValue("font-size") == "37px");
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
