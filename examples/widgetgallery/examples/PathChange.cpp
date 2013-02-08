#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WLink>
#include <Wt/WAnchor>
#include <Wt/WText>

namespace {

void handlePathChange(Wt::WText *out)
{
    Wt::WApplication *app = Wt::WApplication::instance();

    if (app->internalPath() == "/navigation/shop")
	out->setText("<p>Currently shopping.</p>");
    else if (app->internalPath() == "/navigation/eat")
	out->setText("<p>Needed some food, eating now!</p>");
    else
	out->setText("<p><i>Idle.</i></p>");
}

}

SAMPLE_BEGIN(PathChange)
/*
 * Create two links to internal paths.
 */
Wt::WContainerWidget *container = new Wt::WContainerWidget();
new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/navigation/shop"),
		"Shop", container);
new Wt::WText(" ", container);
new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/navigation/eat"),
		"Eat", container);

/*
 * Handle the internal path events.
 */
Wt::WText *out = new Wt::WText(container);
out->setInline(false);

Wt::WApplication *app = Wt::WApplication::instance();

app->internalPathChanged().connect(std::bind([=] () {
     handlePathChange(out);
}));

handlePathChange(out);
        
SAMPLE_END(return container)
