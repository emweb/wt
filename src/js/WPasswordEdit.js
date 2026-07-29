/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WPasswordEdit", function(APP, edit, textVisibilityChangedConnected) {
  const CLASS_HOVER = "hover";
  const CLASS_UNSELECTABLE = "unselectable";

  edit.wtPObj = this;

  const WT = APP.WT;

  function isRevealButtonEnabled() {
    return edit.classList.contains("Wt-passwordedit-reveal");
  }

  this.setTextVisibilityChangedConnected = function(connected) {
    textVisibilityChangedConnected = connected;
  };

  function isOverButton(event) {
    const xy = WT.widgetCoordinates(edit, event);
    return xy.x > edit.offsetWidth - 40;
  }

  this.mouseOut = function(_o, _event) {
    edit.classList.remove(CLASS_HOVER);
  };

  this.mouseMove = function(_o, event) {
    if (!isRevealButtonEnabled()) {
      return;
    }

    edit.classList.toggle(CLASS_HOVER, isOverButton(event));
  };

  this.mouseDown = function(_o, event) {
    if (!isRevealButtonEnabled()) {
      return;
    }

    if (isOverButton(event)) {
      edit.classList.add(CLASS_UNSELECTABLE);
    }
  };

  this.mouseUp = function(_o, event) {
    edit.classList.remove(CLASS_UNSELECTABLE);

    if (!isRevealButtonEnabled()) {
      return;
    }

    if (isOverButton(event)) {
      edit.type = edit.type === "password" ? "text" : "password";
      const emit = textVisibilityChangedConnected ? APP.emit : APP.emitOnUpdate;
      emit(edit, "textVisibility", edit.type !== "password");
    }
  };
});
