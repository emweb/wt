#include <Wt/WContainerWidget.h>
#include <Wt/WCalendar.h>
#include <Wt/WDate.h>
#include <Wt/WDatePicker.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(DatePicker)

auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("dateEdit-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

auto dp1 = form->bindWidget("from", std::make_unique<Wt::WDatePicker>());
dp1->setDate(Wt::WDate::currentServerDate().addDays(1));

auto dp2 = form->bindWidget("to", std::make_unique<Wt::WDatePicker>());
dp2->setFormat("dd MM yyyy"); // Apply a different date format.
dp2->calendar()->setHorizontalHeaderFormat(Wt::CalendarHeaderFormat::SingleLetterDayNames);
dp2->setBottom(dp1->date());

auto button = form->bindWidget("save", std::make_unique<Wt::WPushButton>("Save"));

auto out = form->bindWidget("out", std::make_unique<Wt::WText>());

dp1->lineEdit()->changed().connect([=] {
    dp2->setBottom(dp1->date());
    out->setText("Date picker 1 is changed.");
});

dp2->lineEdit()->changed().connect([=] {
    dp1->setTop(dp2->date());
    out->setText("Date picker 2 is changed.");
});

button->clicked().connect([=] {
    if (dp1->lineEdit()->text().empty() || dp2->lineEdit()->text().empty())
        out->setText("You should enter two dates!");
    else {
        int days = dp1->date().daysTo(dp2->date()) + 1;
	if (days == 0)
	    out->setText("It's fine to take holiday just for one day!");
	else if (days > 1)
	    out->setText(Wt::WString("So, you want to take holiday for a period "
				     "of {1} days?...").arg(days));
	else
	    out->setText("Invalid period!");
   }
});

SAMPLE_END(return std::move(form))

