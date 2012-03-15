/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WTextEdit"
#include "Wt/WBoostAny"

#include "DomElement.h"

namespace Wt {

typedef std::map<std::string, boost::any> SettingsMapType;

WTextEdit::WTextEdit(WContainerWidget *parent)
  : WTextArea(parent),
    contentChanged_(false)
{
  init();
}

WTextEdit::WTextEdit(const WT_USTRING& text, WContainerWidget *parent)
  : WTextArea(text, parent),
    contentChanged_(false)
{
  init();
}

void WTextEdit::init()
{
  WApplication *app = WApplication::instance();

  setInline(false);

  initTinyMCE();
  
  setJavaScriptMember
    (WT_RESIZE_JS,
     "function(e,w,h){" WT_CLASS ".tinyMCEResize(e, w, h); };");

  std::string direction = app->layoutDirection() == LeftToRight ? "ltr" : "rtl";
  setConfigurationSetting("directionality", direction);

  std::string toolbar = 
    "fontselect,|,bold,italic,underline,|,fontsizeselect,|"
    ",forecolor,backcolor,|"
    ",justifyleft,justifycenter,justifyright,justifyfull,|,anchor,|"
    ",numlist,bullist";
  setToolBar(0, toolbar);
  for (int i = 1; i <= 3; i++)
    setToolBar(i, std::string());

  //this setting is no longer mentioned in the tinymce documenation though...
  setConfigurationSetting("button_tile_map", true);
  
  setConfigurationSetting("doctype", wApp->docType());
  setConfigurationSetting("relative_urls", true);
  setConfigurationSetting("theme", std::string("advanced"));
  setConfigurationSetting("theme_advanced_toolbar_location", 
			  std::string("top"));
  setConfigurationSetting("theme_advanced_toolbar_align", std::string("left"));
}

WTextEdit::~WTextEdit()
{
  // to have virtual renderRemoveJs():
  setParentWidget(0);
}

void WTextEdit::setStyleSheet(const std::string& uri)
{
  setConfigurationSetting("content_css", uri);
}

const std::string WTextEdit::styleSheet() const
{
  return asString(configurationSetting("content_css")).toUTF8();
}

void WTextEdit::setExtraPlugins(const std::string& plugins)
{
  setConfigurationSetting("plugins", plugins);
}

const std::string WTextEdit::extraPlugins() const
{
  return asString(configurationSetting("plugins")).toUTF8();
}

void WTextEdit::setToolBar(int i, const std::string& config)
{
  setConfigurationSetting
    ("theme_advanced_buttons" + boost::lexical_cast<std::string>(i + 1),
     config);
}

const std::string WTextEdit::toolBar(int i) const
{
  return asString(configurationSetting
		  ("theme_advanced_buttons"
		   + boost::lexical_cast<std::string>(i + 1))).toUTF8();
}

std::string WTextEdit::renderRemoveJs()
{
  if (isRendered())
    return jsRef() + ".ed.remove();" WT_CLASS ".remove('" + id() + "');";
  else
    return WTextArea::renderRemoveJs();
}

void WTextEdit::initTinyMCE()
{
  std::string tinyMCEBaseURL = WApplication::resourcesUrl() + "tiny_mce/";

  WApplication::readConfigurationProperty("tinyMCEBaseURL", tinyMCEBaseURL);

  if (!tinyMCEBaseURL.empty()
      && tinyMCEBaseURL[tinyMCEBaseURL.length()-1] != '/')
    tinyMCEBaseURL += '/';

  WApplication *app = WApplication::instance();

  if (app->environment().ajax())
    app->doJavaScript("window.tinyMCE_GZ = { loaded: true };", false);

  if (app->require(tinyMCEBaseURL + "tiny_mce.js", "window['tinyMCE']")) {
    /*
      we should not use display:none for hiding?
    */

    app->doJavaScript("if (!tinymce.dom.Event.domLoaded)"
		      "  tinymce.dom.Event.domLoaded = true;"
		      "tinyMCE.init();", false);
    app->styleSheet().addRule(".mceEditor", "height: 100%;");

    // Adjust the height: this can only be done by adjusting the iframe height.
    app->doJavaScript
      (WT_CLASS ".tinyMCEResize=function(e,w,h){"
       """e.style.height = (h - 2) + 'px';"
       ""
       """var iframe = " WT_CLASS ".getElement(e.id + '_ifr');"
       """if (iframe) {"
       ""  "var row=iframe.parentNode.parentNode,"
       ""      "tbl=row.parentNode.parentNode,"
       ""      "i, il;"
       ""
       // deduct height of toolbars
       ""  "for (i=0, il=tbl.rows.length; i<il; i++) {"
       ""    "if (tbl.rows[i] != row)"
       ""      "h -= Math.max(28, tbl.rows[i].offsetHeight);"
       ""  "}"
       ""
       ""  "h = (h - 2) + 'px';"
       ""
       ""  "if (iframe.style.height != h) iframe.style.height=h;"
       """}"
       "};", false);
  }
}

