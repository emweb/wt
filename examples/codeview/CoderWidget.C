/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WText.h>

#include "CoderWidget.h"

using namespace Wt;

class BufferEditorWidget : public WContainerWidget
{
public:
  BufferEditorWidget()
  {
    setStyleClass("editor");

    this->addWidget(cpp14::make_unique<WText>("File: "));
    nameEdit_ = this->addWidget(cpp14::make_unique<WLineEdit>());

    textArea_ = this->addWidget(cpp14::make_unique<WTextArea>());
    textArea_->setAttributeValue("spellcheck", "false");
    textArea_->setInline(false);
    textArea_->setColumns(80);
    textArea_->resize(WLength::Auto, 300);
  }

  WString name() { return nameEdit_->text(); }
  WString text() {
    std::u32string t = textArea_->text();

    if (textArea_->hasFocus()) {
      if (textArea_->hasSelectedText()) {
	int i = textArea_->selectionStart();
        int j = ((std::u32string) textArea_->selectedText()).length();

        t = escape(t.substr(0, i)) + U"<span class=\"sel\">"
          + escape(t.substr(i, j)) + U"</span>"
	  + escape(t.substr(i + j));
      } else {
	int i = textArea_->cursorPosition();
	if (i >= 0) {
          std::u32string s;
          s = escape(t.substr(0, i)) + U"<span class=\"pos\">";

	  if (i + 1 < (int)t.length()) {
	    if (t[i] == '\n')
              s += U" ";
            s += escape(t.substr(i, 1)) + U"</span>";
	    s += escape(t.substr(i + 1));
	  } else
            s += U" </span>";

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

  std::u32string escape(const std::u32string& s) {
    return (std::u32string) escapeText(std::u32string(s));
  }
};

CoderWidget::CoderWidget()
{
  WApplication::instance()->enableUpdates(true);

  session_ = std::make_shared<CodeSession>(std::bind(&CoderWidget::sessionChanged, this));
  CodeSession::addSession(session_);

  WApplication::instance()->setInternalPath("/" + session_->id());

  auto *addBuffer = this->addWidget(cpp14::make_unique<WPushButton>("Add file"));
  observerCount_ = this->addWidget(cpp14::make_unique<WText>("Observers: 0"));

  addBuffer->clicked().connect(this, &CoderWidget::addBuffer);

  buffers_ = this->addWidget(cpp14::make_unique<WContainerWidget>());

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

  std::unique_ptr<BufferEditorWidget> editor(cpp14::make_unique<BufferEditorWidget>());
  editor->keyWentUp().connect(std::bind(&CoderWidget::changed, this, editor.get()));
  editor->clicked().connect(std::bind(&CoderWidget::changed, this, editor.get()));
  editor->textArea()->blurred()
    .connect(std::bind(&CoderWidget::changed, this, editor.get()));

  buffers_->insertWidget(index, std::move(editor));
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
