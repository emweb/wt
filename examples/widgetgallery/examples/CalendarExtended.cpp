#include <Wt/WCalendar.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDate.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(CalendarExtended)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WCalendar *c2 = container->addNew<Wt::WCalendar>();
c2->setSelectionMode(Wt::SelectionMode::Extended);

Wt::WText* out = container->addNew<Wt::WText>();
out->addStyleClass("help-block");

c2->selectionChanged().connect([=] {
    Wt::WString selected;
    std::set<Wt::WDate> selection = c2->selection();

    for (auto &date : c2->selection()) {
	if (!selected.empty())
	    selected += ", ";

        selected += date.toString("dd/MM/yyyy");
    }

    out->setText(Wt::WString("<p>You selected the following dates: {1}</p>")
		 .arg(selected));
});

SAMPLE_END(return std::move(container))
