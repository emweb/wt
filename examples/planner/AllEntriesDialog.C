/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "AllEntriesDialog.h"
#include "PlannerApplication.h"
#include "EntryDialog.h"
#include "Entry.h"

#include <Wt/WTemplate>
#include <Wt/WString>
#include <Wt/WText>
#include <Wt/WPushButton>

using namespace Wt;

AllEntriesDialog::AllEntriesDialog(const WString& title, CalendarCell* cell)
  : WDialog(title)
{
  WTemplate* t = new WTemplate(tr("calendar.all-entries"), contents());
  WContainerWidget* wc = new WContainerWidget();
  t->bindWidget("entries", wc);

  dbo::Session& session = PlannerApplication::plannerApplication()->session;
  dbo::Transaction transaction(session);

  typedef dbo::collection< dbo::ptr<Entry> > Entries;

  Entries entries = 
    cell->user()->entriesInRange(cell->date(), cell->date().addDays(1));

  WString format = EntryDialog::timeFormat;
  for (Entries::const_iterator i = entries.begin(); i != entries.end(); ++i) {
    wc->addWidget(new WText((*i)->start.toString(format) + 
			    "-" + 
			    (*i)->stop.toString(format) + 
			    ": " + (*i)->summary));
  }

  transaction.commit();

  WPushButton* button = new WPushButton(tr("calendar.cell.all-entries.close"));
  t->bindWidget("close", button);
  button->clicked().connect(this, &AllEntriesDialog::closeDialog);
}

void AllEntriesDialog::closeDialog()
{
  hide();
}
