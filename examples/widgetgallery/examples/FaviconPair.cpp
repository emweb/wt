#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDocRootDataInfo.h>
#include <Wt/WFaviconPair.h>
#include <Wt/WPushButton.h>
#include <Wt/WResourceFavicon.h>

SAMPLE_BEGIN(FaviconPair)

auto container = std::make_unique<Wt::WContainerWidget>();
auto setCustomFaviconButton = container->addWidget(std::make_unique<Wt::WPushButton>("Set custom favicon"));
auto updateFaviconButton = container->addWidget(std::make_unique<Wt::WPushButton>("Update favicon"));
auto resetFaviconButton = container->addWidget(std::make_unique<Wt::WPushButton>("Reset favicon"));

setCustomFaviconButton->clicked().connect([=](){
#ifndef WT_TARGET_JAVA
  auto defaultFavicon = std::make_unique<Wt::WResourceFavicon>(Wt::WDocRootDataInfo("icons/wt.png").filePath());
  auto updatedFavicon = std::make_unique<Wt::WResourceFavicon>(Wt::WDocRootDataInfo("icons/wt-update.png").filePath());
#else
  auto defaultFavicon = std::make_unique<Wt::WResourceFavicon>(Wt::WDocRootDataInfo("icons/jwt.png").filePath());
  auto updatedFavicon = std::make_unique<Wt::WResourceFavicon>(Wt::WDocRootDataInfo("icons/jwt-update.png").filePath());
#endif
  auto faviconPair = std::make_unique<Wt::WFaviconPair>(std::move(defaultFavicon), std::move(updatedFavicon));
  Wt::WApplication::instance()->setFavicon(std::move(faviconPair));
});

updateFaviconButton->clicked().connect([=](){
  Wt::WApplication::instance()->favicon()->update();
});

resetFaviconButton->clicked().connect([=](){
  Wt::WApplication::instance()->favicon()->reset();
});

SAMPLE_END(return std::move(container))
