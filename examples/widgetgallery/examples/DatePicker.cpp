#include <Wt/WContainerWidget>
#include <Wt/WCalendar>
#include <Wt/WDate>
#include <Wt/WDatePicker>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTemplate>

SAMPLE_BEGIN(DatePicker)
Wt::WTemplate *form = new Wt::WTemplate(Wt::WString::tr("dateEdit-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WDatePicker *dp1 = new Wt::WDatePicker();
form->bindWidget("from", dp1);
dp1->setDate(Wt::WDate::currentServerDate().addDays(1));

Wt::WDatePicker *dp2 = new Wt::WDatePicker();
form->bindWidget("to", dp2);
dp2->setFormat("dd MM yyyy"); // Apply a different date format.
dp2->calendar()->setHorizontalHeaderFormat(Wt::WCalendar::SingleLetterDayNames);
dp2->setBottom(dp1->date());

Wt::WPushButton *button = new Wt::WPushButton("Save");
form->bindWidget("save", button);

Wt::WText *out = new Wt::WText();
form->bindWidget("out", out);

dp1->lineEdit()->changed().connect(std::bind([=] () {
    dp2->setBottom(dp1->date());
    out->setText("Date picker 1 is changed.");
}));

dp2->lineEdit()->changed().connect(std::bind([=] () {
    dp1->setTop(dp2->date());
    out->setText("Date picker 2 is changed.");
}));

button->clicked().connect(std::bind([=] () {
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
}));

SAMPLE_END(return form)

