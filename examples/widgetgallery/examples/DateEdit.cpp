#include <Wt/WCalendar>
#include <Wt/WDate>
#include <Wt/WDateEdit>
#include <Wt/WLabel>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WString>

SAMPLE_BEGIN(DateEdit)
Wt::WTemplate *form = new Wt::WTemplate(Wt::WString::tr("dateEdit-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WDateEdit *de1 = new Wt::WDateEdit();
form->bindWidget("from", de1);
de1->setDate(Wt::WDate::currentServerDate().addDays(1));

Wt::WDateEdit *de2 = new Wt::WDateEdit();
form->bindWidget("to", de2);
de2->setFormat("dd MM yyyy"); // Apply a different date format.
de2->calendar()->setHorizontalHeaderFormat(Wt::WCalendar::SingleLetterDayNames);
de2->setBottom(de1->date());

Wt::WPushButton *button = new Wt::WPushButton("Save");
form->bindWidget("save", button);

Wt::WText *out = new Wt::WText();
form->bindWidget("out", out);

de1->changed().connect(std::bind([=] () {
    if (de1->validate() == Wt::WValidator::Valid) {
        de2->setBottom(de1->date());
	out->setText("Date picker 1 is changed.");
    }
}));

de2->changed().connect(std::bind([=] () {
    if (de1->validate() == Wt::WValidator::Valid) {
        de1->setTop(de2->date());
	out->setText("Date picker 2 is changed.");
    }
}));

button->clicked().connect(std::bind([=] () {
    if (de1->text().empty() || de2->text().empty())
	out->setText("You should enter two dates!");
    else {
	int days = de1->date().daysTo(de2->date()) + 1;
	if (days == 1)
	    out->setText("It's fine to take holiday just for one day!");
	else if (days > 1) 
	    out->setText(Wt::WString("So, you want to take holiday for a period of "
				     "{1} days?...").arg(days));
	else
	    out->setText("Invalid period!");
    }
}));

SAMPLE_END(return form)

