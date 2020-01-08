// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTEXT_H_
#define WTEXT_H_

#include <Wt/WInteractWidget.h>
#include <Wt/WString.h>

namespace Wt {

/*! \class WText Wt/WText.h Wt/WText.h
 *  \brief A widget that renders (XHTML) text.
 *
 * The text is provided through a WString, which may either hold a
 * literal text, or a key to localized text which is looked up in
 * locale dependent XML files (see WString::tr()).
 *
 * Use setTextFormat() to configure the textFormat of the text. The
 * default textFormat is Wt::TextFormat::XHTML, which allows XHMTL markup to
 * be included in the text. Tags and attributes that indicate "active"
 * content are not allowed and stripped out, to avoid security risks
 * exposed by JavaScript such as the common web-based <a
 * href="http://en.wikipedia.org/wiki/Cross_site_scriptingCross-Site">
 * Cross-Site Scripting (XSS)</a> malicious attack. XSS is the
 * situation where one user of your web application is able to execute
 * a script in another user's browser while your application only
 * intended to display a message entered by the mailicious user to the
 * other user. To defeat this attack, %Wt assumes that content in a
 * %WText is intended to be passive, and not contain any scripting
 * elements.
 *
 * The Wt::TextFormat::XHTML format will automatically change to
 * Wt::TextFormat::Plain if the text is not valid XML. Properly
 * formatted HTML, which is not valid XHTML (e.g. a
 * <tt>&lt;br&gt;</tt> tag without
 * closing tag) will thus be shown literally, since the HTML markup
 * will be escaped. Wt does this as a safety measure, since it cannot
 * reliably run the XSS filter without parsing the XML successfully.
 *
 * The Wt::TextFormat::Plain format will display the text literally
 * (escaping any HTML special characters).
 *
 * In some situations, Wt::TextFormat::UnsafeXHTML may be useful to explicitly
 * allow scripting content. Like TextFormat::XHTML, it allows XHTML markup,
 * but it also allows potentially dangerous tags and attributes. Use
 * this if you're sure that a user cannot interfere with the text set,
 * and TextFormat::XHTML is too limiting.
 *
 * %WText is by default \link WWidget::setInline()
 * inline\endlink, unless the XHTML contents starts with an
 * element such as <tt>&lt;div&gt;</tt>, <tt>&lt;h&gt;</tt> or
 * <tt>&lt;p&gt;</tt> that is displayed as a block, in which case the
 * widget will also display as a block.
 *
 * \if cpp
 * Usage examples:
 * \code
 * auto container = std::make_unique<Wt::WContainerWidget>();
 *
 * // display an XHTML text:
 * container->addWidget(
 *   std::make_unique<Wt::WText>("Hello <i>dear</i> visitor."));
 *
 * // display a plain text:
 * container->addWidget(
 *   std::make_unique<Wt::WText>("The <i> tag displays italic text.", Wt::TextFormat::Plain));
 *
 * // display an XHTML fragment from a resource bundle:
 * container->addWidget(
 *   std::make_unique<Wt::WText>(tr("introduction")));
 * \endcode
 * \endif
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to an HTML <tt>&lt;span&gt;</tt> tag or an
 * HTML <tt>&lt;div&gt;</tt> depending on whether the widget is inline.
 *
 * This widget does not provide styling, 
 * and can be styled using inline or external CSS as appropriate.
 *
 * \sa WApplication::setLocale()
 * \if cpp
 * \sa WApplication::messageResourceBundle()
 * \endif
 */
class WT_API WText : public WInteractWidget
{
public:
  /*! \brief Creates a text widget with an empty text.
   */
  WText();

  /*! \brief Creates a text widget with given text.
   *
   * The textFormat is set to Wt::TextFormat::XHTML, unless the \p text is
   * literal (not created using WString::tr()) and it could not be
   * parsed as valid XML. In that case the textFormat is set to
   * Wt::TextFormat::Plain.
   *
   * Therefore, if you wish to use Wt::TextFormat::XHTML, 
   * but cannot be sure about \p text being valid XML, 
   * you should verify that the textFormat() is Wt::TextFormat::XHTML
   * after construction.
   *
   * The XML parser will silently discard malicious tags and
   * attributes for literal Wt::TextFormat::XHTML text.
   */
  WText(const WString& text);

  /*! \brief Creates a text widget with given text and format
   *
   * If <i>textFormat</i> is Wt::TextFormat::XHTML and \p text is not
   * literal (not created using WString::tr()), then if the
   * \p text could not be parsed as valid XML, the textFormat is
   * changed to Wt::TextFormat::Plain.
   *
   * Therefore, if you wish to use Wt::TextFormat::XHTML, but cannot be sure
   * about \p text being valid XML, you should verify that the
   * textFormat() is Wt::TextFormat::XHTML after construction.
   *
   * The XML parser will silently discard malicious tags and
   * attributes for literal Wt::TextFormat::XHTML text.
   */
  WText(const WString& text, TextFormat textFormat);

  /*! \brief Destructor.
   */
  ~WText();

  /*! \brief Returns the text.
   *
   * When a literal XHTMLFormatted text was set, this may differ from
   * the text that was set since malicious tags/attributes may have
   * been stripped.
   *
   * \sa setText(const WString&)
   */
  const WString& text() const { return text_.text; }

