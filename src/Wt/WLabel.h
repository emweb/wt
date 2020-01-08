// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLABEL_H_
#define WLABEL_H_

#include <Wt/WInteractWidget.h>

namespace Wt {

  class WFormWidget;
  class WImage;
  class WText;

/*! \class WLabel Wt/WLabel.h Wt/WLabel.h
 *  \brief A label for a form field.
 *
 * The label may contain an image and/or text. It acts like a proxy
 * for giving focus to a WFormWidget. When both an image and text are
 * specified, the image is put to the left of the text.
 *
 * Usage example:
 * \if cpp
 * \code
 * auto w = std::make_unique<Wt::WContainerWidget>();
 * Wt::WLabel *label = w->addWidget(std::make_unique<Wt::WLabel>("Favourite Actress: "));
 * Wt::WLineEdit *edit = w->addWidget(std::make_unique<Wt::WLineEdit>("Renee Zellweger"));
 * label->setBuddy(edit);
 * \endcode
 * \elseif java
 * \code
 * WContainerWidget w = new WContainerWidget();
 * WLabel label = new WLabel("Favourite Actress: ", w);
 * WLineEdit edit = new WLineEdit("Renee Zellweger", w);
 * label.setBuddy(edit);
 * \endcode
 * \endif
 *
 * The widget corresponds to the HTML <tt>&lt;label&gt;</tt> tag. When
 * no buddy is set, it is rendered using an HTML <tt>&lt;span&gt;</tt>
 * or <tt>&lt;div&gt;</tt> to avoid click event handling misbehavior
 * on Microsoft Internet Explorer.
 *
 * %WLabel is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 * 
 * This widget does not provide styling, and can be styled using
 * inline or external CSS as appropriate.  A label's text may be
 * styled via a nested <tt>&lt;span&gt;</tt> element, and it's
 * image may be styled via a nested <tt>&lt;img&gt;</tt> element.
 */
class WT_API WLabel : public WInteractWidget
{
public:
  /*! \brief Creates a label with empty text and optional parent.
   */
  WLabel();

  /*! \brief Creates a label with a given text.
   */
  WLabel(const WString& text);

  /*! \brief Creates a label with an image.
   */
  WLabel(std::unique_ptr<WImage> image);

  ~WLabel();

  /*! \brief Returns the buddy of this label.
   *
   * \sa setBuddy(WFormWidget *)
   */
  WFormWidget *buddy() const;

  /*! \brief Sets the buddy of this label.
   *
   * Sets the buddy FormWidget for which this label acts as a proxy.
   * 
   * \sa WFormWidget::label(), buddy()
   */
  void setBuddy(WFormWidget *buddy);

  /*! \brief Sets the label text.
   */
  void setText(const WString& text);

  /*! \brief Returns the label text.
   */
  WString text() const;

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
  TextFormat textFormat() const;

  /*! \brief Sets the image.
   */
  void setImage(std::unique_ptr<WImage> image, Side side = Side::Left);

  /*! \brief Returns the image.
   */
  WImage *image() const { return image_.get(); }

  /*! \brief Configures word wrapping.
   *
   * When \p wordWrap is \c true, the widget may break lines, creating a
   * multi-line text. When \p wordWrap is \c false, the text will displayed
   * on a single line, unless the text contains end-of-lines (for
   * Wt::TextFormat::Plain) or &lt;br /&gt; tags or other block-level tags
   * (for Wt::TextFormat::XHTML).
   *
   * The default value is \c false.
   *
   * \sa wordWrap()
   */
  void setWordWrap(bool wordWrap);

  /*! \brief Returns whether word wrapping is on.
   *
   * \sa setWordWrap()
   */
  bool wordWrap() const;

private:
  observing_ptr<WFormWidget> buddy_;
  std::unique_ptr<WText> text_;
  std::unique_ptr<WImage> image_;
  Side imageSide_;

  bool buddyChanged_, newImage_, newText_;

protected:
  virtual void           updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void           getDomChanges(std::vector<DomElement *>& result,
                                       WApplication *app) override;
  virtual void           propagateRenderOk(bool deep) override;
  virtual void           propagateSetEnabled(bool enabled) override;

  virtual void		 iterateChildren(const HandleWidgetMethod &method) const override;

  void updateImage(DomElement& element, bool all, WApplication *app, int pos);
  void updateText(DomElement& element, bool all, WApplication *app, int pos);

  friend class WAnchor;
};

}

#endif // WLABEL_H_
