#include <Wt/WLineEdit>
#include <Wt/WMenu>
#include <Wt/WMessageBox>
#include <Wt/WNavigationBar>
#include <Wt/WPopupMenu>
#include <Wt/WPopupMenuItem>
#include <Wt/WStackedWidget>
#include <Wt/WText>

SAMPLE_BEGIN(NavigationBar)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

// Create a navigation bar with a link to a web page.
Wt::WNavigationBar *navigation = new Wt::WNavigationBar(container);
navigation->setTitle("Corpy Inc.",
		     "http://www.google.com/search?q=corpy+inc");
navigation->setResponsive(true);

Wt::WStackedWidget *contentsStack = new Wt::WStackedWidget(container);
contentsStack->addStyleClass("contents");

// Setup a Left-aligned menu.
Wt::WMenu *leftMenu = new Wt::WMenu(contentsStack, container);
navigation->addMenu(leftMenu);

Wt::WText *searchResult = new Wt::WText("Buy or Sell... Bye!");

leftMenu->addItem("Home", new Wt::WText("There is no better place!"));
leftMenu->addItem("Layout", new Wt::WText("Layout contents"))
    ->setLink(Wt::WLink(Wt::WLink::InternalPath, "/layout"));
leftMenu->addItem("Sales", searchResult);

// Setup a Right-aligned menu.
Wt::WMenu *rightMenu = new Wt::WMenu();
navigation->addMenu(rightMenu, Wt::AlignRight);

// Create a popup submenu for the Help menu.
Wt::WPopupMenu *popup = new Wt::WPopupMenu();
popup->addItem("Contents");
popup->addItem("Index");
popup->addSeparator();
popup->addItem("About");

popup->itemSelected().connect(std::bind([=] (Wt::WMenuItem *item) {
    Wt::WMessageBox *messageBox = new Wt::WMessageBox
	("Help",
	 Wt::WString::fromUTF8("<p>Showing Help: {1}</p>").arg(item->text()),
	 Wt::Information, Wt::Ok);

    messageBox->buttonClicked().connect(std::bind([=] () {
	delete messageBox;
    }));

    messageBox->show();
}, std::placeholders::_1));

Wt::WMenuItem *item = new Wt::WMenuItem("Help");
item->setMenu(popup);
rightMenu->addItem(item);

// Add a Search control.
Wt::WLineEdit *edit = new Wt::WLineEdit();
edit->setEmptyText("Enter a search item");

edit->enterPressed().connect(std::bind([=] () {
    leftMenu->select(2); // is the index of the "Sales"
    searchResult->setText(Wt::WString("Nothing found for {1}.")
			  .arg(edit->text()));
}));

navigation->addSearch(edit, Wt::AlignRight);

container->addWidget(contentsStack);

SAMPLE_END(return container)
