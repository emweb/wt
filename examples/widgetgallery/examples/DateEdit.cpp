#include <Wt/WContainerWidget>
#include <Wt/WCalendar>
#include <Wt/WDate>
#include <Wt/WDateEdit>
#include <Wt/WLabel>
#include <Wt/WPushButton>
#include <Wt/WText>

SAMPLE_BEGIN(DateEdit)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new Wt::WText("<p>When do you want to take your holiday?</p>", container);

Wt::WLabel *de1Label = new Wt::WLabel("From", container);
Wt::WDateEdit *de1 = new Wt::WDateEdit(container);
de1->setDate(Wt::WDate::currentServerDate().addDays(1));
de1Label->setBuddy(de1);

Wt::WLabel *de2Label = new Wt::WLabel("To (format \"dd MM yyyy\")", container);
Wt::WDateEdit *de2 = new Wt::WDateEdit(container);
de2->setFormat("dd MM yyyy"); // Apply a different date format.
de2->calendar()->setHorizontalHeaderFormat(Wt::WCalendar::SingleLetterDayNames);
de2->setBottom(de1->date());
de2Label->setBuddy(de2);

new Wt::WText("<p></p>", container);
Wt::WPushButton *button = new Wt::WPushButton("Save", container);

Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Left);

de1->changed().connect(std::bind([=] () {
    if (de1->validate() == Wt::WValidator::Valid) {
        de2->setBottom(de1->date());
	out->setText("<p>Date picker 1 is changed.</p>");
    }
}));

de2->changed().connect(std::bind([=] () {
    if (de1->validate() == Wt::WValidator::Valid) {
        de1->setTop(de2->date());
	out->setText("<p>Date picker 2 is changed.</p>");
    }
}));

button->clicked().connect(std::bind([=] () {
    if (de1->text().empty() || de2->text().empty())
	out->setText("<p>You should enter two dates!</p>");
    else {
	int days = de1->date().daysTo(de2->date()) + 1;
	if (de1->date() == de2->date())
	    out->setText("<p>It's fine to take holiday just for one day!"
			 "</p>");
	else if (de1->date() < de2->date()) {
	    out->setText("<p>So, you want to take holiday for a period of " +
			 boost::lexical_cast<std::string>(days) +
			 " days?...</p>");
	} else {
	    out->setText("<p>Invalid period!</p>");
	}
    }
}));

SAMPLE_END(return container)

