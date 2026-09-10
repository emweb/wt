/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"
#include "../framework/ActionChains.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTextEdit.h>

#include <map>

using namespace Wt;
using namespace Selenium;

namespace {
  const std::string editorConfig = R"XML(
<server>
  <application-settings location="*">
    <properties>
      <property name="resourcesURL">resources/</property>
      <property name="tinyMCEVersion">6</property>
      <property name="tinyMCEBaseURL">resources/tinymce/</property>
    </properties>
  </application-settings>
</server>
)XML";

  class EditorBrowser
  {
  public:
    EditorBrowser(SeleniumAPI& api, SeleniumWait& wait)
      : api_(api)
    {
      BOOST_REQUIRE(wait.until([&] {
        return (*this)("return document.querySelector('textarea')?.ed?.initialized ? 'ready' : '';") == "ready";
      }));
      BOOST_TEST((*this)("return tinymce.majorVersion + '.' + tinymce.minorVersion;") == "6.8.4");
    }

    std::string operator()(const std::string& source)
    {
      PyObject* result = PyObject_CallMethod(api_.driver(), "execute_script", "s", source.c_str());
      if (!result) {
        PyErr_Print();
        throw std::runtime_error("Failed to execute WTextEdit test script");
      }
      std::string text = PythonInterpreter::asUTF8(result).value_or("");
      Py_DECREF(result);
      return text;
    }

  private:
    SeleniumAPI& api_;
  };

  class WTextEditTestApp : public WApplication
  {
  public:
    WTextEditTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      // Make the script probes independent of debug-only JS globals.
      doJavaScript("window.currentApp = " + javaScriptClass() + ";");
      auto container = root()->addNew<WContainerWidget>();
      container->resize(500, 300);
      auto editor = container->addNew<WTextEdit>();
      editor->focussed().connect([=] { ++counts_["focus"]; });
      editor->blurred().connect([=] { ++counts_["blur"]; });
      editor->textInput().connect([=] { ++counts_["input"]; });
      editor->keyWentDown().connect([=] { ++counts_["keydown"]; });
      editor->keyPressed().connect([=] { ++counts_["keypress"]; });
      editor->keyWentUp().connect([=] { ++counts_["keyup"]; });
      editor->enterPressed().connect([=] { ++counts_["enter"]; });
      editor->enterPressed().preventDefaultAction();
      editor->escapePressed().connect([=] { ++counts_["escape"]; });
      editor->clicked().connect([=] { ++counts_["click"]; });
      editor->doubleClicked().connect([=] { ++counts_["dblclick"]; });
      editor->mouseWheel().connect([=] { ++counts_["wheel"]; });
      auto changes = root()->addNew<WText>("0");
      changes->setId("changes");
      editor->changed().connect([=] {
        changes->setText(std::to_string(++changes_));
      });

      auto report = root()->addNew<WText>();
      report->setId("report");
      auto checkpoint = root()->addNew<WPushButton>("Report and reset");
      checkpoint->setId("checkpoint");
      checkpoint->clicked().connect([=] {
        std::string result = "checkpoint:" + std::to_string(++checkpoint_) + ";";
        for (const auto& count : counts_) {
          result += count.first + ":" + std::to_string(count.second) + ";";
        }
        report->setText(result);
        counts_.clear();
      });
    }

  private:
    std::map<std::string, int> counts_;
    int checkpoint_ = 0;
    int changes_ = 0;
  };
}

// Requires explicitly installed TinyMCE 6.8.4 assets.
BOOST_AUTO_TEST_SUITE(selenium_wtextedit, * boost::unit_test::disabled())

