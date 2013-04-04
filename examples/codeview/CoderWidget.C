/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WTextArea>
#include <Wt/WText>

#include "CoderWidget.h"

using namespace Wt;

class BufferEditorWidget : public WContainerWidget
{
public:
  BufferEditorWidget()
  {
    setStyleClass("editor");

    new WText("File: ", this);
    nameEdit_ = new WLineEdit(this);

    textArea_ = new WTextArea(this);
    textArea_->setAttributeValue("spellcheck", "false");
    textArea_->setInline(false);
    textArea_->setColumns(80);
    textArea_->resize(WLength::Auto, 300);
  }

  WString name() { return nameEdit_->text(); }
  WString text() {
    std::wstring t = textArea_->text();

    if (textArea_->hasFocus()) {
      if (textArea_->hasSelectedText()) {
	int i = textArea_->selectionStart();
	int j = ((std::wstring) textArea_->selectedText()).length();

	t = escape(t.substr(0, i)) + L"<span class=\"sel\">"
	  + escape(t.substr(i, j)) + L"</span>"
	  + escape(t.substr(i + j));
      } else {
	int i = textArea_->cursorPosition();
	if (i >= 0) {
	  std::wstring s;
	  s = escape(t.substr(0, i)) + L"<span class=\"pos\">";

	  if (i + 1 < (int)t.length()) {
	    if (t[i] == '\n')
	      s += L' ';
	    s += escape(t.substr(i, 1)) + L"</span>";
	    s += escape(t.substr(i + 1));
	  } else
	    s += L" </span>";

	  t = s;
	}
      }
    } else
      t = escape(t);

    return t; 
  }

  WTextArea *textArea() { return textArea_; }

private:
  WLineEdit *nameEdit_;
  WTextArea *textArea_;

  std::wstring escape(const std::wstring& s) {
    return (std::wstring) escapeText(WString(s));
  }
};

CoderWidget::CoderWidget()
{
  WApplication::instance()->enableUpdates(true);

  session_ = new CodeSession(boost::bind(&CoderWidget::sessionChanged, this));

  WApplication::instance()->setInternalPath("/" + session_->id());

  WPushButton *addBuffer = new WPushButton("Add file", this);
  observerCount_ = new WText("Observers: 0", this);

  addBuffer->clicked().connect(this, &CoderWidget::addBuffer);

  buffers_ = new WContainerWidget(this);

  insertBuffer(0);
}

CoderWidget::~CoderWidget()
{
  session_->removeCoder();
  WApplication::instance()->enableUpdates(false);
}

void CoderWidget::addBuffer()
{
  insertBuffer(buffers_->count());
}

void CoderWidget::insertBuffer(int index)
{
  session_->insertBuffer(index);

  BufferEditorWidget *editor = new BufferEditorWidget();
  editor->keyWentUp().connect(boost::bind(&CoderWidget::changed, this, editor));
  editor->clicked().connect(boost::bind(&CoderWidget::changed, this, editor));
  editor->textArea()->blurred()
    .connect(boost::bind(&CoderWidget::changed, this, editor));

  buffers_->insertWidget(index, editor);
}

void CoderWidget::changed(BufferEditorWidget *editor)
{
  session_->updateBuffer(buffers_->indexOf(editor), editor->name(),
			 editor->text());
}

void CoderWidget::sessionChanged()
{
  observerCount_->setText(WString("Observers: {1}")
			  .arg(session_->observerCount()));
  WApplication::instance()->triggerUpdate();
}
