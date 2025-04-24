
/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
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

#include "thirdparty/rapidxml/rapidxml.hpp"
#include "thirdparty/rapidxml/rapidxml_print.hpp"
#include "thirdparty/rapidxml/rapidxml_utils.hpp"

#include <iterator>
#include <memory>
#include <sstream>
#include <tuple>

class TemplateLocalizedStrings final : public Wt::WLocalizedStrings {
public:
  Wt::LocalizedString resolveKey(const Wt::WLocale &locale, const std::string &key) override
  {
    if (key == "value") {
      return { "<div>Hello</div>", Wt::TextFormat::XHTML };
    } else if (key == "script") {
      return { "<script>Hello</script>", Wt::TextFormat::XHTML };
    } else {
      return {};
    }
  }
};

namespace {
  // RapidXML does not take a copy. The object needs to be alive at point of printing.
  std::vector<std::string> replacedStrings;
}

void filterOutIdAndName(Wt::rapidxml::xml_node<char>* node)
{
  auto idAttr = node->first_attribute("id");
  auto nameAttr = node->first_attribute("name");
  auto targetAttr = node->first_attribute("target");

  if (idAttr) {
    node->remove_attribute(idAttr);
  }

  if (nameAttr) {
    node->remove_attribute(nameAttr);
  }

  if (targetAttr) {
    node->remove_attribute(targetAttr);
  }

  // Filter out of src and action
  auto srcAttr = node->first_attribute("src");
  auto actionAttr = node->first_attribute("action");

  if (srcAttr) {
    auto srcContent = std::string(srcAttr->value());
    auto start = srcContent.find("resource=");
    auto end = srcContent.find("&", start);

    if (start != std::string::npos) {
      srcContent.replace(start + 9, (end - start - 9), "");
      replacedStrings.push_back(srcContent);
      srcAttr->value(replacedStrings.back().c_str());
    }
  }

  if (actionAttr) {
    auto actionContent = std::string(actionAttr->value());
    auto start = actionContent.find("resource=");
    auto end = actionContent.find("&", start);

    if (start != std::string::npos) {
      actionContent.replace(start + 9, (end - start - 9), "");
      replacedStrings.push_back(actionContent);
      actionAttr->value(replacedStrings.back().c_str());
    }
  }
}

void iterateAndFilter(Wt::rapidxml::xml_node<char>* node)
{
  using namespace Wt::rapidxml;

  std::string const name(node->name(), node->name_size());

  filterOutIdAndName(node);
  for (xml_node<>* child = node->first_node(); child; child = child->next_sibling()) {
    iterateAndFilter(child);
  }
}

