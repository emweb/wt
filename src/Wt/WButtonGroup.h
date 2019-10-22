// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBUTTONGROUP_H_
#define WBUTTONGROUP_H_

#include <Wt/WObject.h>
#include <Wt/WSignal.h>

namespace Wt {

class WRadioButton;

/*! \class WButtonGroup Wt/WButtonGroup.h Wt/WButtonGroup.h
 *  \brief A class for grouping radio buttons logically together.
 *
 * A button group manages a set of \link WRadioButton radio
 * buttons\endlink, making them exclusive of each other.
 *
 * It is not a widget, but instead provides only a logical
 * grouping. Radio buttons are aware of the group in which they have
 * been added, see WRadioButton::group(). When a button is deleted, it
 * is automatically removed its button group.
 *
 * It allows you to associate id's with each button, which you may use
 * to identify a particular button. The special value of -1 is
 * reserved to indicate <i>no button</i>.
 *
 * \if cpp
 * Usage example:
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
 * \endif
 *
 * \sa WRadioButton
 */
class WT_API WButtonGroup : public WObject
#ifndef WT_TARGET_JAVA
                            , public std::enable_shared_from_this<WButtonGroup>
#endif
{
public:
  /*! \brief Creates a new empty button group.
   *
   * \note The \link WRadioButton WRadioButtons\endlink associated with this
   *       WButtonGroup keep a shared_ptr to this WButtonGroup. Therefore, you
   *       should store a WButtonGroup in a shared_ptr (e.g. construct it with
   * 	   make_shared) before adding any radio buttons to it.
   */
  WButtonGroup();

  virtual ~WButtonGroup();

  /*! \brief Adds a radio button to the group.
   *
   * You can assign an id to the button. If \p id is -1, then a
   * unique id will be generated.
   *
   * \note WButtonGroup should be owned by a shared_ptr before addButton is
   *       called on it!
   *
   * \sa removeButton(WRadioButton *)
   */
  void addButton(WRadioButton *button, int id = -1);

  /*! \brief Removes a radio button from the group.
   *
   * \sa addButton(WRadioButton *, int)
   */
  void removeButton(WRadioButton *button);

  /*! \brief Returns the button for the given id.
   *
   * Returns \c nullptr if no button exists for the given id.
   *
   * \sa id(), addButton()
   */
  WRadioButton *button(int id) const;

  /*! \brief Returns the id for the given button.
   *
   * \sa button(), addButton()
   */
  int id(WRadioButton *button) const;

  virtual const std::string id() const override { return WObject::id(); }

  /*! \brief Returns the buttons in this group.
   */
  std::vector<WRadioButton *> buttons() const;

  /*! \brief Returns the number of radiobuttons in this group.
   */
  int count() const;

  /*! \brief Returns the id of the checked button.
   *
   * Returns the id of the currently checked button, or -1 if no button
   * is currently checked.
   */
  int checkedId() const;

  /*! \brief Sets the currently checked radiobutton.
   *
   * The button \p button of this group is checked. A value of \c 0
   * will uncheck all radiobuttons.
   *
   * Initially, no button is checked.
   *
   * \sa checkedId()
   */
  void setCheckedButton(WRadioButton *button);

  /*! \brief Returns the checked radiobutton.
   *
   * If there is no radiobutton currently checked this function
   * returns \c nullptr.
   *
   * \sa setCheckedButton(), selectedButtonIndex()
   */
  WRadioButton* checkedButton() const;

  /*! \brief Sets the currently checked radiobutton.
   *
   * Sets the \p idx'th radiobutton checked. A value of -1 will
   * uncheck all radiobuttons.
   *
   * Initially, no button is checked.
   */
  void setSelectedButtonIndex(int idx);

  /*! \brief Returns the index of the checked radiobutton.
   *
   * The index reflects the order in which the buttons have been added
   * to the button group. Use checkedId() if you want to know the id
   * of the button that is currently checked. If there is no
   * radiobutton selected this function returns -1.
   *
   * \sa checkedId()
   */
  int selectedButtonIndex() const;

  /*! \brief %Signal emitted when a button was checked.
   *
   * The argument passed is the new checkedButton().
   */
  Signal<WRadioButton *>& checkedChanged();

private:
  struct Button {
    WRadioButton *button;
    int           id;
  };

  std::vector<Button> buttons_;
  Signal<WRadioButton *> checkedChanged_;
  bool                   checkedChangedConnected_;

  void uncheckOthers(WRadioButton *button);
  int  generateId() const;
  void onButtonChange();
  virtual void setFormData(const FormData& formData) override;

  friend class WRadioButton;
};

}

#endif // WBUTTONGROUP_H_
