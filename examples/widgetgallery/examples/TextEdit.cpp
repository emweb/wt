#include <Wt/Utils.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTextEdit.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(TextEdit)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WTextEdit *edit = container->addNew<Wt::WTextEdit>();
edit->setHeight(300);
edit->setText("<p>"
    "<span style=\"font-family: 'courier new', courier; font-size: medium;\">"
    "<strong>WTextEdit</strong></span></p>"
    "<p>Hey, I'm a <strong>WTextEdit</strong> and you can make me"
        " <span style=\"text-decoration: underline;\"><em>rich</em></span>"
        " by adding your <span style=\"color: #ff0000;\"><em>style</em>"
	"</span>!</p>"
    "<p>Other widgets like...</p>"
      "<ul style=\"padding: 0px; margin: 0px 0px 10px 25px;\">"
        "<li>WLineEdit</li>"
        "<li>WTextArea</li>"
        "<li>WSpinBox</li>"
      "</ul>"
    "<p>don't have style.</p>");

Wt::WPushButton *button = container->addNew<Wt::WPushButton>("Get text");
button->setMargin(10, Wt::Side::Top | Wt::Side::Bottom);

Wt::WText *out = container->addNew<Wt::WText>();
out->setStyleClass("xhtml-output");

button->clicked().connect([=] {
    out->setText("<pre>" + Wt::Utils::htmlEncode(edit->text()) + "</pre>");
});

SAMPLE_END(return std::move(container))
