#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WLineEdit>
#include <Wt/WText>
#include <Wt/WCheckBox>

namespace {

extern 
    void addOptionToggle(Wt::WWidget *widget, const char *option, 
			 const char *styleClass, Wt::WContainerWidget *parent) {
  	Wt::WCheckBox *checkBox = new Wt::WCheckBox(option, parent);
	checkBox->setInline(false);
	checkBox->changed().connect(std::bind([=] () {
	      widget->toggleStyleClass(styleClass, checkBox->isChecked());
	}));
    }
}

SAMPLE_BEGIN(StyledTable)
Wt::WTable *table = new Wt::WTable();
table->setHeaderCount(1);

table->elementAt(0, 0)->addWidget(new Wt::WText("#"));
table->elementAt(0, 1)->addWidget(new Wt::WText("First Name"));
table->elementAt(0, 2)->addWidget(new Wt::WText("Last Name"));
table->elementAt(0, 3)->addWidget(new Wt::WText("Pay"));

for (unsigned i = 0; i < 3; ++i) {
    Employee& employee = employees[i];
    int row = i + 1;

    new Wt::WText(Wt::WString::fromUTF8("{1}").arg(row),
		  table->elementAt(row, 0));
    new Wt::WText(employee.firstName, table->elementAt(row, 1));
    new Wt::WText(employee.lastName, table->elementAt(row, 2));
    new Wt::WLineEdit(Wt::WString::fromUTF8("{1}").arg(employee.pay),
		      table->elementAt(row, 3));
}

table->addStyleClass("table form-inline");

Wt::WContainerWidget *result = new Wt::WContainerWidget();
result->addWidget(table);

new Wt::WText("Options:", result);

addOptionToggle(table, "borders", "table-bordered", result);
addOptionToggle(table, "hover", "table-hover", result);
addOptionToggle(table, "condensed", "table-condensed", result);
addOptionToggle(table, "stripes", "table-striped", result);

SAMPLE_END(return result)
