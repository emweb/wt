/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/FontSupport.h"

#include "Wt/WDllDefs.h"

#include "Wt/WException.h"
#include "Wt/WFontMetrics.h"
#include "Wt/WLogger.h"
#include "Wt/WPointF.h"
#include "Wt/WRasterImage.h"
#include "Wt/WRectF.h"
#include "Wt/WString.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>

// FIXME: addFontCollection?
// FIXME: rendering?

namespace {

// This enum value is not always available when building
const DWRITE_WORD_WRAPPING WORD_WRAPPING_WHOLE_WORD = (DWRITE_WORD_WRAPPING)3;

const double EPSILON = 1e-4;

bool isEpsilonMore(double x, double limit) {
  return x - EPSILON > limit;
}

bool isWhitespace(WCHAR c)
{
  return c == L' ' || c == L'\t' || c == L'\n' || c == L'\r';
}

template <class T> void SafeRelease(T *&ppT)
{
  if (ppT) {
    ppT->Release();
    ppT = NULL;
  }
}

DWRITE_FONT_WEIGHT dwriteFontWeight(const Wt::WFont &font)
{
  switch (font.weight()) {
  case Wt::FontWeight::Lighter:
    return DWRITE_FONT_WEIGHT_LIGHT;
  case Wt::FontWeight::Normal:
    return DWRITE_FONT_WEIGHT_NORMAL;
  case Wt::FontWeight::Bold:
    return DWRITE_FONT_WEIGHT_BOLD;
  case Wt::FontWeight::Bolder:
    return DWRITE_FONT_WEIGHT_EXTRA_BOLD;
  default:
    return (DWRITE_FONT_WEIGHT)font.weightValue();
  }
}

DWRITE_FONT_STYLE dwriteFontStyle(const Wt::WFont &font)
{
  switch (font.style()) {
  case Wt::FontStyle::Normal:
    return DWRITE_FONT_STYLE_NORMAL;
  case Wt::FontStyle::Oblique:
    return DWRITE_FONT_STYLE_OBLIQUE;
  case Wt::FontStyle::Italic:
    return DWRITE_FONT_STYLE_ITALIC;
  default:
    return DWRITE_FONT_STYLE_NORMAL;
  }
}

bool isTrueType(const std::wstring &filename)
{
  return filename.size() > 4 && (boost::iends_with(filename, L".ttf") || boost::iends_with(filename, L".ttc"));
}

std::wstring nameForFontFamily(IDWriteFontFamily *fontFamily)
{
  HRESULT hr = S_OK;

  IDWriteLocalizedStrings *names = NULL;
  hr = fontFamily->GetFamilyNames(&names);

  UINT32 length = 0;
  if (SUCCEEDED(hr)) {
    hr = names->GetStringLength(0, &length);
  }

  std::wstring result;
  if (SUCCEEDED(hr)) {
    WCHAR *buffer = new WCHAR[length + 1];
    hr = names->GetString(0, buffer, length + 1);
    result = std::wstring(buffer, length);
    delete[] buffer;
  }

  SafeRelease(names);

  return result;
}

std::wstring familyNameForFont(IDWriteFont *font)
{
  HRESULT hr = S_OK;

  IDWriteFontFamily *fontFamily = NULL;
  hr = font->GetFontFamily(&fontFamily);

  if (SUCCEEDED(hr))
    return nameForFontFamily(fontFamily);

  return L"";
}

std::wstring fileNameForFontFace(IDWriteFontFace *fontFace)
{
  std::wstring result;
  HRESULT hr = S_OK;
  UINT32 numFiles = 1;
  IDWriteFontFile *fontFile = NULL;
  hr = fontFace->GetFiles(&numFiles, &fontFile);
  IDWriteFontFileLoader *fontLoader = NULL;
  hr = fontFile->GetLoader(&fontLoader);
  IDWriteLocalFontFileLoader *localFontLoader = NULL;
  hr = fontLoader->QueryInterface(&localFontLoader);
  if (localFontLoader) {
    const void *referenceKey = NULL;
    UINT32 referenceKeySize = UINT32_MAX;
    hr = fontFile->GetReferenceKey(&referenceKey, &referenceKeySize);
    UINT32 filePathLength = UINT32_MAX;
    hr = localFontLoader->GetFilePathLengthFromKey(referenceKey, referenceKeySize, &filePathLength);
    WCHAR *wFileName = new WCHAR[filePathLength + 1];
    hr = localFontLoader->GetFilePathFromKey(referenceKey, referenceKeySize, wFileName, filePathLength + 1);
    result = wFileName;
    delete[] wFileName;
  }
  SafeRelease(localFontLoader);
  SafeRelease(fontLoader);
  SafeRelease(fontFile);
  return result;
}

std::wstring fileNameForFont(IDWriteFont *font)
{
  HRESULT hr = S_OK;
  IDWriteFontFace *fontFace = NULL;
  hr = font->CreateFontFace(&fontFace);
  std::wstring result = fileNameForFontFace(fontFace);
  SafeRelease(fontFace);
  return result;
}

std::wstring ttfFontFromFamily(IDWriteFontFamily *fontFamily, const Wt::WFont &f)
{
  std::wstring result;
  HRESULT hr = S_OK;
  IDWriteFontList *fontList = NULL;
  hr = fontFamily->GetMatchingFonts(dwriteFontWeight(f), DWRITE_FONT_STRETCH_NORMAL, dwriteFontStyle(f), &fontList);
  UINT32 fontCount = fontList->GetFontCount();
  bool ttfFound = false;
  for (UINT32 i = 0; i < fontCount && !ttfFound; ++i) {
    IDWriteFont *font = NULL;
    hr = fontList->GetFont(i, &font);
    std::wstring fontFileName = fileNameForFont(font);
    if (isTrueType(fontFileName)) {
      result = fontFileName;
      ttfFound = true;
    }
    SafeRelease(font);
  }
  SafeRelease(fontList);
  return result;
}

std::wstring ttfForFont(IDWriteFontCollection *collection, const Wt::WFont &f, IDWriteFont *mappedFont, IDWriteFontFace *fontFace)
{
  if (mappedFont == NULL) {
    HRESULT hr = S_OK;
    IDWriteFont *font = NULL;
    hr = collection->GetFontFromFontFace(fontFace, &font);
    std::wstring result = ttfForFont(NULL, f, font, fontFace);
    SafeRelease(font);
    return result;
  }
  std::wstring fontFileName = fontFace != NULL ? fileNameForFontFace(fontFace) : fileNameForFont(mappedFont);
  if (isTrueType(fontFileName))
    return fontFileName;
  else {
    fontFileName.clear();
    HRESULT hr = S_OK;
    IDWriteFontFamily *fontFamily = NULL;
    hr = mappedFont->GetFontFamily(&fontFamily);
    fontFileName = ttfFontFromFamily(fontFamily, f);
    SafeRelease(fontFamily);
    return fontFileName;
  }
}

std::wstring ttfForFont(const Wt::WFont &f, IDWriteFont *font)
{
  return ttfForFont(NULL, f, font, NULL);
}

std::wstring ttfForFontFace(IDWriteFontCollection *collection, const Wt::WFont &f, IDWriteFontFace *fontFace)
{
  return ttfForFont(collection, f, NULL, fontFace);
}

std::wstring genericFamilyToName(Wt::FontFamily family)
{
  switch (family) {
  case Wt::FontFamily::Default:
  case Wt::FontFamily::Serif:
    return L"Times New Roman";
    break;
  case Wt::FontFamily::SansSerif:
    return L"Arial";
    break;
  case Wt::FontFamily::Monospace:
    return L"Consolas";
    break;
  case Wt::FontFamily::Fantasy:
    return L"Gabriola"; // IE/Edge on Windows choose Gabriola, so let's use that
    break;
  case Wt::FontFamily::Cursive:
    return L"Comic Sans MS"; // Apparently, all browsers on Windows choose Comic Sans
  default:
    return L"";
  }
}

}

