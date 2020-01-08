/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WStackedWidget",
 function (APP, widget) {
  widget.wtObj = this;

  var WT = APP.WT, scrollTops = [], scrollLefts = [],
    lastResizeWidth = null, lastResizeHeight = null;

  function isProperChild(el) {
    return el.nodeType == 1 && !$(el).hasClass("wt-reparented") && !$(el).hasClass("resize-sensor");
  }

  this.reApplySize = function() {
    if (lastResizeHeight) {
      this.wtResize(widget, lastResizeWidth, lastResizeHeight, false);
    }
  }
  
  this.wtResize = function(self, w, h, setSize) {
    lastResizeWidth = w;
    lastResizeHeight = h;

    var hdefined = h >= 0;

    if (setSize) {
      if (hdefined) {
        self.style.height = h + 'px';
	self.lh = true;
      } else {
        self.style.height = '';
	self.lh = false;
      }
    } else
      self.lh = false;

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

      if (isProperChild(c)) {
	if (!WT.isHidden(c) && !$(c).hasClass("out")) {
	  if (hdefined) {
	    var ch = h - marginV(c);
	    if (ch > 0) {
	      /*
		to prevent that the first child widget's top margin bleeds
		to shift this child down, we set overflow. See also #2809
		and the original work-around 548948b63
	      */
	      if (c.offsetTop > 0) {
		var of = WT.css(c, 'overflow');
		if (of === 'visible' || of === '')
		  c.style.overflow = 'auto';
	      }

	      if (c.wtResize) {
		c.wtResize(c, w, ch, true);
	      }	else {
		var cheight = ch + 'px';
		if (c.style.height != cheight) {
		  c.style.height = cheight;
		  c.lh = true;
		}
	      }
	    }
	  } else {
	    if (c.wtResize)
	      c.wtResize(c, w, -1, true);
	    else {
	      c.style.height = '';
	      c.lh = false;
	    }
	  }
	}
      }
    }
  };

  this.wtGetPs = function(self, child, dir, size) {
    return size;
  };

  this.adjustScroll = function(child) {
    var j, jl, c;
    var sl = widget.scrollLeft, st = widget.scrollTop;

    for (j = 0, jl = widget.childNodes.length; j < jl; ++j) {
      c = widget.childNodes[j];

      if (isProperChild(c)) {
	if (c != child) {
	  if (c.style.display != 'none') {
	    scrollLefts[j] = sl;
	    scrollTops[j] = st;
	  }
	} else {
	  if (typeof scrollLefts[j] !== 'undefined') {
	    widget.scrollLeft = scrollLefts[j];
	    widget.scrollTop = scrollTops[j];
	  } else {
	    widget.scrollLeft = 0;
	    widget.scrollTop = 0;
	  }
	}
      }
    }
  };

  this.setCurrent = function(child) {
    var j, jl, c;

    this.adjustScroll(child);

    for (j = 0, jl = widget.childNodes.length; j < jl; ++j) {
      c = widget.childNodes[j];

      if (isProperChild(c)) {
	if (c != child) {
	  if (c.style.display != 'none') {
	    c.style.display = 'none';
	  }
	} else {
          if (c.style.flexFlow)
            c.style.display = 'flex';
          else
	    c.style.display = '';

	  if (widget.lh) {
	    widget.lh = false;
	    widget.style.height = '';
	  }
	}
      }
    }

    this.reApplySize();
  };
 });

