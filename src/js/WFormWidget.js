/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WFormWidget",
 function(APP, el, emptyText) {
   el.wtObj = this;

   var self = this, WT = APP.WT, emptyTextStyle = 'Wt-edit-emptyText';

   this.applyEmptyText = function() {
     if (WT.hasFocus(el)) {
       if ($(el).hasClass(emptyTextStyle)) {
	 if (!WT.isIE && el.oldtype) {
	   el.type = el.oldtype;
	 }
	 $(el).removeClass(emptyTextStyle);
	 el.value = '';
       }
     } else {
       if (el.value == '') {
	 if (el.type == 'password') {
	   if (!WT.isIE) {
	     el.oldtype = 'password';
	     el.type = 'text';
	   } else
	     return;
	 }
	 $(el).addClass(emptyTextStyle);
	 el.value = emptyText;
       } else {
	 $(el).removeClass(emptyTextStyle);
       }
     }
   };

   this.setEmptyText = function(newEmptyText) {
     emptyText = newEmptyText;

     if ($(el).hasClass(emptyTextStyle))
       el.value = emptyText;
   };

   this.applyEmptyText();
 });
