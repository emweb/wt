// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_SPIN_BOX_H_
#define WABSTRACT_SPIN_BOX_H_

#include <Wt/WLineEdit.h>

namespace Wt {

/*! \class WAbstractSpinBox Wt/WAbstractSpinBox.h Wt/WAbstractSpinBox.h
 *  \brief An abstract spin box.
 *
 * Although the element can be rendered using a native HTML5 control,
 * by default it is rendered using an HTML4 compatibility workaround
 * which is implemented using JavaScript and CSS, as most browsers do
 * not yet implement the HTML5 native element.
 */
class WT_API WAbstractSpinBox : public WLineEdit
{
public:
  /*! \brief Configures whether a native HTML5 control should be used.
   *
   * When \p native, the new "number" input element, specified by
   * HTML5 and when implemented by the browser, is used rather than
   * the built-in element. The native control is styled by the browser
   * (usually in sync with the OS) rather than through the theme
   * chosen.
   *
   * The default is \p false (as native support is now well implemented).
   */
  void setNativeControl(bool nativeControl);

  /*! \brief Returns whether a native HTML5 control is used.
   *
   * Taking into account the preference for a native control,
   * configured using setNativeControl(), this method returns whether
   * a native control is actually being used.
   */
  bool nativeControl() const;

  /*! \brief Sets a prefix.
   *
   * Option to set a prefix string shown in front of the value, e.g.:
   *
   * \code
   *   spinBox->setPrefix("$ ");
   * \endcode
   *
   * The default prefix is empty.
   *
   * \note Not supported by the native controls.
   */
  void setPrefix(const WString& prefix);

  /*! \brief Returns the prefix.
   *
   * \sa setPrefix()
   */
  const WString& prefix() const { return prefix_; }

  /*! \brief Sets a suffix.
   *
   * Option to set a suffix string shown to the right of the value, e.g.:
   *
   * \code
   *   spinBox->setSuffix(" crates");
   * \endcode
   *
   * The default suffix is empty.
   *
   * \note Not supported by the native controls.
   */
  void setSuffix(const WString& suffix);

  /*! \brief Returns the suffix.
   *
   * \sa setSuffix()
   */
  const WString& suffix() const { return suffix_; }

  virtual void setText(const WT_USTRING& text) override;

  virtual ValidationState validate() override;

  virtual void refresh() override;

  JSignal<int,int> &jsValueChanged() { return jsValueChanged_; }

protected:
  /*! \brief Constructor.
   */
  WAbstractSpinBox();

  virtual void updateDom(DomElement& element, bool all) override;
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void setFormData(const FormData& formData) override;
  virtual void propagateRenderOk(bool deep) override;

  virtual std::string jsMinMaxStep() const = 0;
  virtual int decimals() const = 0;
  virtual bool parseNumberValue(const std::string& text) = 0;
  virtual WT_USTRING textFromValue() const = 0;
  virtual std::unique_ptr<WValidator> createValidator() = 0; // for nativeControl
  virtual WValidator::Result validateRange() const = 0;

  virtual int boxPadding(Orientation orientation) const override;

  bool changed_;
  bool valueChangedConnection_;

private:
  bool preferNative_, setup_;
  WString prefix_, suffix_;

  void defineJavaScript();
  void connectJavaScript(Wt::EventSignalBase& s,
			 const std::string& methodName);
  void setup();
  bool parseValue(const WT_USTRING& text);

  JSignal<int,int> jsValueChanged_;

  friend class SpinBoxValidator;
};

}

#endif // WABSTRACT_SPIN_BOX_H_
