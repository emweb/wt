/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPopupWidget.h>
#include <Wt/Test/WTestEnvironment.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( containerwidget_addnew )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  BOOST_CHECK_EQUAL(container->parent(), testApp.root());

  auto popupMenu = testApp.root()->addNew<WPopupMenu>();
  BOOST_CHECK_NE(popupMenu->parent(), testApp.root());

  auto popupWidget = testApp.root()->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  BOOST_CHECK_NE(popupWidget->parent(), testApp.root());

  auto dialog = testApp.root()->addNew<WDialog>();
  BOOST_CHECK_NE(dialog->parent(), testApp.root());

  auto messageBox = testApp.root()->addNew<WMessageBox>();
  BOOST_CHECK_NE(messageBox->parent(), testApp.root());
}
