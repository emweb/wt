/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WDateEdit.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WPasswordEdit.h>
#include <Wt/WSlider.h>
#include <Wt/WSpinBox.h>
#include <Wt/WTimeEdit.h>
#include <Wt/Test/WTestEnvironment.h>

#include <memory>

namespace {
  struct DefaultNativeControlFixture {
    ~DefaultNativeControlFixture() {
      Wt::WWebWidget::setDefaultNativeControl(false);
    }
  };
}

using namespace Wt;

BOOST_AUTO_TEST_CASE( test_set_default_native_control )
{
  DefaultNativeControlFixture f;

  Wt::Test::WTestEnvironment environment;
  environment.setUserAgent("Chrome");
  Wt::WApplication app(environment);
  auto c = app.root();

  WWebWidget::setDefaultNativeControl(false);

  auto dateEditf = c->addNew<WDateEdit>();
  BOOST_REQUIRE(!dateEditf->nativeControl());
  auto doubleSpinBoxf = c->addNew<WDoubleSpinBox>();
  BOOST_REQUIRE(!doubleSpinBoxf->nativeControl());
  auto passwordEdit1f = c->addNew<WPasswordEdit>();
  BOOST_REQUIRE(!passwordEdit1f->nativeControl());
  auto passwordEdit2f = c->addNew<WPasswordEdit>("test");
  BOOST_REQUIRE(!passwordEdit2f->nativeControl());
  auto slider1f = c->addNew<WSlider>();
  BOOST_REQUIRE(!slider1f->nativeControl());
  auto slider2f = c->addNew<WSlider>(Orientation::Vertical);
  BOOST_REQUIRE(!slider2f->nativeControl());
  auto spinBoxf = c->addNew<WSpinBox>();
  BOOST_REQUIRE(!spinBoxf->nativeControl());
  auto timeEditf = c->addNew<WTimeEdit>();
  BOOST_REQUIRE(!timeEditf->nativeControl());

  WWebWidget::setDefaultNativeControl(true);

  auto dateEditt = c->addNew<WDateEdit>();
  BOOST_REQUIRE(dateEditt->nativeControl());
  auto doubleSpinBoxt = c->addNew<WDoubleSpinBox>();
  BOOST_REQUIRE(doubleSpinBoxt->nativeControl());
  auto passwordEdit1t = c->addNew<WPasswordEdit>();
  BOOST_REQUIRE(passwordEdit1t->nativeControl());
  auto passwordEdit2t = c->addNew<WPasswordEdit>("test");
  BOOST_REQUIRE(passwordEdit2t->nativeControl());
  auto slider1t = c->addNew<WSlider>();
  BOOST_REQUIRE(slider1t->nativeControl());
  auto slider2t = c->addNew<WSlider>(Orientation::Vertical);
  BOOST_REQUIRE(slider2t->nativeControl());
  auto spinBoxt = c->addNew<WSpinBox>();
  BOOST_REQUIRE(spinBoxt->nativeControl());
  auto timeEditt = c->addNew<WTimeEdit>();
  BOOST_REQUIRE(timeEditt->nativeControl());

  // Check that changing the default did not change the existing widget
  BOOST_REQUIRE(!dateEditf->nativeControl());
  BOOST_REQUIRE(!doubleSpinBoxf->nativeControl());
  BOOST_REQUIRE(!passwordEdit1f->nativeControl());
  BOOST_REQUIRE(!passwordEdit2f->nativeControl());
  BOOST_REQUIRE(!slider1f->nativeControl());
  BOOST_REQUIRE(!slider2f->nativeControl());
  BOOST_REQUIRE(!spinBoxf->nativeControl());
  BOOST_REQUIRE(!timeEditf->nativeControl());
}
