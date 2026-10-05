/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <limits>
#include <string>

#include <Wt/Render/WTextRenderer.h>
#include <Wt/WFont.h>
#include <Wt/WFontMetrics.h>
#include <Wt/WLength.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPainter.h>
#include <Wt/WPainterPath.h>
#include <Wt/WPointF.h>
#include <Wt/WRectF.h>
#include <Wt/WString.h>
#include <Wt/WTextF.h>

#define kMinInputLength 4
#define kMaxInputLength 4096

namespace {

// A paint device that measures but does not paint.
//
// Wt::Render needs font metrics to lay text out, and the devices that provide
// them are not usable here: WSvgImage::measureText() goes through
// WApplication::instance()->serverSideFontMetrics(), and WPdfImage needs
// libharu. This device returns synthetic metrics instead, which keeps layout
// deterministic and free of any dependency on fonts, a session or a server.
//
// The metrics are self-consistent: measureText() and fontMetrics() derive every
// width and height from the same per-character advance, so the line breaking
// loop in Render::Block always makes progress. They are also clamped away from
// zero, since WFontMetrics::size() is ascent + descent and Block treats a zero
// font height as "nothing was laid out".
class FuzzPaintDevice final : public Wt::WPaintDevice
{
public:
  FuzzPaintDevice(double width, double height)
    : width_(width),
      height_(height)
  { }

  Wt::WFlags<Wt::PaintDeviceFeatureFlag> features() const override
  {
    return Wt::PaintDeviceFeatureFlag::FontMetrics
      | Wt::PaintDeviceFeatureFlag::WordWrap;
  }

  Wt::WLength width() const override { return Wt::WLength(width_); }
  Wt::WLength height() const override { return Wt::WLength(height_); }

  void setChanged(WT_MAYBE_UNUSED Wt::WFlags<Wt::PainterChangeFlag> flags) override { }

  void drawArc(WT_MAYBE_UNUSED const Wt::WRectF& rect,
               WT_MAYBE_UNUSED double startAngle,
               WT_MAYBE_UNUSED double spanAngle) override { }

  // Overridden because WPaintDevice's default implementation throws, which
  // would end layout on the first <img> element.
  void drawImage(WT_MAYBE_UNUSED const Wt::WRectF& rect,
                 WT_MAYBE_UNUSED const std::string& imageUri,
                 WT_MAYBE_UNUSED int imgWidth,
                 WT_MAYBE_UNUSED int imgHeight,
                 WT_MAYBE_UNUSED const Wt::WRectF& sourceRect) override { }

  void drawImage(WT_MAYBE_UNUSED const Wt::WRectF& rect,
                 WT_MAYBE_UNUSED const Wt::WAbstractDataInfo* imageInfo,
                 WT_MAYBE_UNUSED int imgWidth,
                 WT_MAYBE_UNUSED int imgHeight,
                 WT_MAYBE_UNUSED const Wt::WRectF& sourceRect) override { }

  void drawLine(WT_MAYBE_UNUSED double x1, WT_MAYBE_UNUSED double y1,
                WT_MAYBE_UNUSED double x2, WT_MAYBE_UNUSED double y2) override { }

  void drawPath(WT_MAYBE_UNUSED const Wt::WPainterPath& path) override { }

  void drawRect(WT_MAYBE_UNUSED const Wt::WRectF& rectangle) override { }

  void drawText(WT_MAYBE_UNUSED const Wt::WRectF& rect,
                WT_MAYBE_UNUSED Wt::WFlags<Wt::AlignmentFlag> alignmentFlags,
                WT_MAYBE_UNUSED Wt::TextFlag textFlag,
                WT_MAYBE_UNUSED const Wt::WTextF& text,
                WT_MAYBE_UNUSED const Wt::WPointF* clipPoint) override { }

  Wt::WTextItem measureText(const Wt::WString& text, double maxWidth = -1,
                            bool wordWrap = false) override
  {
    const std::string s = text.toUTF8();
    const double cw = charWidth();

    if (maxWidth < 0 || textWidth(s, s.length(), cw) <= maxWidth) {
      return Wt::WTextItem(text, textWidth(s, s.length(), cw));
    }

    // Longest prefix that fits, never splitting a multi-byte character.
    std::size_t fit = 0;
    for (std::size_t i = 1; i <= s.length(); ++i) {
      if (i < s.length() && isContinuationByte(s[i])) {
        continue;
      }
      if (textWidth(s, i, cw) > maxWidth) {
        break;
      }
      fit = i;
    }

    // With word wrapping, retreat to the last whitespace inside that prefix.
    // Wt includes the whitespace in the returned text but not in the width.
    std::size_t brk = fit;
    if (wordWrap) {
      brk = 0;
      for (std::size_t i = 0; i < fit; ++i) {
        if (isWhitespace(s[i])) {
          brk = i + 1;
        }
      }
    }

    if (brk == 0) {
      // Nothing fits. Block starts a new line, and asks for the width of a
      // single word when there is not even room for one.
      return Wt::WTextItem(Wt::WString(), 0);
    }

    std::size_t widthEnd = brk;
    while (widthEnd > 0 && isWhitespace(s[widthEnd - 1])) {
      --widthEnd;
    }

    return Wt::WTextItem(Wt::WString::fromUTF8(s.substr(0, brk)),
                         textWidth(s, widthEnd, cw));
  }

