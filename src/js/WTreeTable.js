/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WTreeTable",
 function(APP, table) {
   jQuery.data(table, 'obj', this);

   var self = this, WT = APP.WT;

   var content = $(table).find('.Wt-content').get(0),
       spacer = $(table).find('.Wt-sbspacer').get(0);

   this.wtResize = function(el, w, h) {
     el.style.height= h + 'px';
     var c = el.lastChild;
     var t = el.firstChild;
     h -= $(t).outerHeight();
     if (h > 0) {
       if (c.style.height != h + 'px')
	 c.style.height = h + 'px';
     }
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
	 self.wtResize(table, 0, h);
     }
   };
 });
