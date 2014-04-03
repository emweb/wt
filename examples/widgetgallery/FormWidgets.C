/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "FormWidgets.h"
#include "TopicTemplate.h"
#include "DeferredWidget.h"

FormWidgets::FormWidgets()
  : TopicWidget()
{
  addText(tr("formwidgets-intro"), this);
}

void FormWidgets::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("Introduction", introduction())->setPathComponent("");
  menu->addItem("Line/Text editor",
		deferCreate(boost::bind
			    (&FormWidgets::textEditors, this)));
  menu->addItem("Check boxes", 
		deferCreate(boost::bind
			    (&FormWidgets::checkBox, this)));
  menu->addItem("Radio buttons", 
		deferCreate(boost::bind
			    (&FormWidgets::radioButton, this)));
  menu->addItem("Combo box", 
		deferCreate(boost::bind
			    (&FormWidgets::comboBox, this)));
  menu->addItem("Selection box", 
		deferCreate(boost::bind
			    (&FormWidgets::selectionBox, this)));
  menu->addItem("Autocomplete", 
		deferCreate(boost::bind
			    (&FormWidgets::autoComplete, this)));
  menu->addItem("Date entry", 
		deferCreate(boost::bind
			    (&FormWidgets::dateEntry, this)));
  menu->addItem("In-place edit", 
		deferCreate(boost::bind
			    (&FormWidgets::inPlaceEdit, this)));
  menu->addItem("Slider", 
		deferCreate(boost::bind
			    (&FormWidgets::slider, this)));
  menu->addItem("Progress bar", 
		deferCreate(boost::bind
			    (&FormWidgets::progressBar, this)));
  menu->addItem("File upload", 
		deferCreate(boost::bind
			    (&FormWidgets::fileUpload, this)));
  menu->addItem("Push button", 
		deferCreate(boost::bind
			    (&FormWidgets::pushButton, this)));
  menu->addItem("Validation", 
		deferCreate(boost::bind
			    (&FormWidgets::validation, this)));
  menu->addItem("Integration example", 
		deferCreate(boost::bind
			    (&FormWidgets::example, this)));
}


#include "examples/SimpleForm.cpp"
#include "examples/FormModel.cpp"

Wt::WWidget *FormWidgets::introduction()
{
  Wt::WTemplate *result = new TopicTemplate("forms-introduction");
  result->bindWidget("SimpleForm", SimpleForm());
  result->bindWidget("FormModel", FormModel());

  // Show the XML-templates as text
  result->bindString("simpleForm-template",
                     reindent(tr("simpleForm-template")), Wt::PlainText);
  result->bindString("form-field",
                     reindent(tr("form-field")), Wt::PlainText);
  result->bindString("userForm-template",
                     reindent(tr("userForm-template")), Wt::PlainText);

  return result;
}


#include "examples/LineEdit.cpp"
#include "examples/InputMask.cpp"
#include "examples/LineEditEvent.cpp"
#include "examples/TextArea.cpp"
#include "examples/TextEdit.cpp"
#include "examples/SpinBox.cpp"
#include "examples/TextSide.cpp"

Wt::WWidget *FormWidgets::textEditors()
{
  Wt::WTemplate *result = new TopicTemplate("forms-textEditors");
  result->bindWidget("LineEdit", LineEdit());
  result->bindWidget("LineEditEvent", LineEditEvent());
  result->bindWidget("TextArea", TextArea());
  result->bindWidget("TextEdit", TextEdit());
  result->bindWidget("SpinBox", SpinBox());
  result->bindWidget("TextSide", TextSide());
  result->bindWidget("InputMask", InputMask());

  // Show the XML-template as text
  result->bindString("lineEdit-template", reindent(tr("lineEdit-template")),
                     Wt::PlainText);
  result->bindString("editSide-template", reindent(tr("editSide-template")),
                     Wt::PlainText);
  return result;
}


#include "examples/CheckBoxInline.cpp"
#include "examples/CheckBoxStack.cpp"

Wt::WWidget *FormWidgets::checkBox()
{
  Wt::WTemplate *result = new TopicTemplate("forms-checkBox");
  result->bindWidget("CheckBoxInline", CheckBoxInline());
  result->bindWidget("CheckBoxStack", CheckBoxStack());

  return result;
}


#include "examples/RadioButtonsLoose.cpp"
#include "examples/RadioButtonGroup.cpp"
#include "examples/RadioButtonStack.cpp"
#include "examples/RadioButtonsActivated.cpp"

Wt::WWidget *FormWidgets::radioButton()
{
  Wt::WTemplate *result = new TopicTemplate("forms-radioButton");
  result->bindWidget("RadioButtonsLoose", RadioButtonsLoose());
  result->bindWidget("RadioButtonGroup", RadioButtonGroup());
  result->bindWidget("RadioButtonStack", RadioButtonStack());
  result->bindWidget("RadioButtonsActivated", RadioButtonsActivated());

  return result;
}


#include "examples/ComboBox.cpp"
#include "examples/ComboBoxActivated.cpp"
#include "examples/ComboBoxModel.cpp"

Wt::WWidget *FormWidgets::comboBox()
{
  Wt::WTemplate *result = new TopicTemplate("forms-comboBox");
  result->bindWidget("ComboBox", ComboBox());
  result->bindWidget("ComboBoxActivated", ComboBoxActivated());
  result->bindWidget("ComboBoxModel", ComboBoxModel());

  return result;
}


