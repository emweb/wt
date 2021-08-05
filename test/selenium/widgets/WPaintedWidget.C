/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WBrush.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>

#include <chrono>
#include <thread>

using namespace Wt;
using namespace Selenium;

namespace {
  class TestPaintedWidget : public WPaintedWidget
  {
  public:
    TestPaintedWidget()
      : paintUpdate_(false),
        blueVisible_(false)
    {
      setId("painted-widget");
      setPreferredMethod(RenderMethod::HtmlCanvas);
      resize(120, 60);
    }

    void addPaintUpdate()
    {
      blueVisible_ = true;
      paintUpdate_ = true;
      update(PaintFlag::Update);
    }

    void resizeWithPaintUpdate(int width, int height)
    {
      resize(width, height);
      paintUpdate_ = true;
      update(PaintFlag::Update);
    }

  protected:
    void paintEvent(WPaintDevice *paintDevice) override
    {
      WPainter painter(paintDevice);
      if (paintUpdate_) {
        painter.setBrush(WColor(StandardColor::Blue));
        painter.drawRect(20, 0, 10, 10);
        paintUpdate_ = false;
      } else {
        painter.setBrush(WColor(StandardColor::Red));
        painter.drawRect(0, 0, 10, 10);
        if (blueVisible_) {
          painter.setBrush(WColor(StandardColor::Blue));
          painter.drawRect(20, 0, 10, 10);
        }
      }
    }

  private:
    bool paintUpdate_;
    bool blueVisible_;
  };

  class WPaintedWidgetTestApp : public WApplication
  {
  public:
    WPaintedWidgetTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      paintedWidget_ = root()->addWidget(std::make_unique<TestPaintedWidget>());
    }

    TestPaintedWidget *paintedWidget() { return paintedWidget_; }

  private:
    TestPaintedWidget *paintedWidget_;
  };

  const std::string canvasIsDprAware = R"JS(
    const canvas = document.getElementById("cpainted-widget");
    const rect = canvas.getBoundingClientRect();
    const transform = canvas.getContext("2d").getTransform();
    const scaleX = canvas.width / canvas.wtWidth;
    const scaleY = canvas.height / canvas.wtHeight;
    const paintedPixel = canvas.getContext("2d").getImageData(5 * scaleX, 5 * scaleY, 1, 1).data;
    const clearPixel = canvas.getContext("2d").getImageData(15 * scaleX, 5 * scaleY, 1, 1).data;
    return rect.width === canvas.wtWidth && rect.height === canvas.wtHeight &&
      canvas.width === Math.round(canvas.wtWidth * window.devicePixelRatio) &&
      canvas.height === Math.round(canvas.wtHeight * window.devicePixelRatio) &&
      transform.a === scaleX && transform.d === scaleY &&
      paintedPixel[0] === 255 && paintedPixel[3] === 255 && clearPixel[3] === 0;
  )JS";

  const std::string paintUpdateIsVisible = R"JS(
    const canvas = document.getElementById("cpainted-widget");
    const scaleX = canvas.width / canvas.wtWidth;
    const scaleY = canvas.height / canvas.wtHeight;
    const pixel = canvas.getContext("2d").getImageData(25 * scaleX, 5 * scaleY, 1, 1).data;
    return pixel[2] === 255 && pixel[3] === 255;
  )JS";
}

BOOST_AUTO_TEST_SUITE(selenium_wpaintedwidget)

SELENIUM_TEST(wpaintedwidget_device_pixel_ratio, WPaintedWidgetTestApp)
  auto waitForScript = [&api](const std::string& script) {
    for (int i = 0; i < 50; ++i) {
      if (api.executeScriptBool(script)) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return false;
  };

  BOOST_REQUIRE(api.executeScriptBool(R"JS(
    Object.defineProperty(window, "devicePixelRatio", {value: 2, configurable: true});
    window.dispatchEvent(new Event("resize"));
    return true;
  )JS"));
  BOOST_REQUIRE(waitForScript(canvasIsDprAware));

  BOOST_REQUIRE(api.executeScriptBool(R"JS(
    const canvas = document.getElementById("cpainted-widget");
    canvas.width = canvas.wtWidth;
    canvas.height = canvas.wtHeight;
    document.getElementById("painted-widget").wtObj.repaint();
    return true;
  )JS"));
  BOOST_REQUIRE(api.executeScriptBool(canvasIsDprAware));

  updateApplication([](WPaintedWidgetTestApp *app) {
    app->paintedWidget()->addPaintUpdate();
  });
  BOOST_REQUIRE(waitForScript(paintUpdateIsVisible));

  BOOST_REQUIRE(api.executeScriptBool(R"JS(
    Object.defineProperty(window, "devicePixelRatio", {value: 1.5, configurable: true});
    window.dispatchEvent(new Event("resize"));
    return true;
  )JS"));
  BOOST_REQUIRE(waitForScript(canvasIsDprAware));
  BOOST_REQUIRE(waitForScript(paintUpdateIsVisible));

  updateApplication([](WPaintedWidgetTestApp *app) {
    app->paintedWidget()->resizeWithPaintUpdate(180, 90);
  });

  bool resized = false;
  for (int i = 0; i < 50 && !resized; ++i) {
    resized = api.executeScriptBool(R"JS(
      const canvas = document.getElementById("cpainted-widget");
      return canvas.wtWidth === 180 && canvas.wtHeight === 90;
    )JS");
    if (!resized) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  BOOST_REQUIRE(resized);
  BOOST_REQUIRE(api.executeScriptBool(canvasIsDprAware));
  BOOST_REQUIRE(api.executeScriptBool(paintUpdateIsVisible));
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
