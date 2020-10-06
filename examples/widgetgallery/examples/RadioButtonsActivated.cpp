#include <Wt/WButtonGroup.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WRadioButton.h>
#include <Wt/WString.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(RadioButtonsActivated)
auto container = std::make_unique<Wt::WContainerWidget>();
auto group = std::make_shared<Wt::WButtonGroup>();

Wt::WRadioButton *rb;

rb = container->addNew<Wt::WRadioButton>("sleeping");
rb->setInline(false);
group->addButton(rb, 1);

rb = container->addNew<Wt::WRadioButton>("eating");
rb->setInline(false);
group->addButton(rb, 2);

rb = container->addNew<Wt::WRadioButton>("driving");
rb->setInline(false);
group->addButton(rb, 3);

rb = container->addNew<Wt::WRadioButton>("learning Wt");
rb->setInline(false);
group->addButton(rb, 4);

group->setSelectedButtonIndex(0); // Select the first button by default.

Wt::WText *out = container->addNew<Wt::WText>();

// Use a raw pointer inside the lambda to prevent memory leak
auto rawGroup = group.get();
group->checkedChanged().connect([=] (Wt::WRadioButton *selection) {
    Wt::WString text;

    switch (rawGroup->id(selection)) {
    case 1: text = Wt::WString("You checked button {1}.")
            .arg(rawGroup->checkedId());
	break;

    case 2: text = Wt::WString("You selected button {1}.")
            .arg(rawGroup->checkedId());
	break;

    case 3: text = Wt::WString("You clicked button {1}.")
            .arg(rawGroup->checkedId());
	break;
    }

    text += Wt::WString("... Are your really {1} now?")
	.arg(selection->text());

    if (rawGroup->id(selection) == 4)
        text = Wt::WString("That's what I expected!");

    out->setText(Wt::WString("<p>") + text + "</p>");
});

SAMPLE_END(return std::move(container))
