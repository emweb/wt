// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010, 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSPIN_BOX_H_
#define WSPIN_BOX_H_

#include <Wt/WAbstractSpinBox.h>

namespace Wt {

/*! \class WSpinBox Wt/WSpinBox.h Wt/WSpinBox.h
 *  \brief An input control for integer numbers.
 *
 * The spin box provides a control for entering an integer number. It
 * consists of a line edit, and buttons which allow to increase or
 * decrease the value. If you rather need input of a fractional
 * number, use WDoubleSpinBox instead.
 *
 * %WSpinBox is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * \sa WDoubleSpinBox
 *
 * \note A spinbox configures a validator for validating the input. Therefore
 *       you cannot set a validator yourself.
 */
class WT_API WSpinBox : public WAbstractSpinBox
{
public:
  /*! \brief Creates a spin-box.
   *
   * The range is (0 - 99) and the step size 1.
   *
   * The initial value is 0.
   */
  WSpinBox();

  /*! \brief Sets the minimum value.
   *
   * The default value is 0.
   */
  void setMinimum(int minimum);

  /*! \brief Returns the minimum value.
   *
   * \sa setMinimum()
   */
  int minimum() const { return min_; }

  /*! \brief Sets the maximum value.
   *
   * The default value is 99.
   */
  void setMaximum(int maximum);

  /*! \brief Returns the maximum value.
   *
   * \sa setMaximum()
   */
  int maximum() const { return max_; }

  /*! \brief Sets the range.
   *
   * \sa setMinimum(), setMaximum()
   */
  void setRange(int minimum, int maximum);

  /*! \brief Sets the step value.
   *
   * The default value is 1.
   */
  void setSingleStep(int step);

  /*! \brief Returns the step value.
   */
  int singleStep() const { return step_; }

  /*! \brief Sets the value.
   *
   * \p value must be a value between minimum() and maximum().
   *
   * The default value is 0
   */
  void setValue(int value);

  /*! \brief Returns the value.
   *
   * \note This value may not correctly reflect the valueText()
   *       of the spin box if valueText() is empty or if the
   *       contents are not in a \link WFormWidget::validate() valid state\endlink.
   */
  int value() const { return value_; }

  /*! \brief Sets if this spinbox wraps around to stay in the valid range.
   *
   * \note Not supported by the native controls.
   */
  void setWrapAroundEnabled(bool enabled);

  /*! \brief Returns if the spinbox wraps around
   *
   * \sa setWrapAroundEnabled()
   */
  bool wrapAroundEnabled() const;

  /*! \brief A %signal that indicates when the value has changed.
   *
   * This signal is emitted when setValue() is called.
   *
   * \sa setValue()
   */
  Signal<int>& valueChanged() { return valueChanged_; }


protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void signalConnectionsChanged() override;

  virtual std::string jsMinMaxStep() const override;
  virtual int decimals() const override;
  virtual bool parseNumberValue(const std::string& text) override;
  virtual WT_USTRING textFromValue() const override;
  virtual std::unique_ptr<WValidator> createValidator() override;
  virtual WValidator::Result validateRange() const override;

private:
  int value_, min_, max_, step_;
  bool wrapAroundEnabled_;

  Signal<int> valueChanged_;

  void onChange();
};

}

#endif // WSPIN_BOX_H_
