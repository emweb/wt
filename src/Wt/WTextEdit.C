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

#ifndef WT_DEBUG_JS
#include "js/WTextEdit.min.js"
#endif

namespace Wt {

typedef std::map<std::string, boost::any> SettingsMapType;

WTextEdit::WTextEdit(WContainerWidget *parent)
  : WTextArea(parent),
    onChange_(this, "change"),
    contentChanged_(false)
{
  init();
}

WTextEdit::WTextEdit(const WT_USTRING& text, WContainerWidget *parent)
  : WTextArea(text, parent),
    onChange_(this, "change"),
    contentChanged_(false)
{
  init();
}

void WTextEdit::init()
{
  WApplication *app = WApplication::instance();

  setInline(false);

  initTinyMCE();

  version_ = getTinyMCEVersion();

  setJavaScriptMember(" WTextEdit", "new " WT_CLASS ".WTextEdit("
		      + app->javaScriptClass() + "," + jsRef() + ");");

  setJavaScriptMember
    (WT_RESIZE_JS,
     "function(e, w, h) { var obj = $('#" + id() + "').data('obj'); "
     "obj.wtResize(e, w, h); };");

  std::string direction = app->layoutDirection() == LeftToRight ? "ltr" : "rtl";
  setConfigurationSetting("directionality", direction);

  std::string toolbar;
  if (version_ < 4)
    toolbar = "fontselect,|,bold,italic,underline,|,fontsizeselect,|"
      ",forecolor,backcolor,|"
      ",justifyleft,justifycenter,justifyright,justifyfull,|,anchor,|"
      ",numlist,bullist";
  else
    toolbar = "undo redo | styleselect | bold italic | link";

  setToolBar(0, toolbar);
  for (int i = 1; i <= 3; i++)
    setToolBar(i, std::string());

  setConfigurationSetting("doctype", wApp->docType());
  setConfigurationSetting("relative_urls", true);

  if (version_ < 4) {
    //this setting is no longer mentioned in the tinymce documenation though...
    setConfigurationSetting("button_tile_map", true);
    setConfigurationSetting("theme", std::string("advanced"));
    setConfigurationSetting("theme_advanced_toolbar_location", 
			    std::string("top"));
    setConfigurationSetting("theme_advanced_toolbar_align",
			    std::string("left"));
  }

  onChange_.connect(this, &WTextEdit::propagateOnChange);
}

WTextEdit::~WTextEdit()
{
  // to have virtual renderRemoveJs():
  setParentWidget(0);
}

void WTextEdit::propagateOnChange()
{
  changed().emit();
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
  std::string setting;
  if (version_ < 4)
    setting = "theme_advanced_buttons";
  else
    setting = "toolbar";

  setConfigurationSetting
    (setting + boost::lexical_cast<std::string>(i + 1), config);
}

const std::string WTextEdit::toolBar(int i) const
{
  std::string setting;
  if (version_ < 4)
    setting = "theme_advanced_buttons";
  else
    setting = "toolbar";

  return asString(configurationSetting
		  (setting + boost::lexical_cast<std::string>(i + 1))).toUTF8();
}

std::string WTextEdit::renderRemoveJs()
{
  if (isRendered())
    return jsRef() + ".ed.remove();" WT_CLASS ".remove('" + id() + "');";
  else
    return WTextArea::renderRemoveJs();
}

int WTextEdit::getTinyMCEVersion()
{
  std::string version = "3";
  WApplication::readConfigurationProperty("tinyMCEVersion", version);
  return boost::lexical_cast<int>(version);
}

void WTextEdit::initTinyMCE()
{
  const char *THIS_JS = "js/WTextEdit.js";

  WApplication *app = WApplication::instance();

  if (!app->javaScriptLoaded(THIS_JS)) {
    if (app->environment().ajax())
      app->doJavaScript("window.tinyMCE_GZ = { loaded: true };", false);

    std::string tinyMCEURL;
    WApplication::readConfigurationProperty("tinyMCEURL", tinyMCEURL);
    if (tinyMCEURL.empty()) {
      int version = getTinyMCEVersion();

      std::string folder = version == 3 ? "tiny_mce/" : "tinymce/";
      std::string jsFile = version == 3 ? "tiny_mce.js" : "tinymce.js";

      std::string tinyMCEBaseURL = WApplication::relativeResourcesUrl() + folder;
      WApplication::readConfigurationProperty("tinyMCEBaseURL", tinyMCEBaseURL);

      if (!tinyMCEBaseURL.empty()
          && tinyMCEBaseURL[tinyMCEBaseURL.length()-1] != '/')
        tinyMCEBaseURL += '/';
      tinyMCEURL = tinyMCEBaseURL + jsFile;
    }

    app->require(tinyMCEURL, "window['tinyMCE']");
    app->styleSheet().addRule(".mceEditor",
			      "display: block; position: absolute;");

    LOAD_JAVASCRIPT(app, THIS_JS, "WTextEdit", wtjs1);
  }
}

void WTextEdit::setReadOnly(bool readOnly)
{
  WTextArea::setReadOnly(readOnly);

  if (readOnly)
    setConfigurationSetting("readonly", std::string("1"));
  else
    setConfigurationSetting("readonly", boost::any());
}

void WTextEdit::propagateSetEnabled(bool enabled)
{
  WTextArea::propagateSetEnabled(enabled);

  setReadOnly(!enabled);
}

void WTextEdit::setPlaceholderText(const WString& placeholder)
{
  throw WException("WTextEdit::setPlaceholderText() is not implemented.");
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

  if (version_ == 3) {
    if (!plugins.empty())
      plugins += ",";
    plugins += "safari";
  }

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
      ",init_instance_callback: obj.init"
      "}";

    DomElement dummy(DomElement::ModeUpdate, DomElement_TABLE);
    updateDom(dummy, true);

    element.callJavaScript("(function() { "
			   """var obj = $('#" + id() + "').data('obj');"
			   """obj.render(" + config.str() + ","
			   + jsStringLiteral(dummy.cssStyle()) + ","
			   + (changed().isConnected() ? "true" : "false")
			   + ");"
			   "})();");

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
  if (!value.empty())
    configurationSettings_[name] = value;
  else
    configurationSettings_.erase(name);
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
