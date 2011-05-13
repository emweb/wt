WT_DECLARE_WT_MEMBER
(1, JavaScriptObject, "WStackedWidget",
 (function() {
   function animateChild(child, effects, timing, duration, style) {
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

     var timings = [ "ease", "linear", "ease-in", "ease-out", "ease-in-out" ];
     var inverseTiming = [ 0, 1, 3, 2, 4, 5 ];
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
       $(to).removeClass(anim + ' in');

       to.style.position = '';
       to.style.left = '';
       to.style.width = '';
       to.style.top = '';
       to.style.height = '';
       to.style['-webkit-animation-duration'] = '';
       to.style['-webkit-animation-timing-function'] = '';

       from.style.display = 'none';
       from.style['-webkit-animation-duration'] = '';
       from.style['-webkit-animation-timing-function'] = '';
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
       $(from).one('webkitAnimationEnd', function() {
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
     case SlideInFromRight: needReverse = !needReverse;
     case SlideInFromLeft: anim = "slide"; break;
     case SlideInFromBottom: anim = "slideup"; break;
     case SlideInFromTop: anim = "slidedown"; break;
     case Pop: anim = "pop"; break;
     }

     if (effects & Fade)
       anim += " fade";

     if (needReverse)
       anim += " reverse";

     from.style['-webkit-animation-duration'] = duration + 'ms';
     to.style['-webkit-animation-duration'] = duration + 'ms';
     from.style['-webkit-animation-timing-function']
       = timings[inverseTiming[timing]];
     to.style['-webkit-animation-timing-function'] = timings[timing];

     $(from).addClass(anim + ' out');
     $(to).addClass(anim + ' in');
     $(to).one('webkitAnimationEnd', restore);
   }

   return {
     animateChild: animateChild
   };
 })()
);
