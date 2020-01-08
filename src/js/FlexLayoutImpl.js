/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "FlexLayout",
 function(APP, id) {
   var WT = APP.WT;

   function init() {
     var el = WT.getElement(id);
     if (!el) return;

     var children = el.childNodes;
     for (var i = 0; i < children.length; ++i) {
       var c = children[i];
       if (c.style.display == 'none' || 
	   $(c).hasClass('out') ||
	   c.className == 'resize-sensor')
	 continue;

       var of = WT.css(c, 'overflow');
       if (of === 'visible' || of === '')
	 c.style.overflow = 'hidden';
     }
   }

   setTimeout(init, 0);
   
   this.adjust = function(spacing) {
     setTimeout(function() {
       var el = WT.getElement(id);
       if (!el) return;

       var children = el.childNodes;
       var totalStretch = 0;
       var flexGrowProp = WT.styleAttribute('flex-grow');
       for (var i = 0; i < children.length; ++i) {
	 var c = children[i];
	 if (c.style.display == 'none' || 
	     $(c).hasClass('out') ||
	     c.className == 'resize-sensor')
	   continue;

	 var flg = c.getAttribute('flg');
	 if (flg === '0')
	   continue; 

	 var flexGrow = WT.css(c, flexGrowProp);
	 totalStretch += parseFloat(flexGrow);
       }

       for (var i = 0; i < children.length; ++i) {
	 var c = children[i];

	 if (c.style.display == 'none' || 
	     $(c).hasClass('out') ||
	     c.className == 'resize-sensor')
	   continue;

	 /* Re-trigger resize-sensor */
	 if (c.resizeSensor)
	   c.resizeSensor.trigger();
	 
	 var stretch;

	 if (totalStretch === 0)
	   stretch = 1;
	 else {
	   var flg = c.getAttribute('flg');
	   if (flg === '0')
	     stretch = 0;
	   else {
	     var flexGrow = WT.css(c, flexGrowProp);
	     stretch = flexGrow;
	   }
	 }

	 c.style[flexGrowProp] = stretch;
       }
     }, 0);
   };
 });
