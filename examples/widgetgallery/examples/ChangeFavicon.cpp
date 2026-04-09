#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDocRootDataInfo.h>
#include <Wt/WPushButton.h>
#include <Wt/WResourceFavicon.h>

SAMPLE_BEGIN(ChangeFavicon)

auto container = std::make_unique<Wt::WContainerWidget>();
auto sunFaviconButton = container->addWidget(std::make_unique<Wt::WPushButton>("Change the favicon to a sun"));
auto cloudFaviconButton = container->addWidget(std::make_unique<Wt::WPushButton>("Change the favicon to a cloud"));
auto snowFaviconButton = container->addWidget(std::make_unique<Wt::WPushButton>("Change the favicon to a snowflake"));
auto defaultFaviconButton = container->addWidget(std::make_unique<Wt::WPushButton>("Go back to the default favicon"));

sunFaviconButton->clicked().connect([=]() {
  auto favicon = std::make_unique<Wt::WResourceFavicon>(Wt::WDocRootDataInfo("icons/sun01.png").filePath());
  Wt::WApplication::instance()->setFavicon(std::move(favicon));
});

cloudFaviconButton->clicked().connect([=]() {
  auto favicon = std::make_unique<Wt::WResourceFavicon>(Wt::WDocRootDataInfo("icons/w_cloud.png").filePath());
  Wt::WApplication::instance()->setFavicon(std::move(favicon));
});

snowFaviconButton->clicked().connect([=]() {
  auto favicon = std::make_unique<Wt::WResourceFavicon>(Wt::WDocRootDataInfo("icons/snow.png").filePath());
  Wt::WApplication::instance()->setFavicon(std::move(favicon));
});

defaultFaviconButton->clicked().connect([=]() {
  Wt::WApplication::instance()->setFavicon(nullptr);
});

SAMPLE_END(return std::move(container))
