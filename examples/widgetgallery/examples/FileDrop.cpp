#include <Wt/WContainerWidget.h>
#include <Wt/WFileDropWidget.h>

SAMPLE_BEGIN(FileDrop)

auto dropWidgetPtr = Wt::cpp14::make_unique<Wt::WFileDropWidget>();
auto dropWidget = dropWidgetPtr.get();

dropWidget->drop().connect(std::bind([=] (const std::vector<Wt::WFileDropWidget::File*>& files) {
  const int maxFiles = 5;
  unsigned prevNbFiles = dropWidget->uploads().size() - files.size();
  for (unsigned i=0; i < files.size(); i++) {
    if (prevNbFiles + i >= maxFiles) {
      dropWidget->cancelUpload(files[i]);
      continue;
    }
    
    Wt::WContainerWidget *block = dropWidget->addWidget(Wt::cpp14::make_unique<Wt::WContainerWidget>());
    block->setToolTip(files[i]->clientFileName());
    block->addStyleClass("upload-block spinner");
  }
  
  if (dropWidget->uploads().size() >= maxFiles)
    dropWidget->setAcceptDrops(false);
}, std::placeholders::_1));

dropWidget->uploaded().connect(std::bind([=] (Wt::WFileDropWidget::File* file) {
  std::vector<Wt::WFileDropWidget::File*> uploads = dropWidget->uploads();
  std::size_t idx = 0;
  for (; idx != uploads.size(); ++idx)
    if (uploads[idx] == file)
      break;
  dropWidget->widget(idx)->removeStyleClass("spinner");
  dropWidget->widget(idx)->addStyleClass("ready");
}, std::placeholders::_1));

#ifndef WT_TARGET_JAVA
dropWidget->tooLarge().connect(std::bind([=] (Wt::WFileDropWidget::File *file) {
#else
dropWidget->tooLarge().connect(std::bind([=] (Wt::WFileDropWidget::File *file, uint64_t size) {
#endif
  std::vector<Wt::WFileDropWidget::File*> uploads = dropWidget->uploads();
  std::size_t idx = 0;
  for (; idx != uploads.size(); ++idx)
    if (uploads[idx] == file)
      break;
  dropWidget->widget(idx)->removeStyleClass("spinner");
  dropWidget->widget(idx)->addStyleClass("failed");
#ifndef WT_TARGET_JAVA
}, std::placeholders::_1));
#else
}, std::placeholders::_1, std::placeholders::_2));
#endif

dropWidget->uploadFailed().connect(std::bind([=] (Wt::WFileDropWidget::File *file) {
  std::vector<Wt::WFileDropWidget::File*> uploads = dropWidget->uploads();
  std::size_t idx = 0;
  for (; idx != uploads.size(); ++idx)
    if (uploads[idx] == file)
      break;
  dropWidget->widget(idx)->removeStyleClass("spinner");
  dropWidget->widget(idx)->addStyleClass("failed");
}, std::placeholders::_1));

SAMPLE_END(return std::move(dropWidgetPtr))
