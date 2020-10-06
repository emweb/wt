#include <Wt/WTemplate.h>
#include <Wt/WDate.h>
#include <Wt/WDateValidator.h>
#include <Wt/WLabel.h>
#include <Wt/WDateEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WValidator.h>

SAMPLE_BEGIN(ValidationDate)

auto t = std::make_unique<Wt::WTemplate>(Wt::WString::tr("date-template"));
t->addFunction("id", &Wt::WTemplate::Functions::id);

auto dateEdit = t->bindWidget("birth-date", std::make_unique<Wt::WDateEdit>());

auto dv = std::make_shared<Wt::WDateValidator>();
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

auto button = t->bindWidget("button", std::make_unique<Wt::WPushButton>("Ok"));

auto out = t->bindWidget("info", std::make_unique<Wt::WText>());
out->setInline(false);
out->hide();

button->clicked().connect([=] {
    out->show();

    Wt::WValidator::Result result = dv->validate(dateEdit->text());
    if (result.state() == Wt::ValidationState::Valid) {
        Wt::WDate d = Wt::WDate::currentServerDate();
        int years = d.year() - dateEdit->date().year();
        int days = d.daysTo(dateEdit->date().addYears(years));
	if (days < 0)
	    days = d.daysTo( dateEdit->date().addYears(years + 1) );
	out->setText("<p>In " + std::to_string(days) +
		     " days, we will be celebrating your next anniversary!</p>");
	out->setStyleClass("alert alert-success");
    } else {
        dateEdit->setFocus(true);
        out->setText(result.message());
        out->setStyleClass("alert alert-danger");
    }
});

dateEdit->enterPressed().connect([=] {
    button->clicked().emit(Wt::WMouseEvent());
});

SAMPLE_END(return std::move(t))
