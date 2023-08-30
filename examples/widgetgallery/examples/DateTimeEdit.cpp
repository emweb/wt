#include <Wt/WDateTimeEdit.h>
#include <Wt/WDateTime.h>
#include <Wt/WLabel.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WTime.h>
#include <Wt/WString.h>

SAMPLE_BEGIN(DateTimeEdit)

auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("dateTimeEdit-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);
form->setCondition("if:show-format", true);

auto dte1 = form->bindWidget("from", std::make_unique<Wt::WDateTimeEdit>());
dte1->setDateTime(Wt::WDateTime::currentDateTime());
dte1->setFormat("yyyy-MM-dd HH:mm");
form->bindString("from-format", dte1->format());

auto dte2 = form->bindWidget("to", std::make_unique<Wt::WDateTimeEdit>());
dte2->setDateTime(Wt::WDateTime::currentDateTime());
#ifndef WT_TARGET_JAVA
dte2->setFormat("dd MM yyyy hh:mm:ss.zzz AP"); //use different format
#else
dte2->setFormat("dd MM yyyy hh:mm:ss.SSS a"); //use different format
#endif
form->bindString("to-format", dte2->format());
dte2->setBottom(dte1->dateTime());

auto button = form->bindWidget("save", std::make_unique<Wt::WPushButton>("Save"));

auto out = form->bindWidget("out", std::make_unique<Wt::WText>());

dte1->changed().connect([=] {
    if (dte1->validate() == Wt::ValidationState::Valid) {
      out->setText("DateTime 1 is changed.");
    }
});

dte2->changed().connect([=] {
    if (dte2->validate() == Wt::ValidationState::Valid) {
      out->setText("DateTime 2 is changed.");
    }
});

button->clicked().connect([=] {
    if (dte1->text().empty() || dte2->text().empty()) {
        out->setText("You should enter two dates and times!");
    } else {
        long secs = dte1->dateTime().time().secsTo(dte2->dateTime().time());
        int days = dte1->dateTime().date().daysTo(dte2->dateTime().date());
        if (days < 0 || (days == 0 && secs < 0)) {
            out->setText(Wt::WString("You cannot rent a car for a negative amount of time."));
        } else if (days >= 1) {
            out->setText(Wt::WString("So, you want to rent a car for {1} day(s)?").arg(days));
        } else if (secs > 60*60) {
            out->setText(Wt::WString("So, you want you want to rent a car for "
                                     "{1} hour(s)?").arg(secs/(60*60)));
        } else if (secs == 60*60) {
            out->setText(Wt::WString("So, you want to rent a car for a hour?"));
        } else {
            out->setText(Wt::WString("Sorry, you can't rent a car for less than a hour."));
        }
    }
});
SAMPLE_END(return std::move(form))
