#include <Wt/WContainerWidget.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WSplitButton.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SplitButton)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WSplitButton *sb = container->addNew<Wt::WSplitButton>("Save");

Wt::WText *out = container->addNew<Wt::WText>();
out->setMargin(10, Wt::Side::Left);

auto popup = Wt::cpp14::make_unique<Wt::WPopupMenu>();
auto popup_ = popup.get();
popup_->addItem("Save As ...");
popup_->addItem("Save Template");

sb->dropDownButton()->setMenu(std::move(popup));

sb->actionButton()->clicked().connect([=] {
    out->setText("Saved!");
});

popup_->itemSelected().connect([=] (Wt::WMenuItem *item) {
    out->setText(item->text());
});

SAMPLE_END(return std::move(container))
