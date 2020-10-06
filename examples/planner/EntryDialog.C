/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EntryDialog.h"
#include "TimeSuggestions.h"
#include "Entry.h"
#include "PlannerApplication.h"

#include <Wt/WTemplate.h>
#include <Wt/WPushButton.h>
#include <Wt/WRegExpValidator.h>

#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;
using namespace Wt::Dbo;

WString EntryDialog::timeFormat = WString("HH:mm");

EntryDialog::EntryDialog(const WString& title, CalendarCell* cell) 
  : WDialog(title)
{
  cell_ = cell;

  WTemplate* t = contents()->addWidget(std::make_unique<WTemplate>());
  t->setTemplateText(tr("calendar.entry"));
  
  auto summaryPtr = std::make_unique<WLineEdit>();
  summary_ = t->bindWidget("summary", std::move(summaryPtr));
  summary_->setValidator(std::make_shared<WValidator>());
		
  auto timeValidator =
    std::make_shared<WRegExpValidator>("^([0-1][0-9]|[2][0-3]):([0-5][0-9])$");
  auto startPtr = std::make_unique<WLineEdit>();
  start_ = t->bindWidget("start", std::move(startPtr));
  start_->setTextSize(5);
  start_->setValidator(timeValidator);

  auto stopPtr = std::make_unique<WLineEdit>();
  stop_ = t->bindWidget("stop", std::move(stopPtr));
  stop_->setTextSize(5);
  stop_->setValidator(timeValidator);

  auto descriptionPtr = std::make_unique<WTextArea>();
  description_ = t->bindWidget("description", std::move(descriptionPtr));
		
  TimeSuggestions* suggestions = contents()->addChild(std::make_unique<TimeSuggestions>());
  suggestions->forEdit(start_);
  suggestions->forEdit(stop_);

  auto okPtr = std::make_unique<WPushButton>(tr("calendar.entry.ok"));
  auto ok = t->bindWidget("ok", std::move(okPtr));
  ok->clicked().connect(this, &EntryDialog::ok);

  auto cancelPtr = std::make_unique<WPushButton>(tr("calendar.entry.cancel"));
  auto cancel = t->bindWidget("cancel", std::move(cancelPtr));
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
    PlannerApplication::plannerApplication()->session.add(
        std::make_unique<Entry>());
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
