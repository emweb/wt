/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLayoutImpl.h>
#include <Wt/WStringStream.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/Test/WTestEnvironment.h>

#include "web/DomElement.h"

#include <memory>

using namespace Wt;

namespace {

class TestContainerWidget : public WContainerWidget
{
public:
  using WContainerWidget::getDomChanges;
  using WContainerWidget::propagateRenderOk;
};

std::string renderUpdates(TestContainerWidget& container,
                          WApplication& application)
{
  std::vector<DomElement *> updates;
  container.getDomChanges(updates, &application);

  WStringStream javaScript;
  for (DomElement *update : updates) {
    update->asJavaScript(javaScript);
    delete update;
  }
  container.propagateRenderOk(true);

  return javaScript.str();
}

}

BOOST_AUTO_TEST_CASE( test_vbox_layout_itemAt_range )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WVBoxLayout>());

  auto item1 = layout->addWidget(std::make_unique<WContainerWidget>());
  auto item2 = layout->addWidget(std::make_unique<WContainerWidget>());


  BOOST_CHECK(layout->itemAt(-1) == nullptr);
  BOOST_CHECK(layout->itemAt(0)->widget() == item1);
  BOOST_CHECK(layout->itemAt(1)->widget() == item2);
  BOOST_CHECK(layout->itemAt(2) == nullptr);
}

BOOST_AUTO_TEST_CASE( test_hbox_layout_itemAt_range )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WHBoxLayout>());

  auto item1 = layout->addWidget(std::make_unique<WContainerWidget>());
  auto item2 = layout->addWidget(std::make_unique<WContainerWidget>());


  BOOST_CHECK(layout->itemAt(-1) == nullptr);
  BOOST_CHECK(layout->itemAt(0)->widget() == item1);
  BOOST_CHECK(layout->itemAt(1)->widget() == item2);
  BOOST_CHECK(layout->itemAt(2) == nullptr);
}

