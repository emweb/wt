#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(SizingRelative)
auto container = std::make_unique<WContainerWidget>();

WLineEdit *edit =
    container->addWidget(std::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-mini");
edit->setStyleClass("input-mini");
container->addWidget(std::make_unique<WBreak>());

edit =
    container->addWidget(std::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-small");
edit->setStyleClass("input-small");
container->addWidget(std::make_unique<WBreak>());

edit =
    container->addWidget(std::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-medium");
edit->setStyleClass("input-medium");
container->addWidget(std::make_unique<WBreak>());

edit =
    container->addWidget(std::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-large");
edit->setStyleClass("input-large");
container->addWidget(std::make_unique<WBreak>());

edit =
    container->addWidget(std::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-xlarge");
edit->setStyleClass("input-xlarge");
container->addWidget(std::make_unique<WBreak>());

edit =
    container->addWidget(std::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-xxlarge");
edit->setStyleClass("input-xxlarge");

SAMPLE_END(return container)
