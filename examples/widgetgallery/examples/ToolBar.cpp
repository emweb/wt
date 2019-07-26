#include <Wt/WContainerWidget.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WToolBar.h>

namespace {
    extern 
    std::unique_ptr<Wt::WPushButton> createColorButton(const char *className,
                                       const Wt::WString& text)
    {
        auto button = Wt::cpp14::make_unique<Wt::WPushButton>();
        button->setTextFormat(Wt::TextFormat::XHTML);
	button->setText(text);
	button->addStyleClass(className);
	return button;
    }
}

SAMPLE_BEGIN(ToolBar)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WToolBar *toolBar =
    container->addNew<Wt::WToolBar>();

toolBar->addButton(createColorButton("btn-primary", "Primary"));
toolBar->addButton(createColorButton("btn-danger", "Danger"));
toolBar->addButton(createColorButton("btn-success", "Success"));
toolBar->addButton(createColorButton("btn-warning", "Warning"));
toolBar->addButton(createColorButton("btn-inverse", "Inverse"));
toolBar->addButton(createColorButton("", "Default"));

auto resetButton = Wt::cpp14::make_unique<Wt::WPushButton>("Reset");

toolBar->addSeparator();
toolBar->addButton(std::move(resetButton));

SAMPLE_END(return std::move(container))
