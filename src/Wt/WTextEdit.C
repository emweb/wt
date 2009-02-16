
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WTextEdit"

#include "DomElement.h"
#include "WebSession.h"

namespace Wt {

WTextEdit::WTextEdit(WContainerWidget *parent)
  : WTextArea(),
    rendered_(false),
    contentChanged_(false)
{
  setInline(false);
  initTinyMCE();

  if (parent)
    parent->addWidget(this);
}

WTextEdit::WTextEdit(const WString& text, WContainerWidget *parent)
  : WTextArea(text),
    rendered_(false),
    contentChanged_(false)
{
  setInline(false);
  initTinyMCE();

  if (parent)
    parent->addWidget(this);
}

WTextEdit::~WTextEdit()
{
  // to have virtual renderRemove():
  setParent(0);
}

void WTextEdit::load()
{
  wApp->addAutoJavaScript("{var e=" + jsRef() + ";"
			  "if(e && e.ed){"
			  "" "e.ed.save();"
			  "" WT_CLASS ".tinyMCEAdjust(e);"
			  "}}");

  buttons_[0] = "fontselect,|,bold,italic,underline,|,fontsizeselect,|"
    ",forecolor,backcolor,|"
    ",justifyleft,justifycenter,justifyright,justifyfull,|,anchor,|"
    ",numlist,bullist";

  WTextArea::load();
}

void WTextEdit::setStyleSheet(const std::string& uri)
{
  styleSheet_ = uri;
}

void WTextEdit::setExtraPlugins(const std::string& plugins)
{
  extraPlugins_ = plugins;
}

void WTextEdit::setToolBar(int i, const std::string& config)
{
  buttons_[i] = config;
}

DomElement *WTextEdit::renderRemove()
{
  DomElement *e = WWebWidget::renderRemove();
  e->callJavaScript(jsRef() + ".ed.remove();", true);
  return e;
}

void WTextEdit::initTinyMCE()
{
  std::string tinyMCEBaseURL = WApplication::resourcesUrl() + "tiny_mce/";

  WApplication::readConfigurationProperty("tinyMCEBaseURL", tinyMCEBaseURL);

  if (!tinyMCEBaseURL.empty()
      && tinyMCEBaseURL[tinyMCEBaseURL.length()-1] != '/')
    tinyMCEBaseURL += '/';

  WApplication *app = WApplication::instance();

  app->doJavaScript("window.tinyMCE_GZ = { loaded: true };", false);
  if (app->require(tinyMCEBaseURL + "tiny_mce.js", "window['tinyMCE']")) {
    /*
      interesting config options:

      directionality, docs_language, language ?
      entities ?

      we should not use display:none for hiding?
    */
    app->doJavaScript("tinymce.dom.Event._pageInit();tinyMCE.init();", false);
    app->styleSheet().addRule(".mceEditor", "height: 100%;");

    // Adjust the height: this can only be done by adjusting the iframe height.
    app->doJavaScript
      (WT_CLASS ".tinyMCEAdjust=function(e){"
       "if (!e.ed.contentAreaContainer) return;"
       "var tbl=" WT_CLASS ".getElement(e.id + '_tbl');"
       "var iframe = e.ed.contentAreaContainer.firstChild;"
       "var th=" WT_CLASS ".pxself(tbl, 'height');"
       "if (th==0)"                                 // no height set in pixels
       """if (e.parentNode.className=='Wt-grtd') {" // are we in a layout?
       ""  "iframe.style.height='0px';"             // momentarily set height to 0
       ""  "th=e.parentNode.offsetHeight"
       ""    "-" WT_CLASS ".pxself(e.parentNode, 'paddingTop')"
       ""    "-" WT_CLASS ".pxself(e.parentNode, 'paddingBottom');"
       """} else "
       ""  "return;"                                // no specific height wanted
       "th -= iframe.parentNode.offsetTop + 2;"
       "if (th <= 0)"
       """return;"
       "var nh=th+'px';"
       "if (iframe.style.height != nh) iframe.style.height=nh;"
       "};", false);
  }
}

void WTextEdit::resize(const WLength& width, const WLength& height)
{
  WTextArea::resize(width, height);
}

void WTextEdit::setText(const WString& text)
{
  WTextArea::setText(text);
  contentChanged_ = true;
}

void WTextEdit::updateDom(DomElement& element, bool all)
{
  WTextArea::updateDom(element, all);

  if (element.type() == DomElement_TEXTAREA)
    element.removeProperty(PropertyStyleDisplay);

  // we are creating the actual element
  if (all && element.type() == DomElement_TEXTAREA) {
    std::stringstream config;

    config <<
      "{button_tile_map:true"
      ",doctype:'" + wApp->docType() + "'"
      ",relative_urls:true"
      ",plugins:'safari";

    if (!extraPlugins_.empty())
      config << ',' << extraPlugins_;
    config << "'";

    config << ",theme:'advanced'";

    for (unsigned i = 0; i < 4; ++i)
      config << ",theme_advanced_buttons" << (i+1) << ":'"
	     << buttons_[i] << '\'';

    config <<
      ",theme_advanced_toolbar_location:'top'"
      ",theme_advanced_toolbar_align:'left'";

    if (!styleSheet_.empty())
      config << ",content_css: '" << styleSheet_ << '\''; 

    std::string init_cb = wApp->javaScriptClass() + ".tmce" + formName();

    config <<
      ",init_instance_callback: '" << init_cb << "'"
      "}";

    DomElement dummy(DomElement::ModeUpdate, DomElement_TABLE);
    updateDom(dummy, true);

    element.callJavaScript("{var e=" + jsRef() + ";e.ed=new tinymce.Editor('"
			   + formName() + "'," + config.str() + ");"
			   "e.ed.render();}");

    /*
     * When initialized, we apply the inline style to the table element
     * and adjust the element height.
     */
    element.callJavaScript(init_cb + "=function(){"
			   "var d=" WT_CLASS ".getElement('" + formName()
			   + "_tbl'); d.style.cssText='width:100%;"
			   + dummy.cssStyle() + "';"
			   WT_CLASS ".tinyMCEAdjust(" + jsRef() + ");};");
    contentChanged_ = false;

    rendered_ = true;
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
  DomElement *e = DomElement::getForUpdate(formName() + "_tbl",
					   DomElement_TABLE);
  updateDom(*e, false);

  WTextArea::getDomChanges(result, app);

  result.push_back(e);
}

}
