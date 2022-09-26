/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WFileDropWidget", function(APP, dropwidget, maxFileSize) {
  dropwidget.wtLObj = this;

  const self = this, WT = APP.WT;
  let hoverClassName = "Wt-dropzone-hover";
  const indicationClassName = "Wt-dropzone-indication";
  const dragClassName = "Wt-dropzone-dragstyle";

  const uploads = [];
  let sending = false;
  let acceptDrops = true;

  let dropIndication = false;
  let bodyDropForward = false;

  /** @type {?Worker} */
  let uploadWorker = null;
  let chunkSize = 0;

  let dragState = 0;

  const hiddenInput = document.createElement("input");
  hiddenInput.type = "file";
  hiddenInput.setAttribute("multiple", "multiple");
  hiddenInput.style.display = "none";
  dropwidget.appendChild(hiddenInput);

  const dropcover = document.createElement("div");
  dropcover.classList.add("Wt-dropcover");
  document.body.appendChild(dropcover);

  /**
   * @param {DragEvent} e The event
   */
  function eventContainsFile(e) {
    const items = e.dataTransfer?.items ?? null;
    const hasFileItem = items !== null && Array.prototype.some.call(items, (item) => item.kind === "file");
    const types = e.dataTransfer?.types ?? null;
    const hasFilesType = types !== null && types.includes("Files");
    return hasFileItem || hasFilesType;
  }

  this.validFileCheck = function(file, callback, url) {
    const reader = new FileReader();
    reader.onload = function() {
      callback(true, url);
    };
    reader.onerror = function() {
      callback(false, url);
    };
    reader.readAsText(file.slice(0, 32)); // try reading some bytes
  };

  dropwidget.setAcceptDrops = function(enable) {
    acceptDrops = enable;
  };

  dropwidget.setDropIndication = function(enable) {
    dropIndication = enable;
  };

  dropwidget.setDropForward = function(enable) {
    bodyDropForward = enable;
  };

  dropwidget.ondragenter = function(e) {
    if (!acceptDrops) {
      return;
    } else if (eventContainsFile(e)) {
      if (dragState === 0) {
        self.setPageHoverStyle();
      }

      dragState = 2;
      self.setWidgetHoverStyle(true);
    }
    e.stopPropagation();
  };

  dropwidget.ondragleave = function(e) {
    const x = e.clientX, y = e.clientY;
    let el = document.elementFromPoint(x, y);
    if (x === 0 && y === 0) { // chrome issue
      el = null;
    }

    if (el === dropcover) {
      self.setWidgetHoverStyle(false);
      dragState = 1;
      return;
    }

    self.resetDragDrop();
  };

  dropwidget.ondragover = function(e) {
    e.preventDefault();
  };

  const bodyDragEnter = function(_e) {
    if (
      !(dropIndication || bodyDropForward) ||
      WT.css(dropwidget, "display") === "none" ||
      !acceptDrops
    ) {
      return;
    }

    dragState = 1;
    self.setPageHoverStyle();
  };
  document.body.addEventListener("dragenter", bodyDragEnter);

  dropcover.ondragover = function(e) {
    e.preventDefault();
    e.stopPropagation();
  };
  dropcover.ondragleave = function(_e) {
    if (!acceptDrops || dragState !== 1) {
      return;
    }
    self.resetDragDrop();
  };
  dropcover.ondrop = function(e) {
    e.preventDefault();
    if (bodyDropForward) {
      dropwidget.ondrop(e);
    } else {
      self.resetDragDrop();
    }
  };

  dropwidget.ondrop = function(e) {
    e.preventDefault();
    if (!acceptDrops) {
      return;
    }

    self.resetDragDrop();
    if (e.dataTransfer.files.length === 0) {
      return;
    }

    self.addFiles(e.dataTransfer.files);
  };

  this.addFiles = function(filesList) {
    const newKeys = [];
    for (const file of filesList) {
      const upload = new Object();
      upload.id = Math.floor(Math.random() * Math.pow(2, 31));
      upload.file = file;

      uploads.push(upload);

      const newUpload = {};
      newUpload["id"] = upload.id;
      newUpload["filename"] = upload.file.name;
      newUpload["type"] = upload.file.type;
      newUpload["size"] = upload.file.size;

      newKeys.push(newUpload);
    }

    APP.emit(dropwidget, "dropsignal", JSON.stringify(newKeys));
  };

  dropwidget.addEventListener("click", function() {
    if (acceptDrops) {
      hiddenInput.value = "";
      hiddenInput.click();
    }
  });

  dropwidget.markForSending = function(files) {
    for (const file of files) {
      const id = file["id"];
      for (const upload of uploads) {
        if (upload.id === id) {
          upload.ready = true;
          break;
        }
      }
    }

    if (!sending) {
      if (uploads[0].ready) {
        self.requestSend();
      }
    }
  };

  this.requestSend = function() {
    if (uploads[0].skip) {
      self.uploadFinished(null);
      return;
    }

    sending = true;
    APP.emit(dropwidget, "requestsend", uploads[0].id);
  };

  dropwidget.send = function(url, useFilter) {
    const upload = uploads[0];
    if (upload.file.size > maxFileSize) {
      APP.emit(dropwidget, "filetoolarge", upload.file.size);
      self.uploadFinished(null);
      return;
    } else {
      const sendFn = (uploadWorker !== null && useFilter) ?
        self.workerSend :
        self.actualSend;
      self.validFileCheck(upload.file, sendFn, url);
    }
  };

  this.actualSend = function(isValid, url) {
    if (!isValid) {
      self.uploadFinished(null);
      return;
    }

    const xhr = new XMLHttpRequest();
    xhr.addEventListener("load", self.uploadFinished);
    xhr.addEventListener("error", self.uploadFinished);
    xhr.addEventListener("abort", self.uploadFinished);
    xhr.addEventListener("timeout", self.uploadFinished);
    xhr.open("POST", url);

    uploads[0].request = xhr;

    const fd = new FormData();
    fd.append("file-id", uploads[0].id);
    fd.append("data", uploads[0].file);
    xhr.send(fd);
  };

  this.workerSend = function(isValid, url) {
    if (!isValid) {
      self.uploadFinished(null);
      return;
    }

    uploadWorker.upload = uploads[0];
    uploadWorker.postMessage({
      "cmd": "send",
      "url": url,
      "upload": uploads[0],
      "chunksize": chunkSize,
    });
  };

  this.uploadFinished = function(e) {
    if (
      (e !== null && typeof e !== "undefined" &&
        e.type === "load" &&
        e.currentTarget.status === 200) ||
      e === true
    ) {
      APP.emit(dropwidget, "uploadfinished", uploads[0].id);
    }
    uploads.splice(0, 1);
    if (uploads[0] && uploads[0].ready) {
      self.requestSend();
    } else {
      sending = false;
      APP.emit(dropwidget, "donesending");
    }
  };

  dropwidget.cancelUpload = function(id) {
    if (uploads[0] && uploads[0].id === id) {
      uploads[0].skip = true;
      if (uploads[0].request) {
        uploads[0].request.abort();
      } else if (uploadWorker && uploadWorker.upload === uploads[0]) {
        uploadWorker.postMessage({
          "cmd": "cancel",
          "upload": uploads[0],
        });
      }
    } else {
      for (let i = 1; i < uploads.length; i++) {
        if (uploads[i].id === id) {
          uploads[i].skip = true;
        }
      }
    }
  };

  hiddenInput.onchange = function() {
    if (!acceptDrops) {
      return;
    }
    if (this.files === null || this.files.length === 0) {
      return;
    }

    self.addFiles(this.files);
  };

  this.setPageHoverStyle = function() {
    if (dropIndication || bodyDropForward) {
      dropcover.classList.add(dragClassName);
      dropwidget.classList.add(dragClassName);

      if (dropIndication) {
        dropwidget.classList.add(indicationClassName);
      }
    }
  };

  this.setWidgetHoverStyle = function(enable) {
    dropwidget.classList.toggle(hoverClassName, enable);
  };

  this.resetDragDrop = function() {
    dropwidget.classList.remove(indicationClassName);
    dropwidget.classList.remove(dragClassName);
    dropcover.classList.remove(dragClassName);
    self.setWidgetHoverStyle(false);

    dragState = 0;
  };

  dropwidget.configureHoverClass = function(className) {
    hoverClassName = className;
  };

  dropwidget.setFilters = function(acceptAttributes) {
    hiddenInput.setAttribute("accept", acceptAttributes);
  };

  dropwidget.setUploadWorker = function(url) {
    if (url && window.Worker) {
      uploadWorker = new Worker(url);
      uploadWorker.onmessage = function(e) {
        if (e.data["workerfeatures"]) {
          if (e.data["workerfeatures"] !== "valid") {
            dropwidget.setUploadWorker(null);
            APP.emit(dropwidget, "filternotsupported");
          }
        } else {
          self.uploadFinished(e.data);
        }
      };
      uploadWorker.postMessage({ "cmd": "check" });
    } else {
      uploadWorker = null;
    }
  };

  dropwidget.setChunkSize = function(size) {
    chunkSize = size;
  };

  dropwidget.destructor = function() {
    document.body.removeEventListener("dragenter", bodyDragEnter);
    document.body.removeChild(dropcover);
  };
});
