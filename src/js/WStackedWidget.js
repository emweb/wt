/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WStackedWidget",
 function (APP, widget) {
  jQuery.data(widget, 'obj', this);

  var WT = APP.WT, heightSet = false;

  this.wtResize = function(self, w, h, item) {
    if (item && item.set[1]) {
      self.style.height = h + 'px';
      heightSet = true;
    } else
      heightSet = false;

     if (WT.boxSizing(self)) {
       h -= WT.px(self, 'marginTop');
       h -= WT.px(self, 'marginBottom');
       h -= WT.px(self, 'borderTopWidth');
       h -= WT.px(self, 'borderBottomWidth');
       h -= WT.px(self, 'paddingTop');
       h -= WT.px(self, 'paddingBottom');

       w -= WT.px(self, 'marginLeft');
       w -= WT.px(self, 'marginRight');
       w -= WT.px(self, 'borderLeftWidth');
       w -= WT.px(self, 'borderRightWidth');
       w -= WT.px(self, 'paddingLeft');
       w -= WT.px(self, 'paddingRight');
     }

     function marginV(el) {
       var result = WT.px(el, 'marginTop');
       result += WT.px(el, 'marginBottom');

       if (!WT.boxSizing(el)) {
	 result += WT.px(el, 'borderTopWidth');
	 result += WT.px(el, 'borderBottomWidth');
	 result += WT.px(el, 'paddingTop');
	 result += WT.px(el, 'paddingBottom');
       }

       return result;
     }

     var j, jl, c;
     for (j = 0, jl = self.childNodes.length; j < jl; ++j) {
       c = self.childNodes[j];

       if (c.nodeType == 1) {
	 if (!WT.isHidden(c)) {
	   var ch = h - marginV(c);

	   if (ch > 0) {
	     if (c.wtResize)
	       c.wtResize(c, w, ch);
	     else {
	       var cheight = ch + 'px';
	       if (c.style.height != cheight)
		 c.style.height = cheight;
	     }
	   }
	 }
       }
     }
   };

   this.wtGetPs = function(self, child, dir, size) {
      return size;
   };

   this.setCurrent = function(child) {
     var j, jl, c;
     for (j = 0, jl = widget.childNodes.length; j < jl; ++j) {
       c = widget.childNodes[j];

       if (c.nodeType == 1) {
	 if (c != child)
	   c.style.display = 'none';
	 else {
	   c.style.display = '';

	   if (heightSet) {
	     heightSet = false;
	     widget.style.height = '';
	   }
	 }
       }
     }
   };
 });

WT_DECLARE_WT_MEMBER
(2, JavaScriptPrototype, "WStackedWidget.prototype.animateChild",
 function (child, effects, timing, duration, style) {
     /* const */ var SlideInFromLeft = 0x1;
     /* const */ var SlideInFromRight = 0x2;
     /* const */ var SlideInFromBottom = 0x3;
     /* const */ var SlideInFromTop = 0x4;
     /* const */ var Pop = 0x5;
     /* const */ var Fade = 0x100;

     /* const */ var Ease = 0;
     /* const */ var Linear = 1;
     /* const */ var EaseIn = 2;
     /* const */ var EaseOut = 3;
     /* const */ var EaseInOut = 4;
     /* const */ var CubicBezier = 5;

     function getPrefix(prop) {
       var prefixes = ['Moz', 'Webkit'],
	 elem = document.createElement('div'),
	 i, il;

       for (i = 0, il = prefixes.length; i < il; ++i) {
	 if ((prefixes[i] + prop) in elem.style)
	   return prefixes[i];
       }

       return '';
     }

     var timings = [ "ease", "linear", "ease-in", "ease-out", "ease-in-out" ],
       inverseTiming = [ 0, 1, 3, 2, 4, 5 ],
       prefix = getPrefix("AnimationDuration");

     var cssAnimation = prefix + "Animation",
       animationEventEnd = prefix == "Moz"
       ? "animationend" : "webkitAnimationEnd";

     /*
      * We only need to implement the show() -- we hide the currently
      * active one at the same time
      */
     if (style.display === 'none')
       return;

     var stack = child.parentNode;
     var reverseIfPrecedes = stack.wtAutoReverse;

     function getIndexes() {
       var i, il=stack.childNodes.length, fromI = -1, toI = -1;

       for (i = 0; i < il && (fromI == -1 || toI == -1); ++i) {
	 var ch = stack.childNodes[i];

	 if (ch == child)
	   toI = i;
	 else if (ch.style.display !== 'none' && !$(ch).hasClass('out'))
	   fromI = i;
       }

       return { from: fromI, to: toI };
     }

     function restore() {
       $(from).removeClass(anim + ' out');
       from.style.display = 'none';
       from.style[cssAnimation + 'Duration'] = '';
       from.style[cssAnimation + 'TimingFunction'] = '';

       $(to).removeClass(anim + ' in');
       to.style.left = '';
       to.style.width = '';
       to.style.top = '';
       to.style.height = '';
       to.style.position = '';
       to.style[cssAnimation + 'Duration'] = '';
       to.style[cssAnimation + 'TimingFunction'] = '';
     }

     var index = getIndexes();

     if (index.from == -1 || index.to == -1 || index.from == index.to)
       return;

     var from = stack.childNodes[index.from],
       to = stack.childNodes[index.to],
       h = stack.offsetHeight, w = stack.offsetWidth;

     /*
      * If an animation is already busy, then wait until it is done
      */
     if ($(from).hasClass("in")) {
       $(from).one(animationEventEnd, function() {
	   animateChild(child, effects, timing, duration, style);
	 });
       return;
     }

     to.style.left = '0px';
     to.style.top = '0px';
     to.style.width = w + 'px';
     to.style.height = h + 'px';
     to.style.position = 'absolute';
     to.style.display = style.display;

     var needReverse = reverseIfPrecedes && (index.to < index.from),
       anim = "";

     switch (effects & 0xFF) {
     case SlideInFromLeft: needReverse = !needReverse;
     case SlideInFromRight: anim = "slide"; break;
     case SlideInFromBottom: anim = "slideup"; break;
     case SlideInFromTop: anim = "slidedown"; break;
     case Pop: anim = "pop"; break;
     }

     if (effects & Fade)
       anim += " fade";

     if (needReverse)
       anim += " reverse";

     from.style[cssAnimation + 'Duration'] = duration + 'ms';
     to.style[cssAnimation + 'Duration'] = duration + 'ms';
     from.style[cssAnimation + 'TimingFunction']
       = timings[inverseTiming[timing]];
     to.style[cssAnimation + 'TimingFunction'] = timings[timing];

     $(from).addClass(anim + ' out');
     $(to).addClass(anim + ' in');

     $(to).one(animationEventEnd, restore);
 });
