#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WLineEdit.h>
#include <Wt/WText.h>

namespace {
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
}

SAMPLE_BEGIN(PlainTable)
auto table = cpp14::make_unique<WTable>();
table->setHeaderCount(1);
table->setWidth(WLength("100%"));

table->elementAt(0, 0)->addWidget(cpp14::make_unique<WText>("#"));
table->elementAt(0, 1)->addWidget(cpp14::make_unique<WText>("First Name"));
table->elementAt(0, 2)->addWidget(cpp14::make_unique<WText>("Last Name"));
table->elementAt(0, 3)->addWidget(cpp14::make_unique<WText>("Pay"));

for (unsigned i = 0; i < 3; ++i) {
    Employee& employee = employees[i];
    int row = i + 1;

    table->elementAt(row, 0)
        ->addWidget(cpp14::make_unique<WText>(WString("{1}")
				  .arg(row)));
    table->elementAt(row, 1)
        ->addWidget(cpp14::make_unique<WText>(employee.firstName));
    table->elementAt(row, 2)
        ->addWidget(cpp14::make_unique<WText>(employee.lastName));
    table->elementAt(row, 3)
        ->addWidget(cpp14::make_unique<WLineEdit>(WString("{1}")
				      .arg(employee.pay)));
}

SAMPLE_END(return std::move(table))
