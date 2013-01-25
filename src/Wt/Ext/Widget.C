/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/Widget"
#include "Wt/Ext/Container"
#include "Wt/Ext/FormField"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "EscapeOStream.h"
#include "DomElement.h"
#include "WebUtils.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdio>

namespace Wt {

  /*! \brief Namespace for \ref ext */
  namespace Ext {

Widget::Widget(WContainerWidget *parent)
  : WWebWidget(parent)
{
  setInline(false);
  initExt();

  // because the stub uses display: none, while Widget uses Ext-based widget
  // hiding showing in an always visible container.
  setLoadLaterWhenInvisible(false);
}

void Widget::initExt()
{
  std::string extBaseURL = "ext";
  WApplication::readConfigurationProperty("extBaseURL", extBaseURL);

  if (!extBaseURL.empty() && extBaseURL[extBaseURL.length()-1] != '/')
    extBaseURL += '/';

  WApplication *app = WApplication::instance();

  if (app->require(extBaseURL + "ext-base.js", "window['Ext']")) {
    app->require(extBaseURL + "ext-all.js", "window.Ext['DomHelper']");
    app->useStyleSheet(extBaseURL + "resources/css/ext-all.css");

    // fixes for Firefox 3:
    app->styleSheet().addRule(".x-date-middle", "width:130px;");

    // rendering glitches on all browsers:
    app->styleSheet().addRule(".ext-gecko .x-form-text", "margin-top: -1px;");
    app->styleSheet().addRule(".ext-safari .x-form-text", "margin-top: -1px;");
    app->styleSheet().addRule(".ext-ie .x-form-text",
			      "margin-top: 0px !important;"
			      "margin-bottom: 0px !important;");
    app->doJavaScript(/*app->javaScriptClass() + '.' + */ "ExtW = new Array();"
		      "Ext.QuickTips.init();"
		      "Ext.BLANK_IMAGE_URL='" + extBaseURL 
		      + "resources/images/default/s.gif';", false);
    app->declareJavaScriptFunction("deleteExtW",
				   "" "function(id){"
				   ""   "var w=ExtW[id];"
				   ""   "if(w){"
				   ""      "if (w.el && w.destroy) w.destroy();"
				   ""      "delete ExtW[id];"
				   ""   "}"
				   "" "}");

    if (app->environment().agentIsIE())
      app->doJavaScript
	("if ((typeof Range !== 'undefined')"
	 ""    "&& !Range.prototype.createContextualFragment) {"
	 """Range.prototype.createContextualFragment = function(html) {"
	 ""  "var startNode = this.startContainer;"
	 ""  "var doc = startNode.nodeType == 9 ? startNode :"
	 ""            "startNode.ownerDocument;"
	 ""  "var container = doc.createElement('div');"
	 ""  "container.innerHTML = html;"
	 ""  "var frag = doc.createDocumentFragment(), n;"
	 ""  "while ( (n = container.firstChild) ) {"
	 ""    "frag.appendChild(n);"
	 ""  "}"
	 ""  "return frag;"
	 """};"
	 "}", false);
    /*
     * Normally, Ext does this in its onReady function, but this is not
     * fired when loading ExtJS on demand.
     */
    std::string bodyClass;
    if (app->environment().agentIsIE()) {
      bodyClass = " ext-ie ";
      bodyClass += app->environment().agent() == WEnvironment::IE6
	? "ext-ie6" : "ext-ie7 ";
    } else if (app->environment().agentIsSafari())
      bodyClass = " ext-safari";
    else if (app->environment().agentIsOpera())
      bodyClass = " ext-opera";
    else if (app->environment().agentIsGecko())
      bodyClass = " ext-gecko";

    const std::string& ua = app->environment().userAgent();

    if (ua.find("Linux") != std::string::npos)
      bodyClass += " ext-linux";
    if (ua.find("Macintosh") != std::string::npos
	|| ua.find("Mac OS X") != std::string::npos)
      bodyClass += " ext-mac";
    
    app->setBodyClass(app->bodyClass() + bodyClass);
    app->setHtmlClass(app->htmlClass() + " ext-strict");
  }
}

Widget::~Widget()
{
  // to have virtual renderRemoveJs():
  setParentWidget(0);

  // in any case, delete Ext classes:
  WApplication *app = WApplication::instance();
  app->doJavaScript(app->javaScriptClass()
		    + ".deleteExtW('" + id() + "');");
}

void Widget::setHidden(bool hidden, const WAnimation& animation)
{
  WWebWidget::setHidden(hidden, animation);

  if (isRendered() || !canOptimizeUpdates())
    addUpdateJS(elVar() + ".setVisible(" + (hidden ? "false" : "true") + ");");
}

std::string Widget::extId() const
{
  return elVar();
}

std::string Widget::configStruct()
{
  std::stringstream config;

  config << "{a:0";
  createConfig(config);
  config << "}";

  return config.str();
}

bool Widget::applySelfCss() const
{
  return true;
}

void Widget::createConfig(std::ostream& config)
{ 
  Container *c = dynamic_cast<Container *>(parent());

  config << ",id:'" << extId() << "'";

  if (applySelfCss()) {
    if (!styleClass().empty())
      config << ",cls:'" << styleClass().toUTF8() << "'";

    std::string cssStyle = inlineCssStyle();

    if (!cssStyle.empty())
      config << ",style:'" << cssStyle << "'";
  }

  if (c)
    c->addLayoutConfig(this, config);
}

const std::string Widget::elVar() const 
{
#ifndef WT_TARGET_JAVA
  char buf[20];
  std::sprintf(buf, "elo%x", rawUniqueId());
  return std::string(buf);
#else
  return "elo" + Utils::toHexString(rawUniqueId());
#endif // WT_TARGET_JAVA
}

const std::string Widget::elRef() const
{
  return /* WApplication::instance()->javaScriptClass() + '.' + */
    "ExtW['" + id() + "']";
}

void Widget::updateDom(DomElement& element, bool all)
{
  WWebWidget::updateDom(element, all);
  element.removeAttribute("title");
}

void Widget::propagateRenderOk(bool deep)
{
  jsUpdates_.clear();

  WWebWidget::propagateRenderOk(deep);
}

void Widget::updateExt()
{ }

std::string Widget::createMixed(const std::vector<WWidget *>& items,
				std::stringstream& js)
{
  std::string refs;
 
  for (unsigned i = 0; i < items.size(); ++i) {
    WWidget *c = items[i];
    Widget *w = dynamic_cast<Widget *>(c);
    FormField *ff = dynamic_cast<FormField *>(c);

    std::string var;
    if (w && !ff) {
      var = w->createExtElement(js, 0);
    } else {
      WStringStream wjs(js);
      var = c->createJavaScript(wjs, "document.body.appendChild(");
    }

    if (i != 0)
      refs += ",";

    refs += var;
  }

  return refs;
}

void Widget::renderExtAdd(WWidget *c)
{
  if (!isRendered())
    return;

  std::stringstream js;

  Widget *w = dynamic_cast<Widget *>(c);
  FormField *ff = dynamic_cast<FormField *>(c);

  if (w && !ff) {
    std::string var = w->createExtElement(js, 0);
    js << elVar() << ".add(" << var << ");";
  } else {
    WStringStream wjs(js);
    c->createJavaScript(wjs, elVar() + ".add(");
  }

  addUpdateJS(js.str());
}

std::string Widget::createExtElement(std::stringstream& alljs,
				     DomElement *inContainer)
{
  if (inContainer) {
    updateDom(*inContainer, true);
    inContainer->removeProperty(PropertyStyleDisplay);
  }

  setRendered(false);

  alljs << "var " << elVar() << ";"
	<< createJS(inContainer)
	<< elRef() << "=" << elVar() << ";";

  if (isHidden())
    alljs << elVar() << ".hide();";

  jsUpdates_.clear();

  if (!inContainer) {
    DomElement *e
      = DomElement::updateGiven(elVar() + ".getEl().dom", domElementType());
    updateDom(*e, true);

    {
      EscapeOStream out(alljs);
      e->asJavaScript(out, DomElement::Update);
    }

    delete e;
  }

  setRendered(true);

  return elVar();
}

void Widget::addOrphan(WWidget *child)
{
  WWebWidget::addChild(child);
}

void Widget::removeOrphan(WWidget *child)
{
  removeChild(child);
}

void Widget::addUpdateJS(const std::string& js)
{
  if (!js.empty()) {
    jsUpdates_ += js;
    repaint();
  }
}

DomElementType Widget::domElementType() const
{
  return isInline() ? DomElement_SPAN : DomElement_DIV;
}

DomElement *Widget::createDomElement(WApplication *app)
{
  DomElement *result = DomElement::createNew(domElementType());
  setId(result, app);

  std::stringstream js;
  createExtElement(js, result);
  result->callJavaScript(js.str());

  return result;
}

void Widget::getDomChanges(std::vector<DomElement *>& result,
			   WApplication *app)
{
  std::string el = elRef() + ".el.dom";

  DomElement *e = DomElement::updateGiven(el, domElementType());

  updateExt();

  if (!jsUpdates_.empty()) {
    e->callJavaScript("var " + elVar() + "=" + elRef() + ";" + jsUpdates_);
    jsUpdates_.clear();
  }

  updateDom(*e, false);
  e->removeProperty(PropertyStyleDisplay);
  result.push_back(e);
}

std::string Widget::renderRemoveJs()
{
  return elRef() + ".hide();" WT_CLASS ".remove('" + id() + "');";
}

void Widget::updateWtSignal(EventSignalBase *s, const std::string& name,
			    const std::string& eventArgs,
			    const std::string& wtArg)
{
  if (s->needsUpdate(false)) {
    addUpdateJS(elVar() + ".wt" + name + "="
		+ jsWtSignalFunction(s, name, eventArgs, wtArg) + ";");
    s->updateOk();
  }
}

std::string Widget::jsWtSignalFunction(EventSignalBase *s,
				       const std::string& name,
				       const std::string& eventArgs,
				       const std::string& wtArg)
{
  return "function(" + eventArgs + "){"
    + s->createUserEventCall(std::string(), std::string(), name, wtArg,
			     std::string(), std::string(), std::string(),
			     std::string(), std::string())
    + "}";
}

void Widget::addWtSignalConfig(const std::string& handler, EventSignalBase *s, 
			       const std::string& name,
			       const std::string& eventArgs,
			       const std::string& wtArg,
			       std::ostream& config)
{
  config << "," << handler << ":function(" << eventArgs << "){"
	 << elVar() << ".wt" << name << "(" << eventArgs << ");}"
	 << ",wt" << name << ":"
	 << jsWtSignalFunction(s, name, eventArgs, wtArg);

  s->updateOk();
}

void Widget::bindEventHandler(const std::string& eventName,
			      const std::string& handler,
			      std::stringstream& js)
{
  js << elVar() << ".on('" << eventName << "',"
     << elVar() << "." << handler << ");";
}

/*! \defgroup ext Ext widgets (Wt::Ext, deprecated)
 *  \brief %Wt %Ext library with JavaScript-only widgets (<b>deprecated</b>).
 *
 * \section bla 1. Introduction
 *
 * An add-on library to %Wt (wtext) provides additional widgets that
 * are all contained within the Wt::Ext namespace. These widgets are
 * implemented using an open-source third-party JavaScript library,
 * extjs (http://extjs.com/), version 2.0, or 2.1 but not higher (does
 * not support Ext 2.2), e.g. http://extjs.com/deploy/ext-2.0.2.zip.
 *
 * Unlike plain %Wt widgets, these widgets require the availability of
 * JavaScript (and a sufficiently recent browser). They do, however,
 * have a polished default look (certainly compared to unstyled plain
 * %Wt widgets), and add several new capabilities to %Wt:
 *
 * <ul>
 *
 *   <li>A Container widget which supports layout using layout
 *     managers, and a Panel, which inherits Container, which adds
 *     standard GUI functionality to a container, such as tool bars,
 *     and support for collapsing and resizing</li>
 *
 *   <li>Form fields that support client-side validation (CheckBox,
 *     ComboBox, DateField, LineEdit, NumberField, RadioButton). This
 *     has been integrated together with the server side validation in
 *     the standard WValidator classes (WDateValidator,
 *     WDoubleValidator, WIntValidator, WLengthValidator,
 *     WRegExpValidator). In this way, a single validator object
 *     specifies at the same time the client- and server-side
 *     validation. In this way, the user is given instant feed-back
 *     using client-side validation, but the data is also validated
 *     (again) as it arrives on the server, since nothing prevents the
 *     client JavaScript code from be hacked or circumvented.</li>
 *
 *   <li>TextEdit: a rich text editor, which may be used to edit
 *     HTML.</li>
 *
 *   <li>TableView displays data from a WAbstractItemModel. The widget
 *     provides sorting, column resizing. In addition, form fiels may be
 *     used for inline editing of data, which is propagated back to the
 *     model.</li>
 *
 *   <li>Good-looking and flexible Menu and ToolBar classes.</li>
 *
 *   <li>A polished Dialog, MessageBox and ProgressDialog.</li>
 *
 *   <li>Standalone Splitter widget, or integrated in BorderLayout</li>
 *
 *   <li>Availability of several themes, created by the ExtJS user
 *     community. Simply import the stylesheet after Ext itself is
 *     loaded.  (for example, try:
 *     useStyleSheet("<i>extBaseURL</i>/resources/css/xtheme-gray.css")). Ext
 *     itself is loaded by inserting an Ext widget, so a good place to
 *     do this is at the end of your application construction.</li>
 *     </ul>
 *
 * The functionality of some of these widgets overlaps with existing %Wt
 * widgets. Whenever possible, the same API was adopted.
 *
 * The following table shows corresponding widgets and comments on the
 * resemblance of the APIs.
 * <table>
 *   <tr><td><b>%Wt widget</b></td><td><b>%Wt %Ext widget</b></td>
 *       <td><b>Comment</b></td></tr>
 *   <tr><td>WCalendar</td><td>Calendar</td>
 *       <td>The %Wt version is more versatile (for example allows multiple
 *         selection, and allows more programmatic control.</td></tr>
 *   <tr><td>WCheckBox</td><td>CheckBox</td>
 *       <td>Identical API.</td></tr>
 *   <tr><td>WComboBox</td><td>ComboBox</td>
 *       <td>The %Ext version adds a number of features that are similar
 *         to those provided by %Wt's SuggestionPopup, and allows keeping
 *         the entire data set at the server.</td></tr>
 *   <tr><td>WDialog</td><td>Dialog</td>
 *       <td>Almost identical API. The %Ext version manages standard
 *          buttons.</td></tr>
 *   <tr><td>WLineEdit</td><td>LineEdit</td>
 *       <td>Identical API.</td></tr>
 *   <tr><td>WMessageBox</td><td>MessageBox</td>
 *       <td>Almost identical API. The %Wt version is more flexible with respect
 *         to buttons, while the %Ext version supports prompting for
 *         input.</td></tr>
 *   <tr><td>WRadioButton</td><td>RadioButton</td>
 *       <td>Identical API.</td></tr>
 *   <tr><td>WTextEdit</td><td>TextEdit</td>
 *       <td>API differences. The %Wt version has more options and is
 *         XHTML-compliant.</td></tr>
 * </table>
 *
 * You can freely mix %Wt widgets and %Ext widgets in your
 * application: %Wt widgets containers may contain %Ext widgets, and
 * vice-versa. From a programmer perspective, there are no differences
 * for using %Wt versus %Ext widgets.
 *
 * \section moh 2. Deployment notes.
 *
 * To use %Ext widgets, you need to download the %Ext JavaScript library
 * (ext-2.x), and deploy the following files to your web server:
 *
 * <ul>
 *  <li>ext-2.x/adapter/ext/ext-base.js to <i>extBaseURL</i>/ext-base.js</li>
 *  <li>ext-2.x/ext-all.js to <i>extBaseURL</i>/ext-all.js</li>
 *  <li>ext-2.x/resources/ to <i>extBaseURL</i>/resources/
 *    (the entire folder) </li>
 * </ul>
 *
 * The default value for <i>extBaseURL</i> is "ext/". This value may
 * be overridden with a URL that points to a folder where these files
 * are located, by configuring the <i>extBaseURL</i> property in your
 * %Wt configuration file.
 *
 * \deprecated Use native widgets instead.
 */

}
}