SELENIUM_TEST_WITH_CONFIG_FILE(wtextedit_signals, WTextEditTestApp, editorConfig)
  EditorBrowser script(api, wait);

  // Reinitialization must not install duplicate forwarding listeners.
  script("const e = document.querySelector('textarea'); e.wtObj.init(); e.wtObj.init();");
  const std::string frameId = script("return document.querySelector('textarea').id + '_ifr';");
  PyObject* switchTo = PythonInterpreter::getAttribute(api.driver(), "switch_to");
  PyObject* result = PyObject_CallMethod(switchTo, "frame", "s", frameId.c_str());
  BOOST_REQUIRE(result);
  Py_DECREF(result);
  auto body = api.getElement(SeleniumAPI::FindBy::TAG_NAME, "body");
  BOOST_REQUIRE(body.has_value());
  BOOST_REQUIRE(body->click());
  BOOST_REQUIRE(body->sendKeys("x"));
  BOOST_REQUIRE(body->sendKeys("\xEE\x80\x87")); // Selenium Keys.ENTER
  BOOST_REQUIRE(body->sendKeys("\xEE\x80\x8C")); // Selenium Keys.ESCAPE
  BOOST_TEST(body->text().find("x") != std::string::npos);

  ActionChains(api).scrollFromElement(*body, 0, 100).perform();

  result = PyObject_CallMethod(switchTo, "default_content", nullptr);
  BOOST_REQUIRE(result);
  Py_DECREF(result);
  Py_DECREF(switchTo);
  auto checkpoint = api.getElement(SeleniumAPI::FindBy::ID, "checkpoint");
  BOOST_REQUIRE(checkpoint.has_value());
  BOOST_REQUIRE(checkpoint->click());
  auto changes = api.getElement(SeleniumAPI::FindBy::ID, "changes");
  BOOST_REQUIRE(changes.has_value());
  auto report = api.getElement(SeleniumAPI::FindBy::ID, "report");
  BOOST_REQUIRE(report.has_value());
  BOOST_REQUIRE(wait.until([&] { return report->text().find("checkpoint:1;") == 0; }));
  BOOST_TEST(std::stoi(changes->text()) >= 1);
  for (const std::string signal : {"blur:1", "click:1", "enter:1", "escape:1", "focus:1", "input:1", "keydown:3", "keypress:1", "keyup:3", "wheel:1"}) {
    BOOST_TEST_CONTEXT(signal) {
      BOOST_TEST(report->text().find(";" + signal + ";") != std::string::npos);
    }
  }

  auto toolbar = api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR, ".tox-statusbar");
  BOOST_REQUIRE(toolbar.has_value());
  BOOST_REQUIRE(toolbar->click());
  BOOST_REQUIRE(checkpoint->click());
  BOOST_REQUIRE(wait.until([&] { return report->text().find("checkpoint:2;") == 0; }));
  BOOST_TEST(report->text() == "checkpoint:2;");

END_SELENIUM_TEST

SELENIUM_TEST_WITH_CONFIG_FILE(wtextedit_handler_cancellation, WTextEditTestApp, editorConfig)
  EditorBrowser script(api, wait);
  // A handler installed after initialization receives the original event
  // and textarea receiver. Its cancellation must affect the iframe event.
  BOOST_TEST(script(R"JS(
    const element = document.querySelector('textarea');
    const editor = element.ed;
    const win = editor.getWin();
    const event = new win.WheelEvent('wheel', {bubbles: true, cancelable: true});
    const original = element.onwheel;
    let calls = 0, propagated = false;
    function observe() { propagated = true; }
    win.addEventListener('wheel', observe);
    element.onwheel = function(e) {
      if (this === element && e === event) { calls++; }
      e.stopPropagation();
      return false;
    };
    const accepted = editor.getBody().dispatchEvent(event);
    element.onwheel = original;
    win.removeEventListener('wheel', observe);
    return !accepted && event.defaultPrevented && calls === 1 && !propagated ? 'ok' : 'failed';
  )JS") == "ok");

END_SELENIUM_TEST

SELENIUM_TEST_WITH_CONFIG_FILE(wtextedit_double_click, WTextEditTestApp, editorConfig)
  EditorBrowser script(api, wait);
  auto frame = api.getElement(SeleniumAPI::FindBy::CSS_SELECTOR, ".tox-edit-area iframe");
  BOOST_REQUIRE(frame.has_value());
  PyObject* switchTo = PythonInterpreter::getAttribute(api.driver(), "switch_to");
  BOOST_REQUIRE(switchTo);
  PyObject* result = PyObject_CallMethod(switchTo, "frame", "O", frame->pyObject());
  BOOST_REQUIRE(result);
  Py_DECREF(result);
  auto body = api.getElement(SeleniumAPI::FindBy::TAG_NAME, "body");
  BOOST_REQUIRE(body.has_value());
  ActionChains(api).doubleClick(*body).perform();
  result = PyObject_CallMethod(switchTo, "default_content", nullptr);
  BOOST_REQUIRE(result);
  Py_DECREF(result);
  Py_DECREF(switchTo);

  auto checkpoint = api.getElement(SeleniumAPI::FindBy::ID, "checkpoint");
  BOOST_REQUIRE(checkpoint.has_value());
  BOOST_REQUIRE(checkpoint->click());
  auto report = api.getElement(SeleniumAPI::FindBy::ID, "report");
  BOOST_REQUIRE(report.has_value());
  BOOST_REQUIRE(wait.until([&] { return report->text().find("checkpoint:1;") == 0; }));
  BOOST_TEST(report->text().find(";dblclick:1;") != std::string::npos);
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