std::string filterOutIds(const std::string& input)
{
  using namespace Wt::rapidxml;

  std::vector<char> text(input.begin(), input.end());
  text.push_back('\0');

  xml_document<> doc;
  doc.parse<parse_trim_whitespace>(&text[0]);

  for (xml_node<>* child = doc.first_node(); child; child = child->next_sibling()) {
    iterateAndFilter(child);
  }

  std::string output;
  print(std::back_inserter(output), doc, print_no_indenting);
  replacedStrings.clear();
  return output;
}
std::tuple<std::string, std::string> extractMonthAndToday(const std::string& input)
{
  using namespace Wt::rapidxml;

  std::vector<char> text(input.begin(), input.end());
  text.push_back('\0');

  xml_document<> doc;
  doc.parse<parse_trim_whitespace>(&text[0]);

  std::string month;
  std::string day;

  // Found in div -> div -> table -> tr -> th (2) -> select
  xml_node<>* table = doc.first_node("div")->first_node("div")->first_node("table");
  xml_node<>* header = table->first_node("tr")->first_node("th")->next_sibling("th");
  xml_node<>* select = header->first_node("select");
  for (xml_node<>* option = select->first_node("option"); option; option = option->next_sibling("option")) {
    auto selectedAttr = option->first_attribute("selected");
    if (selectedAttr && std::string(selectedAttr->value()) == "selected") {
      month = option->value();
    }
  }

  for (xml_node<>* row = table->first_node("tr"); row; row = row->next_sibling("tr")) {
    for (xml_node<>* column = row->first_node("td"); column; column = column->next_sibling("td")) {
      for (xml_node<>* tableElement = column->first_node("div"); tableElement; tableElement = tableElement->next_sibling("div")) {
        auto titleAttr = tableElement->first_attribute("title");
        auto classAttr = tableElement->first_attribute("class");
        if (classAttr && titleAttr
            && std::string(classAttr->value()) == " Wt-cal-now"
            && std::string(titleAttr->value()) == "Today") {
          day = tableElement->value();
        }
      }
    }
  }

  return std::make_tuple(month, day);
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_xss_filter)
{
  // Checks whether scripts are correctly removed.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<script>alert('boo');</script>");
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_doctype_filter)
{
  // Tests whether DOCTYPE is removed

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<!DOCTYPE html>");
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_type_filters)
{
  // Tests whether certain tags are disallowed, namely:
  // base, body, embed, head, iframe, link, meta, object, style, title

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<base></base><body></body><embed></embed><head></head><iframe></iframe><link></link><meta></meta><object></object><style></style><title></title>");
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_all_selfclosing_tags)
{
  // Tests whether all valid and allowed selfclosing tags correctly
  // render.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<area/>"
                  "<br/>"
                  "<col/>"
                  "<hr/>"
                  "<input/>"
                  "<img/>");

  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<area/>"
                             "<br/>"
                             "<col/>"
                             "<hr/>"
                             "<input/>"
                             "<img/>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_all_selfclosing_tags_replace)
{
  // Test whether tags that are considered self-closing are replaced
  // with their selfclosing counterpart.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<area></area>"
                  "<br></br>"
                  "<col></col>"
                  "<hr></hr>"
                  "<input></input>"
                  "<img></img>");

  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<area/>"
                             "<br/>"
                             "<col/>"
                             "<hr/>"
                             "<input/>"
                             "<img/>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_all_valid_tags)
{
  // Tests whether all valid and allowed tags correctly render.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<!---->"
                  "<!DOCTYPE html>"
                  "<a></a>"
                  "<abbr></abbr>"
                  "<address></address>"
                  "<b></b>"
                  "<bdi></bdi>"
                  "<bdo></bdo>"
                  "<blockquote></blockquote>"
                  "<button></button>"
                  "<canvas></canvas>"
                  "<caption></caption>"
                  "<cite></cite>"
                  "<code></code>"
                  "<colgroup></colgroup>"
                  "<data></data>"
                  "<datalist></datalist>"
                  "<dd></dd>"
                  "<del></del>"
                  "<details></details>"
                  "<dfn></dfn>"
                  "<dialog></dialog>"
                  "<div></div>"
                  "<dl></dl>"
                  "<dt></dt>"
                  "<em></em>"
                  "<fieldset></fieldset>"
                  "<footer></footer>"
                  "<form></form>"
                  "<h1></h1>"
                  "<header></header>"
                  "<hgroup></hgroup>"
                  "<html></html>"
                  "<i></i>"
                  "<ins></ins>"
                  "<kbd></kbd>"
                  "<label></label>"
                  "<legend></legend>"
                  "<li></li>"
                  "<main></main>"
                  "<map></map>"
                  "<mark></mark>"
                  "<menu></menu>"
                  "<meter></meter>"
                  "<nav></nav>"
                  "<noscript></noscript>"
                  "<ol></ol>"
                  "<optgroup></optgroup>"
                  "<option></option>"
                  "<output></output>"
                  "<p></p>"
                  "<param></param>"
                  "<picture></picture>"
                  "<pre></pre>"
                  "<progress></progress>"
                  "<q></q>"
                  "<rp></rp>"
                  "<rt></rt>"
                  "<ruby></ruby>"
                  "<s></s>"
                  "<samp></samp>"
                  "<search></search>"
                  "<section></section>"
                  "<select></select>"
                  "<small></small>"
                  "<source></source>"
                  "<span></span>"
                  "<strong></strong>"
                  "<sub></sub>"
                  "<summary></summary>"
                  "<sup></sup>"
                  "<svg></svg>"
                  "<table></table>"
                  "<tbody></tbody>"
                  "<td></td>"
                  "<template></template>"
                  "<textarea></textarea>"
                  "<tfoot></tfoot>"
                  "<th></th>"
                  "<thead></thead>"
                  "<time></time>"
                  "<tr></tr>"
                  "<track></track>"
                  "<u></u>"
                  "<ul></ul>"
                  "<var></var>"
                  "<video></video>"
                  "<wbr></wbr>");

  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<!---->"
                             "<a></a>"
                             "<abbr></abbr>"
                             "<address></address>"
                             "<b></b>"
                             "<bdi></bdi>"
                             "<bdo></bdo>"
                             "<blockquote></blockquote>"
                             "<button></button>"
                             "<canvas></canvas>"
                             "<caption></caption>"
                             "<cite></cite>"
                             "<code></code>"
                             "<colgroup></colgroup>"
                             "<data></data>"
                             "<datalist></datalist>"
                             "<dd></dd>"
                             "<del></del>"
                             "<details></details>"
                             "<dfn></dfn>"
                             "<dialog></dialog>"
                             "<div></div>"
                             "<dl></dl>"
                             "<dt></dt>"
                             "<em></em>"
                             "<fieldset></fieldset>"
                             "<footer></footer>"
                             "<form></form>"
                             "<h1></h1>"
                             "<header></header>"
                             "<hgroup></hgroup>"
                             "<html></html>"
                             "<i></i>"
                             "<ins></ins>"
                             "<kbd></kbd>"
                             "<label></label>"
                             "<legend></legend>"
                             "<li></li>"
                             "<main></main>"
                             "<map></map>"
                             "<mark></mark>"
                             "<menu></menu>"
                             "<meter></meter>"
                             "<nav></nav>"
                             "<noscript></noscript>"
                             "<ol></ol>"
                             "<optgroup></optgroup>"
                             "<option></option>"
                             "<output></output>"
                             "<p></p>"
                             "<param></param>"
                             "<picture></picture>"
                             "<pre></pre>"
                             "<progress></progress>"
                             "<q></q>"
                             "<rp></rp>"
                             "<rt></rt>"
                             "<ruby></ruby>"
                             "<s></s>"
                             "<samp></samp>"
                             "<search></search>"
                             "<section></section>"
                             "<select></select>"
                             "<small></small>"
                             "<source></source>"
                             "<span></span>"
                             "<strong></strong>"
                             "<sub></sub>"
                             "<summary></summary>"
                             "<sup></sup>"
                             "<svg></svg>"
                             "<table></table>"
                             "<tbody></tbody>"
                             "<td></td>"
                             "<template></template>"
                             "<textarea></textarea>"
                             "<tfoot></tfoot>"
                             "<th></th>"
                             "<thead></thead>"
                             "<time></time>"
                             "<tr></tr>"
                             "<track></track>"
                             "<u></u>"
                             "<ul></ul>"
                             "<var></var>"
                             "<video></video>"
                             "<wbr></wbr>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_bound_text_string)
{
  // Tests whether bound test is substituted correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<div>${string}</div>");
  t.bindString("string", "Hello");
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<div>Hello</div>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_bound_text_string_xhtml_literal)
{
  // Tests whether bound test is substituted correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<div>${string}</div>");
  t.bindString("string", "<div>Hello</div>", Wt::TextFormat::XHTML);
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<div><div>Hello</div></div>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_bound_text_string_xhtml_literal_xss)
{
  // Tests whether bound test is substituted correctly, but filtered out by XSS.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<div>${string}</div>");
  t.bindString("string", "<script>Hello</script>", Wt::TextFormat::XHTML);
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<div></div>");
}


BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_bound_text_string_xhtml_resource)
{
  // Tests whether bound test is substituted correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);
  app.setLocalizedStrings(std::make_shared<TemplateLocalizedStrings>());

  Wt::WTemplate t("<div>${string}</div>");
  t.bindString("string", Wt::WString::tr("value"), Wt::TextFormat::XHTML);
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<div><div>Hello</div></div>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_bound_text_string_xhtml_resource_xss)
{
  // Tests whether bound test is substituted correctly,
  // and is not omitted by the XSS filter.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);
  app.setLocalizedStrings(std::make_shared<TemplateLocalizedStrings>());

  Wt::WTemplate t("<div>${string}</div>");
  t.bindString("string", Wt::WString::tr("script"), Wt::TextFormat::XHTML);
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<div><script>Hello</script></div>");
}

void resetStream(std::stringstream& stream)
{
  stream.str("");
  stream.clear();
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_bound_widgets)
{
  // Tests whether a bound widget can be resolved correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<div>${widget}</div>");
  std::stringstream output;

  // WInteractWidget
  t.bindWidget("widget", std::make_unique<Wt::WContainerWidget>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WImage>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><img alt=\"\" src=\"data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WLabel>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><span/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WProgressBar>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div class=\"Wt-progressbar\" style=\"display:inline-flex;\"><div class=\"Wt-pgb-bar\" style=\"width:0.000000%;\"/><div class=\"Wt-pgb-label\">0 %</div></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTable>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><table><colgroup/><tbody/></table></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTemplate>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WText>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><span/></div>");

  // WFormWidget
  t.bindWidget("widget", std::make_unique<Wt::WCheckBox>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><input type=\"checkbox\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WRadioButton>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><input type=\"radio\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WColorPicker>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><input type=\"color\" value=\"#000000\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WComboBox>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><select/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WEmailEdit>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><input type=\"email\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WSpinBox>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><input size=\"10\" title=\"\" type=\"text\" value=\"0\" class=\"Wt-spinbox\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WLineEdit>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><input size=\"10\" type=\"text\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WPushButton>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><button type=\"button\" class=\"Wt-btn\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WSlider>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div tabindex=\"0\" class=\"Wt-slider-h\" style=\"position:relative;width:150.0px;height:50.0px;left:0.0px;right:auto;top:0.0px;bottom:auto;display:inline-block;\"><div class=\"Wt-w\"/><div class=\"Wt-e\"/><div class=\"Wt-slider-bg\" style=\"width:140.0px;height:50.0px;\"><div style=\"position:relative;overflow-x:hidden;overflow-y:hidden;\"><canvas height=\"50\" width=\"140\" style=\"display:block;\"/></div></div><div class=\"fill\" style=\"position:absolute;width:10.0px;\"/><div tabindex=\"0\" class=\"handle\" style=\"position:absolute;width:20.0px;height:50.0px;left:0.0px;right:auto;top:0.0px;bottom:auto;\"/></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTextArea>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><textarea cols=\"20\" rows=\"5\"/></div>");

  // WWebWidget (not WInteractWidget)
  t.bindWidget("widget", std::make_unique<Wt::WBreak>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><br/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WFileUpload>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><form action=\"?wtd=testwtd&amp;request=resource&amp;resource=&amp;ver=1\" enctype=\"multipart/form-data\" method=\"post\" style=\"display:inline-block;\"><span><iframe src=\"?wtd=testwtd&amp;request=resource&amp;resource=&amp;ver=1\" class=\"Wt-resource\"/></span><input accept=\"\" size=\"20\" type=\"file\"/></form></div>");
  auto staticModel = Wt::makeStaticModel<std::function<std::unique_ptr<Wt::WWidget>()>>([] { return std::make_unique<Wt::WText>(); });
  t.bindWidget("widget", std::move(staticModel));
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><span><span/></span></div>");

  // WCompositeWidget
  t.bindWidget("widget", std::make_unique<Wt::WTableView>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div unselectable=\"on\" class=\"Wt-itemview Wt-tableview unselectable\" style=\"position:relative;\"><div style=\"position:absolute;left:0;right:0;\"><div class=\"Wt-header Wt-headerdiv headerrh tcontainer\" style=\"visibility:hidden;display:none;box-sizing:border-box;-moz-box-sizing:border-box;\"/><div class=\"Wt-header headerrh tcontainer\" style=\"overflow-x:hidden;overflow-y:hidden;visibility:hidden;box-sizing:border-box;-moz-box-sizing:border-box;\"><div class=\"Wt-headerdiv headerrh\"/></div><span style=\"visibility:hidden;display:none;box-sizing:border-box;-moz-box-sizing:border-box;\">...</span><div class=\"tcontainer\" style=\"position:absolute;overflow-x:auto;overflow-y:auto;visibility:hidden;box-sizing:border-box;-moz-box-sizing:border-box;\"><div class=\"Wt-spacer\" style=\"position:relative;line-height:20.0px;\"><div class=\"Wt-tv-contents\" style='position:absolute;width:100.0%;background-image:url(\"resources/themes/default/no-stripes/no-stripe-20px.gif\");'/></div></div></div></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTreeView>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div unselectable=\"on\" class=\"Wt-itemview Wt-treeview unselectable\" style=\"position:relative;display:flex;flex-flow:column;-moz-flex-flow:column;\"><div flg=\"0\" class=\"Wt-header headerrh cwidth\" style=\"overflow-x:hidden;overflow-y:hidden;flex:0 0 auto;-moz-flex:0 0 auto;\"><div class=\"Wt-headerdiv headerrh\"><div class=\"Wt-tv-row\" style=\"float:right;\"/></div></div><div class=\"cwidth\" style=\"overflow-x:auto;overflow-y:auto;flex:1 1 auto;-moz-flex:1 1 auto;\"><div style=\"height:0.0px;\"><div><div><div class=\"Wt-item\"></div></div></div></div></div></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WCalendar>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  auto result = extractMonthAndToday(output.str());
  BOOST_REQUIRE(std::get<0>(result) == Wt::WDate::longMonthName(Wt::WDate::currentDate().month()));
  BOOST_REQUIRE(std::get<1>(result) == std::to_string(Wt::WDate::currentDate().day()));
  t.bindWidget("widget", std::make_unique<Wt::WDatePicker>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><span style=\"white-space: nowrap\"><input size=\"10\" type=\"text\" style=\"vertical-align:middle;\"/><img alt=\"\" src=\"resources/date.gif\" style=\"width:16.0px;height:16.0px;vertical-align:middle;\"/></span></div>");
  t.bindWidget("widget", std::make_unique<Wt::WDefaultLoadingIndicator>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div class=\"Wt-loading\">Loading...</div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WGoogleMap>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WIconPair>("", ""));
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><span style=\"cursor:pointer;\"><img alt=\"\" src=\"data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7\"/><img alt=\"\" src=\"data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7\" style=\"display:none;\"/></span></div>");
  t.bindWidget("widget", std::make_unique<Wt::WInPlaceEdit>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><span><span style=\"cursor:default;\"/><span style=\"display:none;\">...</span></span></div>");
  t.bindWidget("widget", std::make_unique<Wt::WMediaPlayer>(Wt::MediaType::Audio));
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div class=\"jp-audio\"><div class=\"jp-type-single\"><div class=\"jp-jplayer\"></div><div class=\"jp-gui\"><div class=\"jp-interface\"><ul class=\"jp-controls\"><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Play\" class=\"jp-play Wt-rr\" style=\"display:block;\"><span>Play</span></a></li><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Pause\" class=\"jp-pause Wt-rr\" style=\"display:block;\"><span>Pause</span></a></li><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Stop\" class=\"jp-stop Wt-rr\" style=\"display:block;\"><span>Stop</span></a></li><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Mute\" class=\"jp-mute Wt-rr\" style=\"display:block;\"><span>Mute</span></a></li><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Unmute\" class=\"jp-unmute Wt-rr\" style=\"display:block;\"><span>Unmute</span></a></li><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Maximum volume\" class=\"jp-volume-max Wt-rr\" style=\"display:block;\"><span>Maximum volume</span></a></li></ul><div class=\"jp-progress\"><div class=\"jp-seek-bar Wt-progressbar\" style=\"display:flex;\"><div class=\"jp-play-bar Wt-pgb-bar\" style=\"width:0.000000%;\"/><div class=\"Wt-pgb-label\"/></div></div><div class=\"jp-volume-bar Wt-progressbar\" style=\"display:flex;\"><div class=\"jp-volume-bar-value Wt-pgb-bar\" style=\"width:80.000000%;\"/><div class=\"Wt-pgb-label\"/></div><div class=\"jp-time-holder\"><div class=\"jp-current-time\"/><div class=\"jp-duration\"/><ul class=\"jp-toggles\"><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Repeat\" class=\"jp-repeat Wt-rr\" style=\"display:block;\"><span>Repeat</span></a></li><li><a href=\"javascript:;\" tabindex=\"1\" title=\"Repeat off\" class=\"jp-repeat-off Wt-rr\" style=\"display:block;\"><span>Repeat off</span></a></li></ul></div></div><div class=\"jp-title\" style=\"display: none\"><ul><li><div/></li></ul></div></div></div></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WMenu>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><ul/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WPanel>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div class=\"Wt-panel Wt-outset\"><div/></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WPopupWidget>(std::make_unique<Wt::WContainerWidget>()));
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div class=\"Wt-popup Wt-outset\" style=\"position:absolute;z-index:1100;display:none;\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WSplitButton>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><span class=\"btn-group\"><button type=\"button\" class=\"Wt-btn\"/><button type=\"button\" class=\"dropdown-toggle Wt-btn\"/></span></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTabWidget>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div><ul class=\"Wt-tabs\"/><div class=\"Wt-stack\" style=\"overflow-x:hidden;overflow-y:hidden;\"/></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WToolBar>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div class=\"btn-group\"/></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTree>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div unselectable=\"on\" class=\"Wt-tree Wt-sentinel unselectable\"><div class=\"Wt-item Wt-end Wt-root\"><span class=\"Wt-ctrl Wt-expand\" style=\"cursor:pointer;display:none;\"><img alt=\"\" src=\"resources/themes/default/nav-plus.gif\" style=\"display:none;\"/><img alt=\"\" src=\"resources/themes/default/nav-minus.gif\"/></span><span class=\"Wt-ctrl Wt-noexpand\"/><div><span class=\"Wt-label\"/></div></div><div style=\"clear: both\"/><ul class=\"Wt-root\"/></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTreeNode>(""));
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div unselectable=\"on\" class=\"Wt-tree Wt-trunk unselectable\"><div class=\"Wt-item  ??selected??\"><span class=\"Wt-ctrl Wt-expand\" style=\"cursor:pointer;display:none;\"><img alt=\"\" src=\"resources/themes/default/nav-plus.gif\"/><img alt=\"\" src=\"resources/themes/default/nav-minus.gif\" style=\"display:none;\"/></span><span class=\"Wt-ctrl Wt-noexpand\"/><div><span class=\"Wt-label\"/></div></div><div style=\"clear: both\"/><ul style=\"display:none;\"/></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WTreeTable>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div class=\"Wt-treetable\" style=\"position:relative;\"><div class=\"Wt-header header\"><div class=\"Wt-sbspacer\"/><div style=\"float:right;\"/><span/></div><div class=\"Wt-content\" style=\"overflow-x:auto;overflow-y:auto;\"><div unselectable=\"on\" class=\"Wt-tree Wt-sentinel unselectable\" style=\"width:100.0%;margin-top:3.0px;margin-right:0.0px;margin-bottom:0.0px;margin-left:0.0px;\"><div class=\"Wt-item Wt-end Wt-root\"><span class=\"Wt-ctrl Wt-expand\" style=\"cursor:pointer;display:none;\"><img alt=\"\" src=\"resources/themes/default/nav-plus.gif\" style=\"display:none;\"/><img alt=\"\" src=\"resources/themes/default/nav-minus.gif\"/></span><span class=\"Wt-ctrl Wt-noexpand\"/><div><span class=\"Wt-label\"/></div></div><div style=\"clear: both\"/><ul class=\"Wt-root\"/></div></div></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::WVirtualImage>(1, 1, 1, 1));
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div style=\"position:relative;width:1.0px;height:1.0px;\"><div style=\"position:absolute;width:100.0%;height:100.0%;overflow-x:hidden;overflow-y:hidden;\"><div style=\"position:absolute;\"/></div></div></div>");

  // WPaintedWidget
  t.bindWidget("widget", std::make_unique<Wt::Chart::WAxisSliderWidget>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div style=\"position:relative;\"><div style=\"position:absolute;left:0;right:0;\"><div style=\"position:relative;overflow-x:hidden;overflow-y:hidden;\"><canvas height=\"0\" width=\"0\" style=\"display:block;\"/></div></div></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::Chart::WCartesianChart>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div style=\"position:relative;\"><div style=\"position:absolute;left:0;right:0;\"><div style=\"position:relative;overflow-x:hidden;overflow-y:hidden;\"><canvas height=\"0\" width=\"0\" style=\"display:block;\"/></div></div></div></div>");
  t.bindWidget("widget", std::make_unique<Wt::Chart::WPieChart>());
  resetStream(output);
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(filterOutIds(output.str()) == "<div><div style=\"position:relative;\"><div style=\"position:absolute;left:0;right:0;\"><div style=\"position:relative;overflow-x:hidden;overflow-y:hidden;\"><canvas height=\"0\" width=\"0\" style=\"display:block;\"/></div></div></div></div>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_condition_true)
{
  // Tests whether a valid condition is filtered out, and applies its
  // content correctly.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<div>${<if:true>}<span>Hello</span>${</if:true>}</div>");
  t.setCondition("if:true", true);
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<div><span>Hello</span></div>");
}

BOOST_AUTO_TEST_CASE(WTemplate_renderTemplateText_condition_false)
{
  // Tests whether a invalid condition is filtered out, and doesn't
  // apply its content.

  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WTemplate t("<div>${<if:true>}<span>Hello</span>${</if:true>}</div>");
  t.setCondition("if:true", false);
  std::stringstream output;
  t.renderTemplateText(output, t.templateText());
  BOOST_REQUIRE(output.str() == "<div></div>");
}

