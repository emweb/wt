#include <Wt/WCalendar>
#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(CalendarSimple)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WCalendar *c1 = new Wt::WCalendar(container);

Wt::WText *out = new Wt::WText(container);
out->addStyleClass("help-block");

c1->selectionChanged().connect(std::bind([=] () {
    std::set<Wt::WDate> selection = c1->selection();
    if (selection.size() != 0) {
        Wt::WDate d;
	d = (*selection.begin());
	Wt::WDate toDate(d.year() + 1, 1, 1);
	int days = d.daysTo(toDate);
	out->setText(Wt::WString("<p>That's {1} days until New Year's Day!</p>")
		     .arg(days));
    }
}));

SAMPLE_END(return container)
