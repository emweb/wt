// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCSSDECORATIONSTYLE_H_
#define WCSSDECORATIONSTYLE_H_

#include <Wt/WBorder.h>
#include <Wt/WColor.h>
#include <Wt/WFont.h>
#include <Wt/WLink.h>
#include <Wt/WWidget.h>

namespace Wt {

/*! \brief Text decoration options
 */
enum class TextDecoration {
  Underline   = 0x1, //!< Underline
  Overline    = 0x2, //!< Overline
  LineThrough = 0x4, //!< LineThrough
  Blink       = 0x8  //!< Blink
};

class DomElement;
class WWebWidget;

/*! \defgroup style Style classes
 *  \brief Collection of classes for markup of widgets.
 *
 * The recommended way to provide style information for your %Wt
 * application is using CSS stylesheets. You may add rules to the
 * inline style sheet using WCssStyleSheet::addRule(), or by linking
 * one or more external stylesheets using WApplication::useStyleSheet().
 *
 * Alternatively, you may also provide style information directly,
 * using WCssDecorationStyle, which you can manipulate for each widget
 * using WWidget::decorationStyle().
 */

/*! \class WCssDecorationStyle Wt/WCssDecorationStyle.h Wt/WCssDecorationStyle.h
 *  \brief A style class for a single widget or style sheet rule.
 *
 * You can manipulate the decoration style of a single widget using
 * WWidget::decorationStyle() or you can use a WCssDecorationStyle to
 * add a rule to the inline style sheet using
 * WCssStyleSheet::addRule(const std::string&, const WCssDecorationStyle& style, const std::string&).
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WWidget *widget = ...
 * widget->decorationStyle().setCursor(Cursor::PointingHand);
 * \endcode
 * \endif
 *
 * \ingroup style
 */
class WT_API WCssDecorationStyle : public WObject
{
public:
  /*! \brief Creates a default style.
   */
  WCssDecorationStyle();

  /*! \brief Copy constructor
   */
  WCssDecorationStyle(const WCssDecorationStyle& other);

  /*! \brief Destructor.
   */
  ~WCssDecorationStyle();

  /*! \brief Assignment operator.
   */
  WCssDecorationStyle& operator= (const WCssDecorationStyle& other);

  /*! \brief Sets the cursor style.
   */
  void setCursor(Cursor c);

  /*! \brief Returns the cursor style.
   *
   * \sa setCursor()
   */
  Cursor cursor() const { return cursor_; }

  /*! \brief Sets a custom cursor image Url.
   * 
   * The Url should point to a .cur file (this shoul be a real .cur file, 
   * renaming an .ico is not enough for Internet Explorer).
   */
  void setCursor(std::string cursorImage,
		 Cursor fallback = Cursor::Arrow);

  /*! \brief Returns the cursor image.
   *
   * \sa setCursor()
   */
  std::string cursorImage() const { return cursorImage_; }

  /*! \brief Sets the background color.
   */
  void setBackgroundColor(WColor color);

  /*! \brief Returns the background color.
   *
   * \sa setBackgroundColor()
   */
  WColor backgroundColor() const { return backgroundColor_; }

  /*! \brief Sets a background image.
   *
   * The \p link may be a URL or a resource.
   *
   * The image may be placed in a particular location by specifying
   * sides by OR'ing Wt::Side values together, e.g. (Side::Right | Side::Top).
   */
  void setBackgroundImage(const WLink& link,
			  WFlags<Orientation> repeat
			    = Orientation::Horizontal | Orientation::Vertical,
			  WFlags<Side> sides = None);

#ifdef WT_TARGET_JAVA
  /*! \brief Sets a background image.
   *
   * The image may be placed in a particular location by specifying
   * sides by OR'ing Wt::Side values together, e.g. (Side::Right | Side::Top).
   */
  void setBackgroundImage(const std::string& url,
			  WFlags<Orientation> repeat
			    = Orientation::Horizontal | Orientation::Vertical, 
			  WFlags<Side> sides = None);
#endif // WT_TARGET_JAVA

  /*! \brief Returns the background image URL.
   *
   * \sa setBackgroundImage()
   */
  std::string backgroundImage() const { return backgroundImage_.url(); }

  /*! \brief Returns the background image repeat.
   *
   * \sa setBackgroundImage()
   */
  WFlags<Orientation> backgroundImageRepeat() const { return backgroundImageRepeat_; }

  /*! \brief Sets the text color.
   */
  void setForegroundColor(WColor color);

  /*! \brief Returns the text color.
   *
   * \sa setForegroundColor()
   */
  WColor foregroundColor() const { return foregroundColor_; }

  /*! \brief Sets the border style.
   *  
   * The given \p border will be set for the specified \p sides.
   *
   * A different border style may be specified for each of the four
   * sides.
   */
  void setBorder(WBorder border, WFlags<Side> sides = AllSides);

  /*! \brief Returns the border style.
   *
   * Returns the border style set using setBorder() for the given \p
   * side.
   *
   * \sa setBorder()
   *
   * \note Prior to version 3.1.9 it was not possible to pass a side
   *       and only one border could be configured.
   */
  WBorder border(Side side = Side::Top) const;

  /*! \brief Sets the text font.
   */
  void setFont(const WFont& font);

  /*! \brief Returns the font.
   *
   * \sa setFont()
   */
  WFont& font() { return font_; }
  const WFont& font() const { return font_; }

  /*! \brief Sets text decoration options.
   *
   * You may logically or together any of the options of the
   * TextDecoration enumeration.
   *
   * The default is 0.
   */
  void setTextDecoration(WFlags<TextDecoration> decoration);

  /*! \brief Returns the text decoration options.
   *
   * \sa setTextDecoration()
   */
  WFlags<TextDecoration> textDecoration() const { return textDecoration_; }

  std::string cssText();
  void updateDomElement(DomElement& element, bool all);

private:
  WWebWidget               *widget_;
  Cursor                    cursor_;
  std::string               cursorImage_;
  std::unique_ptr<WBorder>  border_[4];
  WColor                    backgroundColor_;
  WColor	            foregroundColor_;
  WLink	                    backgroundImage_;
  WFlags<Orientation>       backgroundImageRepeat_;
  WFlags<Side>              backgroundImageLocation_;
  WFont		            font_;
  WFlags<TextDecoration>    textDecoration_;

  bool		            cursorChanged_;
  bool		            borderChanged_;
  bool		            foregroundColorChanged_;
  bool		            backgroundColorChanged_;
  bool	  	            backgroundImageChanged_;
  bool		            fontChanged_;
  bool                      textDecorationChanged_;

  void changed(WFlags<RepaintFlag> flags = None);
  void backgroundImageResourceChanged();
  WBorder borderI(unsigned i) const;

  void copy(const WCssDecorationStyle& other);

  void setWebWidget(WWebWidget *widget);
  friend class WWebWidget;
};

W_DECLARE_OPERATORS_FOR_FLAGS(TextDecoration)

}

#endif // WCSSDECORATIONSTYLE_H_
