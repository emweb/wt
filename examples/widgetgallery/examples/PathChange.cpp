#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLink.h>
#include <Wt/WAnchor.h>
#include <Wt/WText.h>

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
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
container->addNew<Wt::WAnchor>(
      Wt::WLink(Wt::LinkType::InternalPath, "/navigation/shop"), "Shop");
container->addNew<Wt::WText>(" ");
container->addNew<Wt::WAnchor>(
      Wt::WLink(Wt::LinkType::InternalPath, "/navigation/eat"), "Eat");

/*
 * Handle the internal path events.
 */
auto out = container->addNew<Wt::WText>();
out->setInline(false);

Wt::WApplication *app = Wt::WApplication::instance();

app->internalPathChanged().connect([=] {
     handlePathChange(out);
});

handlePathChange(out);
        
SAMPLE_END(return std::move(container))
