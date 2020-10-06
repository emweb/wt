/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "FormWidgets.h"
#include "TopicTemplate.h"
#include "DeferredWidget.h"

#include <Wt/WMenu.h>
#include <Wt/WString.h>

FormWidgets::FormWidgets()
  : TopicWidget()
{
  addText(tr("formwidgets-intro"), this);
}

void FormWidgets::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("Introduction", introduction())->setPathComponent("");
  menu->addItem("Line/Text editor",
                deferCreate([this]{ return textEditors(); }));
  menu->addItem("Check boxes", 
                deferCreate([this]{ return checkBox(); }));
  menu->addItem("Radio buttons", 
                deferCreate([this]{ return radioButton(); }));
  menu->addItem("Combo box", 
                deferCreate([this]{ return comboBox(); }));
  menu->addItem("Selection box", 
                deferCreate([this]{ return selectionBox(); }));
  menu->addItem("Autocomplete", 
                deferCreate([this]{ return autoComplete(); }));
  menu->addItem("Date & Time entry",
                deferCreate([this]{ return dateEntry(); }));
  menu->addItem("In-place edit", 
                deferCreate([this]{ return inPlaceEdit(); }));
  menu->addItem("Slider", 
                deferCreate([this]{ return slider(); }));
  menu->addItem("Progress bar", 
                deferCreate([this]{ return progressBar(); }));
  menu->addItem("File upload", 
                deferCreate([this]{ return fileUpload(); }));
  menu->addItem("Push button", 
                deferCreate([this]{ return pushButton(); }));
  menu->addItem("Validation", 
                deferCreate([this]{ return validation(); }));
  menu->addItem("Integration example", 
                deferCreate([this]{ return example(); }));
}


#include "examples/SimpleForm.cpp"
#include "examples/FormModel.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::introduction()
{
  auto result = std::make_unique<TopicTemplate>("forms-introduction");
  result->bindWidget("SimpleForm", SimpleForm());
  result->bindWidget("FormModel", FormModel());

  // Show the XML-templates as text
  result->bindString("simpleForm-template",
		     reindent(tr("simpleForm-template")), Wt::TextFormat::Plain);
  result->bindString("form-field",
		     reindent(tr("form-field")), Wt::TextFormat::Plain);
  result->bindString("userForm-template",
		     reindent(tr("userForm-template")), Wt::TextFormat::Plain);

  return std::move(result);
}


#include "examples/LineEdit.cpp"
#include "examples/InputMask.cpp"
#include "examples/LineEditEvent.cpp"
#include "examples/TextArea.cpp"
#include "examples/TextEdit.cpp"
#include "examples/SpinBox.cpp"
#include "examples/TextSide.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::textEditors()
{
  auto result = std::make_unique<TopicTemplate>("forms-textEditors");
  result->bindWidget("LineEdit", LineEdit());
  result->bindWidget("LineEditEvent", LineEditEvent());
  result->bindWidget("TextArea", TextArea());
  result->bindWidget("TextEdit", TextEdit());
  result->bindWidget("SpinBox", SpinBox());
  result->bindWidget("TextSide", TextSide());
  result->bindWidget("InputMask", InputMask());

  // Show the XML-template as text
  result->bindString("lineEdit-template", reindent(tr("lineEdit-template")),
		     Wt::TextFormat::Plain);
  result->bindString("editSide-template", reindent(tr("editSide-template")),
		     Wt::TextFormat::Plain);
  return std::move(result);
}


#include "examples/CheckBoxInline.cpp"
#include "examples/CheckBoxStack.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::checkBox()
{
  auto result = std::make_unique<TopicTemplate>("forms-checkBox");
  result->bindWidget("CheckBoxInline", CheckBoxInline());
  result->bindWidget("CheckBoxStack", CheckBoxStack());

  return std::move(result);
}


#include "examples/RadioButtonsLoose.cpp"
#include "examples/RadioButtonGroup.cpp"
#include "examples/RadioButtonStack.cpp"
#include "examples/RadioButtonsActivated.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::radioButton()
{
  auto result = std::make_unique<TopicTemplate>("forms-radioButton");
  result->bindWidget("RadioButtonsLoose", RadioButtonsLoose());
  result->bindWidget("RadioButtonGroup", RadioButtonGroup());
  result->bindWidget("RadioButtonStack", RadioButtonStack());
  result->bindWidget("RadioButtonsActivated", RadioButtonsActivated());

  return std::move(result);
}


#include "examples/ComboBox.cpp"
#include "examples/ComboBoxActivated.cpp"
#include "examples/ComboBoxModel.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::comboBox()
{
  auto result = std::make_unique<TopicTemplate>("forms-comboBox");
  result->bindWidget("ComboBox", ComboBox());
  result->bindWidget("ComboBoxActivated", ComboBoxActivated());
  result->bindWidget("ComboBoxModel", ComboBoxModel());

  return std::move(result);
}


