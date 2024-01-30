#include "MyUploadResource.h"

#include <Wt/Http/Response.h>
#include <Wt/Utils.h>
#include <Wt/WApplication.h>

#include <fstream>

MyUploadResource::MyUploadResource(Wt::WFileDropWidget *fileDropWidget, Wt::WFileDropWidget::File *file)
  : Wt::WResource(),
    parent_(fileDropWidget),
    currentFile_(file)
{
  setUploadProgress(true);
}

void MyUploadResource::handleRequest(const Wt::Http::Request &request,
                                     Wt::Http::Response &response)
{
  Wt::WApplication::UpdateLock lock(Wt::WApplication::instance());
  const Wt::Http::UploadedFile* file = request.getUploadedFile("data");
  if (!file) {
    response.setStatus(404);
    return;
  }
  Wt::log("info") << request.hostName() << " (" << request.clientAddress() << ") uploads "
                  << request.contentLength() << " bytes of type " << request.contentType() 
                  << " to " << request.path() << ". " << file->clientFileName() 
                  << " will be stored in " << file->spoolFileName();
  currentFile_->handleIncomingData(*file, true);
  parent_->proceedToNextFile();
  response.setMimeType("text/plain");
}
