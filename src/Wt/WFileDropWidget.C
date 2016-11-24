#include "Wt/WFileDropWidget"

#include "Wt/WFileUpload"

namespace Wt {

WFileDropWidget::WFileDropWidget(WContainerWidget *parent)
  : WContainerWidget(parent),
    fileUpload_(new WFileUpload(this))
{
  fileUpload_->setHidden(true);
  init();
}

WFileDropWidget::WFileDropWidget(WFileUpload *upload, WContainerWidget *parent)
  : WContainerWidget(parent),
    fileUpload_(upload)
{
  init();
}

void WFileDropWidget::init()
{
  fileUpload_->registerDropWidgetInternal(this);

  addStyleClass("Wt-filedropzone");
  setHoverStyleClass("Wt-filedropzone-hover");
  
  std::string jsHandlerSetup =
    jsRef() + ".ondragenter = function(e) {"
    "  if (e.dataTransfer.items.length > 0 "
    "      && e.dataTransfer.items[0].kind == 'file')"
    "    this.setHoverStyle(true);"
    "};" +
    jsRef() + ".ondragleave = function(e) {"
    "  this.setHoverStyle(false);"
    "};" +
    jsRef() + ".ondragover = function(e) {"
    "  e.preventDefault();"
    "};";

  doJavaScript(jsHandlerSetup);
}

void WFileDropWidget::setHoverStyleClass(const std::string& className)
{
  std::string jsHoverStyleFunction =
    jsRef() + ".setHoverStyle = function(enable) {"
    "  if (enable)"
    "    $('#" + id() + "').addClass('" + className + "');"
    "  else"
    "    $('#" + id() + "').removeClass('" + className + "');"
    "}";
  doJavaScript(jsHoverStyleFunction);
}

void WFileDropWidget::removeDefaultStyling()
{
  removeStyleClass("Wt-filedropzone");
  setHoverStyleClass("");
}


void WFileDropWidget::resetUpload()
{
  WFileUpload *f = new WFileUpload(this);
  f->setHidden(true);
  setFileUpload(f);
}
  
void WFileDropWidget::setFileUpload(WFileUpload *fileUpload)
{
  if (fileUpload == fileUpload_)
    return;

  fileUpload_ = fileUpload;
  fileUpload->registerDropWidgetInternal(this);
}

}
