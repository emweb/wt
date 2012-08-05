/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "CalendarCell.h"
#include "EntryDialog.h"
#include "AllEntriesDialog.h"
#include "PlannerApplication.h"
#include "Entry.h"

#include <boost/lexical_cast.hpp>

#include <Wt/WDate>
#include <Wt/WText>

using namespace Wt;

CalendarCell::CalendarCell()
  : WContainerWidget()
{
  resize(100, 120);

  setStyleClass("cell");
  setToolTip(tr("calendar.cell.tooltip"));

  clicked().connect(this, &CalendarCell::showEntryDialog);
}

void CalendarCell::update(const dbo::ptr<UserAccount>& user, const WDate& date)
{
  date_ = date;
  user_ = user;

  clear();

  dbo::Session& session = PlannerApplication::plannerApplication()->session;
  dbo::Transaction transaction(session);
  
  WString day;
  day += boost::lexical_cast<std::string>(date.day());
  if (date.day() == 1)
    day += " " + WDate::longMonthName(date.month());
  WText* header = new WText(day);
  header->setStyleClass("cell-header");
  addWidget(header);

  typedef dbo::collection< dbo::ptr<Entry> > Entries;
  Entries entries = user->entriesInRange(date, date.addDays(1));

  const unsigned maxEntries = 4;
  unsigned counter = 0;
  for (Entries::const_iterator i = entries.begin();
       i != entries.end(); ++i, ++counter) {
    if (counter == maxEntries) {
      WText* extra = 
	new WText(tr("calendar.cell.extra")
		  .arg((int)(entries.size() - maxEntries)));
      extra->setStyleClass("cell-extra");
      addWidget(extra);

      extra->clicked().preventPropagation();
      extra->clicked().connect(this, &CalendarCell::showAllEntriesDialog);
      
      break;
    }

    WString format = EntryDialog::timeFormat;
    addWidget(new WText((*i)->start.toString(format) +
			"-" + 
			(*i)->stop.toString(format) + 
			": " + (*i)->summary));
  }

  transaction.commit();
}

void CalendarCell::showEntryDialog()
{
  WString title =
    tr("calendar.entry.title").arg(date_.toString("ddd, d MMM yyyy"));

  EntryDialog* ed = new EntryDialog(title, this);
  ed->show();
}

void CalendarCell::showAllEntriesDialog()
{
  WString title =
    tr("calendar.cell.all-entries.title")
    .arg(date_.toString("ddd, d MMM yyyy"));
  
  AllEntriesDialog* dialog = new AllEntriesDialog(title, this);
  dialog->show();
}
