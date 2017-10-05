// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTEXTEDIT_H_
#define WTEXTEDIT_H_

#include <Wt/WAny.h>
#include <Wt/WTextArea.h>
#include <Wt/WJavaScript.h>

namespace Wt {

/*! \class WTextEdit Wt/WTextEdit.h Wt/WTextEdit.h
 *  \brief A rich-text XHTML editor.
 *
 * The editor provides interactive editing of XHTML text. By default
 * it provides basic mark-up (font, formatting, color, links, and
 * lists), but additional buttons may be added to the tool bars that
 * add additional formatting options.
 *
 * The implementation is based on <a
 * href="http://tinymce.moxiecode.com/">TinyMCE</a>. The widget may be
 * configured and tailored using the setConfigurationSetting() and
 * related methods that provide direct access to the underlying
 * TinyMCE component.
 *
 * \if cpp
 * You can use this widget with TinyMCE version 3 or 4, but this requires
 * different configurations.
 *
 * By default, %Wt assumes a 3.x version of TinyMCE. You need to
 * download TinyMCE (version 3.5b1 or later) and deploy the
 * <tt>tinymce/jscripts/tiny_mce</tt> folder to
 * <i>tinyMCEBaseURL</i>. The default value for <i>tinyMCEBaseURL</i>
 * is <i>resourcesURL</i><tt>/tiny_mce</tt>, where <i>resourcesURL</i>
 * is the configuration property that locates the %Wt
 * <tt>resources/</tt> folder (i.e., we assume by default that you
 * copy the <tt>tiny_mce</tt> folder to the <tt>resources/</tt>
 * folder), see also \ref deployment "deployment and resources".
 *
 * If you prefer to use TinyMCE 4 (or later) instead, you'll need to
 * set the <i>tinyMCEVersion</i> property to 4 (or later). Note that
 * TinyMCE changed its folder layout in version 4. You will need to
 * deploy the <tt>js/tinymce</tt> folder to <i>tinyMCEBaseURL</i>. The
 * default value for <i>tinyMCEBaseURL</i> for TinyMCE 4 (or later) is
 * is <i>resourcesURL</i><tt>/tinymce</tt> (i.e., we assume by default that you
 * copy the <tt>tiny_mce</tt> folder to the <tt>resources/</tt>
 * folder).
 *
 * If the name of the main TinyMCE JavaScript file is not tinymce.js
 * (e.g., you are using the minified version from the release package,
 * or the CDN <tt>//tinymce.cachefly.net/4.0/tinymce.min.js</tt> or
 * <tt>//cdnjs.cloudflare.com/ajax/libs/tinymce/3.5.8/tiny_mce.js</tt>),
 * the URL to the main script file should be specified via <i>tinyMCEURL</i>.
 *
 * Because the default folder names are different ("tiny_mce" for
 * version 3 versus "tinymce" for version 4, you can have the
 * resources for TinyMCE3 and TinyMCE4 alongside each other while
 * experimenting with the old and new TinyMCE).
 *
 * The default location for the TinyMCE resources may be overridden by
 * configuring the <i>tinyMCEBaseURL</i> property in your %Wt
 * configuration file, see \ref config_general "configuration properties".
 *
 * \endif
 *
 * \if java
 * You can use this widget with TinyMCE version 3 or version 4.
 *
 * The choice is global and set using
 * {@link Configuration#setTinyMCEVersion(int)}.
 * \endif
 *
 * \if cpp
 * Usage example:
 * \code
 * auto w = std::make_unique<Wt::WContainerWidget>();
 * Wt::WLabel *label = w->addWidget(std::make_unique<Wt::WLabel>("Comments:"));
 * Wt::WTextEdit *edit = w->addWidget(std::make_unique<Wt::WTextEdit>(""));
 * label->setBuddy(edit);
 * \endcode
 * \endif
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 *
 * \image html WTextEdit-1.png "Default configuration of a WTextEdit"
 */
class WT_API WTextEdit : public WTextArea
{
public:
  /*! \brief Creates a new text editor.
   */
  WTextEdit();

  /*! \brief Creates a new text editor and initialize with given text.
   *
   * The \p text should be valid XHTML.
   */
  WTextEdit(const WT_USTRING& text);

  /*! \brief Destructor.
   */
  ~WTextEdit();

  /*! \brief Returns the TinyMCE version.
   *
   * This returns the configured version of TinyMCE (currently 3 or 4).
   */
  int version() const { return version_; }

  /*! \brief Sets the content.
   *
   * The \p text should be valid XHTML.
   *
   * The default value is "".
   */
  virtual void setText(const WT_USTRING& text) override;

