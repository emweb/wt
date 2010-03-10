/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WFormWidget",
 function(APP, el, emptyText) {
   jQuery.data(el, 'obj', this);

   var self = this;
   var WT = APP.WT;

   this.updateEmptyText = function() {
     var emptyTextStyle = 'Wt-edit-emptyText';

     if (WT.hasFocus(el)) {
       if ($(el).hasClass(emptyTextStyle)) {
	 $(el).removeClass(emptyTextStyle);
	 el.value = '';
       }
     } else {
       if (el.value == '') {
	 $(el).addClass(emptyTextStyle);
	 el.value = emptyText;
       }
     }
   };

   this.updateEmptyText();
 });
