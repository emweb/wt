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

table->elementAt(0, 0)->addNew<Wt::WText>("#");
table->elementAt(0, 1)->addNew<Wt::WText>("First Name");
table->elementAt(0, 2)->addNew<Wt::WText>("Last Name");
table->elementAt(0, 3)->addNew<Wt::WText>("Pay");

for (unsigned i = 0; i < 3; ++i) {
    Employee& employee = employees[i];
    int row = i + 1;

    table->elementAt(row, 0)
        ->addNew<Wt::WText>(Wt::WString("{1}").arg(row));
    table->elementAt(row, 1)
        ->addNew<Wt::WText>(employee.firstName);
    table->elementAt(row, 2)
        ->addNew<Wt::WText>(employee.lastName);
#ifndef WT_TARGET_JAVA
    table->elementAt(row, 3)
        ->addNew<Wt::WLineEdit>(Wt::WString("{1}").arg(employee.pay));
#else // WT_TARGET_JAVA
    table->elementAt(row, 3)
        ->addNew<Wt::WLineEdit>(Wt::WString("{1}").arg(employee.pay).toUTF8());
#endif // WT_TARGET_JAVA
}

SAMPLE_END(return std::move(table))