  Wt::WFontMetrics fontMetrics() override
  {
    const double size = fontSize();
    return Wt::WFontMetrics(painter_ ? painter_->font() : Wt::WFont(),
                            0.0, 0.8 * size, 0.2 * size);
  }

  void init() override { }
  void done() override { }
  bool paintActive() const override { return painter_ != nullptr; }

protected:
  Wt::WPainter *painter() const override { return painter_; }
  void setPainter(Wt::WPainter *painter) override { painter_ = painter; }

private:
  static bool isContinuationByte(char c)
  {
    return (static_cast<unsigned char>(c) & 0xC0) == 0x80;
  }

  static bool isWhitespace(char c)
  {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
  }

  // The font size drives every metric. It is taken from the painter so that
  // CSS font sizes still affect layout, but clamped so that the size is never
  // zero, negative, NaN or large enough to make layout unbounded.
  double fontSize() const
  {
    double size = 16.0;
    if (painter_) {
      size = painter_->font().sizeLength(16).toPixels();
    }
    if (!(size >= 1.0)) {
      size = 1.0;
    }
    if (size > 512.0) {
      size = 512.0;
    }
    return size;
  }

  double charWidth() const { return 0.5 * fontSize(); }

  // Width of the first nbytes of s, counting UTF-8 characters rather than
  // bytes so that the result matches what Block sees.
  static double textWidth(const std::string& s, std::size_t nbytes, double cw)
  {
    std::size_t chars = 0;
    for (std::size_t i = 0; i < nbytes && i < s.length(); ++i) {
      if (!isContinuationByte(s[i])) {
        ++chars;
      }
    }
    return cw * static_cast<double>(chars);
  }

  double width_, height_;
  Wt::WPainter *painter_ = nullptr;
};

// Renders onto FuzzPaintDevice. Pages are reused: the same device and painter
// serve every page, so a document that spans many pages costs no allocations.
class FuzzTextRenderer final : public Wt::Render::WTextRenderer
{
public:
  FuzzTextRenderer(double pageWidth, double pageHeight, double margin)
    : device_(pageWidth, pageHeight),
      pageWidth_(pageWidth),
      pageHeight_(pageHeight),
      margin_(margin)
  { }

  double pageWidth(WT_MAYBE_UNUSED int page) const override { return pageWidth_; }
  double pageHeight(WT_MAYBE_UNUSED int page) const override { return pageHeight_; }
  double margin(WT_MAYBE_UNUSED Wt::Side side) const override { return margin_; }

  Wt::WPaintDevice *startPage(WT_MAYBE_UNUSED int page) override
  {
    return &device_;
  }

  void endPage(WT_MAYBE_UNUSED Wt::WPaintDevice *device) override
  {
    painter_.end();
  }

  Wt::WPainter *getPainter(Wt::WPaintDevice *device) override
  {
    painter_.begin(device);
    return &painter_;
  }

private:
  FuzzPaintDevice device_;
  Wt::WPainter painter_;
  double pageWidth_, pageHeight_, margin_;
};

}

// Fuzzes the Wt::Render XHTML/CSS layout engine (src/Wt/Render/Block.C and
// friends) through WTextRenderer::render(), the entry point WPdfRenderer uses
// to turn rich text into a paint device.
//
// One call covers: rapidxml in parse_xhtml_entity_translation mode, the CSS
// cascade (CssParser, CssData, Specificity), Block::determineDisplay(),
// normalizeWhitespace(), two layout passes over blocks, tables, floats and
// inline text, page breaking, and finally the paint pass over every page.
//
// Input layout:
//   byte 0        selector
//   bytes 1..2    style sheet length, little endian
//   remainder     that many bytes of style sheet, then the XHTML
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size < kMinInputLength || Size > kMaxInputLength) {
    return 0;
  }

  const uint8_t selector = Data[0];
  const size_t bodyLen = Size - 3;
  const char *body = reinterpret_cast<const char *>(Data + 3);

  size_t styleLen = static_cast<size_t>(Data[1]) | (static_cast<size_t>(Data[2]) << 8);
  if (styleLen > bodyLen) {
    styleLen = bodyLen;
  }

  const std::string styleSheet(body, styleLen);
  const std::string xhtml(body + styleLen, bodyLen - styleLen);

  // A page wide enough that ordinary text does not break on every character,
  // and short enough that page breaking is exercised.
  const double pageWidth = (selector & 0x04) ? 240 : 800;
  const double pageHeight = (selector & 0x08) ? 200 : 1000;

  FuzzTextRenderer renderer(pageWidth, pageHeight, 10);

  if (selector & 0x01) {
    // Ignores the return value on purpose: a style sheet that fails to parse
    // leaves the previous one in place, which is also worth rendering with.
    renderer.setStyleSheetText(Wt::WString::fromUTF8(styleSheet));
  }

  if (selector & 0x02) {
    renderer.setFontScale(1.5);
  }

  try {
    renderer.render(Wt::WString::fromUTF8(xhtml));
  } catch (...) {
    // render() propagates rapidxml::parse_error, and Block propagates
    // WException for unresolvable <img> sources and malformed CSS lengths.
  }

  return 0;
}
