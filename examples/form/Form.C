#include "Form.h"

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WDateEdit.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTableCell.h>
#include <Wt/WTextArea.h>
#include <Wt/WText.h>

Form::Form()
  : WTable()
{
  createUI();
}

void Form::createUI()
{
  WLabel *label;
  int row = 0;

  // Title
  elementAt(row, 0)->setColumnSpan(3);
  elementAt(row, 0)->setContentAlignment(AlignmentFlag::Top | AlignmentFlag::Center);
  elementAt(row, 0)->setPadding(10);
  WText *title = elementAt(row,0)->addWidget(std::make_unique<WText>(tr("example.form")));
  title->decorationStyle().font().setSize(FontSize::XLarge);

  // error messages
  ++row;
  elementAt(row, 0)->setColumnSpan(3);
  feedbackMessages_ = elementAt(row, 0);
  feedbackMessages_->setPadding(5);

  WCssDecorationStyle& errorStyle = feedbackMessages_->decorationStyle();
  errorStyle.setForegroundColor(WColor("red"));
  errorStyle.font().setSize(FontSize::Smaller);
  errorStyle.font().setWeight(FontWeight::Bold);
  errorStyle.font().setStyle(FontStyle::Italic);

  // Name
  ++row;
  nameEdit_ = elementAt(row,2)->addWidget(std::make_unique<WLineEdit>());
  label = elementAt(row,0)->addWidget(std::make_unique<WLabel>(tr("example.name")));
  label->setBuddy(nameEdit_);
  nameEdit_->setValidator(std::make_shared<WValidator>(true));
  nameEdit_->enterPressed().connect(this, &Form::submit);

  // First name
  ++row;
  firstNameEdit_ = elementAt(row,2)->addWidget(std::make_unique<WLineEdit>());
  label = elementAt(row,0)->addWidget(std::make_unique<WLabel>(tr("example.firstname")));
  label->setBuddy(firstNameEdit_);

  // Country
  ++row;
  countryEdit_ = elementAt(row,2)->addWidget(std::make_unique<WComboBox>());
  countryEdit_->addItem("");
  countryEdit_->addItem("Belgium");
  countryEdit_->addItem("Netherlands");
  countryEdit_->addItem("United Kingdom");
  countryEdit_->addItem("United States");
  label = elementAt(row,0)->addWidget(std::make_unique<WLabel>(tr("example.country")));
  label->setBuddy(countryEdit_);
  countryEdit_->setValidator(std::make_shared<WValidator>(true));
  countryEdit_->changed().connect(this, &Form::countryChanged);

  // City
  ++row;
  cityEdit_ = elementAt(row,2)->addWidget(std::make_unique<WComboBox>());
  cityEdit_->addItem(tr("example.choosecountry"));
  label = elementAt(row,0)->addWidget(std::make_unique<WLabel>(tr("example.city")));
  label->setBuddy(cityEdit_);

  // Birth date
  ++row;
  birthDateEdit_ = elementAt(row, 2)->addWidget(std::make_unique<WDateEdit>());
  birthDateEdit_->setBottom(WDate(1900, 1, 1));
  birthDateEdit_->setTop(WDate::currentDate());
  label = elementAt(row,0)->addWidget(std::make_unique<WLabel>(tr("example.birthdate")));
  label->setBuddy(birthDateEdit_);
  birthDateEdit_->setFormat("dd/MM/yyyy");
  birthDateEdit_->validator()->setMandatory(true);

  // Child count
  ++row;
  childCountEdit_ = elementAt(row,2)->addWidget(std::make_unique<WLineEdit>("0"));
  label = elementAt(row, 0)->addWidget(std::make_unique<WLabel>(tr("example.childcount")));
  label->setBuddy(childCountEdit_);
  childCountEdit_->setValidator(std::make_shared<WIntValidator>(0,30));
  childCountEdit_->validator()->setMandatory(true);

  ++row;
  remarksEdit_ = elementAt(row,2)->addWidget(std::make_unique<WTextArea>());
  remarksEdit_->setColumns(40);
  remarksEdit_->setRows(5);
  label = elementAt(row,0)->addWidget(std::make_unique<WLabel>(tr("example.remarks")));
  label->setBuddy(remarksEdit_);

  // Submit
  ++row;
  WPushButton *submit = elementAt(row,0)->addWidget(std::make_unique<WPushButton>(tr("submit")));
  submit->clicked().connect(this, &Form::submit);
  submit->setMargin(15, Side::Top);
  elementAt(row, 0)->setColumnSpan(3);
  elementAt(row, 0)->setContentAlignment(AlignmentFlag::Top | AlignmentFlag::Center);

  // Set column widths for label and validation icon
  elementAt(2, 0)->resize(WLength(30, LengthUnit::FontEx), WLength::Auto);
  elementAt(2, 1)->resize(20, WLength::Auto);
}

void Form::countryChanged()
{
  cityEdit_->clear();
  cityEdit_->addItem("");
  cityEdit_->setCurrentIndex(-1);

  switch (countryEdit_->currentIndex()) {
  case 0:
    break;
  case 1:
    cityEdit_->addItem("Antwerp");
    cityEdit_->addItem("Brussels");
    cityEdit_->addItem("Oekene");
    break;
  case 2:
    cityEdit_->addItem("Amsterdam");
    cityEdit_->addItem("Den Haag");
    cityEdit_->addItem("Rotterdam");
    break;
  case 3:
    cityEdit_->addItem("London");
    cityEdit_->addItem("Bristol");
    cityEdit_->addItem("Oxford");
    cityEdit_->addItem("Stonehenge");
    break;
  case 4:
    cityEdit_->addItem("Boston");
    cityEdit_->addItem("Chicago");
    cityEdit_->addItem("Los Angeles");
    cityEdit_->addItem("New York");
    break;
  }
}

bool Form::checkValid(WFormWidget *edit, const WString& text)
{
  if (edit->validate() != ValidationState::Valid) {
    feedbackMessages_->addWidget(std::make_unique<WText>(text));
    feedbackMessages_->addWidget(std::make_unique<WBreak>());
    edit->label()->decorationStyle().setForegroundColor(WColor("red"));
    edit->setStyleClass("Wt-invalid");
    return false;
  } else {
    edit->label()->decorationStyle().setForegroundColor(WColor());
    edit->setStyleClass("");

    return true;
  }
}

bool Form::validate()
{
  feedbackMessages_->clear();
  bool valid = true;

  if (!checkValid(nameEdit_, tr("error.name")))
    valid = false;
  if (!checkValid(countryEdit_, tr("error.country")))
    valid = false;
  if (!checkValid(birthDateEdit_, tr("error.birthdate")))
    valid = false;
  if (!checkValid(childCountEdit_, tr("error.childcount")))
    valid = false;

  return valid;
}

void Form::submit()
{
  if (validate()) {
    // do something useful with the data...
    WString name = WString("{1} {2}")
	.arg(firstNameEdit_->text())
	.arg(nameEdit_->text());

    WString remarks
      = remarksEdit_->text();

    clear();

    elementAt(0,0)->addWidget(std::make_unique<WText>(
                                WString("<p>Thank you, {1}, "
                               "for all this precious data.</p>").arg(name)));

    if (!remarks.empty())
      elementAt(0,0)->addWidget(std::make_unique<WText>(
				  WString("<p>You had some remarks. Splendid !</p>")));

    wApp->quit();
  }
}
