#include <Wt/WContainerWidget>
#include <Wt/WFileDropWidget>

void errHandler(Wt::WFileDropWidget *dropWidget,
		Wt::WFileDropWidget::File* file) {
  std::vector<Wt::WFileDropWidget::File*> uploads =dropWidget->uploads();
  std::vector<Wt::WFileDropWidget::File*>::iterator it =
    std::find(uploads.begin(), uploads.end(), file);
  std::size_t idx = it - uploads.begin();
  if (idx < dropWidget->count()) {
    dropWidget->widget(idx)->removeStyleClass("spinner");
    dropWidget->widget(idx)->addStyleClass("failed");
  }
};

SAMPLE_BEGIN(FileDrop)

Wt::WFileDropWidget *dropWidget = new Wt::WFileDropWidget();

dropWidget->drop().connect(std::bind([=] (const std::vector<Wt::WFileDropWidget::File*>& files) {
  const int maxFiles = 5;
  unsigned prevNbFiles = dropWidget->uploads().size() - files.size();
  for (unsigned i=0; i < files.size(); i++) {
    if (prevNbFiles + i >= maxFiles) {
      dropWidget->cancelUpload(files[i]);
      continue;
    }
    
    Wt::WContainerWidget *block = new Wt::WContainerWidget(dropWidget);
    block->setToolTip(files[i]->clientFileName());
    block->addStyleClass("upload-block spinner");
  }
  
  if (dropWidget->uploads().size() >= maxFiles)
    dropWidget->setAcceptDrops(false);
}, std::placeholders::_1));

dropWidget->uploaded().connect(std::bind([=] (Wt::WFileDropWidget::File* file) {
  std::vector<Wt::WFileDropWidget::File*> uploads =dropWidget->uploads();
  std::vector<Wt::WFileDropWidget::File*>::iterator it =
    std::find(uploads.begin(), uploads.end(), file);
  std::size_t idx = it - uploads.begin();
  if (idx < uploads.size()) {
    dropWidget->widget(idx)->removeStyleClass("spinner");
    dropWidget->widget(idx)->addStyleClass("ready");
  }
}, std::placeholders::_1));

dropWidget->tooLarge().connect(std::bind(errHandler, dropWidget, std::placeholders::_1));
dropWidget->uploadFailed().connect(std::bind(errHandler, dropWidget, std::placeholders::_1));

SAMPLE_END(return dropWidget)
