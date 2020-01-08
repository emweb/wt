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
     if (edit.options)
	 v = edit.options.item(edit.selectedIndex).text;
     else
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

     $edit
	 .toggleClass("Wt-valid", validStyle)
	 .toggleClass("Wt-invalid", invalidStyle);

     if (typeof edit.defaultTT === 'undefined')
       edit.defaultTT = edit.getAttribute('title') || '';

     if (state)
       edit.setAttribute('title', edit.defaultTT);
     else
       edit.setAttribute('title', message);
 });
