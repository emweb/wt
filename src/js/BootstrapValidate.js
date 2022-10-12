/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptFunction, "validate", function(edit) {
  var v;
  if (edit.options) {
    if (edit.options.item(edit.selectedIndex) == null) {
      v = "";
    } else {
      v = edit.options.item(edit.selectedIndex).text;
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
  /* const */ var ValidationInvalidStyle = 0x1;
  /* const */ var ValidationValidStyle = 0x2;

  var validStyle = state && ((styles & ValidationValidStyle) != 0);
  var invalidStyle = !state && ((styles & ValidationInvalidStyle) != 0);

  var validClass = "Wt-valid";
  var invalidClass = "Wt-invalid";
  var theme = this.theme;
  if (typeof theme === "object") {
    validClass = theme.classes.valid;
    invalidClass = theme.classes.invalid;
  }
  edit.classList.toggle(validClass, validStyle);
  edit.classList.toggle(invalidClass, invalidStyle);

  let controlGroup;
  var success;
  var error;

  controlGroup = edit.closest(".control-group");

  if (controlGroup) { // bootstrapVersion === 2
    success = "success";
    error = "error";
  } else {
    controlGroup = edit.closest(".form-group");
    if (controlGroup) { // bootstrapVersion === 3
      success = "has-success";
      error = "has-error";
    }
  }

  if (controlGroup) {
    const validationMsgs = controlGroup.querySelectorAll(".Wt-validation-message");
    for (const validationMsg of validationMsgs) {
      if (state) {
        validationMsg.textContent = edit.defaultTT;
      } else {
        validationMsg.textContent = message;
      }
    }

    controlGroup.classList.toggle(success, validStyle);
    controlGroup.classList.toggle(error, invalidStyle);
  }

  if (state) {
    edit.setAttribute("title", edit.defaultTT);
  } else {
    edit.setAttribute("title", message);
  }
});
