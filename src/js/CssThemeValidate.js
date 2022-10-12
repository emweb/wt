/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptFunction, "validate", function(edit) {
  let v;
  if (edit.options) {
    /** @type {?HTMLOptionElement} */
    const item = edit.options.item(edit.selectedIndex);
    if (item === null) {
      v = "";
    } else {
      v = item.text;
    }
  } else if (
    typeof edit.wtLObj === "object" &&
    typeof edit.wtLObj.getValue === "function"
  ) {
    v = edit.wtLObj.getValue();
  } else {
    v = edit.value;
  }

  v = edit.wtValidate.validate(v);

  this.setValidationState(edit, v.valid, v.message, 1);
});

WT_DECLARE_WT_MEMBER(2, JavaScriptFunction, "setValidationState", function(edit, state, message, styles) {
  const ValidationInvalidStyle = 0x1;
  const ValidationValidStyle = 0x2;

  const validStyle = state && ((styles & ValidationValidStyle) !== 0);
  const invalidStyle = !state && ((styles & ValidationInvalidStyle) !== 0);

  edit.classList.toggle("Wt-valid", validStyle);
  edit.classList.toggle("Wt-invalid", invalidStyle);

  if (typeof edit.defaultTT === "undefined") {
    edit.defaultTT = edit.getAttribute("title") || "";
  }

  if (state) {
    edit.setAttribute("title", edit.defaultTT);
  } else {
    edit.setAttribute("title", message);
  }
});