namespace Wt {

LOGGER("FontSupportDirectWrite");

class FontSupport::TextFragmentRenderer : public IDWriteTextRenderer {
public:
  TextFragmentRenderer(IDWriteFontCollection *collection,
                       const WFont &f,
                       std::vector<FontSupport::TextFragment> &fragments,
                       FontSupport *fontSupport,
                       double &width,
                       double &nextWidth,
                       double maxWidth,
                       bool wordWrap)
    : refcount_(1),
      collection_(collection),
      f_(f),
      fragments_(fragments),
      offset_(0),
      fontSupport_(fontSupport),
      width_(width),
      nextWidth_(nextWidth),
      maxWidth_(maxWidth),
      wordWrap_(wordWrap),
      maxWidthReached_(false)
  { }

  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
  {
    if (iid == __uuidof(IUnknown) ||
        iid == __uuidof(IDWritePixelSnapping) ||
        iid == __uuidof(IDWriteTextRenderer)) {
      *ppvObject = static_cast<IDWriteTextRenderer*>(this);
      AddRef();
      return S_OK;
    } else
      return E_NOINTERFACE;
  }

  virtual ULONG STDMETHODCALLTYPE AddRef(void)
  {
    return (ULONG)InterlockedIncrement(&refcount_);
  }

  virtual ULONG STDMETHODCALLTYPE Release(void)
  {
    ULONG res = (ULONG)InterlockedDecrement(&refcount_);
    if (res == 0)
      delete this;
    return res;
  }

