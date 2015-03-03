#include <Wt/WCalendar>
#include <Wt/WDate>
#include <Wt/WTimeEdit>
#include <Wt/WLabel>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WString>

SAMPLE_BEGIN(TimeEdit)
Wt::WTemplate *form = new Wt::WTemplate(Wt::WString::tr("timeEdit-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WTimeEdit *de1 = new Wt::WTimeEdit();
form->bindWidget("from", de1);
de1->setTime(Wt::WTime::currentServerTime());

Wt::WTimeEdit *de2 = new Wt::WTimeEdit();
form->bindWidget("to", de2);
de2->setFormat("HH:mm:ss"); // Apply a different date format.

Wt::WPushButton *button = new Wt::WPushButton("Save");
form->bindWidget("save", button);

Wt::WText *out = new Wt::WText();
form->bindWidget("out", out);

de1->changed().connect(std::bind([=] () {
    if (de1->validate() == Wt::WValidator::Valid) {
    out->setText("Time picker 1 is changed.");
    }
}));

de2->changed().connect(std::bind([=] () {
    if (de1->validate() == Wt::WValidator::Valid) {
    out->setText("Time picker 2 is changed.");
    }
}));

button->clicked().connect(std::bind([=] () {
    if (de1->text().empty() || de2->text().empty())
	out->setText("You should enter two time!");
    else {
	int secs = de1->time().secsTo(de2->time()) + 1;
	if (secs <= 3600)
	    out->setText("This is a really small range of time");
	else if (secs > 3600) 
	    out->setText(Wt::WString("So, you want to be delivered between "
				     "{1} and {2} ?...").arg(de1->time().toString()).arg(de2->time().toString()));
	else
	    out->setText("Invalid period!");
    }

  
}));


SAMPLE_END(return form)

