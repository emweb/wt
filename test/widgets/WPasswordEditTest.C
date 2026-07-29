/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>

#include <web/DomElement.h>

#include <Wt/WContainerWidget.h>
#include <Wt/WPasswordEdit.h>

namespace {

// Exposes the protected updateDom() so a partial update can be driven
// directly, since the test environment only renders full elements.
struct TestPasswordEdit final : Wt::WPasswordEdit {
  using Wt::WPasswordEdit::updateDom;
};

}

BOOST_AUTO_TEST_CASE( password_edit_default_type_render )
{
  // Tests that a password edit renders as a password input by default
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<Wt::WPasswordEdit>();

  BOOST_TEST(!edit->isTextVisible());

  auto domElement = edit->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("type") == "password");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( password_edit_text_visible_render )
{
  // Tests that a password edit with visible text renders as a text input
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<Wt::WPasswordEdit>();
  edit->setTextVisible(true);

  BOOST_TEST(edit->isTextVisible());

  auto domElement = edit->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("type") == "text");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( password_edit_text_visible_partial_update )
{
  // Tests that toggling the text visibility after the initial render
  // updates the type attribute, and only then
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<TestPasswordEdit>();

  delete edit->createSDomElement(&app);

  auto update = Wt::DomElement::getForUpdate(edit, Wt::DomElementType::INPUT);
  edit->updateDom(*update, false);
  BOOST_TEST(update->getAttribute("type") == "");
  delete update;

  edit->setTextVisible(true);

  update = Wt::DomElement::getForUpdate(edit, Wt::DomElementType::INPUT);
  edit->updateDom(*update, false);
  BOOST_TEST(update->getAttribute("type") == "text");
  delete update;

  edit->setTextVisible(false);

  update = Wt::DomElement::getForUpdate(edit, Wt::DomElementType::INPUT);
  edit->updateDom(*update, false);
  BOOST_TEST(update->getAttribute("type") == "password");
  delete update;

  edit->setTextVisible(false);

  update = Wt::DomElement::getForUpdate(edit, Wt::DomElementType::INPUT);
  edit->updateDom(*update, false);
  BOOST_TEST(update->getAttribute("type") == "");
  delete update;
}

BOOST_AUTO_TEST_CASE( password_edit_text_visible_full_update_consumes_change )
{
  // Tests that a full DOM update consumes a pending visibility change
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<TestPasswordEdit>();
  edit->setTextVisible(true);

  auto full = Wt::DomElement::createNew(Wt::DomElementType::INPUT);
  edit->updateDom(*full, true);
  BOOST_TEST(full->getAttribute("type") == "text");
  delete full;

  auto update = Wt::DomElement::getForUpdate(edit, Wt::DomElementType::INPUT);
  edit->updateDom(*update, false);
  BOOST_TEST(update->getAttribute("type") == "");
  delete update;

  update = Wt::DomElement::getForUpdate(edit, Wt::DomElementType::INPUT);
  edit->updateDom(*update, false);
  BOOST_TEST(update->getAttribute("type") == "");
  delete update;
}

BOOST_AUTO_TEST_CASE( password_edit_text_visible_keeps_validation )
{
  // Tests that revealing the text does not change validation
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<Wt::WPasswordEdit>();
  edit->setTextVisible(true);

  BOOST_REQUIRE(edit->validate() != Wt::ValidationState::Valid);

  edit->setText("secret");
  BOOST_REQUIRE(edit->validate() == Wt::ValidationState::Valid);

  edit->setMinLength(10);
  BOOST_REQUIRE(edit->validate() != Wt::ValidationState::Valid);

  edit->setNativeControl(true);
  BOOST_REQUIRE(edit->validate() != Wt::ValidationState::Valid);
}

BOOST_AUTO_TEST_CASE( password_edit_reveal_button_style_class )
{
  // Tests that enabling the reveal button toggles the style class that
  // themes use to show the button
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<Wt::WPasswordEdit>();

  BOOST_TEST(!edit->revealButtonEnabled());
  BOOST_TEST(!edit->hasStyleClass("Wt-passwordedit-reveal"));

  edit->setRevealButtonEnabled(true);

  BOOST_TEST(edit->revealButtonEnabled());
  BOOST_TEST(edit->hasStyleClass("Wt-passwordedit-reveal"));

  edit->setRevealButtonEnabled(false);

  BOOST_TEST(!edit->revealButtonEnabled());
  BOOST_TEST(!edit->hasStyleClass("Wt-passwordedit-reveal"));
}

BOOST_AUTO_TEST_CASE( password_edit_visibility_signal_connection_mode )
{
  // Tests that visibility updates are only sent immediately when requested
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto deferred = app.root()->addNew<Wt::WPasswordEdit>();
  deferred->setRevealButtonEnabled(true);
  delete deferred->createSDomElement(&app);
  BOOST_TEST(deferred->javaScriptMember(" WPasswordEdit").find(",false);") !=
             std::string::npos);

  auto immediate = app.root()->addNew<Wt::WPasswordEdit>();
  immediate->textVisibilityChanged().connect([](bool) {});
  immediate->setRevealButtonEnabled(true);
  delete immediate->createSDomElement(&app);
  BOOST_TEST(immediate->javaScriptMember(" WPasswordEdit").find(",true);") !=
             std::string::npos);
}

BOOST_AUTO_TEST_CASE( password_edit_visibility_signal_updates_state_first )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<TestPasswordEdit>();
  edit->setText("secret");
  delete edit->createSDomElement(&app);

  int calls = 0;
  edit->textVisibilityChanged().connect([&](bool visible) {
    BOOST_TEST(edit->isTextVisible() == visible);
    BOOST_TEST(edit->text() == "secret");
    ++calls;
  });

  edit->textVisibilityChanged().emit(true);
  BOOST_TEST(edit->isTextVisible());

  auto update = Wt::DomElement::getForUpdate(edit, Wt::DomElementType::INPUT);
  edit->updateDom(*update, false);
  BOOST_TEST(update->getAttribute("type") == "");
  delete update;

  edit->textVisibilityChanged().emit(false);
  BOOST_TEST(!edit->isTextVisible());
  BOOST_TEST(calls == 2);
}

BOOST_AUTO_TEST_CASE( password_edit_native_control_text_visible_render )
{
  // Tests that a native control password edit with visible text renders
  // as a text input, keeping its validation attributes
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto edit = app.root()->addNew<Wt::WPasswordEdit>();
  edit->setNativeControl(true);
  edit->setMinLength(4);
  edit->setTextVisible(true);

  auto domElement = edit->createSDomElement(&app);

  BOOST_TEST(domElement->getAttribute("type") == "text");
  BOOST_TEST(domElement->getAttribute("minlength") == "4");

  delete domElement;
}
