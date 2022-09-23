/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WTreeTable", function(APP, table) {
  table.wtObj = this;

  const self = this, WT = APP.WT;
  const content = table.querySelector(".Wt-content"),
    spacer = table.querySelector(".Wt-sbspacer");

  this.wtResize = function(el, w, h, setSize) {
    const hdefined = h >= 0;

    if (setSize) {
      if (hdefined) {
        el.style.height = h + "px";
      } else {
        el.style.height = "";
      }
    }

    const c = el.lastChild;
    const t = el.firstChild;
    h -= t.getBoundingClientRect().height;

    if (hdefined && h > 0) {
      if (c.style.height !== h + "px") {
        c.style.height = h + "px";
      }
    } else {
      c.style.height = "";
    }
  };

  this.autoJavaScript = function() {
    if (table.parentNode) {
      if (content.scrollHeight > content.offsetHeight) {
        spacer.style.display = "block";
      } else {
        spacer.style.display = "none";
      }

      const h = WT.pxself(table, "height");
      if (h) {
        self.wtResize(table, 0, h, false);
      }
    }
  };
});
