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
auto table = Wt::cpp14::make_unique<Wt::WTable>();
table->setHeaderCount(1);
table->setWidth(Wt::WLength("100%"));

table->elementAt(0, 0)->addWidget(Wt::cpp14::make_unique<Wt::WText>("#"));
table->elementAt(0, 1)->addWidget(Wt::cpp14::make_unique<Wt::WText>("First Name"));
table->elementAt(0, 2)->addWidget(Wt::cpp14::make_unique<Wt::WText>("Last Name"));
table->elementAt(0, 3)->addWidget(Wt::cpp14::make_unique<Wt::WText>("Pay"));

for (unsigned i = 0; i < 3; ++i) {
    Employee& employee = employees[i];
    int row = i + 1;

    table->elementAt(row, 0)
        ->addWidget(Wt::cpp14::make_unique<Wt::WText>(Wt::WString("{1}")
				  .arg(row)));
    table->elementAt(row, 1)
        ->addWidget(Wt::cpp14::make_unique<Wt::WText>(employee.firstName));
    table->elementAt(row, 2)
        ->addWidget(Wt::cpp14::make_unique<Wt::WText>(employee.lastName));
    table->elementAt(row, 3)
        ->addWidget(Wt::cpp14::make_unique<Wt::WLineEdit>(Wt::WString("{1}")
				      .arg(employee.pay)));
}

SAMPLE_END(return std::move(table))
