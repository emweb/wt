/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WFont.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"

#include "DomElement.h"

#include <algorithm>

namespace Wt {

WFont::WFont()
  : widget_(nullptr),
    genericFamily_(FontFamily::Default),
    style_(FontStyle::Normal),
    variant_(FontVariant::Normal),
    weight_(FontWeight::Normal),
    weightValue_(400),
    size_(FontSize::Medium),
    familyChanged_(false),
    styleChanged_(false),
    variantChanged_(false),
    weightChanged_(false),
    sizeChanged_(false)
{ }

WFont::WFont(FontFamily family)
  : widget_(nullptr),
    genericFamily_(family),
    style_(FontStyle::Normal),
    variant_(FontVariant::Normal),
    weight_(FontWeight::Normal),
    weightValue_(400),
    size_(FontSize::Medium),
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
    && sizeLength_       == other.sizeLength_;
}

bool WFont::operator!=(const WFont& other) const
{
  return !(*this == other);
}

void WFont::setFamily(FontFamily genericFamily,
		      const WString& specificFamilies)
{
  genericFamily_ = genericFamily;
  specificFamilies_ = specificFamilies;
  familyChanged_ = true;
  if (widget_) widget_->repaint(RepaintFlag::SizeAffected);
}

void WFont::setStyle(FontStyle style)
{
  style_ = style;
  styleChanged_ = true;
  if (widget_) widget_->repaint(RepaintFlag::SizeAffected);
}

void WFont::setVariant(FontVariant variant)
{
  variant_ = variant;
  variantChanged_ = true;
  if (widget_) widget_->repaint(RepaintFlag::SizeAffected);
}

void WFont::setWeight(FontWeight weight, int value)
{
  weight_ = weight;
  weightValue_ = value;
  weightChanged_ = true;
  if (widget_) widget_->repaint(RepaintFlag::SizeAffected);
}

FontWeight WFont::weight() const
{
  if (weight_ != FontWeight::Value)
    return weight_;
  else
    return weightValue_ >= 700 ? FontWeight::Bold : FontWeight::Normal;
}

int WFont::weightValue() const
{
  switch (weight_) {
  case FontWeight::Normal:
  case FontWeight::Lighter:
    return 400;
  case FontWeight::Bold:
  case FontWeight::Bolder:
    return 700;
  case FontWeight::Value:
    return weightValue_;
  }

  assert(false);
  return -1;
}

void WFont::setSize(FontSize size)
{
  size_ = size;
  sizeLength_ = WLength::Auto;
  sizeChanged_ = true;
  if (widget_) widget_->repaint(RepaintFlag::SizeAffected);
}

void WFont::setSize(const WLength& size)
{
  size_ = FontSize::FixedSize;
  sizeLength_ = size;
  sizeChanged_ = true;
  if (widget_) widget_->repaint(RepaintFlag::SizeAffected);
}

FontSize WFont::size(double mediumSize) const
{
  if (size_ != FontSize::FixedSize)
    return size_;
  else {
    double pixels = sizeLength_.toPixels();

    if (pixels == mediumSize)
      return FontSize::Medium;
    else if (pixels > mediumSize) {
      if (pixels < 1.2 * 1.19 * mediumSize)
	return FontSize::Large;
      else if (pixels < 1.2 * 1.2 * 1.19 * mediumSize)
	return FontSize::XLarge;
      else
	return FontSize::XXLarge;
    } else {
      if (pixels > mediumSize / 1.2 / 1.19)
	return FontSize::Small;
      else if (pixels > mediumSize / 1.2 / 1.2 / 1.19)
	return FontSize::XSmall;
      else
	return FontSize::XXSmall;
    }
  }
}

WLength WFont::sizeLength(double mediumSize) const
{
  switch (size_) {
  case FontSize::FixedSize:
    return sizeLength_;
  case FontSize::XXSmall:
    return WLength(mediumSize / 1.2 / 1.2 / 1.2);
  case FontSize::XSmall:
    return WLength(mediumSize / 1.2 / 1.2);
  case FontSize::Small:
    return WLength(mediumSize / 1.2);
  case FontSize::Medium:
    return WLength(mediumSize);
  case FontSize::Large:
    return WLength(mediumSize * 1.2);
  case FontSize::XLarge:
    return WLength(mediumSize * 1.2 * 1.2);
  case FontSize::XXLarge:
    return WLength(mediumSize * 1.2 * 1.2 * 1.2);    
  case FontSize::Smaller:
    return WLength(1 / 1.2, LengthUnit::FontEm);
  case FontSize::Larger:
    return WLength(1.2, LengthUnit::FontEm);
  }

  assert(false);
  return WLength();
}

