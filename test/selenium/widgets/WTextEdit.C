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
      editor->mouseWentDown().connect([=] { ++counts_["mousedown"]; });
      editor->mouseMoved().connect([=] { ++counts_["mousemove"]; });
      editor->mouseWentUp().connect([=] { ++counts_["mouseup"]; });
      auto recordTouch = [=](const std::string& type, const WTouchEvent& event) {
        ++counts_[type];
        int list = 0;
        for (const auto *touches : {&event.touches(), &event.targetTouches(), &event.changedTouches()}) {
          for (const auto& touch : *touches) {
            coordinates_ += type + "/" + std::to_string(list) + "/" + std::to_string(touch.identifier());
            for (const auto& point : {touch.document(), touch.window(), touch.widget(), touch.screen()}) {
              coordinates_ += "," + std::to_string(point.x) + "," + std::to_string(point.y);
            }
            coordinates_ += ";";
          }
          ++list;
        }
      };
      editor->touchStarted().connect([=](const WTouchEvent& event) { recordTouch("touchstart", event); });
      editor->touchMoved().connect([=](const WTouchEvent& event) { recordTouch("touchmove", event); });
      editor->touchEnded().connect([=](const WTouchEvent& event) { recordTouch("touchend", event); });
      auto changes = root()->addNew<WText>("0");
      changes->setId("changes");
      editor->changed().connect([=] {
        changes->setText(std::to_string(++changes_));
      });

      auto report = root()->addNew<WText>();
      report->setId("report");
      auto coordinates = root()->addNew<WText>();
      coordinates->setId("coordinates");
      auto checkpoint = root()->addNew<WPushButton>("Report and reset");
      checkpoint->setId("checkpoint");
      checkpoint->clicked().connect([=] {
        std::string result = "checkpoint:" + std::to_string(++checkpoint_) + ";";
        for (const auto& count : counts_) {
          result += count.first + ":" + std::to_string(count.second) + ";";
        }
        report->setText(result);
        counts_.clear();
        coordinates->setText(coordinates_);
        coordinates_.clear();
      });
    }

  private:
    std::map<std::string, int> counts_;
    std::string coordinates_;
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

