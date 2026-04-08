#include <Wt/WContainerWidget.h>
#include <Wt/WBadge.h>

SAMPLE_BEGIN(Badge)

auto container = std::make_unique<Wt::WContainerWidget>();
auto badge = container->addWidget(std::make_unique<Wt::WBadge>("A Simple Badge"));

SAMPLE_END(return std::move(container))