  /*! \brief Sets the text.
   *
   * When the current format is Wt::TextFormat::XHTML, and
   * \p text is literal (not created using WString::tr()), it is
   * parsed using an XML parser which discards malicious tags and
   * attributes silently. When the parser encounters an XML parse
   * error, the textFormat is changed to Wt::TextFormat::Plain.
   *
   * Returns whether the text could be set using the current
   * textFormat. A return value of \c false indicates that the textFormat
   * was changed in order to be able to accept the new text.
   *
   * \sa text(), setText()
   */
  bool setText(const WString& text);

  /*! \brief Sets the text format.
   *
   * The textFormat controls how the string should be interpreted:
   * either as plain text, which is displayed literally, or as
   * XHTML-markup.
   *
   * When changing the textFormat to Wt::TextFormat::XHTML, and the
   * current text is literal (not created using WString::tr()), the
   * current text is parsed using an XML parser which discards
   * malicious tags and attributes silently. When the parser
   * encounters an XML parse error, the textFormat is left unchanged,
   * and this method returns false.
   *
   * Returns whether the textFormat could be set for the current text.
   *
   * The default format is Wt::TextFormat::XHTML.
   */
  bool setTextFormat(TextFormat format);

  /*! \brief Returns the text format.
   *
   * \sa setTextFormat()
   */
  TextFormat textFormat() const { return text_.format; }

  /*! \brief Configures word wrapping.
   *
   * When \p wordWrap is \c true, the widget may break lines, creating a
   * multi-line text. When \p wordWrap is \c false, the text will displayed
   * on a single line, unless the text contains end-of-lines (for
   * Wt::TextFormat::Plain) or &lt;br /&gt; tags or other block-level tags
   * (for Wt::TextFormat::XHTML).
   *
   * The default value is \c true.
   *
   * \sa wordWrap()
   */
  void setWordWrap(bool wordWrap);

  /*! \brief Returns whether the widget may break lines.
   *
   * \sa setWordWrap(bool)
   */
  bool wordWrap() const { return flags_.test(BIT_WORD_WRAP); }

  /*! \brief Specifies how text is aligned.
   *
   * Only the horizontal alignment can be specified. Note that there
   * is no way to specify vertical alignment. You can put the
   * text in a layout with vertical alignment options though, or (misuse)
   * the line-height CSS property for single line texts.
   */
  void setTextAlignment(AlignmentFlag textAlignment);

  /*! \brief Returns the alignment of children
   *
   * \sa setTextAlignment()
   */
  AlignmentFlag textAlignment() const;

  /*! \brief Sets padding inside the widget
   *
   * Setting padding has the effect of adding distance between the
   * widget children and the border.
   *
   * \note for an \link setInline() inline\endlink %WText padding
   * is only supported on the left and/or right. Setting padding on
   * the top or bottom has no effect.
   */
  void setPadding(const WLength& padding, 
		  WFlags<Side> sides = Side::Left | Side::Right);

  /*! \brief Returns the padding set for the widget.
   *
   * \sa setPadding(const WLength&, WFlags<Side>)
   */
  WLength padding(Side side) const;

  /*! \brief Enables internal path encoding of anchors in the XHTML text.
   *
   * Anchors to internal paths are represented differently depending
   * on the session implementation (plain HTML, Ajax or HTML5
   * history). By enabling this option, anchors which reference an
   * internal path (by referring a URL of the form
   * <tt>href="#/..."</tt>), are re-encoded to link to the internal
   * path.
   *
   * When using Wt::TextFormat::XHTML (or Wt::TextFormat::UnsafeXHTML) formatted text,
   * the text is pasted verbatim in the browser (with the exception of
   * XSS filtering if applicable). With this option, however, the
   * XHTML text may be transformed at the cost of an additional XML
   * parsing step.
   *
   * The default value is \c false.
   *
   * \sa WAnchor::setInternalPath()
   */
  void setInternalPathEncoding(bool enabled);

  /*! \brief Returns whether internal paths are encoded.
   *
   * \sa setInternalPathEncoding()
   */
  bool hasInternalPathEncoding() const
    { return flags_.test(BIT_ENCODE_INTERNAL_PATHS); }

  virtual void refresh() override;

private:
  struct RichText {
    RichText();

    WString text;
    TextFormat format;

    bool setText(const WString& text);
    bool setFormat(TextFormat format);
    bool checkWellFormed();
    std::string formattedText() const;
  };

  RichText text_;

  static const int BIT_WORD_WRAP = 0;
  static const int BIT_TEXT_CHANGED = 1;
  static const int BIT_WORD_WRAP_CHANGED = 2;
  static const int BIT_PADDINGS_CHANGED = 3;
  static const int BIT_ENCODE_INTERNAL_PATHS = 4;
  static const int BIT_TEXT_ALIGN_LEFT = 5;
  static const int BIT_TEXT_ALIGN_CENTER = 6;
  static const int BIT_TEXT_ALIGN_RIGHT = 7;
  static const int BIT_TEXT_ALIGN_CHANGED = 8;

  std::bitset<9> flags_;

  std::string formattedText() const;
  void autoAdjustInline();

  WLength *WT_ARRAY padding_;

protected:
  virtual void           render(WFlags<RenderFlag> flags) override;
  virtual void           updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void           propagateRenderOk(bool deep) override;

  friend class WAbstractToggleButton;
  friend class WLabel;
  friend class WPushButton;
};

}

#endif // WTEXT_H_