SELENIUM_TEST_WITH_CONFIG_FILE(wtextedit_ancestor_capture, WTextEditTestApp, editorConfig)
  EditorBrowser script(api, wait);
  // An ancestor capturing an iframe event still needs explicit delivery:
  // forwarding to the textarea does not bubble through its outer parents.
  BOOST_TEST(script(R"JS(
    const element = document.querySelector('textarea');
    const win = element.ed.getWin();
    const child = element.ed.getBody().querySelector('p');
    const parent = element.parentElement;
    const WT = window.currentApp.WT;
    let calls = 0;
    for (const [move, end, EventType] of [
      ['mousemove', 'mouseup', win.MouseEvent], ['touchmove', 'touchend', win.TouchEvent]
    ]) {
      parent['on' + move] = () => { calls++; };
      parent['on' + end] = () => { calls++; };
      WT.capture(parent);
      child.dispatchEvent(new EventType(move, {bubbles: true}));
      child.dispatchEvent(new EventType(end, {bubbles: true}));
      document.body.dispatchEvent(new EventType(move, {bubbles: true}));
      delete parent['on' + move];
      delete parent['on' + end];
    }
    return calls === 4 ? 'ok' : String(calls);
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

SELENIUM_TEST_WITH_CONFIG_FILE(wtextedit_capture_coordinates, WTextEditTestApp, editorConfig)
  EditorBrowser script(api, wait);
  auto report = api.getElement(SeleniumAPI::FindBy::ID, "report");
  BOOST_REQUIRE(report.has_value());
  // Capture must deliver each move once, both inside and outside the
  // editor. Scroll both documents to distinguish all coordinate spaces.
  script(R"JS(
    const element = document.querySelector('textarea');
    element.parentElement.style.margin = '120px 100px';
    document.body.style.width = '1800px';
    document.body.style.height = '1500px';
    const child = element.ed.getBody().querySelector('p');
    child.style.width = '1000px';
    child.style.height = '1000px';
    element.ed.getWin().scrollTo(7, 11);
    window.scrollTo(31, 47);
    const frame = document.createElement('iframe');
    frame.id = 'other-frame';
    frame.style.cssText = 'position:absolute;left:900px;top:120px;width:200px;height:200px';
    document.body.appendChild(frame);
  )JS");
  BOOST_REQUIRE(script(R"JS(
    const editor = document.querySelector('textarea').ed;
    const rect = editor.getWin().frameElement.getBoundingClientRect();
    return rect.left > 0 && rect.top > 0 && scrollX === 31 && scrollY === 47 &&
      editor.getWin().scrollX === 7 && editor.getWin().scrollY === 11 ? 'ok' : 'failed';
  )JS") == "ok");
  auto coordinates = api.getElement(SeleniumAPI::FindBy::ID, "coordinates");
  BOOST_REQUIRE(coordinates.has_value());
  int checkpointNumber = 0;
  for (const std::string location : {"editor", "document", "iframe"}) {
    script("window.moveLocation = '" + location + "';");
    script(R"JS(
      const editor = document.querySelector('textarea').ed;
      const child = editor.getBody().querySelector('p');
      const target = moveLocation === 'editor' ? child : moveLocation === 'document' ?
        document.body : document.getElementById('other-frame').contentDocument.body;
      for (const type of ['mousedown', 'mousemove', 'mouseup']) {
        const node = type === 'mousedown' ? child : target;
        const win = node.ownerDocument.defaultView;
        node.dispatchEvent(new win.MouseEvent(type, {
          bubbles: true, cancelable: true, clientX: 20, clientY: 30,
          button: 0, buttons: type === 'mouseup' ? 0 : 1, view: win
        }));
      }
      document.getElementById('checkpoint').click();
    )JS");
    const std::string mouseCheckpoint = "checkpoint:" + std::to_string(++checkpointNumber) + ";";
    BOOST_REQUIRE(wait.until([&] { return report->text().find(mouseCheckpoint) == 0; }));
    BOOST_TEST_CONTEXT("mouse capture moving in " << location) {
      BOOST_TEST(report->text() == mouseCheckpoint + "mousedown:1;mousemove:1;mouseup:1;");
    }

    // Release restores ordinary delivery: outside moves are ignored,
    // while an uncaptured editor move still reaches its handler once.
    script(R"JS(
      const editor = document.querySelector('textarea').ed;
      const win = editor.getWin();
      document.body.dispatchEvent(new MouseEvent('mousemove', {bubbles: true}));
      editor.getBody().querySelector('p').dispatchEvent(new win.MouseEvent('mousemove', {bubbles: true}));
      document.getElementById('checkpoint').click();
    )JS");
    const std::string moveCheckpoint = "checkpoint:" + std::to_string(++checkpointNumber) + ";";
    BOOST_REQUIRE(wait.until([&] { return report->text().find(moveCheckpoint) == 0; }));
    BOOST_TEST(report->text() == moveCheckpoint + "mousemove:1;");

    const std::string expectedCoordinates = script(R"JS(
      const element = document.querySelector('textarea');
      const editor = element.ed;
      const child = editor.getBody().querySelector('p');
      const target = moveLocation === 'editor' ? child : moveLocation === 'document' ?
        document.body : document.getElementById('other-frame').contentDocument.body;
      const widget = element.getBoundingClientRect();
      // Establish capture explicitly for this synthetic touch sequence.
      window.currentApp.WT.capture(element);
      let expected = '';
      for (const type of ['touchstart', 'touchmove', 'touchend']) {
        const node = type === 'touchstart' ? child : target;
        const win = node.ownerDocument.defaultView;
        const frame = win.frameElement;
        const rect = frame ? frame.getBoundingClientRect() : {left: 0, top: 0};
        const touches = [0, 1].map(i => new win.Touch({
          identifier: i + 1, target: node, clientX: 20 + i * 10, clientY: 30 + i * 10,
          pageX: 20 + i * 10 + win.scrollX, pageY: 30 + i * 10 + win.scrollY,
          screenX: 320 + i * 10, screenY: 230 + i * 10
        }));
        const active = type === 'touchend' ? [] : touches;
        for (const [list, points] of [active, active, touches].entries()) {
          for (const t of points) {
            const x = t.clientX + rect.left + (frame ? frame.clientLeft : 0);
            const y = t.clientY + rect.top + (frame ? frame.clientTop : 0);
            expected += type + '/' + list + '/' + t.identifier + ',' + [
              x + scrollX, y + scrollY, x, y, x - widget.left, y - widget.top,
              t.screenX, t.screenY
            ].map(Math.round).join(',') + ';';
          }
        }
        node.dispatchEvent(new win.TouchEvent(type, {
          bubbles: true, cancelable: true, touches: active, targetTouches: active,
          changedTouches: touches, view: win
        }));
      }
      document.body.dispatchEvent(new TouchEvent('touchmove', {bubbles: true}));
      document.getElementById('checkpoint').click();
      return expected;
    )JS");
    const std::string touchCheckpoint = "checkpoint:" + std::to_string(++checkpointNumber) + ";";
    BOOST_REQUIRE(wait.until([&] { return report->text().find(touchCheckpoint) == 0; }));
    BOOST_TEST_CONTEXT("touch capture moving in " << location) {
      BOOST_TEST(report->text() == touchCheckpoint + "touchend:1;touchmove:1;touchstart:1;");
      BOOST_TEST(coordinates->text() == expectedCoordinates);
    }
  }

END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
