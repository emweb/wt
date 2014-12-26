#include <Wt/WTemplate>
#include <Wt/WDate>
#include <Wt/WDateValidator>
#include <Wt/WLabel>
#include <Wt/WDateEdit>
#include <Wt/WPushButton>
#include <Wt/WValidator>

SAMPLE_BEGIN(ValidationDate)

Wt::WTemplate *t = new Wt::WTemplate(Wt::WString::tr("date-template"));
t->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WDateEdit *dateEdit = new Wt::WDateEdit();
t->bindWidget("birth-date", dateEdit);

Wt::WDateValidator *dv = new Wt::WDateValidator();
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

dateEdit->setValidator(dv);

Wt::WPushButton *button = new Wt::WPushButton("Ok");
t->bindWidget("button", button);

Wt::WText *out = new Wt::WText();
out->setInline(false);
out->hide();
t->bindWidget("info", out);

button->clicked().connect(std::bind([=] () {
    out->show();

    Wt::WValidator::Result result = dv->validate(dateEdit->text());
    if (result.state() == Wt::WValidator::Valid) {
	Wt::WDate d = Wt::WDate::currentServerDate();
	int years = d.year() - dateEdit->date().year();
	int days = d.daysTo(dateEdit->date().addYears(years));
	if (days < 0)
	    days = d.daysTo( dateEdit->date().addYears(years + 1) );
	out->setText("<p>In " + boost::lexical_cast<std::string>(days) +
		     " days, we will be celebrating your next anniversary!</p>");
	out->setStyleClass("alert alert-success");
    } else {
	dateEdit->setFocus(true);
	out->setText(result.message());
	out->setStyleClass("alert alert-danger");
    }
}));

dateEdit->enterPressed().connect(std::bind([=] () {
    button->clicked().emit(Wt::WMouseEvent());
}));

SAMPLE_END(return t)
