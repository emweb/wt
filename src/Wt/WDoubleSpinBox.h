// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010, 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDOUBLE_SPIN_BOX_H_
#define WDOUBLE_SPIN_BOX_H_

#include <Wt/WAbstractSpinBox.h>

namespace Wt {

/*! \class WDoubleSpinBox Wt/WDoubleSpinBox.h Wt/WDoubleSpinBox.h
 *  \brief An input control for fixed point numbers.
 *
 * The spin box provides a control for entering a fixed point
 * number. It consists of a line edit, and buttons which allow to
 * increase or decrease the value. If you rather need input of an
 * integer number number, use WSpinBox instead.
 *
 * %WDoubleSpinBox is an \link WWidget::setInline(bool) inline
 * \endlink widget.
 *
 * \sa WSpinBox
 *
 * \note A spinbox configures a validator for validating the input. Therefore
 *       you cannot set a validator yourself.
 */
class WT_API WDoubleSpinBox : public WAbstractSpinBox
{
public:
  /*! \brief Creates a spin-box.
   *
   * The range is (0.0 - 99.99), the step size 1.0, and the spin box
   * has a precision of 2 decimals.
   *
   * The initial value is 0.0.
   */
  WDoubleSpinBox();

  /*! \brief Sets the minimum value.
   *
   * The default value is 0.0.
   */
  void setMinimum(double minimum);

  /*! \brief Returns the minimum value.
   *
   * \sa setMinimum()
   */
  double minimum() const { return min_; }

  /*! \brief Sets the maximum value.
   *
   * The default value is 99.99.
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

  /*! \brief Sets the step value.
   *
   * The default value is 1.0.
   */
  void setSingleStep(double step);

  /*! \brief Returns the step value.
   *
   * \sa setSingleStep()
   */
  double singleStep() const { return step_; }

  /*! \brief Sets the precision.
   *
   * This sets the number of digits after the decimal point shown
   *
   * The default precision is 2.
   */
  void setDecimals(int precision);

  /*! \brief Returns the precision.
   *
   * \sa setDecimals()
   */
  int decimals() const override;

  /*! \brief Sets the value.
   *
   * \p value must be a value between minimum() and maximum().
   *
   * The default value is 0
   */
  void setValue(double value);

  /*! \brief Returns the value.
   *
   * \note This value may not correctly reflect the valueText()
   *       of the spin box if valueText() is empty or if the
   *       contents are not in a \link WFormWidget::validate() valid state\endlink.
   */
  double value() const { return value_; }

  /*! \brief A %signal that indicates when the value has changed.
   *
   * This signal is emitted when changed() is emitted, but supplies the
   * new value as an argument. The changed() signal is emitted
   * when the user changes the value of the spinbox
   * by pressing the up/down arrow, or entering a different value and pressing
   * enter or moving focus.
   *
   * \sa changed()
   */
  Signal<double>& valueChanged() { return valueChanged_; }

  virtual void refresh() override;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void signalConnectionsChanged() override;

  virtual std::string jsMinMaxStep() const override;
  virtual bool parseNumberValue(const std::string& text) override;
  virtual WT_USTRING textFromValue() const override;
  virtual std::unique_ptr<WValidator> createValidator() override;
  virtual WValidator::Result validateRange() const override;

private:
  bool setup_;
  double value_, min_, max_, step_;
  int precision_;

  Signal<double> valueChanged_;

  void setup();
  void onChange();
};

}

#endif // WDOUBLE_SPIN_BOX_H_
