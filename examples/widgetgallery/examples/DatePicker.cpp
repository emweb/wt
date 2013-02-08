#include <Wt/WContainerWidget>
#include <Wt/WCalendar>
#include <Wt/WDate>
#include <Wt/WDatePicker>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

SAMPLE_BEGIN(DatePicker)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new Wt::WText("<p>When do you want to take your holiday?</p>", container);

Wt::WLabel *dp1Label = new Wt::WLabel("From", container);
Wt::WDatePicker *dp1 = new Wt::WDatePicker(container);
dp1->setDate(Wt::WDate::currentServerDate().addDays(1));
dp1Label->setBuddy(dp1->lineEdit());

Wt::WLabel *dp2Label = new Wt::WLabel("To (format \"dd MM yyyy\")", container);
Wt::WDatePicker *dp2 = new Wt::WDatePicker(container);
dp2->setFormat("dd MM yyyy"); // Apply a different date format.
dp2->calendar()->setHorizontalHeaderFormat(Wt::WCalendar::SingleLetterDayNames);
dp2Label->setBuddy(dp2->lineEdit());

new Wt::WText("<p></p>", container);
Wt::WPushButton *button = new Wt::WPushButton("Save", container);

Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Left);

dp1->lineEdit()->changed().connect(std::bind([=] () {
    dp2->setBottom(dp1->date());
    out->setText("<p>Date picker 1 is changed.</p>");
}));

dp2->lineEdit()->changed().connect(std::bind([=] () {
    dp1->setTop(dp2->date());
    out->setText("<p>Date picker 2 is changed.</p>");
}));

button->clicked().connect(std::bind([=] () {
    if (dp1->lineEdit()->text().empty() || dp2->lineEdit()->text().empty())
	out->setText("<p>You should enter two dates!</p>");
    else {
	int days = dp1->date().daysTo(dp2->date()) + 1;
	if (dp1->date() == dp2->date())
	    out->setText("<p>It's fine to take holiday just for one day!"
			 "</p>");
	else if (dp1->date() < dp2->date()) {
	    out->setText("<p>So, you want to take holiday for a period of " +
			 boost::lexical_cast<std::string>(days) +
			 " days?...</p>");
	} else {
	    out->setText("<p>Invalid period!</p>");
	}
    }
}));

SAMPLE_END(return container)

