#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WLineEdit.h>
#include <Wt/WText.h>
#include <Wt/WCheckBox.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

namespace {
#ifdef WT_EXAMPLE
    struct Employee {
	std::string firstName;
	std::string lastName;
	double pay;

        Employee(const std::string& aFirstName, 
		 const std::string& aLastName,
		 double aPay)
	  : firstName(aFirstName),
	    lastName(aLastName),
	    pay(aPay) { }
    };

    Employee employees[] = {
      Employee("Mark", "Otto", 100),
      Employee("Jacob", "Thornton", 50),
      Employee("Larry the Bird", "", 10)
    };
#endif // WT_EXAMPLE

extern 
    void addOptionToggle(WWidget *widget, const char *option,
                         const char *styleClass, WContainerWidget *parent) {
        WCheckBox *checkBox =
            parent->addWidget(cpp14::make_unique<WCheckBox>(option));
	checkBox->setInline(false);
	checkBox->changed().connect([=] {
	      widget->toggleStyleClass(styleClass, checkBox->isChecked());
	});
    }
}

SAMPLE_BEGIN(StyledTable)
auto table = cpp14::make_unique<WTable>();
auto table_ = table.get();
table_->setHeaderCount(1);

table_->elementAt(0, 0)->addWidget(cpp14::make_unique<WText>("#"));
table_->elementAt(0, 1)->addWidget(cpp14::make_unique<WText>("First Name"));
table_->elementAt(0, 2)->addWidget(cpp14::make_unique<WText>("Last Name"));
table_->elementAt(0, 3)->addWidget(cpp14::make_unique<WText>("Pay"));

for (unsigned i = 0; i < 3; ++i) {
    Employee& employee = employees[i];
    int row = i + 1;

    table_->elementAt(row,0)->
        addWidget(cpp14::make_unique<WText>(WString("{1}").arg(row)));
    table_->elementAt(row,1)->
        addWidget(cpp14::make_unique<WText>(employee.firstName));
    table_->elementAt(row,2)->
        addWidget(cpp14::make_unique<WText>(employee.lastName));
#ifndef WT_TARGET_JAVA
    table_->elementAt(row,3)->
        addWidget(cpp14::make_unique<WLineEdit>(WString("{1}").arg(employee.pay)));
#else // WT_TARGET_JAVA
    table_->elementAt(row,3)->
        addWidget(cpp14::make_unique<WLineEdit>(WString("{1}").arg(employee.pay).toUTF8()));
#endif // WT_TARGET_JAVA
}

table_->addStyleClass("table form-inline");

auto result = cpp14::make_unique<WContainerWidget>();
auto result_ = result.get();
result_->addWidget(std::move(table));

result_->addWidget(cpp14::make_unique<WText>("Options:"));

addOptionToggle(table_, "borders", "table-bordered", result_);
addOptionToggle(table_, "hover", "table-hover", result_);
addOptionToggle(table_, "condensed", "table-condensed", result_);
addOptionToggle(table_, "stripes", "table-striped", result_);

SAMPLE_END(return std::move(result))