#include "examples/SelectionBoxSimple.cpp"
#include "examples/SelectionBoxExtended.cpp"

Wt::WWidget *FormWidgets::selectionBox()
{
  Wt::WTemplate *result = new TopicTemplate("forms-selectionBox");
  result->bindWidget("SelectionBoxSimple", SelectionBoxSimple());
  result->bindWidget("SelectionBoxExtended", SelectionBoxExtended());

  return result;
}


#include "examples/AutoComplete.cpp"

Wt::WWidget *FormWidgets::autoComplete()
{
  Wt::WTemplate *result = new TopicTemplate("forms-autoComplete");
  result->bindWidget("AutoComplete", AutoComplete());

  return result;
}


#include "examples/CalendarSimple.cpp"
#include "examples/CalendarExtended.cpp"
#include "examples/DateEdit.cpp"
#include "examples/DatePicker.cpp"

Wt::WWidget *FormWidgets::dateEntry()
{
  Wt::WTemplate *result = new TopicTemplate("forms-dateEntry");
  result->bindWidget("CalendarSimple", CalendarSimple());
  result->bindWidget("CalendarExtended", CalendarExtended());
  result->bindWidget("DateEdit", DateEdit());
  result->bindWidget("DatePicker", DatePicker());

  return result;
}


#include "examples/InPlaceEditButtons.cpp"
#include "examples/InPlaceEdit.cpp"

Wt::WWidget *FormWidgets::inPlaceEdit()
{
  Wt::WTemplate *result = new TopicTemplate("forms-inPlaceEdit");
  result->bindWidget("InPlaceEditButtons", InPlaceEditButtons());
  result->bindWidget("InPlaceEdit", InPlaceEdit());

  return result;
}


#include "examples/Slider.cpp"
#include "examples/SliderVertical.cpp"

Wt::WWidget *FormWidgets::slider()
{
  Wt::WTemplate *result = new TopicTemplate("forms-slider");
  result->bindWidget("Slider", Slider());
  result->bindWidget("SliderVertical", SliderVertical());

  return result;
}


#include "examples/ProgressBar.cpp"

Wt::WWidget *FormWidgets::progressBar()
{
  Wt::WTemplate *result = new TopicTemplate("forms-progressBar");
  result->bindWidget("ProgressBar", ProgressBar());

  return result;
}


#include "examples/FileUpload.cpp"

Wt::WWidget *FormWidgets::fileUpload()
{
  Wt::WTemplate *result = new TopicTemplate("forms-fileUpload");
  result->bindWidget("FileUpload", FileUpload());

  return result;
}


#include "examples/PushButton.cpp"
#include "examples/PushButtonOnce.cpp"
#include "examples/PushButtonLink.cpp"
#include "examples/PushButtonDropdownAppended.cpp"
#include "examples/PushButtonColor.cpp"
#include "examples/PushButtonSize.cpp"
#include "examples/PushButtonPrimary.cpp"
#include "examples/PushButtonAction.cpp"

Wt::WWidget *FormWidgets::pushButton()
{
  Wt::WTemplate *result = new TopicTemplate("forms-pushButton");
  result->bindWidget("PushButton", PushButton());
  result->bindWidget("PushButtonOnce", PushButtonOnce());
  result->bindWidget("PushButtonLink", PushButtonLink());
  result->bindWidget("PushButtonDropdownAppended",
                     PushButtonDropdownAppended());
  result->bindWidget("PushButtonColor", PushButtonColor());
  result->bindWidget("PushButtonSize", PushButtonSize());
  result->bindWidget("PushButtonPrimary", PushButtonPrimary());
  result->bindWidget("PushButtonAction", PushButtonAction());

  // Show the XML-templates as text
  result->bindString("appendedDropdownButton-template",
                     reindent(tr("appendedDropdownButton-template")),
                     Wt::PlainText);
  result->bindString("pushButtonColor-template",
                     reindent(tr("pushButtonColor-template")), Wt::PlainText);
  result->bindString("pushButtonSize-template",
                     reindent(tr("pushButtonSize-template")), Wt::PlainText);
  result->bindString("pushButtonAction-template",
                     reindent(tr("pushButtonAction-template")), Wt::PlainText);
  return result;
}


#include "examples/Validation.cpp"
#include "examples/ValidationDate.cpp"
#include "examples/ValidationModel.cpp"

Wt::WWidget *FormWidgets::validation()
{
  Wt::WTemplate *result = new TopicTemplate("forms-validation");
  result->bindWidget("Validation", Validation());
  result->bindWidget("ValidationDate", ValidationDate());
  result->bindWidget("ValidationModel", ValidationModel());

  // Show the XML-template as text
  result->bindString("validation-template", reindent(tr("validation-template")),
                     Wt::PlainText);
  return result;  
}


Wt::WWidget *FormWidgets::example()
{
  Wt::WTemplate *result = new TopicTemplate("forms-integration-example");
  result->bindWidget("FormModel", FormModel());

  // Show the XML-templates as text
  result->bindString("form-field",
                     reindent(tr("form-field")), Wt::PlainText);
  result->bindString("userForm-template",
                     reindent(tr("userForm-template")), Wt::PlainText);
  return result;
}
