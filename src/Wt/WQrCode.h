// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_QRCODE_H_
#define WT_QRCODE_H_

#include "Wt/WPaintedWidget.h"

#include "thirdparty/qrcodegen/QrCode.hpp"

namespace Wt {

class WT_API WQrCode : public WPaintedWidget
{
public:
  typedef qrcodegen::Ecc ErrorCorrectionLevel;

  WQrCode();

  WQrCode(const std::string& message, double squareSize_);

  WQrCode(const std::string& message, ErrorCorrectionLevel ecl = ErrorCorrectionLevel::LOW, double squareSize_ = 5.0);

  void setErrorCorrectionLevel(ErrorCorrectionLevel ecl);
  ErrorCorrectionLevel errorCorrectionLevel() const { return errCorrLvl_; }

  void setMessage(const std::string& message);
  std::string message() const { return msg_; }

  void setSquareSize(double size);
  double squareSize() const { return squareSize_; }

  bool error() const { return !code(); }

protected:
  void paintEvent(WPaintDevice* paintDevice) override;

private:
  ErrorCorrectionLevel errCorrLvl_;
  std::string msg_;
  std::unique_ptr<qrcodegen::QrCode> code_;
  double squareSize_;

  void init();
  const qrcodegen::QrCode* code() const { return code_.get(); }
  void generateCode();
  void updateSize();
};

}
#endif // WT_QRCODE_H_
