#include "Form.h"

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WDateEdit>
#include <Wt/WSelectionBox>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WIntValidator>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTableCell>
#include <Wt/WTextArea>
#include <Wt/WText>

Form::Form(WContainerWidget *parent)
  : WTable(parent)
{
  createUI();
}

void Form::createUI()
{
  WLabel *label;
  int row = 0;

  // Title
  elementAt(row, 0)->setColumnSpan(3);
  elementAt(row, 0)->setContentAlignment(AlignTop | AlignCenter);
  elementAt(row, 0)->setPadding(10);
  WText *title = new WText(tr("example.form"),
			   elementAt(row, 0));
  title->decorationStyle().font().setSize(WFont::XLarge);

  // error messages
  ++row;
  elementAt(row, 0)->setColumnSpan(3);
  feedbackMessages_ = elementAt(row, 0);
  feedbackMessages_->setPadding(5);

  WCssDecorationStyle& errorStyle = feedbackMessages_->decorationStyle();
  errorStyle.setForegroundColor(Wt::red);
  errorStyle.font().setSize(WFont::Smaller);
  errorStyle.font().setWeight(WFont::Bold);
  errorStyle.font().setStyle(WFont::Italic);

  // Name
  ++row;
  nameEdit_ = new WLineEdit(elementAt(row, 2));
  label = new WLabel(tr("example.name"), elementAt(row, 0));
  label->setBuddy(nameEdit_);
  nameEdit_->setValidator(new WValidator(true));
  nameEdit_->enterPressed().connect(this, &Form::submit);

  // First name
  ++row;
  firstNameEdit_ = new WLineEdit(elementAt(row, 2));
  label = new WLabel(tr("example.firstname"), elementAt(row,0));
  label->setBuddy(firstNameEdit_);

  // Country
  ++row;
  countryEdit_ = new WComboBox(elementAt(row, 2));
  countryEdit_->addItem("");
  countryEdit_->addItem("Belgium");
  countryEdit_->addItem("Netherlands");
  countryEdit_->addItem("United Kingdom");
  countryEdit_->addItem("United States");
  label = new WLabel(tr("example.country"), elementAt(row, 0));
  label->setBuddy(countryEdit_);
  countryEdit_->setValidator(new WValidator(true));
  countryEdit_->changed().connect(this, &Form::countryChanged);

  // City
  ++row;
  cityEdit_ = new WComboBox(elementAt(row, 2));
  cityEdit_->addItem(tr("example.choosecountry"));
  label = new WLabel(tr("example.city"), elementAt(row, 0));
  label->setBuddy(cityEdit_);

  // Birth date
  ++row;

  birthDateEdit_ = new WDateEdit(elementAt(row, 2));
  birthDateEdit_->setBottom(WDate(1900, 1, 1));
  birthDateEdit_->setTop(WDate::currentDate());
  label = new WLabel(tr("example.birthdate"), elementAt(row, 0));
  label->setBuddy(birthDateEdit_);
  birthDateEdit_->setFormat("dd/MM/yyyy");
  birthDateEdit_->validator()->setMandatory(true);

  // Child count
  ++row;
  childCountEdit_ = new WLineEdit("0", elementAt(row, 2));
  label = new WLabel(tr("example.childcount"),
		     elementAt(row, 0));
  label->setBuddy(childCountEdit_);
  childCountEdit_->setValidator(new WIntValidator(0,30));
  childCountEdit_->validator()->setMandatory(true);

  ++row;
  remarksEdit_ = new WTextArea(elementAt(row, 2));
  remarksEdit_->setColumns(40);
  remarksEdit_->setRows(5);
  label = new WLabel(tr("example.remarks"),
		     elementAt(row, 0));
  label->setBuddy(remarksEdit_);

  // Submit
  ++row;
  WPushButton *submit = new WPushButton(tr("submit"),
					elementAt(row, 0));
  submit->clicked().connect(this, &Form::submit);
  submit->setMargin(15, Top);
  elementAt(row, 0)->setColumnSpan(3);
  elementAt(row, 0)->setContentAlignment(AlignTop | AlignCenter);

  // Set column widths for label and validation icon
  elementAt(2, 0)->resize(WLength(30, WLength::FontEx), WLength::Auto);
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
  if (edit->validate() != WValidator::Valid) {
    feedbackMessages_->addWidget(new WText(text));
    feedbackMessages_->addWidget(new WBreak());
    edit->label()->decorationStyle().setForegroundColor(Wt::red);
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
    std::wstring name
      = firstNameEdit_->text() + L" " + nameEdit_->text();

    std::wstring remarks
      = remarksEdit_->text();

    clear();

    new WText(WString::fromUTF8("<p>Thank you, {1}, "
				"for all this precious data.</p>").arg(name),
	      elementAt(0, 0));
    
    if (!remarks.empty())
      new WText("<p>You had some remarks. Splendid !</p>", elementAt(0, 0));

    wApp->quit();
  }
}
