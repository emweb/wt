#include <Wt/WLineEdit.h>
#include <Wt/WMenu.h>
#include <Wt/WMessageBox.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPopupMenuItem.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(NavigationBar)

auto container = std::make_unique<Wt::WContainerWidget>();

// Create a navigation bar with a link to a web page.
Wt::WNavigationBar *navigation = container->addNew<Wt::WNavigationBar>();
navigation->setTitle("Corpy Inc.",
		     "https://www.google.com/search?q=corpy+inc");
navigation->setResponsive(true);

Wt::WStackedWidget *contentsStack = container->addNew<Wt::WStackedWidget>();
contentsStack->addStyleClass("contents");

// Setup a Left-aligned menu.
auto leftMenu = std::make_unique<Wt::WMenu>(contentsStack);
auto leftMenu_ = navigation->addMenu(std::move(leftMenu));

auto searchResult = std::make_unique<Wt::WText>("Buy or Sell... Bye!");
auto searchResult_ = searchResult.get();

leftMenu_->addItem("Home", std::make_unique<Wt::WText>("There is no better place!"));
leftMenu_->addItem("Layout", std::make_unique<Wt::WText>("Layout contents"))
    ->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/layout"));
leftMenu_->addItem("Sales", std::move(searchResult));

// Setup a Right-aligned menu.
auto rightMenu = std::make_unique<Wt::WMenu>();
auto rightMenu_ = navigation->addMenu(std::move(rightMenu), Wt::AlignmentFlag::Right);

// Create a popup submenu for the Help menu.
auto popupPtr = std::make_unique<Wt::WPopupMenu>();
auto popup = popupPtr.get();
popup->addItem("Contents");
popup->addItem("Index");
popup->addSeparator();
popup->addItem("About");

popup->itemSelected().connect([=] (Wt::WMenuItem *item) {
#ifndef WT_TARGET_JAVA
    auto messageBox = popup->addChild(
            std::make_unique<Wt::WMessageBox>
#else // WT_TARGET_JAVA
    auto messageBox = new Wt::WMessageBox
#endif // WT_TARGET_JAVA
            ("Help",
             Wt::WString("<p>Showing Help: {1}</p>").arg(item->text()),
             Wt::Icon::Information,
#ifndef WT_TARGET_JAVA
             Wt::StandardButton::Ok));
#else // WT_TARGET_JAVA
             Wt::WFlags<Wt::StandardButton>(Wt::StandardButton::Ok));
#endif // WT_TARGET_JAVA

    messageBox->buttonClicked().connect([=] {
#ifndef WT_TARGET_JAVA
        popup->removeChild(messageBox);
#else // WT_TARGET_JAVA
        delete messageBox;
#endif // WT_TARGET_JAVA
    });

    messageBox->show();
});

auto item = std::make_unique<Wt::WMenuItem>("Help");
item->setMenu(std::move(popupPtr));
rightMenu_->addItem(std::move(item));

// Add a Search control.
auto editPtr = std::make_unique<Wt::WLineEdit>();
auto edit = editPtr.get();
edit->setPlaceholderText("Enter a search item");

edit->enterPressed().connect([=] {
    leftMenu_->select(2); // is the index of the "Sales"
    searchResult_->setText(Wt::WString("Nothing found for {1}.")
                          .arg(edit->text()));
});

navigation->addSearch(std::move(editPtr), Wt::AlignmentFlag::Right);

SAMPLE_END(return std::move(container))
