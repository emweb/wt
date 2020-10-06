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
        WCheckBox *checkBox = parent->addNew<WCheckBox>(option);
	checkBox->setInline(false);
	checkBox->changed().connect([=] {
	      widget->toggleStyleClass(styleClass, checkBox->isChecked());
	});
    }
}

SAMPLE_BEGIN(StyledTable)
auto table = std::make_unique<WTable>();
auto table_ = table.get();
table_->setHeaderCount(1);

table_->elementAt(0, 0)->addNew<WText>("#");
table_->elementAt(0, 1)->addNew<WText>("First Name");
table_->elementAt(0, 2)->addNew<WText>("Last Name");
table_->elementAt(0, 3)->addNew<WText>("Pay");

for (unsigned i = 0; i < 3; ++i) {
    Employee& employee = employees[i];
    int row = i + 1;

    table_->elementAt(row,0)->
        addNew<WText>(WString("{1}").arg(row));
    table_->elementAt(row,1)->
        addNew<WText>(employee.firstName);
    table_->elementAt(row,2)->
        addNew<WText>(employee.lastName);
#ifndef WT_TARGET_JAVA
    table_->elementAt(row,3)->
        addNew<WLineEdit>(WString("{1}").arg(employee.pay));
#else // WT_TARGET_JAVA
    table_->elementAt(row,3)->
        addNew<WLineEdit>(WString("{1}").arg(employee.pay).toUTF8());
#endif // WT_TARGET_JAVA
}

table_->addStyleClass("table form-inline");

#ifndef WT_TARGET_JAVA
auto resultPtr = std::make_unique<WContainerWidget>();
auto result = resultPtr.get();
#else // WT_TARGET_JAVA
auto result = new WContainerWidget();
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
result->addWidget(std::move(table));
#else // WT_TARGET_JAVA
result->addWidget(std::unique_ptr<Wt::WWidget>(table));
#endif // WT_TARGET_JAVA

result->addNew<WText>("Options:");

addOptionToggle(table_, "borders", "table-bordered", result);
addOptionToggle(table_, "hover", "table-hover", result);
addOptionToggle(table_, "condensed", "table-condensed", result);
addOptionToggle(table_, "stripes", "table-striped", result);

#ifndef WT_TARGET_JAVA
SAMPLE_END(return std::move(resultPtr))
#else // WT_TARGET_JAVA
SAMPLE_END(return std::move(result))
#endif // WT_TARGET_JAVA
