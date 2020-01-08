/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "AllEntriesDialog.h"
#include "PlannerApplication.h"
#include "EntryDialog.h"
#include "Entry.h"

#include <Wt/WTemplate.h>
#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>

AllEntriesDialog::AllEntriesDialog(const WString& title, CalendarCell* cell)
  : WDialog(title)
{
  WTemplate* t = contents()->addWidget(cpp14::make_unique<WTemplate>(
                                         tr("calendar.all-entries")));
  auto wc = cpp14::make_unique<WContainerWidget>();
  auto container = t->bindWidget("entries", std::move(wc));

  dbo::Session& session = PlannerApplication::plannerApplication()->session;
  dbo::Transaction transaction(session);

  typedef dbo::collection< dbo::ptr<Entry> > Entries;

  Entries entries = 
    cell->user()->entriesInRange(cell->date(), cell->date().addDays(1));

  WString format = EntryDialog::timeFormat;
  for (auto& entry : entries) {
    container->addWidget(cpp14::make_unique<WText>(entry->start.toString(format) +
			    "-" + 
			    entry->stop.toString(format) +
			    ": " + entry->summary));
  }

  transaction.commit();

  auto button = cpp14::make_unique<WPushButton>(tr("calendar.cell.all-entries.close"));
  auto buttonPtr = t->bindWidget("close", std::move(button));
  buttonPtr->clicked().connect(this, &AllEntriesDialog::closeDialog);
}

void AllEntriesDialog::closeDialog()
{
  hide();
}
