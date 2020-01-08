// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FORM_H_
#define FORM_H_

#include <Wt/WTable.h>

using namespace Wt;

namespace Wt {
  class WContainerWidget;
  class WText;
  class WTextArea;
  class WLineEdit;
  class WComboBox;
  class WFormWidget;
  class WDateEdit;
}

/**
 * @addtogroup formexample
 */
/*@{*/

/*!\brief A simple Form.
 *
 * Shows how a simple form can made, with an emphasis on how
 * to handle validation.
 */
class Form : public WTable
{
public:
  /*!\brief Instantiate a new form.
   */
  Form();

private:
  /*!\brief The user selected a new country: adjust the cities combo box.
   */
  void countryChanged();

  /*!\brief Submit the form.
   */
  void submit(); 

  void createUI();
 
  WContainerWidget *feedbackMessages_;

  WLineEdit *nameEdit_;
  WLineEdit *firstNameEdit_;

  WComboBox *countryEdit_;
  WComboBox *cityEdit_;

  WDateEdit *birthDateEdit_;
  WLineEdit *childCountEdit_;
  WLineEdit *weightEdit_;

  WTextArea *remarksEdit_;

  /*!\brief Add a validation feedback for a field
   */
  void addValidationStatus(int row, WFormWidget *field);

  /*!\brief Validate the form, and return whether succesfull.
   */
  bool validate();

  /*!\brief Validate a single form field.
   *
   * Checks the given field, and appends the given text to the error
   * messages on problems.
   */
  bool checkValid(WFormWidget *edit, const WString& text);
};

/*@}*/

#endif // FORM_H_
