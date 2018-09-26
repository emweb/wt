#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(SizingRelative)
auto container = cpp14::make_unique<WContainerWidget>();

WLineEdit *edit =
    container->addWidget(cpp14::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-mini");
edit->setStyleClass("input-mini");
container->addWidget(cpp14::make_unique<WBreak>());

edit =
    container->addWidget(cpp14::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-small");
edit->setStyleClass("input-small");
container->addWidget(cpp14::make_unique<WBreak>());

edit =
    container->addWidget(cpp14::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-medium");
edit->setStyleClass("input-medium");
container->addWidget(cpp14::make_unique<WBreak>());

edit =
    container->addWidget(cpp14::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-large");
edit->setStyleClass("input-large");
container->addWidget(cpp14::make_unique<WBreak>());

edit =
    container->addWidget(cpp14::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-xlarge");
edit->setStyleClass("input-xlarge");
container->addWidget(cpp14::make_unique<WBreak>());

edit =
    container->addWidget(cpp14::make_unique<WLineEdit>());
edit->setPlaceholderText(".input-xxlarge");
edit->setStyleClass("input-xxlarge");

SAMPLE_END(return container)
