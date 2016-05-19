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
form->bindString("from-format", de1->format());
de1->setTime(Wt::WTime::currentTime());

Wt::WTimeEdit *de2 = new Wt::WTimeEdit();
form->bindWidget("to", de2);
#ifndef WT_TARGET_JAVA
de2->setFormat("h:mm:ss.zzz AP");
#else
de2->setFormat("h:mm:ss.SSS a");
#endif
de2->setTime(Wt::WTime::currentTime().addSecs(60*15));
form->bindString("to-format", de2->format());

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
	out->setText("You should enter two times!");
    else {
    long secs = de1->time().secsTo(de2->time()) + 1;
	if (secs <= 60*10)
	  out->setText("This is a really small range of time");
	else
	  out->setText
	    (Wt::WString("So, you want your package to be delivered between "
			 "{1} and {2} ?...")
	     .arg(de1->time().toString())
	     .arg(de2->time().toString()));
    }  
}));


SAMPLE_END(return form)

