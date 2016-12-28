#include <Wt/WContainerWidget.h>

SAMPLE_BEGIN(Painting3D)

auto container = cpp14::make_unique<WContainerWidget>();

SAMPLE_END(return std::move(container))
