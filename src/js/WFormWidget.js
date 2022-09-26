/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WFormWidget", function(APP, el, emptyText) {
  el.wtObj = this;

  const WT = APP.WT, emptyTextStyle = "Wt-edit-emptyText";

  this.applyEmptyText = function() {
    if (WT.hasFocus(el)) {
      if (el.classList.contains(emptyTextStyle)) {
        if (!WT.isIE && el.oldtype) {
          el.type = el.oldtype;
        }
        el.classList.remove(emptyTextStyle);
        el.value = "";
      }
    } else {
      if (el.value === "") {
        if (el.type === "password") {
          if (!WT.isIE) {
            el.oldtype = "password";
            el.type = "text";
          } else {
            return;
          }
        }
        el.classList.add(emptyTextStyle);
        el.value = emptyText;
      } else {
        el.classList.remove(emptyTextStyle);
      }
    }
  };

  this.setEmptyText = function(newEmptyText) {
    emptyText = newEmptyText;

    if (el.classList.contains(emptyTextStyle)) {
      el.value = emptyText;
    }
  };

  this.applyEmptyText();
});