  /*! \brief Sets the stylesheet for displaying the content.
   *
   * The content is rendered using the rules defined in this
   * stylesheet. The stylesheet is also used to derive additional
   * styles that are available in the text editor, for example in the
   * "styleselect" button.
   *
   * Multiple stylesheets may be specified as a comma separated list.
   */
  void setStyleSheet(const std::string& uri);

  /*! \brief Returns the content stylesheet.
   *
   * \sa setStyleSheet()
   */
  const std::string styleSheet() const;

  /*! \brief Loads additional TinyMCE plugins.
   *
   * %Wt loads by default only the plugin 'safari' (which adds support
   * for the Safari web browser). Use this method to load additional
   * plugins. Multiple plugins may be specified as a comma separated
   * list.
   *
   * The various plugins are described in the <a
   * href="http://www.tinymce.com/wiki.php/Plugins">TinyMCE
   * documentation</a>.
   *
   * \note Plugins can only be loaded before the initial display of
   * the widget.
   */
  void setExtraPlugins(const std::string& plugins);

  /*! \brief Returns the extra plugins.
   *
   * \sa setExtraPlugins()
   */
  const std::string extraPlugins() const;

  /*! \brief Configures a tool bar.
   *
   * This configures the buttons for the \p i'th tool bar (with 0
   * <= \p i <= 3).
   *
   * <h3>TinyMCE 3</h3>
   * The syntax and available buttons is documented in the <a
   * href="http://www.tinymce.com/wiki.php/Configuration3x:theme_advanced_buttons_1_n">TinyMCE
   * documentation</a>.
   *
   * The default <i>config</i> for the first tool bar (\p i = 0)
    is: "fontselect, |, bold, italic, underline, |, fontsizeselect, |,
    forecolor, backcolor, |, justifyleft, justifycenter, justifyright,
    justifyfull, |, anchor, |, numlist, bullist".
   *
   * By default, the other three tool bars are disabled (\p config = "").
   *
   * <h3>TinyMCE 4</h3>
   *
   * The syntax and available buttons is documented in the <a
   * href="http://www.tinymce.com/wiki.php/Configuration:toolbar%3CN%3E">
   * TinyMCEdocumentation</a>.
   *
   * The default <i>config</i> for the first tool bar (\p i = 0)
    is: "undo redo | styleselect | bold italic | link".
   *
   * Some buttons are only available after loading extra plugins using
   * setExtraPlugins().
   *
   * \note The tool bar configuration can only be set before the
   * initial display of the widget.
   */
  void setToolBar(int i, const std::string& config);

  /*! \brief Returns a tool bar configuration.
   *
   * \sa setToolBar()
   */
  const std::string toolBar(int i) const;

  /*! \brief Configure a TinyMCE setting.
   *
   * A list of possible settings can be found at:
   * http://tinymce.moxiecode.com/wiki.php/Configuration
   *
   * The widget itself will also define a number of configuration settings
   * and these may be overridden using this method.
   */
  void setConfigurationSetting(const std::string& name, 
			       const cpp17::any& value);

  /*! \brief Returns a TinyMCE configuration setting's value.
   *
   * An empty Any is returned when no value could be found for the 
   * provided argument.
   */
  cpp17::any configurationSetting(const std::string& name) const;

  /*! \brief Sets the placeholder text.
   *
   * This method is not supported on WTextEdit and will thrown an exception
   * instead.
   */
  virtual void setPlaceholderText(const WString& placeholder) override;

  virtual void setReadOnly(bool readOnly) override;
  virtual void propagateSetEnabled(bool enabled) override;
  virtual void resize(const WLength& width, const WLength& height) override;

  /*! \brief %Signal emitted when rendered.
   *
   * A text edit is instantiated asynchronously as it depends on
   * additional JavaScript libraries and initialization. This signal
   * is emitted when the component is initialized. The underlying
   * TinyMCE editor component is accessible as jsRef() + ".ed".
   */
  JSignal<>& rendered() { return onRender_; }

protected:
  virtual std::string renderRemoveJs(bool recursive) override;
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual bool domCanBeSaved() const override;

  virtual int boxPadding(Orientation orientation) const override;
  virtual int boxBorder(Orientation orientation) const override;

private:
  JSignal<> onChange_;
  JSignal<> onRender_;
  int version_;
  bool contentChanged_;
  std::map<std::string, cpp17::any> configurationSettings_;

  std::string plugins() const;

  void init();
  void propagateOnChange();
  static void initTinyMCE();
  static int getTinyMCEVersion();
};

}

#endif // WTEXTEDIT_H_
