#include <Wt/WContainerWidget>
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>
#include <Wt/WSplitButton>
#include <Wt/WText>

SAMPLE_BEGIN(SplitButton)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WSplitButton *sb = new Wt::WSplitButton("Save", container);

Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Left);

Wt::WPopupMenu *popup = new Wt::WPopupMenu();
popup->addItem("Save As ...");
popup->addItem("Save Template");

sb->dropDownButton()->setMenu(popup);

sb->actionButton()->clicked().connect(std::bind([=] () {
    out->setText("Saved!");
}));

popup->itemSelected().connect(std::bind([=] (Wt::WMenuItem *item) {
    out->setText(item->text());
}, std::placeholders::_1));

SAMPLE_END(return container)
