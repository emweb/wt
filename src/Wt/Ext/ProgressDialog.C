/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/ProgressDialog"

namespace Wt {
  namespace Ext {

ProgressDialog::ProgressDialog(bool i18n)
  : MessageBox("Progress...", "", NoIcon, Cancel, i18n),
    canceled_(this),
    cancelButtonText_("Cancel"),
    minimum_(0),
    maximum_(100),
    value_(minimum_),
    wasCanceled_(false)
{ 
  enableProgressBar(true);
  buttonClicked().connect(this, &ProgressDialog::onButtonClick);

  //show();
}

ProgressDialog::ProgressDialog(const WString& labelText,
			       const WString& cancelButtonText,
			       int minimum, int maximum, bool i18n)
  : MessageBox("Progress...", labelText, NoIcon, Cancel, i18n),
    canceled_(this),
    cancelButtonText_(cancelButtonText),
    minimum_(minimum),
    maximum_(maximum),
    value_(minimum),
    wasCanceled_(false)
{
  enableProgressBar(true);
  buttonClicked().connect(this, &ProgressDialog::onButtonClick);

  //show();
}

void ProgressDialog::setMinimum(int minimum)
{
  minimum_ = minimum;
  if (value_ < minimum_)
    reset();
}

void ProgressDialog::setMaximum(int maximum)
{
  maximum_ = maximum;
  if (value_ > maximum_)
    reset();
}

void ProgressDialog::setRange(int minimum, int maximum)
{
  minimum_ = minimum;
  maximum_ = maximum;

  if (value_ < minimum_ || value_ > maximum_)
    reset();
}

void ProgressDialog::cancel()
{
  wasCanceled_ = true;
  hide();
}

void ProgressDialog::reset()
{
  setValue(minimum_);
  wasCanceled_ = false;
}

void ProgressDialog::setValue(int progress)
{
  value_ = std::max(std::min(progress, maximum_), minimum_);

  if (value_ == maximum_) {
    hide();
    value_ = minimum_;
    wasCanceled_ = false;
  } else
    updateProgress(static_cast<double>(value_ - minimum_)
		   /(maximum_ - minimum_));
}

void ProgressDialog::setCancelButtonText(const WString& text)
{
  cancelButtonText_ = text;
}

void ProgressDialog::onButtonClick(StandardButton button)
{
  wasCanceled_ = true;
  canceled_.emit();
}

std::string ProgressDialog::buttonText(int buttonIndex) const
{
  if (buttonIndex == 3 && cancelButtonText_ != WString::Empty)
    return cancelButtonText_.toUTF8();
  else
    return MessageBox::buttonText(buttonIndex);
}

  }
}
