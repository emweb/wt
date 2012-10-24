/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WTextEdit",
 function(APP, el) {
   jQuery.data(el, 'obj', this);

   var lastW, lastH;

   var self = this,
       WT = APP.WT,
       css;

   if (!tinymce.dom.Event.domLoaded)
     tinymce.dom.Event.domLoaded = true;

   tinyMCE.init({mode:"none"});

   this.render = function(config, aCss) {
     css = aCss;
     el.ed = new tinymce.Editor(el.id, config);
     el.ed.render();
   };

   this.init = function() {
     var iframe = WT.getElement(el.id + '_ifr');

     var row = iframe.parentNode.parentNode,
       tbl = row.parentNode.parentNode;

     tbl.style.cssText='width:100%;' + css;
     tbl.wtResize = el.wtResize;

     el.style.height = tbl.offsetHeight + 'px';

     self.wtResize(el, lastW, lastH);
   };

   this.wtResize = function(e, w, h) {
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

     var iframe = WT.getElement(e.id + '_ifr');

     if (iframe) {
       var row = iframe.parentNode.parentNode,
         tbl = row.parentNode.parentNode,
	 span = tbl.parentNode,
	 i, il;

       var staticStyle = el.style.position !== 'absolute';

       if (!staticStyle) {
	 span.style.left = e.style.left;
	 span.style.top = e.style.top;
	 if (typeof w !== 'undefined')
	   span.style.width = tbl.style.width = (w) + 'px';
	 span.style.height = tbl.style.height = (h) + 'px';
       } else {
	 span.style.position = 'static';
	 span.style.display = 'block';
       }

       // deduct height of toolbars
       for (i=0, il=tbl.rows.length; i<il; i++) {
          if (tbl.rows[i] != row)
            h -= Math.max(28, tbl.rows[i].offsetHeight);
        }

        h = h + 'px';

        if (iframe.style.height != h) iframe.style.height=h;
      } else {
	lastW = w;
	lastH = h;
      }
   };

   lastH = el.offsetHeight;
 });
