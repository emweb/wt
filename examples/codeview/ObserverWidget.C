/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ObserverWidget.h"

#include <Wt/WApplication.h>
#include <Wt/WText.h>

using namespace Wt;

class BufferViewWidget : public WContainerWidget
{
public:
  BufferViewWidget()
  {
    setStyleClass("viewer");

    WApplication::instance()->require("prettify/prettify.min.js");
    WApplication::instance()->useStyleSheet("prettify/prettify.css");

    this->addWidget(cpp14::make_unique<WText>("File: "));
    bufferName_ = this->addWidget(cpp14::make_unique<WText>());

    bufferText_ = this->addWidget(cpp14::make_unique<WText>());
    bufferText_->setInline(false);
    bufferText_->setStyleClass("prettyprint");
  }

  void setName(const WString& name) {
    bufferName_->setText(name);
  }

  void setText(const WString& text) {
    WApplication::instance()->doJavaScript
      (bufferText_->jsRef() + ".innerHTML="
       "'<pre class=\"prettyprint\">' + prettyPrintOne("
       + text.jsStringLiteral() + ", " + bufferText_->jsRef()
       + ") + '</pre>';");
  }

private:
  WText *bufferName_;
  WText *bufferText_;
};

ObserverWidget::ObserverWidget(const std::string& id)
{
  WApplication::instance()->enableUpdates(true);

  session_ = CodeSession::addObserver
    (id, std::bind(&ObserverWidget::updateBuffer, this, std::placeholders::_1, std::placeholders::_2));

  if (session_) {
    std::vector<CodeSession::Buffer> buffers = session_->buffers();

    for (unsigned i = 0; i < buffers.size(); ++i)
      insertBuffer(buffers[i], i);
  }
}

ObserverWidget::~ObserverWidget()
{
  if (session_)
    session_->removeObserver();

  WApplication::instance()->enableUpdates(false);
}

void ObserverWidget::insertBuffer(const CodeSession::Buffer& buffer, int i)
{
  std::unique_ptr<BufferViewWidget> w(cpp14::make_unique<BufferViewWidget>());
  w->setName(buffer.name);
  w->setText(buffer.text);

  insertWidget(i, std::move(w));
}

void ObserverWidget::updateBuffer(int buffer, CodeSession::BufferUpdate update)
{
  switch (update) {
  case CodeSession::Inserted:
    insertBuffer(session_->buffer(buffer), buffer);
    break;
  case CodeSession::Deleted:
    this->removeWidget(widget(buffer));
    break;
  case CodeSession::Changed:
    {
      BufferViewWidget *w = dynamic_cast<BufferViewWidget *>(widget(buffer));
      w->setName(session_->buffer(buffer).name);
      w->setText(session_->buffer(buffer).text);
    }
  }

  WApplication::instance()->triggerUpdate();
}
