/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ObserverWidget.h"

#include <Wt/WApplication>
#include <Wt/WText>

using namespace Wt;

class BufferViewWidget : public WContainerWidget
{
public:
  BufferViewWidget()
  {
    setStyleClass("viewer");

    WApplication::instance()->require("prettify/prettify.min.js");
    WApplication::instance()->useStyleSheet("prettify/prettify.css");

    new WText("File: ", this);
    bufferName_ = new WText(this);

    bufferText_ = new WText(this);
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
    (id, boost::bind(&ObserverWidget::updateBuffer, this, _1, _2));

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
  BufferViewWidget *w = new BufferViewWidget();
  w->setName(buffer.name);
  w->setText(buffer.text);

  insertWidget(i, w);
}

void ObserverWidget::updateBuffer(int buffer, CodeSession::BufferUpdate update)
{
  switch (update) {
  case CodeSession::Inserted:
    insertBuffer(session_->buffer(buffer), buffer);
    break;
  case CodeSession::Deleted:
    delete widget(buffer);
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
