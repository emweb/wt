/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EntryDialog.h"
#include "TimeSuggestions.h"
#include "Entry.h"
#include "PlannerApplication.h"

#include <Wt/WTemplate>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>

#include <Wt/Dbo/WtSqlTraits>

using namespace Wt;
using namespace Wt::Dbo;

WString EntryDialog::timeFormat = WString::fromUTF8("HH:mm");

EntryDialog::EntryDialog(const WString& title, CalendarCell* cell) 
  : WDialog(title)
{
  cell_ = cell;

  WTemplate* t = new WTemplate(contents());
  t->setTemplateText(tr("calendar.entry"));
  
  summary_ = new WLineEdit();
  summary_->setValidator(new WValidator());
  t->bindWidget("summary", summary_);
		
  WValidator* timeValidator = 
    new WRegExpValidator("^([0-1][0-9]|[2][0-3]):([0-5][0-9])$");
  start_ = new WLineEdit();
  start_->setTextSize(5);
  start_->setValidator(timeValidator);
  t->bindWidget("start", start_);
  stop_ = new WLineEdit();
  stop_->setTextSize(5);
  stop_->setValidator(timeValidator);
  t->bindWidget("stop", stop_);
  description_ = new WTextArea();
  t->bindWidget("description", description_);
		
  TimeSuggestions* suggestions = new TimeSuggestions(contents());
  suggestions->forEdit(start_);
  suggestions->forEdit(stop_);

  WPushButton* ok = new WPushButton(tr("calendar.entry.ok"));
  t->bindWidget("ok", ok);
  ok->clicked().connect(this, &EntryDialog::ok);

  WPushButton* cancel = new WPushButton(tr("calendar.entry.cancel"));
  t->bindWidget("cancel", cancel);
  cancel->clicked().connect(this, &EntryDialog::cancel);
}

WDateTime EntryDialog::timeStamp(const WString& time, const WDate& day)
{
  WString timeStamp = day.toString("dd/MM/yyyy ");
  timeStamp += time;
  return WDateTime::fromString(timeStamp, "dd/MM/yyyy " + timeFormat);
}

WString EntryDialog::description() 
{
  return description_->text();
}

void EntryDialog::ok()
{
  Session& session = PlannerApplication::plannerApplication()->session;
  Transaction transaction(session);

  ptr<Entry> e = 
    PlannerApplication::plannerApplication()->session.add(new Entry());
  e.modify()->start = timeStamp(start_->text(), cell_->date());
  e.modify()->stop = timeStamp(stop_->text(), cell_->date());
  e.modify()->summary = summary_->text().toUTF8();
  e.modify()->text = description().toUTF8();
  e.modify()->user = cell_->user();
  
  cell_->update(cell_->user(), cell_->date());
  
  transaction.commit();

  hide();
}

void EntryDialog::cancel()
{
  hide();
}
