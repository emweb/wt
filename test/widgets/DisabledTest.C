/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>

#include <Wt/Chart/WAxisSliderWidget.h>
#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WPieChart.h>

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WCalendar.h>
#include <Wt/WCheckBox.h>
#include <Wt/WColorPicker.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDatePicker.h>
#include <Wt/WDefaultLoadingIndicator.h>
#include <Wt/WEmailEdit.h>
#include <Wt/WFileUpload.h>
#include <Wt/WGoogleMap.h>
#include <Wt/WIconPair.h>
#include <Wt/WImage.h>
#include <Wt/WInPlaceEdit.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMediaPlayer.h>
#include <Wt/WMenu.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPanel.h>
#include <Wt/WPopupWidget.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WRadioButton.h>
#include <Wt/WSlider.h>
#include <Wt/WSpinBox.h>
#include <Wt/WSplitButton.h>
#include <Wt/WTable.h>
#include <Wt/WTableView.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WToolBar.h>
#include <Wt/WTree.h>
#include <Wt/WTreeNode.h>
#include <Wt/WTreeTable.h>
#include <Wt/WTreeView.h>
#include <Wt/WViewWidget.h>
#include <Wt/WVirtualImage.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE(disabled_widget_test)
{
  // Tests whether a widget that is being actively disabled (using
  // setDisabled()) is correctly marked as such, and receives the
  // correct (theme-dependent) styling.

  Test::WTestEnvironment env;
  WApplication app(env);

  // WInteractWidget
  auto container = app.root()->addNew<WContainerWidget>();
  auto image = app.root()->addNew<WImage>();
  auto label = app.root()->addNew<WLabel>();
  auto progressBar = app.root()->addNew<WProgressBar>();
  auto table = app.root()->addNew<WTable>();
  auto templat = app.root()->addNew<WTemplate>();
  auto text = app.root()->addNew<WText>();

  // WFormWidget
  auto checkBox = app.root()->addNew<WCheckBox>();
  auto radioButton = app.root()->addNew<WRadioButton>();
  auto colorPicker = app.root()->addNew<WColorPicker>();
  auto comboBox = app.root()->addNew<WComboBox>();
  auto emailEdit = app.root()->addNew<WEmailEdit>();
  auto spinBox = app.root()->addNew<WSpinBox>();
  auto lineEdit = app.root()->addNew<WLineEdit>();
  auto pushButton = app.root()->addNew<WPushButton>();
  auto slider = app.root()->addNew<WSlider>();
  auto textArea = app.root()->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = app.root()->addNew<WBreak>();
  auto fileUpload = app.root()->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = app.root()->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = app.root()->addNew<WTableView>();
  auto treeView = app.root()->addNew<WTreeView>();
  auto calendar = app.root()->addNew<WCalendar>();
  auto datePicker = app.root()->addNew<WDatePicker>();
  auto loading = app.root()->addNew<WDefaultLoadingIndicator>();
  auto googleMap = app.root()->addNew<WGoogleMap>();
  auto iconPair = app.root()->addNew<WIconPair>("", "");
  auto inPlaceEdit = app.root()->addNew<WInPlaceEdit>();
  auto mediaPlayer = app.root()->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = app.root()->addNew<WMenu>();
  auto panel = app.root()->addNew<WPanel>();
  auto popupWidget = app.root()->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = app.root()->addNew<WSplitButton>();
  auto tabWidget = app.root()->addNew<WTabWidget>();
  auto toolBar = app.root()->addNew<WToolBar>();
  auto tree = app.root()->addNew<WTree>();
  auto treeNode = app.root()->addNew<WTreeNode>("");
  auto treeTable = app.root()->addNew<WTreeTable>();
  auto virtualImage = app.root()->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = app.root()->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = app.root()->addNew<Chart::WCartesianChart>();
  auto pieChart = app.root()->addNew<Chart::WPieChart>();

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(container->isEnabled());
  BOOST_REQUIRE(!container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(image->isEnabled());
  BOOST_REQUIRE(!image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(label->isEnabled());
  BOOST_REQUIRE(!label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(progressBar->isEnabled());
  BOOST_REQUIRE(!progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(table->isEnabled());
  BOOST_REQUIRE(!table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(templat->isEnabled());
  BOOST_REQUIRE(!templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(text->isEnabled());
  BOOST_REQUIRE(!text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(checkBox->isEnabled());
  BOOST_REQUIRE(!checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(radioButton->isEnabled());
  BOOST_REQUIRE(!radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(colorPicker->isEnabled());
  BOOST_REQUIRE(!colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(comboBox->isEnabled());
  BOOST_REQUIRE(!comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(emailEdit->isEnabled());
  BOOST_REQUIRE(!emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(spinBox->isEnabled());
  BOOST_REQUIRE(!spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(lineEdit->isEnabled());
  BOOST_REQUIRE(!lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(pushButton->isEnabled());
  BOOST_REQUIRE(!pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(slider->isEnabled());
  BOOST_REQUIRE(!slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(textArea->isEnabled());
  BOOST_REQUIRE(!textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(wbreak->isEnabled());
  BOOST_REQUIRE(!wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(fileUpload->isEnabled());
  BOOST_REQUIRE(!fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(viewWidget->isEnabled());
  BOOST_REQUIRE(!viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(tableView->isEnabled());
  BOOST_REQUIRE(!tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(treeView->isEnabled());
  BOOST_REQUIRE(!treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(calendar->isEnabled());
  BOOST_REQUIRE(!calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(datePicker->isEnabled());
  BOOST_REQUIRE(!datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(googleMap->isEnabled());
  BOOST_REQUIRE(!googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(iconPair->isEnabled());
  BOOST_REQUIRE(!iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(inPlaceEdit->isEnabled());
  BOOST_REQUIRE(!inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(loading->isEnabled());
  BOOST_REQUIRE(!loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(mediaPlayer->isEnabled());
  BOOST_REQUIRE(!mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(menu->isEnabled());
  BOOST_REQUIRE(!menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(panel->isEnabled());
  BOOST_REQUIRE(!panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(splitButton->isEnabled());
  BOOST_REQUIRE(!splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(tabWidget->isEnabled());
  BOOST_REQUIRE(!tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(toolBar->isEnabled());
  BOOST_REQUIRE(!toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(tree->isEnabled());
  BOOST_REQUIRE(!tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(treeNode->isEnabled());
  BOOST_REQUIRE(!treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(treeTable->isEnabled());
  BOOST_REQUIRE(!treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(virtualImage->isEnabled());
  BOOST_REQUIRE(!virtualImage->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(axisSlider->isEnabled());
  BOOST_REQUIRE(!axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(cartesianChart->isEnabled());
  BOOST_REQUIRE(!cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(pieChart->isEnabled());
  BOOST_REQUIRE(!pieChart->hasStyleClass("Wt-disabled"));

  // WInteractWidget
  container->setDisabled(true);
  image->setDisabled(true);
  label->setDisabled(true);
  progressBar->setDisabled(true);
  table->setDisabled(true);
  templat->setDisabled(true);
  text->setDisabled(true);

  // WFormWidget
  checkBox->setDisabled(true);
  radioButton->setDisabled(true);
  colorPicker->setDisabled(true);
  comboBox->setDisabled(true);
  emailEdit->setDisabled(true);
  spinBox->setDisabled(true);
  lineEdit->setDisabled(true);
  pushButton->setDisabled(true);
  slider->setDisabled(true);
  textArea->setDisabled(true);

  // WWebWidget (not WInteractWidget)
  wbreak->setDisabled(true);
  fileUpload->setDisabled(true);
  viewWidget->setDisabled(true);

  // WCompositeWidget
  tableView->setDisabled(true);
  treeView->setDisabled(true);
  calendar->setDisabled(true);
  datePicker->setDisabled(true);
  googleMap->setDisabled(true);
  iconPair->setDisabled(true);
  inPlaceEdit->setDisabled(true);
  loading->setDisabled(true);
  mediaPlayer->setDisabled(true);
  menu->setDisabled(true);
  panel->setDisabled(true);
  popupWidget->setDisabled(true);
  splitButton->setDisabled(true);
  tabWidget->setDisabled(true);
  toolBar->setDisabled(true);
  tree->setDisabled(true);
  treeNode->setDisabled(true);
  treeTable->setDisabled(true);
  virtualImage->setDisabled(true);

  // WPaintedWidget
  axisSlider->setDisabled(true);
  cartesianChart->setDisabled(true);
  pieChart->setDisabled(true);

  // WInteractWidget
  BOOST_REQUIRE(container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(popupWidget->isDisabled());
  BOOST_REQUIRE(!popupWidget->isEnabled());
  BOOST_REQUIRE(popupWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));
}

BOOST_AUTO_TEST_CASE(disabled_parent_widget_test)
{
  // Tests whether a widget that is being passively disabled (receiving
  // the disabled state from its parent) is correctly marked as such,
  // and receives the correct (theme-dependent) styling.
  // This means the widget itself only inherits the state, but is NOT
  // disabled itself.

  Test::WTestEnvironment env;
  WApplication app(env);

  // WInteractWidget
  auto container = app.root()->addNew<WContainerWidget>();
  auto image = app.root()->addNew<WImage>();
  auto label = app.root()->addNew<WLabel>();
  auto progressBar = app.root()->addNew<WProgressBar>();
  auto table = app.root()->addNew<WTable>();
  auto templat = app.root()->addNew<WTemplate>();
  auto text = app.root()->addNew<WText>();

  // WFormWidget
  auto checkBox = app.root()->addNew<WCheckBox>();
  auto radioButton = app.root()->addNew<WRadioButton>();
  auto colorPicker = app.root()->addNew<WColorPicker>();
  auto comboBox = app.root()->addNew<WComboBox>();
  auto emailEdit = app.root()->addNew<WEmailEdit>();
  auto spinBox = app.root()->addNew<WSpinBox>();
  auto lineEdit = app.root()->addNew<WLineEdit>();
  auto pushButton = app.root()->addNew<WPushButton>();
  auto slider = app.root()->addNew<WSlider>();
  auto textArea = app.root()->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = app.root()->addNew<WBreak>();
  auto fileUpload = app.root()->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = app.root()->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = app.root()->addNew<WTableView>();
  auto treeView = app.root()->addNew<WTreeView>();
  auto calendar = app.root()->addNew<WCalendar>();
  auto datePicker = app.root()->addNew<WDatePicker>();
  auto loading = app.root()->addNew<WDefaultLoadingIndicator>();
  auto googleMap = app.root()->addNew<WGoogleMap>();
  auto iconPair = app.root()->addNew<WIconPair>("", "");
  auto inPlaceEdit = app.root()->addNew<WInPlaceEdit>();
  auto mediaPlayer = app.root()->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = app.root()->addNew<WMenu>();
  auto panel = app.root()->addNew<WPanel>();
  auto popupWidget = app.root()->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = app.root()->addNew<WSplitButton>();
  auto tabWidget = app.root()->addNew<WTabWidget>();
  auto toolBar = app.root()->addNew<WToolBar>();
  auto tree = app.root()->addNew<WTree>();
  auto treeNode = app.root()->addNew<WTreeNode>("");
  auto treeTable = app.root()->addNew<WTreeTable>();
  auto virtualImage = app.root()->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = app.root()->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = app.root()->addNew<Chart::WCartesianChart>();
  auto pieChart = app.root()->addNew<Chart::WPieChart>();

  // Set parent of all widgets to disabled
  // This will NOT mark widgets as disabled, but propagate the visual style only.
  app.root()->setDisabled(true);

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));
}

BOOST_AUTO_TEST_CASE(disabled_parent_widget_added_after_test)
{
  // Tests whether a widget that is being passively disabled (receiving
  // the disabled state from its parent) is correctly marked as such,
  // and receives the correct (theme-dependent) styling.
  // This should be the case even when it is added to the disabled
  // parent after the propagation of the disabled state has been
  // performed (so the parent is marked setDisabled() before the child
  // is added.

  Test::WTestEnvironment env;
  WApplication app(env);

  // Set parent of all widgets to disabled
  // This will NOT mark widgets as disabled, but propagate the visual style only.
  app.root()->setDisabled(true);

  // WInteractWidget
  auto container = app.root()->addNew<WContainerWidget>();
  auto image = app.root()->addNew<WImage>();
  auto label = app.root()->addNew<WLabel>();
  auto progressBar = app.root()->addNew<WProgressBar>();
  auto table = app.root()->addNew<WTable>();
  auto templat = app.root()->addNew<WTemplate>();
  auto text = app.root()->addNew<WText>();

  // WFormWidget
  auto checkBox = app.root()->addNew<WCheckBox>();
  auto radioButton = app.root()->addNew<WRadioButton>();
  auto colorPicker = app.root()->addNew<WColorPicker>();
  auto comboBox = app.root()->addNew<WComboBox>();
  auto emailEdit = app.root()->addNew<WEmailEdit>();
  auto spinBox = app.root()->addNew<WSpinBox>();
  auto lineEdit = app.root()->addNew<WLineEdit>();
  auto pushButton = app.root()->addNew<WPushButton>();
  auto slider = app.root()->addNew<WSlider>();
  auto textArea = app.root()->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = app.root()->addNew<WBreak>();
  auto fileUpload = app.root()->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = app.root()->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = app.root()->addNew<WTableView>();
  auto treeView = app.root()->addNew<WTreeView>();
  auto calendar = app.root()->addNew<WCalendar>();
  auto datePicker = app.root()->addNew<WDatePicker>();
  auto loading = app.root()->addNew<WDefaultLoadingIndicator>();
  auto googleMap = app.root()->addNew<WGoogleMap>();
  auto iconPair = app.root()->addNew<WIconPair>("", "");
  auto inPlaceEdit = app.root()->addNew<WInPlaceEdit>();
  auto mediaPlayer = app.root()->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = app.root()->addNew<WMenu>();
  auto panel = app.root()->addNew<WPanel>();
  auto popupWidget = app.root()->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = app.root()->addNew<WSplitButton>();
  auto tabWidget = app.root()->addNew<WTabWidget>();
  auto toolBar = app.root()->addNew<WToolBar>();
  auto tree = app.root()->addNew<WTree>();
  auto treeNode = app.root()->addNew<WTreeNode>("");
  auto treeTable = app.root()->addNew<WTreeTable>();
  auto virtualImage = app.root()->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = app.root()->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = app.root()->addNew<Chart::WCartesianChart>();
  auto pieChart = app.root()->addNew<Chart::WPieChart>();

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));
}

BOOST_AUTO_TEST_CASE(disabled_parent_widget_later_enabled_test)
{
  // Tests whether a widget that is being passively disabled (receiving
  // the disabled state from its parent) is correctly re-enabled in
  // case the parent's state changes to no longer be disabled.

  Test::WTestEnvironment env;
  WApplication app(env);

  // Set parent of all widgets to disabled
  // This will NOT mark widgets as disabled, but propagate the visual style only.
  app.root()->setDisabled(true);

  // WInteractWidget
  auto container = app.root()->addNew<WContainerWidget>();
  auto image = app.root()->addNew<WImage>();
  auto label = app.root()->addNew<WLabel>();
  auto progressBar = app.root()->addNew<WProgressBar>();
  auto table = app.root()->addNew<WTable>();
  auto templat = app.root()->addNew<WTemplate>();
  auto text = app.root()->addNew<WText>();

  // WFormWidget
  auto checkBox = app.root()->addNew<WCheckBox>();
  auto radioButton = app.root()->addNew<WRadioButton>();
  auto colorPicker = app.root()->addNew<WColorPicker>();
  auto comboBox = app.root()->addNew<WComboBox>();
  auto emailEdit = app.root()->addNew<WEmailEdit>();
  auto spinBox = app.root()->addNew<WSpinBox>();
  auto lineEdit = app.root()->addNew<WLineEdit>();
  auto pushButton = app.root()->addNew<WPushButton>();
  auto slider = app.root()->addNew<WSlider>();
  auto textArea = app.root()->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = app.root()->addNew<WBreak>();
  auto fileUpload = app.root()->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = app.root()->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = app.root()->addNew<WTableView>();
  auto treeView = app.root()->addNew<WTreeView>();
  auto calendar = app.root()->addNew<WCalendar>();
  auto datePicker = app.root()->addNew<WDatePicker>();
  auto loading = app.root()->addNew<WDefaultLoadingIndicator>();
  auto googleMap = app.root()->addNew<WGoogleMap>();
  auto iconPair = app.root()->addNew<WIconPair>("", "");
  auto inPlaceEdit = app.root()->addNew<WInPlaceEdit>();
  auto mediaPlayer = app.root()->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = app.root()->addNew<WMenu>();
  auto panel = app.root()->addNew<WPanel>();
  auto popupWidget = app.root()->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = app.root()->addNew<WSplitButton>();
  auto tabWidget = app.root()->addNew<WTabWidget>();
  auto toolBar = app.root()->addNew<WToolBar>();
  auto tree = app.root()->addNew<WTree>();
  auto treeNode = app.root()->addNew<WTreeNode>("");
  auto treeTable = app.root()->addNew<WTreeTable>();
  auto virtualImage = app.root()->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = app.root()->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = app.root()->addNew<Chart::WCartesianChart>();
  auto pieChart = app.root()->addNew<Chart::WPieChart>();

  // Enable the parent again, visually enabling all widgets in the
  // process.
  app.root()->setDisabled(false);

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(container->isEnabled());
  BOOST_REQUIRE(!container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(image->isEnabled());
  BOOST_REQUIRE(!image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(label->isEnabled());
  BOOST_REQUIRE(!label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(progressBar->isEnabled());
  BOOST_REQUIRE(!progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(table->isEnabled());
  BOOST_REQUIRE(!table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(templat->isEnabled());
  BOOST_REQUIRE(!templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(text->isEnabled());
  BOOST_REQUIRE(!text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(checkBox->isEnabled());
  BOOST_REQUIRE(!checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(radioButton->isEnabled());
  BOOST_REQUIRE(!radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(colorPicker->isEnabled());
  BOOST_REQUIRE(!colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(comboBox->isEnabled());
  BOOST_REQUIRE(!comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(emailEdit->isEnabled());
  BOOST_REQUIRE(!emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(spinBox->isEnabled());
  BOOST_REQUIRE(!spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(lineEdit->isEnabled());
  BOOST_REQUIRE(!lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(pushButton->isEnabled());
  BOOST_REQUIRE(!pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(slider->isEnabled());
  BOOST_REQUIRE(!slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(textArea->isEnabled());
  BOOST_REQUIRE(!textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(wbreak->isEnabled());
  BOOST_REQUIRE(!wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(fileUpload->isEnabled());
  BOOST_REQUIRE(!fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(viewWidget->isEnabled());
  BOOST_REQUIRE(!viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(tableView->isEnabled());
  BOOST_REQUIRE(!tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(treeView->isEnabled());
  BOOST_REQUIRE(!treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(calendar->isEnabled());
  BOOST_REQUIRE(!calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(datePicker->isEnabled());
  BOOST_REQUIRE(!datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(googleMap->isEnabled());
  BOOST_REQUIRE(!googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(iconPair->isEnabled());
  BOOST_REQUIRE(!iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(inPlaceEdit->isEnabled());
  BOOST_REQUIRE(!inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(loading->isEnabled());
  BOOST_REQUIRE(!loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(mediaPlayer->isEnabled());
  BOOST_REQUIRE(!mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(menu->isEnabled());
  BOOST_REQUIRE(!menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(panel->isEnabled());
  BOOST_REQUIRE(!panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(splitButton->isEnabled());
  BOOST_REQUIRE(!splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(tabWidget->isEnabled());
  BOOST_REQUIRE(!tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(toolBar->isEnabled());
  BOOST_REQUIRE(!toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(tree->isEnabled());
  BOOST_REQUIRE(!tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(treeNode->isEnabled());
  BOOST_REQUIRE(!treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(treeTable->isEnabled());
  BOOST_REQUIRE(!treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(virtualImage->isEnabled());
  BOOST_REQUIRE(!virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(axisSlider->isEnabled());
  BOOST_REQUIRE(!axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(cartesianChart->isEnabled());
  BOOST_REQUIRE(!cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(pieChart->isEnabled());
  BOOST_REQUIRE(!pieChart->hasStyleClass("Wt-disabled"));
}

BOOST_AUTO_TEST_CASE(disabled_parent_widget_reparent_test)
{
  // Tests whether a widget that is being passively disabled (receiving
  // the disabled state from its parent) is correctly marked as such,
  // and receives the correct (theme-dependent) styling.
  // When the widget is added to an enabled parent again, it will no
  // longer carry the disabled visual state.

  Test::WTestEnvironment env;
  WApplication app(env);

  auto disabledContainer = app.root()->addNew<Wt::WContainerWidget>();
  auto enabledContainer = app.root()->addNew<Wt::WContainerWidget>();

  // WInteractWidget
  auto container = disabledContainer->addNew<WContainerWidget>();
  auto image = disabledContainer->addNew<WImage>();
  auto label = disabledContainer->addNew<WLabel>();
  auto progressBar = disabledContainer->addNew<WProgressBar>();
  auto table = disabledContainer->addNew<WTable>();
  auto templat = disabledContainer->addNew<WTemplate>();
  auto text = disabledContainer->addNew<WText>();

  // WFormWidget
  auto checkBox = disabledContainer->addNew<WCheckBox>();
  auto radioButton = disabledContainer->addNew<WRadioButton>();
  auto colorPicker = disabledContainer->addNew<WColorPicker>();
  auto comboBox = disabledContainer->addNew<WComboBox>();
  auto emailEdit = disabledContainer->addNew<WEmailEdit>();
  auto spinBox = disabledContainer->addNew<WSpinBox>();
  auto lineEdit = disabledContainer->addNew<WLineEdit>();
  auto pushButton = disabledContainer->addNew<WPushButton>();
  auto slider = disabledContainer->addNew<WSlider>();
  auto textArea = disabledContainer->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = disabledContainer->addNew<WBreak>();
  auto fileUpload = disabledContainer->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = disabledContainer->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = disabledContainer->addNew<WTableView>();
  auto treeView = disabledContainer->addNew<WTreeView>();
  auto calendar = disabledContainer->addNew<WCalendar>();
  auto datePicker = disabledContainer->addNew<WDatePicker>();
  auto loading = disabledContainer->addNew<WDefaultLoadingIndicator>();
  auto googleMap = disabledContainer->addNew<WGoogleMap>();
  auto iconPair = disabledContainer->addNew<WIconPair>("", "");
  auto inPlaceEdit = disabledContainer->addNew<WInPlaceEdit>();
  auto mediaPlayer = disabledContainer->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = disabledContainer->addNew<WMenu>();
  auto panel = disabledContainer->addNew<WPanel>();
  auto popupWidget = disabledContainer->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = disabledContainer->addNew<WSplitButton>();
  auto tabWidget = disabledContainer->addNew<WTabWidget>();
  auto toolBar = disabledContainer->addNew<WToolBar>();
  auto tree = disabledContainer->addNew<WTree>();
  auto treeNode = disabledContainer->addNew<WTreeNode>("");
  auto treeTable = disabledContainer->addNew<WTreeTable>();
  auto virtualImage = disabledContainer->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = disabledContainer->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = disabledContainer->addNew<Chart::WCartesianChart>();
  auto pieChart = disabledContainer->addNew<Chart::WPieChart>();

  // Set parent of all widgets to disabled
  // This will NOT mark widgets as disabled, but propagate the visual style only.
  disabledContainer->setDisabled(true);

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));

  // Reparent the items, which removes the disabled visual state
  // WInteractWidget
  enabledContainer->addWidget(disabledContainer->removeWidget(container));
  enabledContainer->addWidget(disabledContainer->removeWidget(image));
  enabledContainer->addWidget(disabledContainer->removeWidget(label));
  enabledContainer->addWidget(disabledContainer->removeWidget(progressBar));
  enabledContainer->addWidget(disabledContainer->removeWidget(table));
  enabledContainer->addWidget(disabledContainer->removeWidget(templat));
  enabledContainer->addWidget(disabledContainer->removeWidget(text));

  // WFormWidget
  enabledContainer->addWidget(disabledContainer->removeWidget(checkBox));
  enabledContainer->addWidget(disabledContainer->removeWidget(radioButton));
  enabledContainer->addWidget(disabledContainer->removeWidget(colorPicker));
  enabledContainer->addWidget(disabledContainer->removeWidget(comboBox));
  enabledContainer->addWidget(disabledContainer->removeWidget(emailEdit));
  enabledContainer->addWidget(disabledContainer->removeWidget(spinBox));
  enabledContainer->addWidget(disabledContainer->removeWidget(lineEdit));
  enabledContainer->addWidget(disabledContainer->removeWidget(pushButton));
  enabledContainer->addWidget(disabledContainer->removeWidget(slider));
  enabledContainer->addWidget(disabledContainer->removeWidget(textArea));

  // WWebWidget (not WInteractWidget)
  enabledContainer->addWidget(disabledContainer->removeWidget(wbreak));
  enabledContainer->addWidget(disabledContainer->removeWidget(fileUpload));
  enabledContainer->addWidget(disabledContainer->removeWidget(viewWidget));

  // WCompositeWidget
  enabledContainer->addWidget(disabledContainer->removeWidget(tableView));
  enabledContainer->addWidget(disabledContainer->removeWidget(treeView));
  enabledContainer->addWidget(disabledContainer->removeWidget(calendar));
  enabledContainer->addWidget(disabledContainer->removeWidget(datePicker));
  enabledContainer->addWidget(disabledContainer->removeWidget(loading));
  enabledContainer->addWidget(disabledContainer->removeWidget(googleMap));
  enabledContainer->addWidget(disabledContainer->removeWidget(iconPair));
  enabledContainer->addWidget(disabledContainer->removeWidget(inPlaceEdit));
  enabledContainer->addWidget(disabledContainer->removeWidget(mediaPlayer));
  enabledContainer->addWidget(disabledContainer->removeWidget(menu));
  enabledContainer->addWidget(disabledContainer->removeWidget(panel));
  // Special case: cannot be removed from container as this is a "global" widget.
  //enabledContainer->addWidget(disabledContainer->removeWidget(popupWidget));
  enabledContainer->addWidget(disabledContainer->removeWidget(splitButton));
  enabledContainer->addWidget(disabledContainer->removeWidget(tabWidget));
  enabledContainer->addWidget(disabledContainer->removeWidget(toolBar));
  enabledContainer->addWidget(disabledContainer->removeWidget(tree));
  enabledContainer->addWidget(disabledContainer->removeWidget(treeNode));
  enabledContainer->addWidget(disabledContainer->removeWidget(treeTable));
  enabledContainer->addWidget(disabledContainer->removeWidget(virtualImage));

  // WPaintedWidget
  enabledContainer->addWidget(disabledContainer->removeWidget(axisSlider));
  enabledContainer->addWidget(disabledContainer->removeWidget(cartesianChart));
  enabledContainer->addWidget(disabledContainer->removeWidget(pieChart));

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(container->isEnabled());
  BOOST_REQUIRE(!container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(image->isEnabled());
  BOOST_REQUIRE(!image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(label->isEnabled());
  BOOST_REQUIRE(!label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(progressBar->isEnabled());
  BOOST_REQUIRE(!progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(table->isEnabled());
  BOOST_REQUIRE(!table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(templat->isEnabled());
  BOOST_REQUIRE(!templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(text->isEnabled());
  BOOST_REQUIRE(!text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(checkBox->isEnabled());
  BOOST_REQUIRE(!checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(radioButton->isEnabled());
  BOOST_REQUIRE(!radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(colorPicker->isEnabled());
  BOOST_REQUIRE(!colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(comboBox->isEnabled());
  BOOST_REQUIRE(!comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(emailEdit->isEnabled());
  BOOST_REQUIRE(!emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(spinBox->isEnabled());
  BOOST_REQUIRE(!spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(lineEdit->isEnabled());
  BOOST_REQUIRE(!lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(pushButton->isEnabled());
  BOOST_REQUIRE(!pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(slider->isEnabled());
  BOOST_REQUIRE(!slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(textArea->isEnabled());
  BOOST_REQUIRE(!textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(wbreak->isEnabled());
  BOOST_REQUIRE(!wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(fileUpload->isEnabled());
  BOOST_REQUIRE(!fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(viewWidget->isEnabled());
  BOOST_REQUIRE(!viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(tableView->isEnabled());
  BOOST_REQUIRE(!tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(treeView->isEnabled());
  BOOST_REQUIRE(!treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(calendar->isEnabled());
  BOOST_REQUIRE(!calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(datePicker->isEnabled());
  BOOST_REQUIRE(!datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(googleMap->isEnabled());
  BOOST_REQUIRE(!googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(iconPair->isEnabled());
  BOOST_REQUIRE(!iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(inPlaceEdit->isEnabled());
  BOOST_REQUIRE(!inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(loading->isEnabled());
  BOOST_REQUIRE(!loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(mediaPlayer->isEnabled());
  BOOST_REQUIRE(!mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(menu->isEnabled());
  BOOST_REQUIRE(!menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(panel->isEnabled());
  BOOST_REQUIRE(!panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(splitButton->isEnabled());
  BOOST_REQUIRE(!splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(tabWidget->isEnabled());
  BOOST_REQUIRE(!tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(toolBar->isEnabled());
  BOOST_REQUIRE(!toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(tree->isEnabled());
  BOOST_REQUIRE(!tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(treeNode->isEnabled());
  BOOST_REQUIRE(!treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(treeTable->isEnabled());
  BOOST_REQUIRE(!treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(virtualImage->isEnabled());
  BOOST_REQUIRE(!virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(axisSlider->isEnabled());
  BOOST_REQUIRE(!axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(cartesianChart->isEnabled());
  BOOST_REQUIRE(!cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(pieChart->isEnabled());
  BOOST_REQUIRE(!pieChart->hasStyleClass("Wt-disabled"));
}

BOOST_AUTO_TEST_CASE(enabled_parent_disabled_widget_reparent_test)
{
  // Tests whether a widget that is being actively disabled (using
  // setDisabled()) is correctly marked as such, and receives the
  // correct (theme-dependent) styling.
  // When the widget is reparented to an enabled widget, it will
  // maintain its state and visual styling.

  Test::WTestEnvironment env;
  WApplication app(env);

  auto enabledContainer = app.root()->addNew<Wt::WContainerWidget>();

  // WInteractWidget
  auto container = app.root()->addNew<WContainerWidget>();
  auto image = app.root()->addNew<WImage>();
  auto label = app.root()->addNew<WLabel>();
  auto progressBar = app.root()->addNew<WProgressBar>();
  auto table = app.root()->addNew<WTable>();
  auto templat = app.root()->addNew<WTemplate>();
  auto text = app.root()->addNew<WText>();

  // WFormWidget
  auto checkBox = app.root()->addNew<WCheckBox>();
  auto radioButton = app.root()->addNew<WRadioButton>();
  auto colorPicker = app.root()->addNew<WColorPicker>();
  auto comboBox = app.root()->addNew<WComboBox>();
  auto emailEdit = app.root()->addNew<WEmailEdit>();
  auto spinBox = app.root()->addNew<WSpinBox>();
  auto lineEdit = app.root()->addNew<WLineEdit>();
  auto pushButton = app.root()->addNew<WPushButton>();
  auto slider = app.root()->addNew<WSlider>();
  auto textArea = app.root()->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = app.root()->addNew<WBreak>();
  auto fileUpload = app.root()->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = app.root()->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = app.root()->addNew<WTableView>();
  auto treeView = app.root()->addNew<WTreeView>();
  auto calendar = app.root()->addNew<WCalendar>();
  auto datePicker = app.root()->addNew<WDatePicker>();
  auto loading = app.root()->addNew<WDefaultLoadingIndicator>();
  auto googleMap = app.root()->addNew<WGoogleMap>();
  auto iconPair = app.root()->addNew<WIconPair>("", "");
  auto inPlaceEdit = app.root()->addNew<WInPlaceEdit>();
  auto mediaPlayer = app.root()->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = app.root()->addNew<WMenu>();
  auto panel = app.root()->addNew<WPanel>();
  auto popupWidget = app.root()->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = app.root()->addNew<WSplitButton>();
  auto tabWidget = app.root()->addNew<WTabWidget>();
  auto toolBar = app.root()->addNew<WToolBar>();
  auto tree = app.root()->addNew<WTree>();
  auto treeNode = app.root()->addNew<WTreeNode>("");
  auto treeTable = app.root()->addNew<WTreeTable>();
  auto virtualImage = app.root()->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = app.root()->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = app.root()->addNew<Chart::WCartesianChart>();
  auto pieChart = app.root()->addNew<Chart::WPieChart>();

  // WInteractWidget
  container->setDisabled(true);
  image->setDisabled(true);
  label->setDisabled(true);
  progressBar->setDisabled(true);
  table->setDisabled(true);
  templat->setDisabled(true);
  text->setDisabled(true);

  // WFormWidget
  checkBox->setDisabled(true);
  radioButton->setDisabled(true);
  colorPicker->setDisabled(true);
  comboBox->setDisabled(true);
  emailEdit->setDisabled(true);
  spinBox->setDisabled(true);
  lineEdit->setDisabled(true);
  pushButton->setDisabled(true);
  slider->setDisabled(true);
  textArea->setDisabled(true);

  // WWebWidget (not WInteractWidget)
  wbreak->setDisabled(true);
  fileUpload->setDisabled(true);
  viewWidget->setDisabled(true);

  // WCompositeWidget
  tableView->setDisabled(true);
  treeView->setDisabled(true);
  calendar->setDisabled(true);
  datePicker->setDisabled(true);
  googleMap->setDisabled(true);
  iconPair->setDisabled(true);
  inPlaceEdit->setDisabled(true);
  loading->setDisabled(true);
  mediaPlayer->setDisabled(true);
  menu->setDisabled(true);
  panel->setDisabled(true);
  popupWidget->setDisabled(true);
  splitButton->setDisabled(true);
  tabWidget->setDisabled(true);
  toolBar->setDisabled(true);
  tree->setDisabled(true);
  treeNode->setDisabled(true);
  treeTable->setDisabled(true);
  virtualImage->setDisabled(true);

  // WPaintedWidget
  axisSlider->setDisabled(true);
  cartesianChart->setDisabled(true);
  pieChart->setDisabled(true);

  // WInteractWidget
  BOOST_REQUIRE(container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(popupWidget->isDisabled());
  BOOST_REQUIRE(!popupWidget->isEnabled());
  BOOST_REQUIRE(popupWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));

  // Reparent the items, which does not change their state
  // WInteractWidget
  enabledContainer->addWidget(app.root()->removeWidget(container));
  enabledContainer->addWidget(app.root()->removeWidget(image));
  enabledContainer->addWidget(app.root()->removeWidget(label));
  enabledContainer->addWidget(app.root()->removeWidget(progressBar));
  enabledContainer->addWidget(app.root()->removeWidget(table));
  enabledContainer->addWidget(app.root()->removeWidget(templat));
  enabledContainer->addWidget(app.root()->removeWidget(text));

  // WFormWidget
  enabledContainer->addWidget(app.root()->removeWidget(checkBox));
  enabledContainer->addWidget(app.root()->removeWidget(radioButton));
  enabledContainer->addWidget(app.root()->removeWidget(colorPicker));
  enabledContainer->addWidget(app.root()->removeWidget(comboBox));
  enabledContainer->addWidget(app.root()->removeWidget(emailEdit));
  enabledContainer->addWidget(app.root()->removeWidget(spinBox));
  enabledContainer->addWidget(app.root()->removeWidget(lineEdit));
  enabledContainer->addWidget(app.root()->removeWidget(pushButton));
  enabledContainer->addWidget(app.root()->removeWidget(slider));
  enabledContainer->addWidget(app.root()->removeWidget(textArea));

  // WWebWidget (not WInteractWidget)
  enabledContainer->addWidget(app.root()->removeWidget(wbreak));
  enabledContainer->addWidget(app.root()->removeWidget(fileUpload));
  enabledContainer->addWidget(app.root()->removeWidget(viewWidget));

  // WCompositeWidget
  enabledContainer->addWidget(app.root()->removeWidget(tableView));
  enabledContainer->addWidget(app.root()->removeWidget(treeView));
  enabledContainer->addWidget(app.root()->removeWidget(calendar));
  enabledContainer->addWidget(app.root()->removeWidget(datePicker));
  enabledContainer->addWidget(app.root()->removeWidget(loading));
  enabledContainer->addWidget(app.root()->removeWidget(googleMap));
  enabledContainer->addWidget(app.root()->removeWidget(iconPair));
  enabledContainer->addWidget(app.root()->removeWidget(inPlaceEdit));
  enabledContainer->addWidget(app.root()->removeWidget(mediaPlayer));
  enabledContainer->addWidget(app.root()->removeWidget(menu));
  enabledContainer->addWidget(app.root()->removeWidget(panel));
  // Special case: cannot be removed from container as this is a "global" widget.
  //enabledContainer->addWidget(app.root()->removeWidget(popupWidget));
  enabledContainer->addWidget(app.root()->removeWidget(splitButton));
  enabledContainer->addWidget(app.root()->removeWidget(tabWidget));
  enabledContainer->addWidget(app.root()->removeWidget(toolBar));
  enabledContainer->addWidget(app.root()->removeWidget(tree));
  enabledContainer->addWidget(app.root()->removeWidget(treeNode));
  enabledContainer->addWidget(app.root()->removeWidget(treeTable));
  enabledContainer->addWidget(app.root()->removeWidget(virtualImage));

  // WPaintedWidget
  enabledContainer->addWidget(app.root()->removeWidget(axisSlider));
  enabledContainer->addWidget(app.root()->removeWidget(cartesianChart));
  enabledContainer->addWidget(app.root()->removeWidget(pieChart));

  // WInteractWidget
  BOOST_REQUIRE(container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(popupWidget->isDisabled());
  BOOST_REQUIRE(!popupWidget->isEnabled());
  BOOST_REQUIRE(popupWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));
}

BOOST_AUTO_TEST_CASE(disabled_parent_reparent_enabled_parent_test)
{
  // Tests whether a widget that is being passively disabled (receiving
  // the disabled state from its parent) correctly keeps the state of
  // its parent (which is actively disabled), is reparented to an
  // enabled widget.

  Test::WTestEnvironment env;
  WApplication app(env);

  auto enabledContainer = app.root()->addNew<Wt::WContainerWidget>();
  auto disabledContainer = app.root()->addNew<Wt::WContainerWidget>();
  disabledContainer->setDisabled(true);

  // WInteractWidget
  auto container = disabledContainer->addNew<WContainerWidget>();
  auto image = disabledContainer->addNew<WImage>();
  auto label = disabledContainer->addNew<WLabel>();
  auto progressBar = disabledContainer->addNew<WProgressBar>();
  auto table = disabledContainer->addNew<WTable>();
  auto templat = disabledContainer->addNew<WTemplate>();
  auto text = disabledContainer->addNew<WText>();

  // WFormWidget
  auto checkBox = disabledContainer->addNew<WCheckBox>();
  auto radioButton = disabledContainer->addNew<WRadioButton>();
  auto colorPicker = disabledContainer->addNew<WColorPicker>();
  auto comboBox = disabledContainer->addNew<WComboBox>();
  auto emailEdit = disabledContainer->addNew<WEmailEdit>();
  auto spinBox = disabledContainer->addNew<WSpinBox>();
  auto lineEdit = disabledContainer->addNew<WLineEdit>();
  auto pushButton = disabledContainer->addNew<WPushButton>();
  auto slider = disabledContainer->addNew<WSlider>();
  auto textArea = disabledContainer->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = disabledContainer->addNew<WBreak>();
  auto fileUpload = disabledContainer->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = disabledContainer->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = disabledContainer->addNew<WTableView>();
  auto treeView = disabledContainer->addNew<WTreeView>();
  auto calendar = disabledContainer->addNew<WCalendar>();
  auto datePicker = disabledContainer->addNew<WDatePicker>();
  auto loading = disabledContainer->addNew<WDefaultLoadingIndicator>();
  auto googleMap = disabledContainer->addNew<WGoogleMap>();
  auto iconPair = disabledContainer->addNew<WIconPair>("", "");
  auto inPlaceEdit = disabledContainer->addNew<WInPlaceEdit>();
  auto mediaPlayer = disabledContainer->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = disabledContainer->addNew<WMenu>();
  auto panel = disabledContainer->addNew<WPanel>();
  auto popupWidget = disabledContainer->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = disabledContainer->addNew<WSplitButton>();
  auto tabWidget = disabledContainer->addNew<WTabWidget>();
  auto toolBar = disabledContainer->addNew<WToolBar>();
  auto tree = disabledContainer->addNew<WTree>();
  auto treeNode = disabledContainer->addNew<WTreeNode>("");
  auto treeTable = disabledContainer->addNew<WTreeTable>();
  auto virtualImage = disabledContainer->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = disabledContainer->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = disabledContainer->addNew<Chart::WCartesianChart>();
  auto pieChart = disabledContainer->addNew<Chart::WPieChart>();

  // Main containers
  BOOST_REQUIRE(!enabledContainer->isDisabled());
  BOOST_REQUIRE(enabledContainer->isEnabled());
  BOOST_REQUIRE(!enabledContainer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(disabledContainer->isDisabled());
  BOOST_REQUIRE(!disabledContainer->isEnabled());
  BOOST_REQUIRE(disabledContainer->hasStyleClass("Wt-disabled"));

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));

  // Reparent the items' parent, which does not change their state
  enabledContainer->addWidget(app.root()->removeWidget(disabledContainer));

  // Main containers
  BOOST_REQUIRE(!enabledContainer->isDisabled());
  BOOST_REQUIRE(enabledContainer->isEnabled());
  BOOST_REQUIRE(!enabledContainer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(disabledContainer->isDisabled());
  BOOST_REQUIRE(!disabledContainer->isEnabled());
  BOOST_REQUIRE(disabledContainer->hasStyleClass("Wt-disabled"));

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));
}

BOOST_AUTO_TEST_CASE(enabled_parent_reparent_disabled_parent_test)
{
  // Tests whether a widget that is enabled correctly updates its state
  // to being passively disabled (receiving the state from its parent),
  // in case the widget's parent is reparented to a disabled widget.

  Test::WTestEnvironment env;
  WApplication app(env);

  auto enabledContainer = app.root()->addNew<Wt::WContainerWidget>();
  auto disabledContainer = app.root()->addNew<Wt::WContainerWidget>();
  disabledContainer->setDisabled(true);

  // WInteractWidget
  auto container = enabledContainer->addNew<WContainerWidget>();
  auto image = enabledContainer->addNew<WImage>();
  auto label = enabledContainer->addNew<WLabel>();
  auto progressBar = enabledContainer->addNew<WProgressBar>();
  auto table = enabledContainer->addNew<WTable>();
  auto templat = enabledContainer->addNew<WTemplate>();
  auto text = enabledContainer->addNew<WText>();

  // WFormWidget
  auto checkBox = enabledContainer->addNew<WCheckBox>();
  auto radioButton = enabledContainer->addNew<WRadioButton>();
  auto colorPicker = enabledContainer->addNew<WColorPicker>();
  auto comboBox = enabledContainer->addNew<WComboBox>();
  auto emailEdit = enabledContainer->addNew<WEmailEdit>();
  auto spinBox = enabledContainer->addNew<WSpinBox>();
  auto lineEdit = enabledContainer->addNew<WLineEdit>();
  auto pushButton = enabledContainer->addNew<WPushButton>();
  auto slider = enabledContainer->addNew<WSlider>();
  auto textArea = enabledContainer->addNew<WTextArea>();

  // WWebWidget (not WInteractWidget)
  auto wbreak = enabledContainer->addNew<WBreak>();
  auto fileUpload = enabledContainer->addNew<WFileUpload>();
  auto staticModel = makeStaticModel<std::function<std::unique_ptr<WWidget>()>>([] { return std::make_unique<WText>("Hello"); });
  auto viewWidget = enabledContainer->addWidget(std::move(staticModel));

  // WCompositeWidget
  auto tableView = enabledContainer->addNew<WTableView>();
  auto treeView = enabledContainer->addNew<WTreeView>();
  auto calendar = enabledContainer->addNew<WCalendar>();
  auto datePicker = enabledContainer->addNew<WDatePicker>();
  auto loading = enabledContainer->addNew<WDefaultLoadingIndicator>();
  auto googleMap = enabledContainer->addNew<WGoogleMap>();
  auto iconPair = enabledContainer->addNew<WIconPair>("", "");
  auto inPlaceEdit = enabledContainer->addNew<WInPlaceEdit>();
  auto mediaPlayer = enabledContainer->addNew<WMediaPlayer>(MediaType::Audio);
  auto menu = enabledContainer->addNew<WMenu>();
  auto panel = enabledContainer->addNew<WPanel>();
  auto popupWidget = enabledContainer->addNew<WPopupWidget>(std::make_unique<WContainerWidget>());
  auto splitButton = enabledContainer->addNew<WSplitButton>();
  auto tabWidget = enabledContainer->addNew<WTabWidget>();
  auto toolBar = enabledContainer->addNew<WToolBar>();
  auto tree = enabledContainer->addNew<WTree>();
  auto treeNode = enabledContainer->addNew<WTreeNode>("");
  auto treeTable = enabledContainer->addNew<WTreeTable>();
  auto virtualImage = enabledContainer->addNew<WVirtualImage>(1, 1, 1, 1);

  // WPaintedWidget
  auto axisSlider = enabledContainer->addNew<Chart::WAxisSliderWidget>();
  auto cartesianChart = enabledContainer->addNew<Chart::WCartesianChart>();
  auto pieChart = enabledContainer->addNew<Chart::WPieChart>();

  // Main containers
  BOOST_REQUIRE(!enabledContainer->isDisabled());
  BOOST_REQUIRE(enabledContainer->isEnabled());
  BOOST_REQUIRE(!enabledContainer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(disabledContainer->isDisabled());
  BOOST_REQUIRE(!disabledContainer->isEnabled());
  BOOST_REQUIRE(disabledContainer->hasStyleClass("Wt-disabled"));

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(container->isEnabled());
  BOOST_REQUIRE(!container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(image->isEnabled());
  BOOST_REQUIRE(!image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(label->isEnabled());
  BOOST_REQUIRE(!label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(progressBar->isEnabled());
  BOOST_REQUIRE(!progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(table->isEnabled());
  BOOST_REQUIRE(!table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(templat->isEnabled());
  BOOST_REQUIRE(!templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(text->isEnabled());
  BOOST_REQUIRE(!text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(checkBox->isEnabled());
  BOOST_REQUIRE(!checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(radioButton->isEnabled());
  BOOST_REQUIRE(!radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(colorPicker->isEnabled());
  BOOST_REQUIRE(!colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(comboBox->isEnabled());
  BOOST_REQUIRE(!comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(emailEdit->isEnabled());
  BOOST_REQUIRE(!emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(spinBox->isEnabled());
  BOOST_REQUIRE(!spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(lineEdit->isEnabled());
  BOOST_REQUIRE(!lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(pushButton->isEnabled());
  BOOST_REQUIRE(!pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(slider->isEnabled());
  BOOST_REQUIRE(!slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(textArea->isEnabled());
  BOOST_REQUIRE(!textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (!not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(wbreak->isEnabled());
  BOOST_REQUIRE(!wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(fileUpload->isEnabled());
  BOOST_REQUIRE(!fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(viewWidget->isEnabled());
  BOOST_REQUIRE(!viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(tableView->isEnabled());
  BOOST_REQUIRE(!tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(treeView->isEnabled());
  BOOST_REQUIRE(!treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(calendar->isEnabled());
  BOOST_REQUIRE(!calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(datePicker->isEnabled());
  BOOST_REQUIRE(!datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(googleMap->isEnabled());
  BOOST_REQUIRE(!googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(iconPair->isEnabled());
  BOOST_REQUIRE(!iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(inPlaceEdit->isEnabled());
  BOOST_REQUIRE(!inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(loading->isEnabled());
  BOOST_REQUIRE(!loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(mediaPlayer->isEnabled());
  BOOST_REQUIRE(!mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(menu->isEnabled());
  BOOST_REQUIRE(!menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(panel->isEnabled());
  BOOST_REQUIRE(!panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(splitButton->isEnabled());
  BOOST_REQUIRE(!splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(tabWidget->isEnabled());
  BOOST_REQUIRE(!tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(toolBar->isEnabled());
  BOOST_REQUIRE(!toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(tree->isEnabled());
  BOOST_REQUIRE(!tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(treeNode->isEnabled());
  BOOST_REQUIRE(!treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(treeTable->isEnabled());
  BOOST_REQUIRE(!treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(virtualImage->isEnabled());
  BOOST_REQUIRE(!virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(axisSlider->isEnabled());
  BOOST_REQUIRE(!axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(cartesianChart->isEnabled());
  BOOST_REQUIRE(!cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(pieChart->isEnabled());
  BOOST_REQUIRE(!pieChart->hasStyleClass("Wt-disabled"));

  // Reparent the items' parent, which does change their state to being
  // passible disabled.
  disabledContainer->addWidget(app.root()->removeWidget(enabledContainer));

  // Main containers
  BOOST_REQUIRE(!enabledContainer->isDisabled());
  BOOST_REQUIRE(!enabledContainer->isEnabled());
  BOOST_REQUIRE(enabledContainer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(disabledContainer->isDisabled());
  BOOST_REQUIRE(!disabledContainer->isEnabled());
  BOOST_REQUIRE(disabledContainer->hasStyleClass("Wt-disabled"));

  // WInteractWidget
  BOOST_REQUIRE(!container->isDisabled());
  BOOST_REQUIRE(!container->isEnabled());
  BOOST_REQUIRE(container->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!image->isDisabled());
  BOOST_REQUIRE(!image->isEnabled());
  BOOST_REQUIRE(image->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!label->isDisabled());
  BOOST_REQUIRE(!label->isEnabled());
  BOOST_REQUIRE(label->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!progressBar->isDisabled());
  BOOST_REQUIRE(!progressBar->isEnabled());
  BOOST_REQUIRE(progressBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!table->isDisabled());
  BOOST_REQUIRE(!table->isEnabled());
  BOOST_REQUIRE(table->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!templat->isDisabled());
  BOOST_REQUIRE(!templat->isEnabled());
  BOOST_REQUIRE(templat->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!text->isDisabled());
  BOOST_REQUIRE(!text->isEnabled());
  BOOST_REQUIRE(text->hasStyleClass("Wt-disabled"));

  // WFormWidget
  BOOST_REQUIRE(!checkBox->isDisabled());
  BOOST_REQUIRE(!checkBox->isEnabled());
  BOOST_REQUIRE(checkBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!radioButton->isDisabled());
  BOOST_REQUIRE(!radioButton->isEnabled());
  BOOST_REQUIRE(radioButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!colorPicker->isDisabled());
  BOOST_REQUIRE(!colorPicker->isEnabled());
  BOOST_REQUIRE(colorPicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!comboBox->isDisabled());
  BOOST_REQUIRE(!comboBox->isEnabled());
  BOOST_REQUIRE(comboBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!emailEdit->isDisabled());
  BOOST_REQUIRE(!emailEdit->isEnabled());
  BOOST_REQUIRE(emailEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!spinBox->isDisabled());
  BOOST_REQUIRE(!spinBox->isEnabled());
  BOOST_REQUIRE(spinBox->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!lineEdit->isDisabled());
  BOOST_REQUIRE(!lineEdit->isEnabled());
  BOOST_REQUIRE(lineEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pushButton->isDisabled());
  BOOST_REQUIRE(!pushButton->isEnabled());
  BOOST_REQUIRE(pushButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!slider->isDisabled());
  BOOST_REQUIRE(!slider->isEnabled());
  BOOST_REQUIRE(slider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!textArea->isDisabled());
  BOOST_REQUIRE(!textArea->isEnabled());
  BOOST_REQUIRE(textArea->hasStyleClass("Wt-disabled"));

  // WWebWidget (not WInteractWidget)
  BOOST_REQUIRE(!wbreak->isDisabled());
  BOOST_REQUIRE(!wbreak->isEnabled());
  BOOST_REQUIRE(wbreak->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!fileUpload->isDisabled());
  BOOST_REQUIRE(!fileUpload->isEnabled());
  BOOST_REQUIRE(fileUpload->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!viewWidget->isDisabled());
  BOOST_REQUIRE(!viewWidget->isEnabled());
  BOOST_REQUIRE(viewWidget->hasStyleClass("Wt-disabled"));

  // WCompositeWidget
  BOOST_REQUIRE(!tableView->isDisabled());
  BOOST_REQUIRE(!tableView->isEnabled());
  BOOST_REQUIRE(tableView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeView->isDisabled());
  BOOST_REQUIRE(!treeView->isEnabled());
  BOOST_REQUIRE(treeView->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!calendar->isDisabled());
  BOOST_REQUIRE(!calendar->isEnabled());
  BOOST_REQUIRE(calendar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!datePicker->isDisabled());
  BOOST_REQUIRE(!datePicker->isEnabled());
  BOOST_REQUIRE(datePicker->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!googleMap->isDisabled());
  BOOST_REQUIRE(!googleMap->isEnabled());
  BOOST_REQUIRE(googleMap->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!iconPair->isDisabled());
  BOOST_REQUIRE(!iconPair->isEnabled());
  BOOST_REQUIRE(iconPair->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!inPlaceEdit->isDisabled());
  BOOST_REQUIRE(!inPlaceEdit->isEnabled());
  BOOST_REQUIRE(inPlaceEdit->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!loading->isDisabled());
  BOOST_REQUIRE(!loading->isEnabled());
  BOOST_REQUIRE(loading->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!mediaPlayer->isDisabled());
  BOOST_REQUIRE(!mediaPlayer->isEnabled());
  BOOST_REQUIRE(mediaPlayer->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!menu->isDisabled());
  BOOST_REQUIRE(!menu->isEnabled());
  BOOST_REQUIRE(menu->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!panel->isDisabled());
  BOOST_REQUIRE(!panel->isEnabled());
  BOOST_REQUIRE(panel->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!splitButton->isDisabled());
  BOOST_REQUIRE(!splitButton->isEnabled());
  BOOST_REQUIRE(splitButton->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tabWidget->isDisabled());
  BOOST_REQUIRE(!tabWidget->isEnabled());
  BOOST_REQUIRE(tabWidget->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!toolBar->isDisabled());
  BOOST_REQUIRE(!toolBar->isEnabled());
  BOOST_REQUIRE(toolBar->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!tree->isDisabled());
  BOOST_REQUIRE(!tree->isEnabled());
  BOOST_REQUIRE(tree->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeNode->isDisabled());
  BOOST_REQUIRE(!treeNode->isEnabled());
  BOOST_REQUIRE(treeNode->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!treeTable->isDisabled());
  BOOST_REQUIRE(!treeTable->isEnabled());
  BOOST_REQUIRE(treeTable->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!virtualImage->isDisabled());
  BOOST_REQUIRE(!virtualImage->isEnabled());
  BOOST_REQUIRE(virtualImage->hasStyleClass("Wt-disabled"));

  // Special case: we do not expect the popups to follow the parent' enabled chain
  // They are not strictly a child of the parent that are added to.
  BOOST_REQUIRE(!popupWidget->isDisabled());
  BOOST_REQUIRE(popupWidget->isEnabled());
  BOOST_REQUIRE(!popupWidget->hasStyleClass("Wt-disabled"));

  // WPaintedWidget
  BOOST_REQUIRE(!axisSlider->isDisabled());
  BOOST_REQUIRE(!axisSlider->isEnabled());
  BOOST_REQUIRE(axisSlider->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!cartesianChart->isDisabled());
  BOOST_REQUIRE(!cartesianChart->isEnabled());
  BOOST_REQUIRE(cartesianChart->hasStyleClass("Wt-disabled"));
  BOOST_REQUIRE(!pieChart->isDisabled());
  BOOST_REQUIRE(!pieChart->isEnabled());
  BOOST_REQUIRE(pieChart->hasStyleClass("Wt-disabled"));
}
