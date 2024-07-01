#pragma once

#include "thirdparty/qrcodegen/QrCode.hpp"

#include <Wt/WBrush.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>

class QrCodePainter : public Wt::WPaintedWidget
{
public:
  static constexpr qrcodegen::Ecc ErrorLevelCorrection = qrcodegen::Ecc::HIGH;
  static constexpr double SQUARE_SIZE = 5;

  QrCodePainter(const std::string& url);

protected:
  void paintEvent(Wt::WPaintDevice* paintDevice) final;

private:
  qrcodegen::QrCode code_;

  const qrcodegen::QrCode& code() { return code_; }
};