void WTextEdit::resize(const WLength& width, const WLength& height)
{
  WTextArea::resize(width, height);
}

void WTextEdit::setText(const WT_USTRING& text)
{
  WTextArea::setText(text);
  contentChanged_ = true;
}

std::string WTextEdit::plugins() const
{
  std::string plugins = extraPlugins();
  if (!plugins.empty())
    plugins += ",";
  plugins += "safari";
  return plugins;
}

void WTextEdit::updateDom(DomElement& element, bool all)
{
  WTextArea::updateDom(element, all);

  if (element.type() == DomElement_TEXTAREA)
    element.removeProperty(PropertyStyleDisplay);

  // we are creating the actual element
  if (all && element.type() == DomElement_TEXTAREA) {
    std::stringstream config;
    config << "{";

    bool first = true;

    for (SettingsMapType::const_iterator it = configurationSettings_.begin();
	 it != configurationSettings_.end(); ++it) {
      if (it->first == "plugins")
	continue;

      if (!first)
	config << ',';

      first = false;

      config << it->first << ": "
	     << Impl::asJSLiteral(it->second, XHTMLUnsafeText);
    }

    if (!first)
      config << ',';

    config << "plugins: '" << plugins() << "'";

    config <<
      ",init_instance_callback: " << jsRef() << ".init" << ""
      "}";

    DomElement dummy(DomElement::ModeUpdate, DomElement_TABLE);
    updateDom(dummy, true);

    /*
     * When initialized, we apply the inline style.
     */
    element.callMethod("init=function(){"
		       "var d=" WT_CLASS ".getElement('" + id() + "_tbl');"
		       "d.style.cssText='width:100%;" + dummy.cssStyle() + "';"
		       "};");
    element.callMethod("ed=new tinymce.Editor('" + id() + "',"
		       + config.str() + ");");
    element.callMethod("ed.render();");

    contentChanged_ = false;
  }

  if (!all && contentChanged_) {
    element.callJavaScript(jsRef() + ".ed.load();");
    contentChanged_ = false;
  }
}

void WTextEdit::getDomChanges(std::vector<DomElement *>& result,
			      WApplication *app)
{
  /*
   * We apply changes directly to the table element, except of the textarea
   * contents. Therefore we first update the TABLE element, then collect
   * the contentChange to the TEXTAREA element, but reverse the order in
   * which they get applied since the load() statement expects the contents
   * to be set in the textarea first.
   */

  /*
   * Problem! ed.render() returns before the element is actually rendered,
   * and therefore, the _tbl element may not yet be available.
   *
   * This causes fail when a text edit is progressively enhanced. The solution
   * is to listen for the onInit() event -> we should be able to add a
   * wrapping ... .onInit(function(ed) { .... }) around the changes
   */
  DomElement *e = DomElement::getForUpdate(formName() + "_tbl",
					   DomElement_TABLE);
  updateDom(*e, false);

  WTextArea::getDomChanges(result, app);

  result.push_back(e);
}

bool WTextEdit::domCanBeSaved() const
{
  return false;
}

int WTextEdit::boxPadding(Orientation orientation) const
{
  return 0;
}

int WTextEdit::boxBorder(Orientation orientation) const
{
  return 0;
}

void WTextEdit::setConfigurationSetting(const std::string& name, 
					const boost::any& value)
{
  configurationSettings_[name] = value;
}

boost::any WTextEdit::configurationSetting(const std::string& name) const
{
  SettingsMapType::const_iterator it = configurationSettings_.find(name);

  if (it != configurationSettings_.end())
    return it->second;
  else
    return boost::any();
}

}
