// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCHECKBOX_H_
#define WCHECKBOX_H_

#include <Wt/WAbstractToggleButton.h>
#include <Wt/WJavaScriptSlot.h>

namespace Wt {

/*! \class WCheckBox Wt/WCheckBox.h Wt/WCheckBox.h
 *  \brief A user control that represents a check box.
 *
 * By default, a checkbox can have two states: Wt::CheckState::Checked or
 * Wt::CheckState::Unchecked, which can be inspected using isChecked(), and set
 * using setChecked().
 *
 * A checkbox may also provide a third state, Wt::CheckState::PartiallyChecked,
 * which is useful to indicate that it is neither checked nor
 * unchecked. %Wt will use native browser support for this HTML5
 * extension when available (Safari and MS IE), and use an image-based
 * workaround otherwise. You may enable support for the third state
 * using setTristate(), and use setCheckState() and checkState() to
 * read all three states.
 * Once a tri-state checkbox is clicked, it cycles through the states
 * Wt::CheckState::Checked and Wt::CheckState::Unchecked.
 *
 * A label is added as a sibling of the checkbox to the same parent.
 *
 * Usage example:
 * \if cpp
 * \code
 * auto box = std::make_unique<Wt::WGroupBox>("In-flight options");
 *
 * Wt::WCheckBox *w1 = box->addWidget(std::make_unique<Wt::WCheckBox>("Vegetarian diet"));
 * box->addWidget(std::make_unique<Wt::WBreak>());
 * Wt::WCheckBox *w2 = box->addWidget(std::make_unique<Wt::WCheckBox>("WIFI access"));
 * box->addWidget(std::make_unique<Wt::WBreak>());
 * Wt::WCheckBox *w3 = box->addWidget(std::make_unique<Wt::WCheckBox>("AC plug"));
 *
 * w1->setChecked(false);
 * w2->setChecked(true);
 * w3->setChecked(true);
 * \endcode
 * \elseif java
 * \code
 * WGroupBox box = new WGroupBox("In-flight options");
 *		 
 * WCheckBox w1 = new WCheckBox("Vegetarian diet", box);
 * box.addWidget(new WBreak());
 * WCheckBox w2 = new WCheckBox("WIFI access", box);
 * box.addWidget(new WBreak());
 * WCheckBox w3 = new WCheckBox("AC plug", box);
 *		 
 * w1.setChecked(false);
 * w2.setChecked(true);
 * w3.setChecked(true);
 * \endcode
 * \endif
 *
 * %WCheckBox is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * This widget is rendered using an HTML <tt>&lt;input
 * type="checkbox"&gt;</tt> tag. When a label is specified, the input
 * element is nested in a <tt>&lt;label&gt;</tt>.
 *
 * This widget does not provide styling, and can be styled using
 * inline or external CSS as appropriate.
 *
 * \sa WAbstractToggleButton
 */
class WT_API WCheckBox : public WAbstractToggleButton
{
public:
  /*! \brief Creates a checkbox without label.
   *
   * A checkbox created by this constructor will not contain a placeholder
   * for a label, and therefore it is not possible to assign a label to it
   * later through setText().
   */
  WCheckBox();

  /*! \brief Creates a checkbox with given label.
   */
  WCheckBox(const WString& text);

  /*! \brief Makes a tristate checkbox.
   *
   * \note You should enable tristate functionality right after construction
   *       and this cannot be modified later.
   */
  void setTristate(bool tristate = true);

  /*! \brief enable or disable cycling throught partial state 
   *
   * \sa isPartialStateSelectable()
   */
  void setPartialStateSelectable(bool b);

  /*! \brief return partial state cycling
   *
   * \sa setPartialStateSelectable();
   */
  bool isPartialStateSelectable() { return partialStateSelectable_; };

  /*! \brief Returns whether the checkbox is tristate.
   *
   * \sa setTristate()
   */
  bool isTristate() const { return triState_; }

  /*! \brief Sets the check state.
   *
   * Unless it is a tri-state checkbox, only Wt::CheckState::Checked and Wt::CheckState::Unchecked are
   * valid states.
   */
  void setCheckState(CheckState state);

  /*! \brief Returns the check state.
   *
   * \sa setCheckState(), isChecked()
   */
  CheckState checkState() const { return state_; }

protected:
  virtual void updateInput(DomElement& input, bool all) override;
  void updateJSlot();
  void updateNextState();

private:
  bool triState_;
  bool partialStateSelectable_;
  std::unique_ptr<JSlot> jslot_;
};

}

#endif // WCHECKBOX_H_
