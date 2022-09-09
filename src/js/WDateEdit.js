/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WDateEdit", function(APP, edit, popup) {
  /** @const */ var CLASS_HOVER = "hover";
  /** @const */ var CLASS_ACTIVE = "active";
  /** @const */ var CLASS_UNSELECTABLE = "unselectable";

  edit.wtDObj = this;

  var self = this, WT = APP.WT;

  function isReadOnly() {
    return edit.readOnly;
  }

  function getPopup() {
    var p = WT.$(popup);
    return p.wtPopup;
  }

  function resetButton() {
    edit.classList.remove(CLASS_ACTIVE);
  }

  function showPopup() {
    var p = getPopup();
    p.bindHide(resetButton);
    p.show(edit, WT.Vertical);
  }

  this.mouseOut = function(o, event) {
    edit.classList.remove(CLASS_HOVER);
  };

  this.mouseMove = function(o, event) {
    if (isReadOnly()) {
      return;
    }

    var xy = WT.widgetCoordinates(edit, event);

    const isHovering = xy.x > edit.offsetWidth - 40;
    edit.classList.toggle(CLASS_HOVER, isHovering);
  };

  this.mouseDown = function(o, event) {
    if (isReadOnly()) {
      return;
    }

    var xy = WT.widgetCoordinates(edit, event);
    if (xy.x > edit.offsetWidth - 40) {
      edit.classList.add(CLASS_UNSELECTABLE);
      edit.classList.add(CLASS_ACTIVE);
    }
  };

  this.mouseUp = function(o, event) {
    edit.classList.remove(CLASS_UNSELECTABLE);

    var xy = WT.widgetCoordinates(edit, event);
    if (xy.x > edit.offsetWidth - 40) {
      showPopup();
    }
  };
});
