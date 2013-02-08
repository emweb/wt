#include <Wt/WCalendar>
#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(CalendarSimple)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WCalendar *c1 = new Wt::WCalendar(container);

Wt::WText *out = new Wt::WText(container);

c1->selectionChanged().connect(std::bind([=] () {
    int days;
    Wt::WDate d;
    std::set<Wt::WDate> selection = c1->selection();
    if (selection.size() != 0)
	d = *selection.begin();
    Wt::WDate toDate(d.year()+1, 1, 1);
    days = d.daysTo(toDate);
    out->setText("<p>That's " + boost::lexical_cast<std::string>(days) +
		 " days until New Year's Day!</p>");
}));

SAMPLE_END(return container)
