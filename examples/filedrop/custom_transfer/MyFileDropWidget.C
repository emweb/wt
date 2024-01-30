#include "MyFileDropWidget.h"

#include <Wt/WResource.h>

MyFileDropWidget::MyFileDropWidget()
{
  setJavaScriptMember("wtUseCustomSend", "true");
  setJavaScriptMember("wtCustomSend", "function(isValid, url, upload) {"
      "console.log('Uploading file ' + upload.file.name + ' of type ' + upload.file.type);"
      "const xhr = new XMLHttpRequest();"
      "xhr.open('POST', 'https://httpbin.org/status/201');"
      "xhr.setRequestHeader('Content-Type', upload.file.type);"
      "xhr.send(upload.file);"
      "var self = " + jsRef() + ".wtLObj;"
      "self.actualSend(isValid, url, upload);"
      "}");
}
