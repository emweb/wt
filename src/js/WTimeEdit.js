/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WTimeEdit", function(APP, edit, popup) {
  const CLASS_HOVER = "hover";
  const CLASS_ACTIVE = "active";
  const CLASS_UNSELECTABLE = "unselectable";

  edit.wtDObj = this;

  const WT = APP.WT;

  function isReadOnly() {
    return edit.readOnly;
  }

  function getPopup() {
    const p = WT.$(popup);
    return p.wtPopup;
  }

  function resetButton() {
    edit.classList.remove(CLASS_ACTIVE);
  }

  function showPopup() {
    const p = getPopup();
    p.bindHide(resetButton);
    p.show(edit, WT.Vertical);
  }

  this.mouseOut = function(_o, _event) {
    edit.classList.remove(CLASS_HOVER);
  };

  this.mouseMove = function(o, event) {
    if (isReadOnly()) {
      return;
    }

    const xy = WT.widgetCoordinates(edit, event);

    const isHovering = xy.x > edit.offsetWidth - 40;
    edit.classList.toggle(CLASS_HOVER, isHovering);
  };

  this.mouseDown = function(o, event) {
    if (isReadOnly()) {
      return;
    }

    const xy = WT.widgetCoordinates(edit, event);
    if (xy.x > edit.offsetWidth - 40) {
      edit.classList.add(CLASS_UNSELECTABLE);
      edit.classList.add(CLASS_ACTIVE);
    }
  };

  this.mouseUp = function(o, event) {
    edit.classList.remove(CLASS_UNSELECTABLE);

    const xy = WT.widgetCoordinates(edit, event);
    if (xy.x > edit.offsetWidth - 40) {
      showPopup();
    }
  };
});
