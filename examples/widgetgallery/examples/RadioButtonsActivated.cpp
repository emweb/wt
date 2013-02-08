#include <Wt/WButtonGroup>
#include <Wt/WContainerWidget>
#include <Wt/WRadioButton>
#include <Wt/WString>
#include <Wt/WText>

SAMPLE_BEGIN(RadioButtonsActivated)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
Wt::WButtonGroup *group = new Wt::WButtonGroup(container);

Wt::WRadioButton *rb;

rb = new Wt::WRadioButton("sleeping", container);
rb->setInline(false);
group->addButton(rb, 1);

rb = new Wt::WRadioButton("eating", container);
rb->setInline(false);
group->addButton(rb, 2);

rb = new Wt::WRadioButton("driving", container);
rb->setInline(false);
group->addButton(rb, 3);

rb = new Wt::WRadioButton("learning Wt", container);
rb->setInline(false);
group->addButton(rb, 4);

group->setSelectedButtonIndex(0); // Select the first button by default.

Wt::WText *out = new Wt::WText(container);

group->checkedChanged().connect(std::bind([=] (Wt::WRadioButton *selection) {
    Wt::WString text;

    switch (group->id(selection)) {
    case 1: text = Wt::WString::fromUTF8("You checked button {1}.")
	    .arg(group->checkedId());
	break;

    case 2: text = Wt::WString::fromUTF8("You selected button {1}.")
	    .arg(group->checkedId());
	break;

    case 3: text = Wt::WString::fromUTF8("You clicked button {1}.")
	    .arg(group->checkedId());
	break;
    }

    text += Wt::WString::fromUTF8("... Are your really {1} now?")
	.arg(selection->text());

    if (group->id(selection) == 4)
	text = Wt::WString::fromUTF8("That's what I expected!");

    out->setText(Wt::WString::fromUTF8("<p>") + text + "</p>");
}, std::placeholders::_1));

SAMPLE_END(return container)
