#include <Wt/WGroupBox>
#include <Wt/WText>

SAMPLE_BEGIN(GroupBox)
Wt::WGroupBox *groupBox = new Wt::WGroupBox("A group box");
groupBox->addStyleClass("centered-example");
groupBox->addWidget(new Wt::WText("<p>Some contents.</p>"));
groupBox->addWidget(new Wt::WText("<p>More contents.</p>"));

SAMPLE_END(return groupBox)
