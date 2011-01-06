/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPdfImage"
#include "Wt/Render/WPdfRenderer"

#include "WtException.h"

#include <hpdf.h>

namespace Wt {
  namespace Render {

WPdfRenderer::WPdfRenderer(HPDF_Doc pdf, HPDF_Page page)
  : pdf_(pdf),
    page_(page),
    dpi_(72)
{
  for (int i = 0; i < 4; ++i)
    margin_[i] = 0;

  //HPDF_SetCurrentEncoder(pdf_, "UTF-8");
}

WPdfRenderer::~WPdfRenderer()
{ }

void WPdfRenderer::setDpi(int dpi)
{
  dpi_ = dpi;
}

void WPdfRenderer::setMargin(double margin, WFlags<Side> sides)
{
  if (sides & Top)
    margin_[0] = margin;
  if (sides & Right)
    margin_[1] = margin;
  if (sides & Bottom)
    margin_[2] = margin;
  if (sides & Left)
    margin_[3] = margin;
}

HPDF_Page WPdfRenderer::createPage(int page)
{
  HPDF_Page result = HPDF_AddPage(pdf_);

  HPDF_Page_SetWidth(result, HPDF_Page_GetWidth(page_));
  HPDF_Page_SetHeight(result, HPDF_Page_GetHeight(page_));

  return result;
}

double WPdfRenderer::margin(Side side) const
{
  const double CmPerInch = 2.54;

  switch (side) {
  case Top:
    return margin_[0] / CmPerInch * dpi_;
  case Right:
    return margin_[1] / CmPerInch * dpi_;
  case Bottom:
    return margin_[2] / CmPerInch * dpi_;
  case Left:
    return margin_[3] / CmPerInch * dpi_;
  default:
    throw WtException("WPdfRenderer::margin(Side) with invalid side");
  }
}

double WPdfRenderer::pageWidth(int page) const
{
  return HPDF_Page_GetWidth(page_) * dpi_ / 72.0;
}
 
double WPdfRenderer::pageHeight(int page) const
{
  return HPDF_Page_GetHeight(page_) * dpi_ / 72.0;
}

WPaintDevice *WPdfRenderer::startPage(int page)
{
  if (page > 0)
    page_ = createPage(page);

  HPDF_Page_Concat (page_, 72.0f/dpi_, 0, 0, 72.0f/dpi_, 0, 0);

  return new WPdfImage(pdf_, page_, 0, 0, pageWidth(page), pageHeight(page));
}

void WPdfRenderer::endPage(WPaintDevice *device)
{
  delete device;

  HPDF_Page_Concat (page_, dpi_/72.0f, 0, 0, dpi_/72.0f, 0, 0);
}

  }
}
