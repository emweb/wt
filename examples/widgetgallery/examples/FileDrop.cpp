#include <Wt/WContainerWidget.h>
#include <Wt/WFileDropWidget.h>

SAMPLE_BEGIN(FileDrop)

auto dropWidgetPtr = Wt::cpp14::make_unique<Wt::WFileDropWidget>();
auto dropWidget = dropWidgetPtr.get();

dropWidget->drop().connect([=] (const std::vector<Wt::WFileDropWidget::File*>& files) {
  const int maxFiles = 5;
  unsigned prevNbFiles = dropWidget->uploads().size() - files.size();
  for (unsigned i=0; i < files.size(); i++) {
    if (prevNbFiles + i >= maxFiles) {
      dropWidget->cancelUpload(files[i]);
      continue;
    }
    
    Wt::WContainerWidget *block = dropWidget->addNew<Wt::WContainerWidget>();
    block->setToolTip(files[i]->clientFileName());
    block->addStyleClass("upload-block spinner");
  }
  
  if (dropWidget->uploads().size() >= maxFiles)
    dropWidget->setAcceptDrops(false);
});

dropWidget->uploaded().connect([=] (Wt::WFileDropWidget::File* file) {
  std::vector<Wt::WFileDropWidget::File*> uploads = dropWidget->uploads();
  std::size_t idx = 0;
  for (; idx != uploads.size(); ++idx)
    if (uploads[idx] == file)
      break;
  dropWidget->widget(idx)->removeStyleClass("spinner");
  dropWidget->widget(idx)->addStyleClass("ready");
});

dropWidget->tooLarge().connect([=] (Wt::WFileDropWidget::File *file, uint64_t size) {
  std::vector<Wt::WFileDropWidget::File*> uploads = dropWidget->uploads();
  std::size_t idx = 0;
  for (; idx != uploads.size(); ++idx)
    if (uploads[idx] == file)
      break;
  dropWidget->widget(idx)->removeStyleClass("spinner");
  dropWidget->widget(idx)->addStyleClass("failed");
});

dropWidget->uploadFailed().connect([=] (Wt::WFileDropWidget::File *file) {
  std::vector<Wt::WFileDropWidget::File*> uploads = dropWidget->uploads();
  std::size_t idx = 0;
  for (; idx != uploads.size(); ++idx)
    if (uploads[idx] == file)
      break;
  dropWidget->widget(idx)->removeStyleClass("spinner");
  dropWidget->widget(idx)->addStyleClass("failed");
});

SAMPLE_END(return std::move(dropWidgetPtr))