  virtual HRESULT STDMETHODCALLTYPE GetCurrentTransform(void *clientDrawingContext,
                                                        DWRITE_MATRIX *transform)
  {
    transform->dx = 0.0f;
    transform->dy = 0.0f;
    transform->m11 = 1.0f;
    transform->m12 = 0.0f;
    transform->m21 = 0.0f;
    transform->m22 = 1.0f;
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void *clientDrawingContext,
                                                    FLOAT *pixelsPerDip)
  {
    *pixelsPerDip = 1.0f;
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void *clientDrawingContext,
                                                            BOOL *isDisabled)
  {
    *isDisabled = FALSE;
    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(void *clientDrawingContext,
                                                 FLOAT baselineOriginX,
                                                 FLOAT baselineOriginY,
                                                 DWRITE_MEASURING_MODE measuringMode,
                                                 const DWRITE_GLYPH_RUN *glyphRun,
                                                 const DWRITE_GLYPH_RUN_DESCRIPTION *glyphRunDescription,
                                                 IUnknown *clientDrawingEffect)
  {
    if (maxWidthReached_)
      return E_ABORT;

    IDWriteFontFace *fontFace = glyphRun->fontFace;
    UINT32 stringLength = glyphRunDescription->stringLength;
    UINT32 textPosition = glyphRunDescription->textPosition;
    std::string path = WString(ttfForFontFace(collection_, f_, fontFace)).toUTF8();
    fontSupport_->drawingFontPath_ = path;
    std::wstring substr(glyphRunDescription->string, glyphRunDescription->stringLength);
    WTextItem textItem = fontSupport_->device_->measureText(Wt::WString(substr), -1, false);
    double fragmentWidth = textItem.width();

    if (wordWrap_ && isEpsilonMore(width_ + fragmentWidth, maxWidth_)) {
      nextWidth_ = fragmentWidth;
      // FIXME: do we need to remove trailing spaces?
      maxWidthReached_ = true;
      return E_ABORT;
    }

    fragments_.push_back(TextFragment(path, textPosition + offset_, stringLength, fragmentWidth));
    width_ += fragmentWidth;

    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE DrawInlineObject(void *clientDrawingContext,
                                                     FLOAT originX,
                                                     FLOAT originY,
                                                     IDWriteInlineObject *inlineObject,
                                                     BOOL isSideways,
                                                     BOOL isRightToLeft,
                                                     IUnknown *clientDrawingEffect)
  {
    return E_NOINTERFACE;
  }

  virtual HRESULT STDMETHODCALLTYPE DrawStrikethrough(void *clientDrawingContext,
                                                      FLOAT baselineOriginX,
                                                      FLOAT baselineOriginY,
                                                      const DWRITE_STRIKETHROUGH *strikethrough,
                                                      IUnknown *clientDrawingEffect)
  {
    return E_NOINTERFACE;
  }

  virtual HRESULT STDMETHODCALLTYPE DrawUnderline(void *clientDrawingContext,
                                                  FLOAT baselineOriginX,
                                                  FLOAT baselineOriginY,
                                                  const DWRITE_UNDERLINE *underline,
                                                  IUnknown *clientDrawingEffect)
  {
    return E_NOINTERFACE;
  }

  bool maxWidthReached() const { return maxWidthReached_; }

  void setOffset(std::size_t offset) { offset_ = offset; }

private:
  LONG refcount_;
  IDWriteFontCollection *collection_;
  const WFont f_;
  std::vector<FontSupport::TextFragment> &fragments_;
  std::size_t offset_;
  FontSupport *fontSupport_;
  double &width_;
  double &nextWidth_;
  double maxWidth_;
  double wordWrap_;
  bool maxWidthReached_;
};

FontSupport::Bitmap::Bitmap(int width, int height)
  : width_(-1),
    height_(-1),
    pitch_(-1),
    buffer_(0)
{ }

FontSupport::Bitmap::~Bitmap()
{ }

bool FontSupport::FontMatch::matched() const
{
  return textFormat_ != NULL && fontFamily_ != NULL && font_ != NULL;
}

std::string FontSupport::FontMatch::fileName() const
{
  if (!matched())
    return "";

  if (fontFamilyFileName_.empty())
    fontFamilyFileName_ = WString(fileNameForFont(font_)).toUTF8();

  return fontFamilyFileName_;
}

std::wstring FontSupport::FontMatch::fontFamilyName() const
{
  if (!matched())
    return L"";

  return nameForFontFamily(fontFamily_);
}

FontSupport::FontMatch::FontMatch()
  : textFormat_(NULL),
    fontFamily_(NULL),
    font_(NULL)
{ }

FontSupport::FontMatch::FontMatch(IDWriteTextFormat *textFormat,
  IDWriteFontFamily *fontFamily,
  IDWriteFont *font,
  std::string fontFamilyFileName)
  : textFormat_(textFormat),
    fontFamily_(fontFamily),
    font_(font),
    fontFamilyFileName_(fontFamilyFileName)
{ }

FontSupport::FontMatch::~FontMatch()
{
  SafeRelease(textFormat_);
  SafeRelease(fontFamily_);
  SafeRelease(font_);
}

FontSupport::FontMatch::FontMatch(const FontSupport::FontMatch &other)
  : textFormat_(other.textFormat_),
    fontFamily_(other.fontFamily_),
    font_(other.font_),
    fontFamilyFileName_(other.fontFamilyFileName_)
{
  if (textFormat_)
    textFormat_->AddRef();
  if (fontFamily_)
    fontFamily_->AddRef();
  if (font_)
    font_->AddRef();
}

FontSupport::FontMatch &FontSupport::FontMatch::operator= (const FontSupport::FontMatch &other)
{
  if (this == &other)
    return *this;

  SafeRelease(textFormat_);
  SafeRelease(fontFamily_);
  SafeRelease(font_);

  textFormat_ = other.textFormat_;
  fontFamily_ = other.fontFamily_;
  font_ = other.font_;
  fontFamilyFileName_ = other.fontFamilyFileName_;

  if (textFormat_)
    textFormat_->AddRef();
  if (fontFamily_)
    fontFamily_->AddRef();
  if (font_)
    font_->AddRef();

  return *this;
}

#ifdef WT_CXX11
FontSupport::FontMatch::FontMatch(FontSupport::FontMatch &&other)
  : textFormat_(other.textFormat_),
    fontFamily_(other.fontFamily_),
    font_(other.font_),
    fontFamilyFileName_(std::move(other.fontFamilyFileName_))
{
  other.textFormat_ = nullptr;
  other.fontFamily_ = nullptr;
  other.font_ = nullptr;
}

FontSupport::FontMatch &FontSupport::FontMatch::operator= (FontSupport::FontMatch &&other)
{
  if (this == &other)
    return *this;

  textFormat_ = other.textFormat_;
  fontFamily_ = other.fontFamily_;
  font_ = other.font_;
  fontFamilyFileName_ = std::move(other.fontFamilyFileName_);

  other.textFormat_ = nullptr;
  other.fontFamily_ = nullptr;
  other.font_ = nullptr;

  return *this;
}
#endif // WT_CXX11

FontSupport::FontSupport(WPaintDevice *device, EnabledFontFormats enabledFontFormats)
  : device_(device),
    enabledFontFormats_(enabledFontFormats),
    writeFactory_(NULL),
    font_(0),
    cache_(10)
{
  HRESULT hr = S_OK;

  if (SUCCEEDED(hr)) {
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (!SUCCEEDED(hr))
      throw WException("DWrite: Error initializing COM: HRESULT " + boost::lexical_cast<std::string>(hr));
  }

  // not clear from documentation: do we have to share the factory created
  // by this call in order to benefit from shared font caching etc, or will
  // a shared font cache be used by specifying the SHARED option? When
  // specifying the SHARED option, is the factory thread-safe? We take the
  // safe approach, and assume that each specifying SHARED will cause state
  // to be shared in a thread-safe manner
  if (SUCCEEDED(hr))
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
      __uuidof(writeFactory_),
      reinterpret_cast<IUnknown **>(&writeFactory_));

  if (!SUCCEEDED(hr))
    throw WException("DWrite: Error creating DWrite factory: HRESULT " + boost::lexical_cast<std::string>(hr));
}

FontSupport::~FontSupport()
{
  SafeRelease(writeFactory_);

  cache_.clear();

  CoUninitialize();
}

void FontSupport::setDevice(WPaintDevice *device)
{
  device_ = device;
}

FontSupport::FontMatch FontSupport::matchFont(const WFont &f) const
{
  for (MatchCache::iterator i = cache_.begin(); i != cache_.end(); ++i) {
    if (i->font == f && i->match.matched()) {
      cache_.splice(cache_.begin(), cache_, i); // implement LRU
      return i->match;
    }
  }

  HRESULT hr = S_OK;
  IDWriteFontCollection *sysFontCollection = NULL;
  if (SUCCEEDED(hr))
    hr = writeFactory_->GetSystemFontCollection(&sysFontCollection);

  DWRITE_FONT_WEIGHT weight = dwriteFontWeight(f);
  DWRITE_FONT_STYLE style = dwriteFontStyle(f);

  std::string fontFamilyFileName;
  std::wstring fontFamilyName;
  WString families = f.specificFamilies();
  if (!families.empty()) {
    std::wstring wFamilies = families;
    std::vector<std::wstring> splitFamilies;
    boost::split(splitFamilies, wFamilies, boost::is_any_of(L","));
    for (std::size_t i = 0; i < splitFamilies.size(); ++i) {
      boost::trim_if(splitFamilies[i], boost::is_any_of(L"\"' "));
      const std::wstring &family = splitFamilies[i];
      UINT32 familyIndex = UINT32_MAX;
      BOOL found = false;
      sysFontCollection->FindFamilyName(family.c_str(), &familyIndex, &found);
      if (found) {
        if (enabledFontFormats_ == TrueTypeOnly) {
          // Only use truetype fonts
          IDWriteFontFamily *idwFamily = NULL;
          hr = sysFontCollection->GetFontFamily(familyIndex, &idwFamily);
          std::wstring fn = ttfFontFromFamily(idwFamily, f);
          SafeRelease(idwFamily);
          if (!fn.empty()) {
            fontFamilyName = family;
            fontFamilyFileName = WString(fn).toUTF8();
            break;
          }
        } else {
          fontFamilyName = family;
          break;
        }
      }
    }
  }

  if (fontFamilyName.empty()) {
    fontFamilyName = genericFamilyToName(f.genericFamily());
  }

  IDWriteTextFormat *textFormat = NULL;
  if (SUCCEEDED(hr))
    hr = writeFactory_->CreateTextFormat(fontFamilyName.c_str(),
      NULL,
      weight,
      style,
      DWRITE_FONT_STRETCH_NORMAL,
      static_cast<FLOAT>(f.sizeLength(12).toPixels()),
      L"",
      &textFormat);
  UINT32 fontIndex = UINT32_MAX;
  BOOL fontExists = FALSE;
  if (SUCCEEDED(hr))
    hr = sysFontCollection->FindFamilyName(fontFamilyName.c_str(), &fontIndex, &fontExists);
  if (!fontExists) {
    LOG_ERROR("Font family with name " << WString(fontFamilyName).toUTF8() << " not found");
    return FontMatch();
  }
  IDWriteFontFamily *fontFamily = NULL;
  if (SUCCEEDED(hr))
    hr = sysFontCollection->GetFontFamily(fontIndex, &fontFamily);
  IDWriteFont *font = NULL;
  if (SUCCEEDED(hr))
    hr = fontFamily->GetFirstMatchingFont(weight, DWRITE_FONT_STRETCH_NORMAL, style, &font);

  SafeRelease(sysFontCollection);

  cache_.pop_back();
  cache_.push_front(Matched(f, FontMatch(textFormat, fontFamily, font, WString(fontFamilyFileName).toUTF8())));

  return cache_.front().match;
}

WFontMetrics FontSupport::fontMetrics(const WFont &f)
{
  font_ = &f;
  FontMatch match = matchFont(f);
  DWRITE_FONT_METRICS metrics;
  match.font()->GetMetrics(&metrics);
  double unitsPerEm = metrics.designUnitsPerEm;
  double ems = match.textFormat()->GetFontSize();
  double pxs = f.sizeLength().toPixels();
  double pxsPerEm = pxs / ems;
  double ascent = metrics.ascent / unitsPerEm * pxsPerEm;
  double descent = metrics.descent / unitsPerEm * pxsPerEm;
  double leading = metrics.lineGap / unitsPerEm * pxsPerEm;
  font_ = 0;
  return WFontMetrics(f, leading, ascent, descent);
}

WTextItem FontSupport::measureText(const WFont &f, const WString &text, double maxWidth, bool wordWrap)
{
  bool wasBusy = busy();
  font_ = &f;
  if (!wasBusy && device_->features().test(PaintDeviceFeatureFlag::FontMetrics)) {
    if (dynamic_cast<WRasterImage*>(device_)) {
      WTextItem textItem = device_->measureText(text, maxWidth, wordWrap);
      font_ = 0;
      return textItem;
    } else {
      std::vector<TextFragment> fragments;
      std::wstring wtext = text;
      double totalWidth = 0;
      double nextWidth = -1;
      layoutText(f, fragments, wtext, totalWidth, nextWidth, maxWidth, wordWrap);
      std::size_t length = fragments.empty() ? 0 : fragments.back().start + fragments.back().length;
      font_ = 0;
      return WTextItem(wtext.substr(0, length), totalWidth, nextWidth);
    }
  } else {
    FontMatch match = matchFont(f);
    HRESULT hr = S_OK;
    std::wstring txt = text;
    IDWriteTextLayout *textLayout = NULL;
    hr = writeFactory_->CreateTextLayout(
      txt.c_str(),
      txt.size(),
      match.textFormat(),
      maxWidth == -1.0 ?
      std::numeric_limits<FLOAT>::infinity() :
      static_cast<FLOAT>(maxWidth),
      std::numeric_limits<FLOAT>::infinity(),
      &textLayout);
    DWRITE_TEXT_METRICS textMetrics;
    hr = textLayout->GetMetrics(&textMetrics);
    SafeRelease(textLayout);
    font_ = 0;
    return WTextItem(text, textMetrics.width);
  }
}

void FontSupport::layoutText(const WFont &f,
  std::vector<FontSupport::TextFragment> &v,
  const std::wstring &s, double &width, double &nextWidth, double maxWidth, bool wordWrap)
{
  if (wordWrap && maxWidth <= 0) {
    return;
  }
  HRESULT hr = S_OK;

  FontMatch match = matchFont(f);

  IDWriteFontCollection *collection = NULL;
  hr = writeFactory_->GetSystemFontCollection(&collection);
    TextFragmentRenderer textFragmentRenderer(collection, f, v, this,
                                              width, nextWidth, maxWidth, wordWrap);
  IDWriteTextLayout *layout = NULL;
  hr = writeFactory_->CreateTextLayout(
    s.c_str(),
    s.size(),
    match.textFormat(),
    maxWidth == -1.0 ?
    std::numeric_limits<FLOAT>::infinity() :
    static_cast<FLOAT>(maxWidth),
    std::numeric_limits<FLOAT>::infinity(),
    &layout);
  hr = layout->SetWordWrapping(WORD_WRAPPING_WHOLE_WORD);
  bool wholeWordWrapping = SUCCEEDED(hr);
  if (wholeWordWrapping)
    hr = layout->Draw(NULL, &textFragmentRenderer, 0.0f, 0.0f);
  SafeRelease(layout);
  if (!wholeWordWrapping) {
    std::size_t start = 0;
    TextFragmentRenderer textFragmentRenderer(collection, f, v, this,
                                              width, nextWidth, maxWidth, wordWrap);
    for (std::size_t pos = 0; pos < s.size(); ++pos) {
      if (pos == s.size() - 1 || isWhitespace(s[pos+1])) {
        hr = writeFactory_->CreateTextLayout(
          s.c_str() + start,
          pos + 1 - start,
          match.textFormat(),
          std::numeric_limits<FLOAT>::infinity(),
          std::numeric_limits<FLOAT>::infinity(),
          &layout);
        hr = layout->Draw(NULL, &textFragmentRenderer, 0.0f, 0.0f);
        SafeRelease(layout);
        if (textFragmentRenderer.maxWidthReached())
          break;
        start = pos + 1;
        textFragmentRenderer.setOffset(start);
      }
    }
  }
  SafeRelease(collection);
}

void FontSupport::drawText(const WFont &f,
                           const WRectF &rect,
                           WFlags<AlignmentFlag> alignmentFlags,
                           const WString &text)
{
  font_ = &f;
  if (dynamic_cast<WRasterImage*>(device_)) {
    device_->drawText(rect, alignmentFlags, TextFlag::WordWrap, text, 0);
  } else {
    std::vector<TextFragment> fragments;
    std::wstring wtext = text;
    double totalWidth = 0;
    double nextWidth = -1;
    layoutText(f, fragments, wtext, totalWidth, nextWidth, rect.width(), true);
    double x = 0;
    AlignmentFlag hAlign = alignmentFlags & AlignHorizontalMask;
    AlignmentFlag vAlign = alignmentFlags & AlignVerticalMask;
    switch (hAlign) {
    case AlignmentFlag::Left:
      x = rect.left();
      break;
    case AlignmentFlag::Right:
      x = rect.right() - totalWidth;
      break;
    case AlignmentFlag::Center:
      x = rect.center().x() - totalWidth / 2;
      break;
    default:
      x = 0;
    }
    for (std::size_t i = 0; i < fragments.size(); ++i) {
      drawingFontPath_ = fragments[i].fontPath;
      device_->drawText(WRectF(x, rect.y(), 1000, rect.height()), WFlags<AlignmentFlag>(AlignmentFlag::Left) | vAlign, TextFlag::SingleLine, Wt::WString(wtext.substr(fragments[i].start, fragments[i].length)), 0);
      x += fragments[i].width;
    }
    drawingFontPath_.clear();
  }
  font_ = 0;
}

void FontSupport::drawText(const WFont& f,
		           const WRectF& rect,
		           const WTransform& transform,
		           Bitmap& bitmap,
		           WFlags<AlignmentFlag> alignmentFlags, const WString& text)
{ }

bool FontSupport::busy() const
{
  return font_;
}

std::string FontSupport::drawingFontPath() const
{
  return drawingFontPath_;
}

bool FontSupport::canRender() const
{
  return true;
}

void FontSupport::addFontCollection(const std::string &directory, bool recursive)
{ }

}
