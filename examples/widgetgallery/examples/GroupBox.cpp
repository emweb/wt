#include <Wt/WGroupBox.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(GroupBox)
auto groupBox = Wt::cpp14::make_unique<Wt::WGroupBox>("A group box");
groupBox->addStyleClass("centered-example");
groupBox->addWidget(Wt::cpp14::make_unique<Wt::WText>("<p>Some contents.</p>"));
groupBox->addWidget(Wt::cpp14::make_unique<Wt::WText>("<p>More contents.</p>"));

SAMPLE_END(return std::move(groupBox))
