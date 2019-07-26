#include <Wt/WCalendar.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(CalendarSimple)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WCalendar *c1 = container->addNew<Wt::WCalendar>();

Wt::WText *out = container->addNew<Wt::WText>();
out->addStyleClass("help-block");

c1->selectionChanged().connect([=] {
    std::set<Wt::WDate> selection = c1->selection();
    if (selection.size() != 0) {
        Wt::WDate d;
	d = (*selection.begin());
	Wt::WDate toDate(d.year() + 1, 1, 1);
	int days = d.daysTo(toDate);
	out->setText(Wt::WString("<p>That's {1} days until New Year's Day!</p>")
		     .arg(days));
    }
});

SAMPLE_END(return std::move(container))