BOOST_AUTO_TEST_CASE( test_hbox_layout_content_sizing_shrink_priority )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = std::make_unique<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WHBoxLayout>());

  auto contentSized = std::make_unique<WContainerWidget>();
  contentSized->setWidth(WLength(0, LengthUnit::MaxContent));
  layout->addWidget(std::move(contentSized), 0);

  auto stretched = std::make_unique<WContainerWidget>();
  stretched->setWidth(400);
  layout->addWidget(std::move(stretched), 1);

  auto fixed = std::make_unique<WContainerWidget>();
  fixed->setWidth(200);
  layout->addWidget(std::move(fixed), 0);

  std::unique_ptr<DomElement> element(container->createSDomElement(&testApp));
  EStream html;
  EStream javaScript;
  DomElement::TimeoutList timeouts;
  element->asHTML(html, javaScript, timeouts);

  BOOST_CHECK(html.str().find("csk=\"1\"") != std::string::npos);
  BOOST_CHECK(html.str().find("width:max-content") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:0 1 auto") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:1 1000000 auto") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:0 0 auto") != std::string::npos);

  auto maximumSizedContainer = std::make_unique<WContainerWidget>();
  maximumSizedContainer->setMaximumSize(99999, 99999);
  layout = maximumSizedContainer->setLayout(std::make_unique<WHBoxLayout>());

  contentSized = std::make_unique<WContainerWidget>();
  contentSized->setWidth(WLength(LengthUnit::FitContent));
  layout->addWidget(std::move(contentSized), 0);

  stretched = std::make_unique<WContainerWidget>();
  stretched->setWidth(400);
  layout->addWidget(std::move(stretched), 1);

  element.reset(maximumSizedContainer->createSDomElement(&testApp));
  html.clear();
  javaScript.clear();
  timeouts.clear();
  element->asHTML(html, javaScript, timeouts);

  BOOST_CHECK(html.str().find("flex:0 0 auto") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:1 0 auto") != std::string::npos);

  auto crossAxisMaximumContainer = std::make_unique<WContainerWidget>();
  crossAxisMaximumContainer->setMaximumSize(WLength::Auto, 99999);
  layout = crossAxisMaximumContainer->setLayout(std::make_unique<WHBoxLayout>());

  contentSized = std::make_unique<WContainerWidget>();
  contentSized->setWidth(WLength(LengthUnit::FitContent));
  layout->addWidget(std::move(contentSized), 0);

  stretched = std::make_unique<WContainerWidget>();
  stretched->setWidth(400);
  layout->addWidget(std::move(stretched), 1);

  element.reset(crossAxisMaximumContainer->createSDomElement(&testApp));
  html.clear();
  javaScript.clear();
  timeouts.clear();
  element->asHTML(html, javaScript, timeouts);

  BOOST_CHECK(html.str().find("flex:0 1 auto") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:1 1000000 auto") != std::string::npos);

  auto fixedMaximumContainer = std::make_unique<TestContainerWidget>();
  fixedMaximumContainer->setWidth(800);
  fixedMaximumContainer->setMaximumSize(99999, WLength::Auto);
  layout = fixedMaximumContainer->setLayout(std::make_unique<WHBoxLayout>());

  auto alignedContentSized = std::make_unique<WContainerWidget>();
  auto alignedContentSizedPtr = alignedContentSized.get();
  alignedContentSized->setWidth(WLength(LengthUnit::FitContent));
  layout->addWidget(std::move(alignedContentSized), 0, AlignmentFlag::Top);

  stretched = std::make_unique<WContainerWidget>();
  stretched->setWidth(400);
  layout->addWidget(std::move(stretched), 1);

  element.reset(fixedMaximumContainer->createSDomElement(&testApp));
  html.clear();
  javaScript.clear();
  timeouts.clear();
  element->asHTML(html, javaScript, timeouts);

  BOOST_CHECK(html.str().find("flex:0 1 auto") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:1 1000000 auto") != std::string::npos);

  const std::string alignedId = alignedContentSizedPtr->id();
  const std::string idAttribute = "id=\"" + alignedId + "\"";
  const std::size_t idPosition = html.str().find(idAttribute);
  BOOST_REQUIRE(idPosition != std::string::npos);
  const std::size_t tagStart = html.str().rfind('<', idPosition);
  const std::size_t tagEnd = html.str().find('>', idPosition);
  BOOST_REQUIRE(tagStart != std::string::npos);
  BOOST_REQUIRE(tagEnd != std::string::npos);
  const std::string alignedTag = html.str().substr(tagStart,
                                                   tagEnd - tagStart + 1);
  BOOST_CHECK(alignedTag.find("flg=\"0\"") != std::string::npos);
  BOOST_CHECK(alignedTag.find("csk=\"1\"") != std::string::npos);
  BOOST_CHECK(alignedTag.find("Wt-fill-") == std::string::npos);

  layout->impl()->update();
  renderUpdates(*fixedMaximumContainer, testApp);

  alignedContentSizedPtr->setWidth(320);
  const std::string numericUpdate = renderUpdates(*fixedMaximumContainer,
                                                   testApp);
  const std::string numericResizeCall =
    "layout.resizeItem(" + alignedId +
    ",\"320.0px\",\"auto\",\"auto\",\"auto\",false)";
  BOOST_CHECK_MESSAGE(numericUpdate.find(numericResizeCall) !=
                      std::string::npos,
                      numericUpdate);

  alignedContentSizedPtr->setWidth(WLength(LengthUnit::FitContent));
  const std::string keywordUpdate = renderUpdates(*fixedMaximumContainer,
                                                   testApp);
  const std::string keywordResizeCall =
    "layout.resizeItem(" + alignedId +
    ",\"fit-content\",\"auto\",\"auto\",\"auto\",true)";
  BOOST_CHECK_MESSAGE(keywordUpdate.find(keywordResizeCall) !=
                      std::string::npos,
                      keywordUpdate);

  auto verticalContainer = std::make_unique<WContainerWidget>();
  auto verticalLayout = verticalContainer->setLayout(std::make_unique<WVBoxLayout>());

  auto verticallyContentSized = std::make_unique<WContainerWidget>();
  verticallyContentSized->setHeight(WLength(0, LengthUnit::MinContent));
  verticalLayout->addWidget(std::move(verticallyContentSized), 0);

  auto verticallyStretched = std::make_unique<WContainerWidget>();
  verticallyStretched->setHeight(400);
  verticalLayout->addWidget(std::move(verticallyStretched), 1);

  element.reset(verticalContainer->createSDomElement(&testApp));
  html.clear();
  javaScript.clear();
  timeouts.clear();
  element->asHTML(html, javaScript, timeouts);

  BOOST_CHECK(html.str().find("csk=\"1\"") != std::string::npos);
  BOOST_CHECK(html.str().find("height:min-content") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:0 1 auto") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:1 1000000 auto") != std::string::npos);

  auto fixedMaximumVerticalContainer = std::make_unique<WContainerWidget>();
  fixedMaximumVerticalContainer->setHeight(800);
  fixedMaximumVerticalContainer->setMaximumSize(WLength::Auto, 99999);
  verticalLayout = fixedMaximumVerticalContainer->setLayout(
    std::make_unique<WVBoxLayout>());

  verticallyContentSized = std::make_unique<WContainerWidget>();
  verticallyContentSized->setHeight(WLength(LengthUnit::MinContent));
  verticalLayout->addWidget(std::move(verticallyContentSized), 0,
                            AlignmentFlag::Left);

  verticallyStretched = std::make_unique<WContainerWidget>();
  verticallyStretched->setHeight(400);
  verticalLayout->addWidget(std::move(verticallyStretched), 1);

  element.reset(fixedMaximumVerticalContainer->createSDomElement(&testApp));
  html.clear();
  javaScript.clear();
  timeouts.clear();
  element->asHTML(html, javaScript, timeouts);

  BOOST_CHECK(html.str().find("flex:0 1 auto") != std::string::npos);
  BOOST_CHECK(html.str().find("flex:1 1000000 auto") != std::string::npos);
}
