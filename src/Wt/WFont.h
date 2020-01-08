// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFONT_H_
#define WFONT_H_

#include <Wt/WLength.h>
#include <Wt/WString.h>

namespace Wt {

/*! \brief The generic font family.
 */
enum class FontFamily {
  Default,   //!< Browser-dependent default
  Serif,     //!< for example: Times
  SansSerif, //!< for example: Helvetica
  Cursive,   //!< for example: Zapf-Chancery
  Fantasy,   //!< for example: Western
  Monospace  //!< for example: Courier
};

/*! \brief The font style.
 */
enum class FontStyle { 
  Normal,   //!< Normal (default)
  Italic,   //!< Italic
  Oblique   //!< Oblique
};

/*! \brief The font variant.
 */
enum class FontVariant {
  Normal,    //!< Normal (default)
  SmallCaps  //!< Small Capitals 
};

/*! \brief The font weight.
 */
enum class FontWeight {
  Normal,  //!< Normal (default) (Value == 400)
  Bold,    //!< Bold (Value == 700)
  Bolder,  //!< Bolder than the parent widget
  Lighter, //!< Lighter than the parent widget
  Value    //!< Specify a value (100 - 900)
};

/*! \brief The font size.
 */
enum class FontSize {
  XXSmall,  //!< Extra Extra small
  XSmall,   //!< Extra small
  Small,    //!< Small
  Medium,   //!< Medium, default
  Large,    //!< Large
  XLarge,   //!< Extra large
  XXLarge,  //!< Extra Extra large
  Smaller,  //!< Relatively smaller than the parent widget
  Larger,   //!< Relatively larger than the parent widget
  FixedSize //!< Explicit size, See also fontFixedSize()
};

class DomElement;
class WWebWidget;

/*! \class WFont Wt/WFont.h Wt/WFont.h
 *  \brief A value class that describes a font.
 *
 * \ingroup style painting
 */
class WT_API WFont
{
public:
  /*! \brief Typedef for enum Wt::FontFamily */
  typedef FontFamily Family;
  /*! \brief Typedef for enum Wt::FontStyle */
  typedef FontStyle Style;
  /*! \brief Typedef for enum Wt::FontVariant */
  typedef FontVariant Variant;
  /*! \brief Typedef for enum Wt::FontWeight */
  typedef FontWeight Weight;
  /*! \brief Typedef for enum Wt::FontSize */
  typedef FontSize Size;

  /*! \brief A default font (dependent on the user agent).
   */
  WFont();

  /*! \brief A font of a given family.
   *
   * Creates a Medium font of the given family.
   */
  WFont(FontFamily family);

  /*! \brief Comparison operator.
   */
  bool operator==(const WFont& other) const;
  bool operator!=(const WFont& other) const;

  /*! \brief Sets the font family.
   *
   * The font family is specified using a generic family name,
   * in addition to a comma-separated list of specific font choices.
   *
   * The first specific font that can be matched will be used, otherwise
   * a generic font will be used.
   *
   * Careful, for a font family name that contains a space, you need
   * to add quotes, to WFont::setFamily(), e.g.
   *
   * \code
   * WFont mono;
   * mono.setFamily(FontFamily::Monospace, "'Courier New'");
   * mono.setSize(18);
   * \endcode
   */
  void setFamily(FontFamily genericFamily,
		 const WString& specificFamilies = WString());

  /*! \brief Returns the font generic family.
   */
  FontFamily genericFamily() const { return genericFamily_; }

  /*! \brief Returns the font specific family names.
   */
  const WString& specificFamilies() const { return specificFamilies_; }

  /*! \brief Sets the font style.
   */
  void setStyle(FontStyle style);

  /*! \brief Returns the font style.
   */
  FontStyle style() const { return style_; }

  /*! \brief Sets the font variant.
   */
  void setVariant(FontVariant variant);

  /*! \brief Returns the font variant.
   */
  FontVariant variant() const { return variant_; }

  /*! \brief Sets the font weight.
   *
   * When setting weight == Value, you may specify a value.
   *
   * Valid values are between 100 and 900, and are rounded to multiples of
   * 100.
   */
  void setWeight(FontWeight weight, int value = 400);

  /*! \brief Returns the font weight.
   */
  FontWeight weight() const;

  /*! \brief Returns the font weight value.
   */
  int weightValue() const;

  /*! \brief Sets the font size.
   *
   * Sets the font size using a predefined CSS size.
   */
  void setSize(FontSize size);

  /*! \brief Sets the font size.
   *
   * Sets the font size.
   */
  void setSize(const WLength& size);

  /*! \brief Returns the font size.
   */
  FontSize size(double mediumSize = 16) const;

  /*! \brief Returns the font size as a numerical value.
   *
   * PositionScheme::Absolute size enumerations are converted to a length assuming a
   * Medium font size of 16 px.
   */
  WLength sizeLength(double mediumSize = 16) const;

  const std::string cssText(bool combined = true) const;

  void updateDomElement(DomElement& element, bool fontall, bool all);

private:
  WWebWidget    *widget_;
  FontFamily    genericFamily_;
  WString       specificFamilies_;
  FontStyle     style_;
  FontVariant   variant_;
  FontWeight    weight_;
  int           weightValue_;
  FontSize      size_;
  WLength       sizeLength_;

  bool familyChanged_;
  bool styleChanged_;
  bool variantChanged_;
  bool weightChanged_;
  bool sizeChanged_;

  std::string cssStyle(bool all) const;
  std::string cssVariant(bool all) const;
  std::string cssWeight(bool all) const;
  std::string cssFamily(bool all) const;
  std::string cssSize(bool all) const;

  void setWebWidget(WWebWidget *w);
  friend class WCssDecorationStyle;
};

}

#endif // WFONT_H_