WT_DECLARE_WT_MEMBER
(2, JavaScriptPrototype, "WStackedWidget.prototype.animateChild",
 function (WT, child, effects, timing, duration, style) {
   var doAnimateChild = function(WT, child, effects, timing, duration, style) {
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

     var timings = [ "ease", "linear", "ease-in", "ease-out", "ease-in-out" ],
       inverseTiming = [ 0, 1, 3, 2, 4, 5 ],
       prefix = WT.vendorPrefix(WT.styleAttribute("animation-duration"));

     var animationEventEnd = prefix == "Webkit"
	   ? "webkitAnimationEnd" : "animationend";

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

     var index = getIndexes();

     if (index.from == -1 || index.to == -1 || index.from == index.to)
       return;

     var from = stack.childNodes[index.from],
       to = stack.childNodes[index.to],
       $from = $(from),
       $to = $(to),
       h = stack.scrollHeight, w = stack.scrollWidth;

     function restoreTo() {
       $to.removeClass(anim + ' in');
       to.style.position = '';
       to.style.left = '';
       to.style.width = '';
       to.style.top = '';
       if (!stack.lh) { // stack has no layout-set height
	 if (!to.lh)    // child has no layout-set height (by itself)
	   to.style.height = '';
       } else
	 to.lh = true;  // height was set before animation

       if (WT.isGecko && (effects & Fade))
         to.style.opacity = '1';
       to.style[WT.styleAttribute('animation-duration')] = '';
       to.style[WT.styleAttribute('animation-timing-function')] = '';
     }

     function restoreFrom() {
       $from.removeClass(anim + ' out');
       from.style.display = 'none';
       if (stack.lh) { // stack has a layout-set height
	 if (to.lh) { // child had a layout-set height
	   to.style.height = '';
	   to.lh = false;
	 }
       }
	 
       from.style[WT.styleAttribute('animation-duration')] = '';
       from.style[WT.styleAttribute('animation-timing-function')] = '';
     }

     /*
      * If an animation is already busy, then wait until it is done
      * FIXME: Ideally, accelerate existing animation to completion
      * For now, reduce duration of any queued animations so that
      * backlogs are minimized for faster resync.
      */
     if ($from.hasClass("in")) {
       $from.one(animationEventEnd, function() {
	   doAnimateChild(WT, child, effects, timing, 1, style);
	 });
       return;
     }
     else if ($to.hasClass("out")) {
       $to.one(animationEventEnd, function() {
	   doAnimateChild(WT, child, effects, timing, 1, style);
	 });
       return;
     }

     h -= WT.px(stack, 'paddingTop');
     h -= WT.px(stack, 'paddingBottom');
     h -= WT.px(to, 'marginTop');
     h -= WT.px(to, 'marginBottom');
     h -= WT.px(to, 'borderTopWidth');
     h -= WT.px(to, 'borderBottomWidth');
     h -= WT.px(to, 'paddingTop');
     h -= WT.px(to, 'paddingBottom');

     w -= WT.px(stack, 'paddingLeft');
     w -= WT.px(stack, 'paddingRight');
     w -= WT.px(to, 'marginLeft');
     w -= WT.px(to, 'marginRight');
     w -= WT.px(to, 'borderLeftWidth');
     w -= WT.px(to, 'borderRightWidth');
     w -= WT.px(to, 'paddingLeft');
     w -= WT.px(to, 'paddingRight');

     to.style.left = from.style.left || WT.px(stack, 'paddingLeft');
     to.style.top = from.style.top || WT.px(stack, 'paddingTop');

     to.style.width = w + 'px';
     to.style.height = h + 'px';
     to.style.position = 'absolute';
     if (WT.isGecko && (effects & Fade))
        to.style.opacity = '0';
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

     from.style[WT.styleAttribute('animation-duration')] = duration + 'ms';
     to.style[WT.styleAttribute('animation-duration')] = duration + 'ms';
     from.style[WT.styleAttribute('animation-timing-function')]
       = timings[inverseTiming[timing]];
     to.style[WT.styleAttribute('animation-timing-function')] = timings[timing];

     $from.addClass(anim + ' out');
     $from.one(animationEventEnd, restoreFrom);

     $to.addClass(anim + ' in');
     $to.one(animationEventEnd, restoreTo);
  };

  doAnimateChild(WT, child, effects, timing, duration, style);
});
