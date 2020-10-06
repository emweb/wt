#include <Wt/WCalendar.h>
#include <Wt/WDate.h>
#include <Wt/WDateEdit.h>
#include <Wt/WLabel.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WString.h>

SAMPLE_BEGIN(DateEdit)

auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("dateEdit-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

auto de1 = form->bindWidget("from", std::make_unique<Wt::WDateEdit>());
de1->setDate(Wt::WDate::currentServerDate().addDays(1));

auto de2 = form->bindWidget("to", std::make_unique<Wt::WDateEdit>());
de2->setFormat("dd MM yyyy"); // Apply a different date format.
de2->calendar()->setHorizontalHeaderFormat(Wt::CalendarHeaderFormat::SingleLetterDayNames);
de2->setBottom(de1->date());

auto button = form->bindWidget("save", std::make_unique<Wt::WPushButton>("Save"));

auto out = form->bindWidget("out", std::make_unique<Wt::WText>());

de1->changed().connect([=] {
    if (de1->validate() == Wt::ValidationState::Valid) {
        de2->setBottom(de1->date());
        out->setText("Date picker 1 is changed.");
    }
});

de2->changed().connect([=] {
    if (de1->validate() == Wt::ValidationState::Valid) {
        de1->setTop(de2->date());
        out->setText("Date picker 2 is changed.");
    }
});

button->clicked().connect([=] {
    if (de1->text().empty() || de2->text().empty())
        out->setText("You should enter two dates!");
    else {
        int days = de1->date().daysTo(de2->date()) + 1;
	if (days == 1)
	    out->setText("It's fine to take holiday just for one day!");
	else if (days > 1) 
	    out->setText(Wt::WString("So, you want to take holiday for a period of "
				     "{1} days?").arg(days));
	else
	    out->setText("Invalid period!");
    }
});

SAMPLE_END(return std::move(form))

