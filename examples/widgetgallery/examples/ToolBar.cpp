#include <Wt/WContainerWidget>
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WToolBar>

namespace {
    extern 
    Wt::WPushButton *createColorButton(const char *className,
				       const Wt::WString& text)
    {
        Wt::WPushButton *button = new Wt::WPushButton();
	button->setTextFormat(Wt::XHTMLText);
	button->setText(text);
	button->addStyleClass(className);
	return button;
    }
}

SAMPLE_BEGIN(ToolBar)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

std::vector<Wt::WPushButton *> colorButtons;

Wt::WToolBar *toolBar = new Wt::WToolBar(container);

toolBar->addButton(createColorButton("btn-primary", "Primary"));
toolBar->addButton(createColorButton("btn-danger", "Danger"));
toolBar->addButton(createColorButton("btn-success", "Success"));
toolBar->addButton(createColorButton("btn-warning", "Warning"));
toolBar->addButton(createColorButton("btn-inverse", "Inverse"));
toolBar->addButton(createColorButton("", "Default"));

Wt::WPushButton *resetButton = new Wt::WPushButton("Reset");

toolBar->addSeparator();
toolBar->addButton(resetButton);

SAMPLE_END(return container)
