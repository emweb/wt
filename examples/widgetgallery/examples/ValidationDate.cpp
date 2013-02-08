#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WDate>
#include <Wt/WDatePicker>
#include <Wt/WDateValidator>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WValidator>

SAMPLE_BEGIN(ValidationDate)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WLabel *label = new Wt::WLabel("When is your birthdate?", container);
Wt::WLineEdit *dateEdit = new Wt::WLineEdit(container);
label->setBuddy(dateEdit);

Wt::WDatePicker *birthDP = new Wt::WDatePicker(dateEdit, container);

// Create and customize a date validator
Wt::WDateValidator *dv = new Wt::WDateValidator();
dateEdit->setValidator(dv);

dv->setBottom(Wt::WDate(1900, 1, 1));
dv->setTop(Wt::WDate::currentDate());
dv->setFormat("dd/MM/yyyy");
dv->setMandatory(true);
dv->setInvalidBlankText("A birthdate is mandatory!");
dv->setInvalidNotADateText("You should enter a date in the format "
			   "\"dd/MM/yyyy\"!");
dv->setInvalidTooEarlyText
    (Wt::WString("That's too early... The date must be {1} or later!"
		 "").arg(dv->bottom().toString("dd/MM/yyyy")));
dv->setInvalidTooLateText
    (Wt::WString("That's too late... The date must be {1} or earlier!"
		 "").arg(dv->top().toString("dd/MM/yyyy")));

new Wt::WBreak(container);

Wt::WPushButton *button = new Wt::WPushButton("OK", container);
Wt::WText *out = new Wt::WText(container);

button->clicked().connect(std::bind([=] () {
    Wt::WValidator::Result result =
        dv->validate(birthDP->lineEdit()->text());
    if (result.state() == Wt::WValidator::Valid) {
	Wt::WDate d = Wt::WDate::currentServerDate();
	int years = d.year() - birthDP->date().year();
	int days = d.daysTo(birthDP->date().addYears(years));
	if (days < 0)
	    days = d.daysTo( birthDP->date().addYears(years + 1) );
	out->setText("<p>In " + boost::lexical_cast<std::string>(days) +
		     " days, we will be celebrating your next anniversary!</p>");
    } else {
	birthDP->lineEdit()->setFocus();
	out->setText(result.message());
    }
}));

SAMPLE_END(return container)
