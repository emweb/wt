#include <Wt/WContainerWidget.h>

SAMPLE_BEGIN(Painting3D)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

SAMPLE_END(return std::move(container))
