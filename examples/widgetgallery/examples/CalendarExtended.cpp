#include <Wt/WCalendar>
#include <Wt/WContainerWidget>
#include <Wt/WDate>
#include <Wt/WText>

SAMPLE_BEGIN(CalendarExtended)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WCalendar *c2 = new Wt::WCalendar(container);
c2->setSelectionMode(Wt::ExtendedSelection);

Wt::WText* out = new Wt::WText(container);
out->addStyleClass("help-block");

c2->selectionChanged().connect(std::bind([=] () {
    Wt::WString selected;
    std::set<Wt::WDate> selection = c2->selection();

    for (std::set<Wt::WDate>::const_iterator it = selection.begin();
	 it != selection.end(); ++it) {
	if (!selected.empty())
	    selected += ", ";

	const Wt::WDate& d = *it;
	selected += d.toString("dd/MM/yyyy");
    }

    out->setText(Wt::WString::fromUTF8
		 ("<p>You selected the following dates: {1}</p>")
		 .arg(selected));
}));

SAMPLE_END(return container)
