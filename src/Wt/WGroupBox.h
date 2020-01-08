// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WGROUP_BOX_H_
#define WGROUP_BOX_H_

#include <Wt/WContainerWidget.h>

namespace Wt {

/*! \class WGroupBox Wt/WGroupBox.h Wt/WGroupBox.h
 *  \brief A widget which group widgets into a frame with a title.
 *
 * This is typically used in a form to group certain form elements
 * together.
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
 * enum Vote { Republican , Democrat , NoVote };
 * 
 * // use a group box as widget container for 3 radio buttons, with a title
 * WGroupBox container = new WGroupBox("USA elections vote");
 * 		 
 * // use a button group to logically group the 3 options
 * WButtonGroup group = new WButtonGroup(this);
 *		 
 * WRadioButton button;
 * button = new WRadioButton("I voted Republican", container);
 * new WBreak(container);
 * group.addButton(button, Vote.Republican.ordinal());
 *
 * button = new WRadioButton("I voted Democrat", container);
 * new WBreak(container);
 * group.addButton(button, Vote.Democrate.ordinal());
 *
 * button = new WRadioButton("I didn't vote", container);
 * new WBreak(container);
 * group.addButton(button, Vote.NoVote.ordinal());
 *		 
 * group.setCheckedButton(group.button(Vote.NoVote.ordinal()));
 * \endcode
 * \endif
 *
 * Like WContainerWidget, %WGroupBox is by default displayed as a
 * \link WWidget::setInline() block\endlink.
 *
 * \image html WGroupBox-1.png "WGroupBox example"
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to the HTML <tt>&lt;fieldset&gt;</tt> tag,
 * and the title in a nested <tt>&lt;legend&gt;</tt> tag. This widget
 * does not provide styling, and can be styled using inline or
 * external CSS as appropriate.
 */
class WT_API WGroupBox : public WContainerWidget
{
public:
  /*! \brief Creates a groupbox with empty title.
   */
  WGroupBox();
  
  /*! \brief Creates a groupbox with given title message.
   */
  WGroupBox(const WString& title);

  /*! \brief Returns the title.
   */
  const WString& title() const { return title_; }

  /*! \brief Sets the title.
   */
  void setTitle(const WString& title);

  virtual void refresh() override;

protected:
  virtual DomElementType domElementType() const override;

  virtual void updateDom(DomElement& element, bool all) override;
  virtual void propagateRenderOk(bool deep) override;
  virtual int firstChildIndex() const override;

private:
  WString title_;
  bool titleChanged_;

  void init();
};

}

#endif // WGROUP_BOX_H_
