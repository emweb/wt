/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WTextEdit",
 function(APP, el) {
   el.wtObj = this;

   var lastW, lastH;
   var badHeightCount = 0;

   var self = this,
       WT = APP.WT,
       css;

   if (!tinymce.dom.Event.domLoaded)
     tinymce.dom.Event.domLoaded = true;

   tinyMCE.init({ mode:"none" });

   this.render = function(config, aCss, connectOnChange) {
     css = aCss;
     el.ed = new tinymce.Editor(el.id, config, tinymce.EditorManager);
     el.ed.render();
     if (connectOnChange) {
	   if (tinymce.EditorManager.majorVersion < 4) {
		 el.ed.onChange.add(function() {
		   APP.emit(el, 'change');
		 });
	   } else  {
		 el.ed.on('change', function() {
		   APP.emit(el, 'change');
		 });
	   }
     }
     setTimeout(function() {APP.emit(el, 'render');}, 0);
   };

   this.init = function() {
     var iframe = WT.getElement(el.id + '_ifr');

     var topLevel, other;

     if (tinymce.EditorManager.majorVersion < 4) {
       var row = iframe.parentNode.parentNode,
         tbl = row.parentNode.parentNode;

       other = tbl;
       topLevel = tbl.parentNode;
     } else {
       var item = iframe.parentNode,
         container = item.parentNode,
         i, il;

       topLevel = container.parentNode;
     }

     if (other) {
       other.style.cssText='width:100%;' + css;
       el.style.height = other.offsetHeight + 'px';
     }

     topLevel.wtResize = el.wtResize;

     if (WT.isGecko)
       setTimeout(function() { self.wtResize(el, lastW, lastH, true); }, 100);
     else
       self.wtResize(el, lastW, lastH, true);

     var doc;

     if (WT.isIE)
       doc = document.frames[iframe.id].document; // one day it may work? :-)
     else
       doc = iframe.contentDocument;

     $(doc.body).bind('paste', function(event) {
       var clipboardData = 
	 event.clipboardData || event.originalEvent.clipboardData,
	 i, il;

       function isImage(t) {
	 return t.indexOf('image/') == 0;
       }

       if (clipboardData && clipboardData.types) {
	 for (i = 0, il = clipboardData.types.length; i < il; ++i) {
	   var t = clipboardData.types[i];
	   if (isImage(clipboardData.types[i]) ||
	       isImage(clipboardData.items[i].type)) {
	     var file = clipboardData.items[i].getAsFile();
	     var reader = new FileReader();
	     reader.onload = function(evt) {
	       el.ed.insertContent("<img src=\"" + this.result + "\"></img>");
	     };
	     reader.readAsDataURL(file);

	     WT.cancelEvent(event);
	   }
	 }
       }
       });
   };

   this.wtResize = function(e, w, h, setSize) {
     if (h < 0)
       return;

     var iframe = WT.getElement(e.id + '_ifr');

     if (iframe) {
       var mx = 0, my = 0;

       mx = WT.px(e, 'marginLeft') + WT.px(e, 'marginRight');
       my = WT.px(e, 'marginTop') + WT.px(e, 'marginBottom');

       if (!WT.boxSizing(e)) {
	 mx += WT.px(e, 'borderLeftWidth') +
           WT.px(e, 'borderRightWidth') +
	   WT.px(e, 'paddingLeft') +
      	   WT.px(e, 'paddingRight');
	 my += WT.px(e, 'borderTopWidth') +
	   WT.px(e, 'borderBottomWidth') +
	   WT.px(e, 'paddingTop') +
	   WT.px(e, 'paddingBottom');
       }

       e.style.height = (h - my) + 'px';

       var topLevel, other;

       var staticStyle = el.style.position !== 'absolute';

       if (tinymce.EditorManager.majorVersion < 4) {
	 var row = iframe.parentNode.parentNode,
           tbl = row.parentNode.parentNode,
	   i, il;

	 other = tbl;
	 topLevel = tbl.parentNode;

         if (!staticStyle && typeof w !== 'undefined')
	   topLevel.style.width = (w - 2) + 'px';

 	 // deduct height of all the rest
	 for (i=0, il=tbl.rows.length; i<il; i++) {
           if (tbl.rows[i] != row)
             h -= tbl.rows[i].offsetHeight;
         }

       } else {
	 var item = iframe.parentNode,
	   container = item.parentNode,
	   i, il;

	 topLevel = container.parentNode;

         if (!staticStyle && typeof w !== 'undefined')
	   topLevel.style.width = (w - 2) + 'px';

	 // deduct height of all the rest
	 for (i=0, il=container.childNodes.length; i<il; i++) {
           if (container.childNodes[i] != item) {
             h -= container.childNodes[i].offsetHeight + 1;
	   }
         }

	 h -= 1;
       }

       if (h < 0) {
	 if (badHeightCount < 10) {
	   var timeoutDelay = Math.pow(2, badHeightCount) * 100;
	   setTimeout(function() { self.wtResize(el, lastW, lastH, true); },
		      timeoutDelay);
	 }
	 badHeightCount += 1;
	 return;
       }

       h = h + 'px';

       if (!staticStyle) {
	 topLevel.style.position = e.style.position;
	 topLevel.style.left = e.style.left;
	 topLevel.style.top = e.style.top;

	 if (!staticStyle && other)
	   other.style.width = (w) + 'px';

	 if (other) {
	   other.style.height = (h) + 'px';
	   topLevel.style.height = e.style.height;
	 }
       } else {
	 topLevel.style.position = 'static';
	 topLevel.style.display = 'block';
       }

       if (iframe.style.height != h) {
	 badHeightCount = 0;
	 iframe.style.height = h;
	 if (APP.layouts2)
	   APP.layouts2.setElementDirty(el);
       }
     } else {
       lastW = w;
       lastH = h;
     }
   };

   lastH = el.offsetHeight;
   lastW = el.offsetWidth;
 });
