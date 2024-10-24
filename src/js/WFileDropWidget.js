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

  let acceptDirectories = false;
  let acceptDirectoriesRecursive = false;

  let dropIndication = false;
  let bodyDropForward = false;

  let filePickerType = "file-selection"; // default for an <input type="file">

  /** @type {?Worker} */
  let uploadWorker = null;
  let chunkSize = 0;

  let dragState = 0;

  // input-tag used to redirect a click event on the drop-widget
  const hiddenInput = document.createElement("input");
  hiddenInput.type = "file";
  hiddenInput.setAttribute("multiple", "multiple");
  hiddenInput.style.display = "none";
  dropwidget.hiddenInput = hiddenInput;
  dropwidget.appendChild(hiddenInput);

  // input-tags that are used by the server to open file/dir-picker
  // Note: these nodes are added to the body because otherwise they also trigger hiddenInput
  const serverFileInput = document.createElement("input");
  serverFileInput.type = "file";
  serverFileInput.setAttribute("multiple", "multiple");
  serverFileInput.style.display = "none";
  window.document.body.appendChild(serverFileInput);
  dropwidget.serverFileInput = serverFileInput;
  const serverDirInput = document.createElement("input");
  serverDirInput.type = "file";
  serverDirInput.setAttribute("multiple", "multiple");
  serverDirInput.setAttribute("webkitdirectory", "webkitdirectory");
  serverDirInput.style.display = "none";
  window.document.body.appendChild(serverDirInput);
  dropwidget.serverDirInput = serverDirInput;

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

  this.validFileCheck = function(upload, callback, url) {
    const reader = new FileReader();
    reader.onload = function() {
      callback(true, url, upload);
    };
    reader.onerror = function() {
      callback(false, url, upload);
    };
    reader.readAsText(upload.file.slice(0, 32)); // try reading some bytes
  };

  dropwidget.setAcceptDrops = function(enable) {
    acceptDrops = enable;
  };

  dropwidget.setAcceptDirectories = function(enable, recursive) {
    acceptDirectories = enable;
    acceptDirectoriesRecursive = recursive;
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

    self.addDataTransferItems(Array.from(e.dataTransfer.items));
  };

  this.addDataTransferItems = async function(itemsList) {
    const newKeys = [];
    const wkItems = itemsList.map((item) => item.webkitGetAsEntry());
    for (const entry of wkItems) {
      const dropItem = createDropItemObject(entry);
      if (entry.isFile) {
        const file = await getEntryFile(entry);
        dropItem["type"] = file.type;
        dropItem["size"] = file.size;
        const upload = addUpload(file);
        dropItem["id"] = upload.id;
      } else if (entry.isDirectory) {
        if (acceptDirectories) {
          dropItem["contents"] = [];
          await addDirectoryFiles(entry, dropItem, acceptDirectoriesRecursive);
        } else {
          console.warn("directory drop not enabled, ignoring entry", entry);
          continue;
        }
      }

      newKeys.push(dropItem);
    }
    if (newKeys.length === 0) {
      return;
    }
    console.log("All newKeys: ", newKeys);
    APP.emit(dropwidget, "dropsignal", JSON.stringify(newKeys));
  };

  this.addFiles = function(filesList) {
    const newKeys = [];
    for (const file of filesList) {
      const upload = addUpload(file);

      const newUpload = {};
      newUpload["id"] = upload.id;
      newUpload["filename"] = upload.file.name;
      newUpload["path"] = upload.file.name;
      newUpload["type"] = upload.file.type;
      newUpload["size"] = upload.file.size;

      newKeys.push(newUpload);
    }

    APP.emit(dropwidget, "dropsignal", JSON.stringify(newKeys));
  };

  async function addDirectoryFiles(dirEntry, parentDropItem, recursive) {
    const dirEntries = await getDirectoryEntries(dirEntry);
    for (let i = 0; i < dirEntries.length; i++) {
      const entry = dirEntries[i];
      const dropItem = createDropItemObject(entry);
      if (entry.isFile) {
        const file = await getEntryFile(entry);
        dropItem["type"] = file.type;
        dropItem["size"] = file.size;
        const upload = addUpload(file);
        dropItem["id"] = upload.id;
      } else if (entry.isDirectory) {
        dropItem["contents"] = [];
        if (recursive) {
          await addDirectoryFiles(entry, dropItem, recursive);
        }
      }

      parentDropItem["contents"].push(dropItem);
    }
  }

  function createDropItemObject(fsEntry) {
    const dropItem = {};
    dropItem["path"] = fsEntry.fullPath;
    dropItem["filename"] = fsEntry.name;
    return dropItem;
  }

  function getEntryFile(entry) {
    return new Promise((resolve) => {
      entry.file(function(result) {
        resolve(result);
      });
    });
  }

  function addUpload(file) {
    const upload = new Object();
    upload.id = Math.floor(Math.random() * Math.pow(2, 31));
    upload.file = file;
    uploads.push(upload);
    return upload;
  }

  function getDirectoryEntries(dirEntry) {
    return new Promise((resolve) => {
      const dirReader = dirEntry.createReader();
      dirReader.readEntries(function(results) {
        resolve(results);
      });
    });
  }

  dropwidget.addEventListener("click", function() {
    if (acceptDrops && filePickerType !== "none") {
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
      if (uploads.length > 0 && uploads[0].ready) {
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
    } else if (typeof dropwidget.wtUseCustomSend === "boolean") {
      if (typeof dropwidget.wtCustomSend !== "function") {
        console.log(
          "Warning: wtUseCustomSend is set, but wtCustomSend is not properly defined as a function. " +
            "Falling back to the default upload mechanism"
        );
      } else if (dropwidget.wtUseCustomSend) {
        self.validFileCheck(upload, dropwidget.wtCustomSend, url);
        return;
      }
    } else {
      const sendFn = (uploadWorker !== null && useFilter) ?
        self.workerSend :
        self.actualSend;
      self.validFileCheck(upload, sendFn, url);
    }
  };

  this.actualSend = function(isValid, url, _upload) {
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

  this.workerSend = function(isValid, url, _upload) {
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

  const fileSelectionHandler = function() {
    if (!acceptDrops) {
      return;
    }
    if (this.files === null || this.files.length === 0) {
      return;
    }

    self.addFiles(this.files);
  };
  hiddenInput.onchange = fileSelectionHandler;
  serverFileInput.onchange = fileSelectionHandler;

  const dirSelectionHandler = function() {
    if (!acceptDrops) {
      return;
    }
    if (this.files === null || this.files.length === 0) {
      return;
    }

    /* Directory selection simply returns a list of files. This code tries
     * to reconstruct the FS tree structure (as returned by the drag-drop API)
     * based on the webkitRelativePath of these files.
     */
    const items = [];
    for (let i = 0; i < this.files.length; i++) {
      addFileToItems(items, this.files[i]);
    }
    APP.emit(dropwidget, "dropsignal", JSON.stringify(items));
  };
  serverDirInput.onchange = dirSelectionHandler;

  function addFileToItems(items, file) {
    const path = file.webkitRelativePath;
    const pathParts = path.split("/");
    if (!acceptDirectoriesRecursive && pathParts.length > 2) {
      return;
    }

    let dirItem = null;
    let partialPath = "";
    for (let i = 0; i < pathParts.length - 1; i++) {
      const pathPart = pathParts[i];
      partialPath += "/" + pathPart;
      const existingDirItem = items.find((item) => item.path === partialPath);
      if (!existingDirItem) {
        const newDirItem = {};
        newDirItem["path"] = partialPath;
        newDirItem["filename"] = pathPart;
        newDirItem["contents"] = [];
        if (dirItem === null) {
          items.push(newDirItem);
        } else {
          dirItem.contents.push(newDirItem);
        }
        dirItem = newDirItem;
      } else {
        dirItem = existingDirItem;
      }
      items = dirItem.contents;
    }
    const fileItem = {};
    const upload = addUpload(file);
    fileItem["id"] = upload.id;
    fileItem["path"] = "/" + path;
    fileItem["filename"] = file.name;
    fileItem["type"] = file.type;
    fileItem["size"] = file.size;
    dirItem.contents.push(fileItem);
  }

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
    serverFileInput.setAttribute("accept", acceptAttributes);
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

  dropwidget.setOnClickFilePicker = function(type) {
    if (type === "directory-selection") {
      dropwidget.hiddenInput.setAttribute("webkitdirectory", "webkitdirectory");
    } else {
      dropwidget.hiddenInput.removeAttribute("webkitdirectory");
      if (type !== "file-selection" && type !== "none") {
        console.warn("unknown filepicker type; using 'file-selection'", type);
        type = "file-selection";
      }
    }
    filePickerType = type;
  };
});
