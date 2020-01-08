// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRADIOBUTTON_H_
#define WRADIOBUTTON_H_

#include <Wt/WAbstractToggleButton.h>
#include <Wt/WButtonGroup.h>

namespace Wt {

class WButtonGroup;

/*! \class WRadioButton Wt/WRadioButton.h Wt/WRadioButton.h
 *  \brief A user control that represents a radio button.
 *
 * Use a WButtonGroup to group together radio buttons that reflect
 * options that are mutually exclusive.
 *
 * Usage example:
 * \if cpp
 * \code
 * enum class Vote { Republican = 1, Democrat = 2, NoVote = 10 };
 *
 * // use a group box as widget container for 3 radio buttons, with a title
 * auto container = std::make_unique<Wt::WGroupBox>("USA elections vote");
 *
 * // use a button group to logically group the 3 options
 * auto group = std::make_shared<Wt::WButtonGroup>();
 *
 * Wt::WRadioButton *button;
 * button = container->addWidget(
 *            std::make_unique<Wt::WRadioButton>(
 *              "I voted Republican"));
 * container->addWidget(std::make_unique<Wt::WBreak>());
 * group->addButton(button, Vote::Republican);
 *
 * button = container->addWidget(
 *            std::make_unique<Wt::WRadioButton>(
 *              "I voted Democrat"));
 * container->addWidget(std::make_unique<Wt::WBreak>());
 * group->addButton(button, Vote::Democrat);
 *
 * button = container->addWidget(
 *            std::make_unique<Wt::WRadioButton>(
 *              "I didn't vote"));
 * container->addWidget(std::make_unique<Wt::WBreak>());
 * group->addButton(button, Vote::NoVote);
 *
 * group->setCheckedButton(group->button(Vote::NoVote));
 * \endcode
 * \elseif java
 * \code
 * enum Vote { Republican, Democrate, NoVote };
 *
 * // use a group box as widget container for 3 radio buttons, with a title
 * WGroupBox container = new WGroupBox("USA elections vote");
		 
 * // use a button group to logically group the 3 options
 * WButtonGroup group = new WButtonGroup(this);
		 
 * WRadioButton button;
 * button = new WRadioButton("I voted Republican", container);
 * new WBreak(container);
 * group.addButton(button, Vote.Republican.ordinal());
 * button = new WRadioButton("I voted Democrat", container);
 * new WBreak(container);
 * group.addButton(button, Vote.Democrate.ordinal());

 * button = new WRadioButton("I didn't vote", container);
 * new WBreak(container);
 * group.addButton(button, Vote.NoVote.ordinal());
		 
 * group.setCheckedButton(group.button(Vote.NoVote.ordinal()));	
 * \endcode
 * \endif
 *
 * %WRadioButton is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * This widget corresponds to the HTML <tt>&lt;input
 * type="radio"&gt;</tt> tag.  When a label is specified, the input
 * element is nested in a <tt>&lt;label&gt;</tt>.
 *
 * This widget does not provide styling, and can be styled using
 * inline or external CSS as appropriate.
 *
 * \sa WAbstractToggleButton, WButtonGroup
 */
class WT_API WRadioButton : public WAbstractToggleButton
{
public:
  /*! \brief Creates an unchecked radio button with empty label and optional
   *         parent.
   */
  WRadioButton();

  /*! \brief Creates an unchecked radio button with given text and optional
   *         parent.
   */
  WRadioButton(const WString& text);

  /*! \brief Destructor.
   */
  ~WRadioButton();

  /*! \brief Returns the button group.
   *
   * Returns the button group to which this button belongs.
   *
   * \sa WButtonGroup::addButton(WRadioButton *, int)
   */
  WButtonGroup *group() const { return buttonGroup_.get(); }

private:
  std::shared_ptr<WButtonGroup> buttonGroup_;

  void setGroup(std::shared_ptr<WButtonGroup> buttonGroup);
  friend class WButtonGroup;

protected:
  virtual void updateInput(DomElement& input, bool all) override;
  virtual void getFormObjects(FormObjectsMap& formObjects) override;

  virtual void setFormData(const FormData& formData) override;
};

}

#endif // WRADIOBUTTON_H_