void WFont::updateDomElement(DomElement& element, bool fontall, bool all)
{
  if (familyChanged_ || fontall || all) {
    std::string family = cssFamily(fontall);

    if (!family.empty())
      element.setProperty(Property::StyleFontFamily, family);

    familyChanged_ = false;
  }

  if (styleChanged_ || fontall || all) {
    std::string style = cssStyle(fontall);

    if (!style.empty())
      element.setProperty(Property::StyleFontStyle, style);

    styleChanged_ = false;
  }

  if (variantChanged_ || fontall || all) {
    std::string variant = cssVariant(fontall);

    if (!variant.empty())
      element.setProperty(Property::StyleFontVariant, variant);

    variantChanged_ = false;
  }

  if (weightChanged_ || fontall || all) {
    std::string weight = cssWeight(fontall);

    if (!weight.empty())
      element.setProperty(Property::StyleFontWeight, weight);

    weightChanged_ = false;
  }

  if (sizeChanged_ || fontall || all) {
    std::string size = cssSize(fontall);

    if (!size.empty())
      element.setProperty(Property::StyleFontSize, size);

    sizeChanged_ = false;
  }
}

std::string WFont::cssStyle(bool all) const
{
  switch (style_) {
  case FontStyle::Normal:
    if (styleChanged_ || all)
      return "normal";
    break;
  case FontStyle::Italic: return "italic";
  case FontStyle::Oblique: return "oblique";
  }

  return std::string();
}

std::string WFont::cssVariant(bool all) const
{
  switch (variant_) {
  case FontVariant::Normal:
    if (variantChanged_ || all)
      return "normal";
    break;
  case FontVariant::SmallCaps:
    return "small-caps";
  }

  return std::string();
}


std::string WFont::cssWeight(bool all) const
{
  switch (weight_) {
  case FontWeight::Normal:
    if (weightChanged_ || all)
      return "normal";
    break;
  case FontWeight::Bold: return "bold";
  case FontWeight::Bolder:return "bolder";
  case FontWeight::Lighter: return "lighter";
  case FontWeight::Value: {
    int v = std::min(900, std::max(100, ((weightValue_ / 100))*100));
    return std::to_string(v);
  }
  }

  return std::string();
}

std::string WFont::cssSize(bool all) const
{
  switch (size_) {
  case FontSize::Medium:
    if (sizeChanged_ || all)
      return "medium";
    break;
  case FontSize::XXSmall: return "xx-small";
  case FontSize::XSmall: return "x-small";
  case FontSize::Small: return "small";
  case FontSize::Large: return "large" ;
  case FontSize::XLarge: return "x-large" ;
  case FontSize::XXLarge: return "xx-large";
  case FontSize::Smaller: return "smaller";
  case FontSize::Larger: return "larger";
  case FontSize::FixedSize: return sizeLength_.cssText();
  }

  return std::string();
}

std::string WFont::cssFamily(bool all) const
{
  std::string family = specificFamilies_.toUTF8();

  if ((!family.empty()) &&
      genericFamily_ != FontFamily::Default)
    family += ',';

  switch (genericFamily_) {
  case FontFamily::Default:
    break;
  case FontFamily::Serif:
    family += "serif"; break;
  case FontFamily::SansSerif:
    family += "sans-serif"; break;
  case FontFamily::Cursive:
    family += "cursive"; break;
  case FontFamily::Fantasy:
    family += "fantasy"; break;
  case FontFamily::Monospace:
    family += "monospace"; break;
  }

  return family;
}

const std::string WFont::cssText(bool combined) const
{
  WStringStream result;

  if (combined) {
    std::string s;
    s = cssStyle(false);
    if (!s.empty())
      result << s << ' ';

    s = cssVariant(false);
    if (!s.empty())
      result << s << ' ';

    s = cssWeight(false);
    if (!s.empty())
      result << s << ' ';

    result << cssSize(true) << ' ';

    s = cssFamily(true);
    if (!s.empty())
      result << s << ' ';
    else
      result << s << " inherit";
  } else {
    std::string s;
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

    // Last because of workaround in WVmlImage that searches for a ','
    s = cssFamily(false);
    if (!s.empty())
      result << "font-family: " << s << ";";

  }

  return result.str();
}

}
