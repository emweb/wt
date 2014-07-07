#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WLineEdit>
#include <Wt/WText>

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
Wt::WTable *table = new Wt::WTable();
table->setHeaderCount(1);
table->setWidth(Wt::WLength("100%"));

table->elementAt(0, 0)->addWidget(new Wt::WText("#"));
table->elementAt(0, 1)->addWidget(new Wt::WText("First Name"));
table->elementAt(0, 2)->addWidget(new Wt::WText("Last Name"));
table->elementAt(0, 3)->addWidget(new Wt::WText("Pay"));

for (unsigned i = 0; i < 3; ++i) {
    Employee& employee = employees[i];
    int row = i + 1;

    table->elementAt(row, 0)
        ->addWidget(new Wt::WText(Wt::WString::fromUTF8("{1}")
				  .arg(row)));
    table->elementAt(row, 1)
        ->addWidget(new Wt::WText(employee.firstName));
    table->elementAt(row, 2)
        ->addWidget(new Wt::WText(employee.lastName));
    table->elementAt(row, 3)
        ->addWidget(new Wt::WLineEdit(Wt::WString::fromUTF8("{1}")
				      .arg(employee.pay)));
}

SAMPLE_END(return table)
