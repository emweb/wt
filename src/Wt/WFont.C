/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/WWidget"
#include "Wt/WWebWidget"
#include "Wt/WFont"

#include "EscapeOStream.h"
#include "DomElement.h"

namespace Wt {

WFont::WFont()
  : widget_(0),
    genericFamily_(Default),
    style_(NormalStyle),
    variant_(NormalVariant),
    weight_(NormalWeight),
    weightValue_(400),
    size_(Medium),
    familyChanged_(false),
    styleChanged_(false),
    variantChanged_(false),
    weightChanged_(false),
    sizeChanged_(false)
{ }

void WFont::setWebWidget(WWebWidget *w)
{
  widget_ = w;
}

bool WFont::operator==(const WFont& other) const
{
  return
       genericFamily_    == other.genericFamily_
    && specificFamilies_ == other.specificFamilies_
    && style_            == other.style_
    && variant_          == other.variant_
    && weight_           == other.weight_
    && weightValue_      == other.weightValue_
    && size_             == other.size_
    && fixedSize_        == other.fixedSize_;
}

bool WFont::operator!=(const WFont& other) const
{
  return !(*this == other);
}

void WFont::setFamily(GenericFamily genericFamily,
		      const WString& specificFamilies)
{
  genericFamily_ = genericFamily;
  specificFamilies_ = specificFamilies;
  familyChanged_ = true;
  if (widget_) widget_->repaint(RepaintPropertyAttribute);
}

void WFont::setStyle(Style style)
{
  style_ = style;
  styleChanged_ = true;
  if (widget_) widget_->repaint(RepaintPropertyAttribute);
}

void WFont::setVariant(Variant variant)
{
  variant_ = variant;
  variantChanged_ = true;
  if (widget_) widget_->repaint(RepaintPropertyAttribute);
}

void WFont::setWeight(Weight weight, int value)
{
  weight_ = weight;
  weightValue_ = value;
  weightChanged_ = true;
  if (widget_) widget_->repaint(RepaintPropertyAttribute);
}

void WFont::setSize(Size size, const WLength& fixedSize)
{
  size_ = size;
  if(size_ == FixedSize) {
    fixedSize_ = fixedSize;
  } else {
    fixedSize_ = WLength::Auto;
  }
  sizeChanged_ = true;
  if (widget_) widget_->repaint(RepaintPropertyAttribute);
}

void WFont::updateDomElement(DomElement& element, bool fontall, bool all)
{
  if (familyChanged_ || fontall || all) {
    std::string family = cssFamily(fontall);

    if (!family.empty())
      element.setProperty(PropertyStyleFontFamily, family);

    familyChanged_ = false;
  }

  if (styleChanged_ || fontall || all) {
    std::string style = cssStyle(fontall);

    if (!style.empty())
      element.setProperty(PropertyStyleFontStyle, style);

    styleChanged_ = false;
  }

  if (variantChanged_ || fontall || all) {
    std::string variant = cssVariant(fontall);

    if (!variant.empty())
      element.setProperty(PropertyStyleFontVariant, variant);

    variantChanged_ = false;
  }

  if (weightChanged_ || fontall || all) {
    std::string weight = cssWeight(fontall);

    if (!weight.empty())
      element.setProperty(PropertyStyleFontWeight, weight);

    weightChanged_ = false;
  }

  if (sizeChanged_ || fontall || all) {
    std::string size = cssSize(fontall);

    if (!size.empty())
      element.setProperty(PropertyStyleFontSize, size);

    sizeChanged_ = false;
  }
}

std::string WFont::cssStyle(bool all) const
{
  switch (style_) {
  case NormalStyle:
    if (styleChanged_ || all)
      return "normal";
    break;
  case Italic: return "italic";
  case Oblique: return "oblique";
  }

  return std::string();
}

std::string WFont::cssVariant(bool all) const
{
  switch (variant_) {
  case NormalVariant:
    if (variantChanged_ || all)
      return "normal";
    break;
  case SmallCaps:
    return "small-caps";
  }

  return std::string();
}


std::string WFont::cssWeight(bool all) const
{
  switch (weight_) {
  case NormalWeight:
    if (weightChanged_ || all)
      return "normal";
    break;
    case Bold: return "bold";
    case Bolder:return "bolder";
    case Lighter: return "lighter";
    case Value: {
      int v = std::min(900, std::max(100, ((weightValue_ / 100))*100));
      return boost::lexical_cast<std::string>(v);
    }
  }

  return std::string();
}

std::string WFont::cssSize(bool all) const
{
  switch (size_) {
  case Medium:
    if (sizeChanged_ || all)
      return "medium";
    break;
  case XXSmall: return "xx-small";
  case XSmall: return "x-small";
  case Small: return "small";
  case Large: return "large" ;
  case XLarge: return "x-large" ;
  case XXLarge: return "xx-large";
  case Smaller: return "smaller";
  case Larger: return "larger";
  case FixedSize: return fixedSize_.cssText();
  }

  return std::string();
}

std::string WFont::cssFamily(bool all) const
{
  std::string family = specificFamilies_.toUTF8();
  if (!family.empty())
    family += ',';

  switch (genericFamily_) {
  case Default:
    if (familyChanged_ || all)
      family = "inherit"; // discard specific families
    break;
  case Serif:
    family += "serif"; break;
  case SansSerif:
    family += "sans-serif"; break;
  case Cursive:
    family += "cursive"; break;
  case Fantasy:
    family += "fantasay"; break;
  case Monospace:
    family += "monospace"; break;
  }

  return family;
}

const std::string WFont::cssText(bool combined) const
{
  SStream result;

  if (combined)
    result << cssStyle(false) << ' ' << cssVariant(false) << ' '
	   << cssWeight(false) << ' ' << cssSize(true) << ' ' << cssFamily(true);
  else {
    std::string s;
    s = cssFamily(false);
    if (!s.empty())
      result << "font-family: " << s << ";";

    s = cssSize(false);
    if (!s.empty())
      result << "font-size: " << s << ";";

    s = cssStyle(false);
    if (!s.empty())
      result << "font-style: " << s << ";";

    s = cssVariant(false);
    if (!s.empty())
      result << "font-variant: " << s << ";";

    s = cssWeight(false);
    if (!s.empty())
      result << "font-weight: " << s << ";";
  }

  return result.str();
}

}
