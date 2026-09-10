/* global tinymce: readonly, tinyMCE: readonly */

/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WTextEdit", function(APP, el, expectedVersion) {
  el.wtObj = this;

  let lastW, lastH;
  let badHeightCount = 0;

  const self = this,
    WT = APP.WT;
  let css;

  if (!tinymce.dom.Event.domLoaded) {
    tinymce.dom.Event.domLoaded = true;
  }

  tinyMCE.init({ mode: "none" });

  // Keep the callback stable so repeated init calls do not add listeners.
  function forwardEvent(event) {
    const handler = el["on" + event.type];
    if (typeof handler === "function" && handler.call(el, event) === false) {
      event.preventDefault();
    }
  }

  function onChange() {
    // Serializing a Wt event saves all editors and can emit change. Wait
    // until that event is queued so it cannot overwrite the change event.
    setTimeout(function() {
      APP.emit(el, "change");
    }, 0);
  }

  this.render = function(config, aCss, connectOnChange, connectOnRender) {
    css = aCss;
    el.ed = new tinymce.Editor(el.id, config, tinymce.EditorManager);
    el.ed.render();
    if (connectOnChange) {
      if (tinymce.EditorManager.majorVersion < 4) {
        el.ed.onChange.add(onChange);
      } else {
        el.ed.on("change", onChange);
      }
    }
    if (connectOnRender) {
      setTimeout(function() {
        APP.emit(el, "render");
      }, 0);
    }
  };

  this.init = function() {
    const iframe = WT.getElement(el.id + "_ifr");

    if (parseInt(tinymce.EditorManager.majorVersion) !== expectedVersion) {
      APP.emit(el, "badVersion", tinymce.EditorManager.majorVersion);
    }

    let topLevel, other;

    if (tinymce.EditorManager.majorVersion < 4) {
      const row = iframe.parentNode.parentNode,
        tbl = row.parentNode.parentNode;

      other = tbl;
      topLevel = tbl.parentNode;
    } else {
      const item = iframe.parentNode,
        container = item.parentNode;

      topLevel = container.parentNode;
    }

    if (other) {
      other.style.cssText = "width:100%;" + css;
      el.style.height = other.offsetHeight + "px";
    }

    topLevel.wtResize = el.wtResize;

    if (WT.isGecko) {
      setTimeout(function() {
        self.wtResize(el, lastW, lastH, true);
      }, 100);
    } else {
      self.wtResize(el, lastW, lastH, true);
    }

    const doc = iframe.contentDocument;
    // Events forwarded below already reach this widget without capture.
    doc.wtCaptureTarget = el;

    for (
      const type of [
        "focus",
        "blur",
        "input",
        "keydown",
        "keypress",
        "keyup",
        "click",
        "dblclick",
        "mousedown",
        "mouseup",
        "mouseover",
        "mouseout",
        "mousemove",
        "wheel",
        "mousewheel",
        "touchstart",
        "touchmove",
        "touchend",
        "gesturestart",
        "gesturechange",
        "gestureend",
        "dragstart",
      ]
    ) {
      doc.addEventListener(type, forwardEvent, {
        capture: type === "focus" || type === "blur",
        passive: false,
      });
    }

    doc.body.addEventListener("paste", function(event) {
      const clipboardData = event.clipboardData || event.originalEvent.clipboardData;

      function isImage(t) {
        return t.indexOf("image/") === 0;
      }

      function isFiles(t) {
        return t === "Files";
      }

      if (clipboardData && clipboardData.types) {
        for (let i = 0, il = clipboardData.types.length; i < il; ++i) {
          const type = clipboardData.types[i];
          const item = clipboardData.items[i];
          if (!isFiles(item.type) && (isImage(type) || isImage(item.type))) {
            const file = clipboardData.items[i].getAsFile();
            const reader = new FileReader();
            reader.onload = function(_evt) {
              el.ed.insertContent('<img src="' + this.result + '"></img>');
            };
            reader.readAsDataURL(file);

            WT.cancelEvent(event);
          }
        }
      }
    });
  };

  this.wtResize = function(e, w, h, _setSize) {
    if (h < 0) {
      return;
    }

    const saveh = h;

    const iframe = WT.getElement(e.id + "_ifr");
    if (iframe) {
      let my = 0;

      if (!WT.boxSizing(e)) {
        // mx += WT.px(e, "borderLeftWidth") +
        //   WT.px(e, "borderRightWidth") +
        //   WT.px(e, "paddingLeft") +
        //   WT.px(e, "paddingRight");
        my += WT.pxComputedStyle(e, "borderTopWidth") +
          WT.pxComputedStyle(e, "borderBottomWidth") +
          WT.pxComputedStyle(e, "paddingTop") +
          WT.pxComputedStyle(e, "paddingBottom");
      }

      e.style.height = (h - my) + "px";

      let topLevel, other;

      const parent = el.parentElement,
        grandparent = parent ? parent.parentElement : null;
      const flexLayout = (parent && parent.classList.contains("Wt-fill-width")) ||
        (grandparent && grandparent.classList.contains("Wt-fill-height"));
      const position = el.style.position;
      const staticStyle = flexLayout ||
        (position !== "absolute" &&
          position !== "relative" &&
          position !== "fixed");

      // In a flex layout, the editor container must remain static and fill the
      // available space. Inline dimensions on the widget are preferred layout
      // sizes, not its final dimensions.

      if (tinymce.EditorManager.majorVersion < 4) {
        const row = iframe.parentNode.parentNode,
          tbl = row.parentNode.parentNode;

        other = tbl;
        topLevel = tbl.parentNode;

        if (staticStyle) {
          topLevel.style.width = "100%";
        } else if (typeof w !== "undefined") {
          topLevel.style.width = (w - 2) + "px";
        }

        // deduct height of all the rest
        for (let i = 0, il = tbl.rows.length; i < il; i++) {
          if (tbl.rows[i] !== row) {
            h -= tbl.rows[i].offsetHeight;
          }
        }
      } else {
        let item;
        if (tinymce.EditorManager.majorVersion < 5) {
          item = iframe.parentNode;
        } else {
          item = iframe.parentNode.parentNode;
        }

        const container = item.parentNode;

        topLevel = container.parentNode;

        if (staticStyle) {
          topLevel.style.width = "100%";
        } else if (typeof w !== "undefined") {
          topLevel.style.width = (w - 2) + "px";
        }

        // deduct height of all the rest
        for (let i = 0, il = container.childNodes.length; i < il; i++) {
          if (container.childNodes[i] !== item) {
            h -= container.childNodes[i].offsetHeight + 1;
          }
        }

        h -= 1;
      }

      topLevel.style.marginInlineStart = getComputedStyle(e).marginInlineStart;
      topLevel.style.marginInlineEnd = getComputedStyle(e).marginInlineEnd;
      topLevel.style.marginBlockStart = getComputedStyle(e).marginBlockStart;
      topLevel.style.marginBlockEnd = getComputedStyle(e).marginBlockEnd;

      if (tinymce.EditorManager.majorVersion < 5 && h < 0) {
        if (badHeightCount < 10) {
          const timeoutDelay = Math.pow(2, badHeightCount) * 100;
          setTimeout(function() {
            self.wtResize(el, lastW, lastH, true);
          }, timeoutDelay);
        }
        badHeightCount += 1;
        return;
      }

      h = h + "px";

      if (!staticStyle) {
        topLevel.style.position = e.style.position;
        topLevel.style.left = e.style.left;
        topLevel.style.insetInlineStart = getComputedStyle(e).insetInlineStart;
        topLevel.style.top = e.style.top;
        topLevel.style.insetBlockStart = getComputedStyle(e).insetBlockStart;

        if (!staticStyle && other) {
          other.style.width = (w) + "px";
        }

        if (other) {
          other.style.height = (h) + "px";
          topLevel.style.height = e.style.height;
        } else if (tinymce.EditorManager.majorVersion >= 5) {
          // TinyMCE 5+ sizes the editor container itself (defaulting to at
          // least 400px), so it must be resized explicitly, just like in the
          // static case, or it can never become smaller (e.g. in a layout).
          topLevel.style.height = saveh + "px";
        }

        if (tinymce.EditorManager.majorVersion >= 5) {
          topLevel.style.height = saveh + "px";
        }
      } else {
        if (tinymce.EditorManager.majorVersion < 5) {
          topLevel.style.position = "static";
          topLevel.style.display = "block";
        } else {
          topLevel.style.height = "100%";
        }
      }

      if (tinymce.EditorManager.majorVersion < 5 && iframe.style.height !== h) {
        badHeightCount = 0;
        iframe.style.height = h;
        if (APP.layouts2) {
          APP.layouts2.setElementDirty(el);
        }
      }
    } else {
      lastW = w;
      lastH = h;
    }
  };

  lastH = el.offsetHeight;
  lastW = el.offsetWidth;
});
