#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WFileUpload.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(FileUpload)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WFileUpload *fu = container->addNew<Wt::WFileUpload>();
fu->setFileTextSize(50); // Set the maximum file size to 50 kB.
fu->setProgressBar(Wt::cpp14::make_unique<Wt::WProgressBar>());
fu->setMargin(10, Wt::Side::Right);

// Provide a button to start uploading.
Wt::WPushButton *uploadButton = container->addNew<Wt::WPushButton>("Send");
uploadButton->setMargin(10, Wt::Side::Left | Wt::Side::Right);

Wt::WText *out = container->addNew<Wt::WText>();

// Upload when the button is clicked.
uploadButton->clicked().connect([=] {
    fu->upload();
    uploadButton->disable();
});

// Upload automatically when the user entered a file.
fu->changed().connect([=] {
    fu->upload();
    uploadButton->disable();
    out->setText("File upload is changed.");
});

// React to a succesfull upload.
fu->uploaded().connect([=] {
    out->setText("File upload is finished.");
});

// React to a file upload problem.
fu->fileTooLarge().connect([=] {
    out->setText("File is too large.");
});

SAMPLE_END(return std::move(container))
