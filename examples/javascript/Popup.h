// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef POPUP_H_
#define POPUP_H_

#include <Wt/WObject.h>
#include <Wt/WString.h>
#include <Wt/WJavaScript.h>

using namespace Wt;

/**
 * @addtogroup javascript
 */
/*@{*/

/*! \brief A JavaScript based popup window, encapsulating the Javascript
 *         functions alert(), confirm(), and prompt().
 *
 * Use one of the create static methods to create a popup. This will not
 * display the popup, until either the show slot is triggered from an
 * event handler, or is executed using it's exec() method.
 *
 * When the user closes the popup, either the okPressed or cancelPressed
 * signal is emitted. For a prompt dialog, the value is passed as a parameter
 * to the okPressed signal.
 */
class Popup : public WObject
{
public:
  /*! \brief Popup type.
   */
  enum Type { Confirm, Alert, Prompt };

  /*! \brief Popup constructor.
   */
  Popup(Type t, const WString& message, const std::string defaultValue);

  /*! \brief Create a confirm dialog.
   */
  static std::unique_ptr<Popup> createConfirm(const WString& message);

  /*! \brief Create a prompt dialog with the given default value
   */
  static std::unique_ptr<Popup> createPrompt(const WString& message,
                             const std::string defaultValue);

  /*! \brief Create an alert dialog.
   */
  static std::unique_ptr<Popup> createAlert(const WString& message);

  /*! \brief Change the message
   */
  void setMessage(const WString& message);

  /*! \brief Change the default value for a prompt dialog.
   */
  void setDefaultValue(const std::string defaultValue);

  /*! \brief Get the current message.
   */
  const WString& message() const { return message_; }

  /*! \brief Get the default value for a prompt dialog.
   */
  const std::string& defaultValue() const { return defaultValue_; }

  /*! \brief Show the dialog.
   *
   * Use show.exec() to show the dialog, or connect the slot to an EventSignal
   * to directly show the dialog without a server round trip.
   */
  JSlot show;

  /*! \brief Signal emitted when ok pressed.
   */
  JSignal<std::string>& okPressed() { return okPressed_; }

  /*! \brief Signal emitted when cancel is pressed.
   */
  JSignal<>&        cancelPressed() { return cancelPressed_; }

private:
  JSignal<std::string>  okPressed_;
  JSignal<>             cancelPressed_;

  Type t_;
  WString message_;
  std::string defaultValue_;

  /*! \brief Update the javascript code.
   */
  void setJavaScript();
};

/*@}*/

#endif // POPUP_H_
