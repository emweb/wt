/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WTreeTable",
 function(APP, table) {
   table.wtObj = this;

   var self = this, WT = APP.WT;

   var content = $(table).find('.Wt-content').get(0),
       spacer = $(table).find('.Wt-sbspacer').get(0);

   this.wtResize = function(el, w, h, setSize) {
     var hdefined = h >= 0;

     if (setSize) {
     	if (hdefined)
	  el.style.height = h + 'px';
	else
	  el.style.height = '';
     }

     var c = el.lastChild;
     var t = el.firstChild;
     h -= $(t).outerHeight();

     if (hdefined && h > 0) {
       if (c.style.height != h + 'px')
	 c.style.height = h + 'px';
     } else
       c.style.height = '';
   };

   this.autoJavaScript = function() {
     if (table.parentNode) {
       if (content.scrollHeight > content.offsetHeight) {
         spacer.style.display='block';
       } else {
         spacer.style.display='none';
       }

       var h = WT.pxself(table, 'height');
       if (h)
	 self.wtResize(table, 0, h, false);
     }
   };
 });
