#include <Wt/WCalendar.h>
#include <Wt/WDate.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WLabel.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WString.h>

SAMPLE_BEGIN(TimeEdit)
auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("timeEdit-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

auto te1 = form->bindWidget("from", std::make_unique<Wt::WTimeEdit>());
form->bindString("from-format", te1->format());
te1->setTime(Wt::WTime::currentTime());

auto te2 = form->bindWidget("to", std::make_unique<Wt::WTimeEdit>());
#ifndef WT_TARGET_JAVA
te2->setFormat("h:mm:ss.zzz AP");
#else
te2->setFormat("h:mm:ss.SSS a");
#endif
te2->setTime(Wt::WTime::currentTime().addSecs(60*15));
form->bindString("to-format", te2->format());

auto button = form->bindWidget("save", std::make_unique<Wt::WPushButton>("Save"));

auto out = form->bindWidget("out", std::make_unique<Wt::WText>());

te1->changed().connect([=] {
    if (te1->validate() == Wt::ValidationState::Valid) {
      out->setText("Time picker 1 is changed.");
    }
});

te2->changed().connect([=] {
    if (te2->validate() == Wt::ValidationState::Valid) {
      out->setText("Time picker 2 is changed.");
    }
});

button->clicked().connect([=] {
    if (te1->text().empty() || te2->text().empty())
        out->setText("You should enter two times!");
    else {
    long secs = te1->time().secsTo(te2->time()) + 1;
	if (secs <= 60*10)
	  out->setText("This is a really small range of time");
	else
	  out->setText
	    (Wt::WString("So, you want your package to be delivered between "
	                 "{1} and {2}?")
	     .arg(te1->time().toString())
	     .arg(te2->time().toString()));
    }  
});

SAMPLE_END(return std::move(form))