#include "examples/SelectionBoxSimple.cpp"
#include "examples/SelectionBoxExtended.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::selectionBox()
{
  auto result = std::make_unique<TopicTemplate>("forms-selectionBox");
  result->bindWidget("SelectionBoxSimple", SelectionBoxSimple());
  result->bindWidget("SelectionBoxExtended", SelectionBoxExtended());

  return std::move(result);
}


#include "examples/AutoComplete.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::autoComplete()
{
  auto result = std::make_unique<TopicTemplate>("forms-autoComplete");
  result->bindWidget("AutoComplete", AutoComplete());

  return std::move(result);
}


#include "examples/CalendarSimple.cpp"
#include "examples/CalendarExtended.cpp"
#include "examples/DateEdit.cpp"
#include "examples/TimeEdit.cpp"
#include "examples/DatePicker.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::dateEntry()
{
  auto result = std::make_unique<TopicTemplate>("forms-dateEntry");
  result->bindWidget("CalendarSimple", CalendarSimple());
  result->bindWidget("CalendarExtended", CalendarExtended());
  result->bindWidget("DateEdit", DateEdit());
  result->bindWidget("TimeEdit", TimeEdit());
  result->bindWidget("DatePicker", DatePicker());

  return std::move(result);
}


#include "examples/InPlaceEditButtons.cpp"
#include "examples/InPlaceEdit.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::inPlaceEdit()
{
  auto result = std::make_unique<TopicTemplate>("forms-inPlaceEdit");
  result->bindWidget("InPlaceEditButtons", InPlaceEditButtons());
  result->bindWidget("InPlaceEdit", InPlaceEdit());

  return std::move(result);
}


#include "examples/Slider.cpp"
#include "examples/SliderVertical.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::slider()
{
  auto result = std::make_unique<TopicTemplate>("forms-slider");
  result->bindWidget("Slider", Slider());
  result->bindWidget("SliderVertical", SliderVertical());

  return std::move(result);
}


#include "examples/ProgressBar.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::progressBar()
{
  auto result = std::make_unique<TopicTemplate>("forms-progressBar");
  result->bindWidget("ProgressBar", ProgressBar());

  return std::move(result);
}


#include "examples/FileUpload.cpp"
#include "examples/FileDrop.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::fileUpload()
{
  auto result = std::make_unique<TopicTemplate>("forms-fileUpload");
  result->bindWidget("FileUpload", FileUpload());
  result->bindWidget("FileDrop", FileDrop());

  return std::move(result);
}


#include "examples/PushButton.cpp"
#include "examples/PushButtonOnce.cpp"
#include "examples/PushButtonLink.cpp"
#include "examples/PushButtonDropdownAppended.cpp"
#include "examples/PushButtonColor.cpp"
#include "examples/PushButtonSize.cpp"
#include "examples/PushButtonPrimary.cpp"
#include "examples/PushButtonAction.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::pushButton()
{
  auto result = std::make_unique<TopicTemplate>("forms-pushButton");
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
		     Wt::TextFormat::Plain);
  result->bindString("pushButtonColor-template",
		     reindent(tr("pushButtonColor-template")), Wt::TextFormat::Plain);
  result->bindString("pushButtonSize-template",
		     reindent(tr("pushButtonSize-template")), Wt::TextFormat::Plain);
  result->bindString("pushButtonAction-template",
		     reindent(tr("pushButtonAction-template")), Wt::TextFormat::Plain);
  return std::move(result);
}


#include "examples/Validation.cpp"
#include "examples/ValidationDate.cpp"
#include "examples/ValidationModel.cpp"

std::unique_ptr<Wt::WWidget> FormWidgets::validation()
{
  auto result = std::make_unique<TopicTemplate>("forms-validation");
  result->bindWidget("Validation", Validation());
  result->bindWidget("ValidationDate", ValidationDate());
  result->bindWidget("ValidationModel", ValidationModel());

  // Show the XML-template as text
  result->bindString("validation-template", reindent(tr("validation-template")),
		     Wt::TextFormat::Plain);
  return std::move(result);
}


std::unique_ptr<Wt::WWidget> FormWidgets::example()
{
  auto result = std::make_unique<TopicTemplate>("forms-integration-example");
  result->bindWidget("FormModel", FormModel());

  // Show the XML-templates as text
  result->bindString("form-field",
		     reindent(tr("form-field")), Wt::TextFormat::Plain);
  result->bindString("userForm-template",
		     reindent(tr("userForm-template")), Wt::TextFormat::Plain);
  return std::move(result);
}
