#include <Wt/WGroupBox.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(GroupBox)
auto groupBox = std::make_unique<Wt::WGroupBox>("A group box");
groupBox->addStyleClass("centered-example");
groupBox->addNew<Wt::WText>("<p>Some contents.</p>");
groupBox->addNew<Wt::WText>("<p>More contents.</p>");

SAMPLE_END(return std::move(groupBox))
