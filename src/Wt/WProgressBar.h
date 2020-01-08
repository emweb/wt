// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Thomas Suckow.
 * Copyright (C) 2010 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPROGRESSBAR_H_
#define WPROGRESSBAR_H_

#include <Wt/WInteractWidget.h>

namespace Wt {

/*! \brief A progress bar.
 *
 * The progress bar can be used to indicate the progress of a certain
 * operation. The text displayed in the progress bar can be customized
 * by specializing text().
 *
 * To use the progress bar, you need to give it a range (minimum and maximum
 * value), and update the progress using setValue().
 *
 * %WProgressBar is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * \note With the advent of HTML5, this widget will be implemented using
 *       the native HTML5 control when available.
 */
class WT_API WProgressBar : public WInteractWidget
{
public:
  /*! \brief Creates a progress bar.
   */
  WProgressBar();

  /*! \brief Sets the minimum value.
   *
   * The minimum value is the value that corresponds to 0%.
   *
   * The default value is 0.
   */
  void setMinimum(double minimum);

  /*! \brief Returns the minimum value.
   *
   * \sa setMinimum()
   */
  double minimum() const { return min_; }

  /*! \brief Sets the maximum value.
   *
   * The maximum value is the value that corresponds to 100%.
   *
   * The default value is 100.
   */
  void setMaximum(double maximum);

  /*! \brief Returns the maximum value.
   *
   * \sa setMaximum()
   */
  double maximum() const { return max_; }

  /*! \brief Sets the range.
   *
   * \sa setMinimum(), setMaximum()
   */
  void setRange(double minimum, double maximum);

  /*! \brief Sets the current progress.
   *
   * \p value must be a value between minimum() and maximum().
   */
  void setValue(double value);

  /*! \brief Returns the current progress.
   */
  double value() const { return value_; }

  /*! \brief Sets the progress format string.
   *
   * The format is used by text() to indicate the progress value.
   *
   * The default value is "%.0f %%"
   */
  void setFormat(const WString& format);

  /*! \brief Returns the progress format string.
   *  
   * \sa setFormat() 
   */
  const WString& format() const { return format_; }

  /*! \brief Returns the text displayed inside the progress bar.
   *
   * This text must be an XHTML formatted text fragment. The default
   * text prints the current progress using format(). You may want to
   * reimplement this method to display a different text corresponding
   * to the current value().
   */
  virtual WString text() const;

  /*! \brief A %signal that indicates when the value has changed.
   *
   * This signal is emitted when setValue() is called.
   *
   * \sa setValue()
   */
  Signal<double>& valueChanged() { return valueChanged_; }

  /*! \brief A %signal that indicates when 100% is reached.
   *
   * This signal is emitted when setValue(maximum()) is called.
   *
   * \sa setValue()
   */
  Signal<>& progressCompleted() { return progressCompleted_; }

  virtual void resize(const WLength& width, const WLength& height) override;

  void setValueStyleClass(const std::string& valueClass);
  void setState(double minimum, double maximum, double value);

protected:
  /*! \brief Update the progress bar itself.
   *
   * Will be called whenever the value changes, and changes
   * the width of the progress bar accordingly.
   *
   * You can reimplement this method to apply certain
   * style changes to the progress bar according to the
   * value. Don't forget to call WProgressBar::updateBar
   * if you still want the width to change.
   */
  virtual void updateBar(DomElement& bar);

  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void           propagateRenderOk(bool deep) override;

private:
  double min_, max_, value_;
  WString format_;
  bool changed_;
  std::string valueStyleClass_;

  void onChange();

  Signal<double> valueChanged_;
  Signal<> progressCompleted_;
  
  double percentage() const;
};

}

#endif // WPROGRESSBAR_H_
