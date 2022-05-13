/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptFunction, "validate",
 function(edit) {
     var v;
     if (edit.options) {
        if (edit.options.item(edit.selectedIndex) == null)
          v = "";
        else
          v = edit.options.item(edit.selectedIndex).text;
     } else if (typeof edit.wtLObj === 'object' &&
                typeof edit.wtLObj.getValue === 'function') {
         v = edit.wtLObj.getValue();
     } else
         v = edit.value;

     v = edit.wtValidate.validate(v);

     this.setValidationState(edit, v.valid, v.message, 1);
 });

WT_DECLARE_WT_MEMBER
(2, JavaScriptFunction, "setValidationState",
 function(edit, state, message, styles) {

     /* const */ var ValidationInvalidStyle = 0x1;
     /* const */ var ValidationValidStyle = 0x2;

     var validStyle = (state == 1) &&
         ((styles & ValidationValidStyle) != 0);
     var invalidStyle = (state != 1) &&
         ((styles & ValidationInvalidStyle) != 0);

     var $edit = $(edit);

     var validClass = 'Wt-valid';
     var invalidClass = 'Wt-invalid';
     var theme = this.theme;
     if (typeof theme === 'object') {
       validClass = theme.classes.valid;
       invalidClass = theme.classes.invalid;
     }
     $edit
       .toggleClass(validClass, validStyle)
       .toggleClass(invalidClass, invalidStyle);

     var controlGroup;
     var success;
     var error;

     controlGroup = $edit.closest(".control-group");

     if (controlGroup.length > 0) { // bootstrapVersion === 2
       success = "success";
       error = "error";
     } else {
       controlGroup = $edit.closest(".form-group");
       if (controlGroup.length > 0) { // bootstrapVersion === 3
         success = "has-success";
         error = "has-error";
       }
     }

     if (controlGroup.length > 0) {
       var validationMsg = controlGroup.find(".Wt-validation-message");
       if (validationMsg) {
         if (state)
           validationMsg.text(edit.defaultTT);
         else
           validationMsg.text(message);
       }

       controlGroup
         .toggleClass(success, validStyle)
         .toggleClass(error, invalidStyle);
     }

     if (typeof edit.defaultTT === 'undefined')
       edit.defaultTT = edit.getAttribute('title') || '';

     if (state)
       edit.setAttribute('title', edit.defaultTT);
     else
       edit.setAttribute('title', message);
 });
