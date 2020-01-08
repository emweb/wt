/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptFunction, "animateDisplay",
 function(APP, id, effects, timing, duration, display) {
  var WT = APP.WT;

  var doAnimateDisplay = function(id, effects, timing, duration, display) {

  /* const */ var NoEffect = 0x0;
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
    inverseTiming = [ 0, 1, 3, 2, 4, 5 ];

  var animationPrefix = WT.vendorPrefix(WT.styleAttribute('animation'));
  var transitionPrefix = WT.vendorPrefix(WT.styleAttribute('transition'));
  var transformPrefix = WT.vendorPrefix(WT.styleAttribute('transform'));
  var $el = $("#" + id), el = $el.get(0),
    animationEventEnd = animationPrefix == "Webkit"
	  ? "webkitAnimationEnd": "animationend",
    transitionEventEnd = transitionPrefix == "Webkit"
      ? "webkitTransitionEnd" : "transitionend";

  if ($el.css("display") !== display) {
    var p = el.parentNode;

    if (p.wtAnimateChild) {
      p.wtAnimateChild(WT, $el.get(0), effects, timing, duration,
		       { display: display });
      return;
    }

    if ($el.hasClass("animating")) {
      $(el).one(transitionEventEnd, function() {
	  doAnimateDisplay(id, effects, timing, duration, display);
	});
      return;
    }

    $el.addClass("animating");

    var effect = effects & 0xFF,
      hide = display === "none",
      cssTiming = timings[hide ? inverseTiming[timing] : timing],
      elStyle = {};

    function set(el, style, savedStyle) {
      var i, il;

      for (i in style) {
	var k = i;

	if (k == "animationDuration" && animationPrefix != '')
	  k = animationPrefix + k.substring(0, 1).toUpperCase()
	      + k.substring(1);
	else if (k == "transform" && transformPrefix != '')
	  k = transformPrefix + k.substring(0, 1).toUpperCase()
	      + k.substring(1);
	else if (k == "transition" && transitionPrefix != '')
	  k = transitionPrefix + k.substring(0, 1).toUpperCase()
	      + k.substring(1);

	if (savedStyle && typeof(savedStyle[k]) === "undefined")
	  savedStyle[k] = el.style[k];
	el.style[k] = style[i];
      }
    }

    var restore = set;

    function onEnd() {
      if (el.wtAnimatedHidden)
	el.wtAnimatedHidden(hide);

      $el.removeClass("animating");

      if (APP.layouts2)
	APP.layouts2.setElementDirty(el);
    }

    function show() {
      el.style.display = display;
      if (el.wtPosition)
	el.wtPosition();
      if (window.onshow) window.onshow();
    }

    function animateStaticVertical() {
      var targetHeight, currentHeight, elcStyle = {}, elc;

      if (hide) {
	currentHeight = $el.height() + "px";
	set(el,
	    {   height: currentHeight,
	      overflow: "hidden" },
	    elStyle);

	if (effect == SlideInFromTop && el.childNodes.length == 1) {
	  elc = el.firstChild;
	  set(elc, { transform: "translateY(0)" }, elcStyle);
	  if (!WT.hasTag(elc, 'TABLE'))
	    set(elc, { display: "block" }, elcStyle);
	}

	targetHeight = "0px";
      } else {
	var $p = $(p), pStyle = { };

	set(p,
	    {   height: $p.height() + "px",
	      overflow: "hidden" },
	    pStyle);

	show();

	if ($el.height() == 0)
	  el.style.height = 'auto';

	targetHeight = $el.height() + "px";
	set(el,
	    {   height: "0px",
	      overflow: "hidden" },
	    elStyle);
	restore(p, pStyle);

	if (effect == SlideInFromTop) {
	  set (el, { WebkitBackfaceVisibility: "visible"} , elStyle);
	  el.scrollTop = 1000;
	}
      }

      if (effects & Fade)
	set(el, { opacity: (hide ? 1 : 0) }, elStyle);

      var currentHeight = el.clientHeight; // force 'absorbing' set height

      setTimeout(function() {
	  set(el,
	      { transition: "all " + duration + "ms " + cssTiming,
		    height: targetHeight },
	      elStyle);

	  if (effects & Fade)
	    set(el, { opacity: (hide ? 0 : 1) });

	  if (elc) {
	    set(elc,
		{ transition: "all " + duration + "ms " + cssTiming,
		   transform: "translateY(-" + currentHeight + ")" },
		elcStyle);
	  }

      $el.one(transitionEventEnd, function() {
	      if (hide)
		el.style.display = display;

	      restore(el, elStyle);
	      if (effect == SlideInFromTop) {
		el.scrollTop = 0;
		if (elc)
		  restore(elc, elcStyle);
	      }

	      onEnd();
	    });
    }, 0);
    }

    function animateAbsolute(cssSize, cssOffset, topleft, U) {
      if (!hide)
	show();

      var size = WT.px(el, cssSize),
	  hiddenU = (WT.px(el, cssOffset) + size) * (topleft ? -1 : 1);

      var targetU;
      if (hide) {
	set(el, { transform: "translate" + U + "(0px)" }, elStyle);
	targetU = hiddenU;
      } else {
	set(el, { transform: "translate" + U + "(" + hiddenU + "px)" },
	    elStyle);
	targetU = 0;
      }

      if (effects & Fade)
	set(el, { opacity: (hide ? 1 : 0) }, elStyle);

      setTimeout(function() {
	  set(el,
	      { transition: "all " + duration + "ms " + cssTiming,
		 transform: "translate" + U + "(" + targetU + "px)" },
	      elStyle);

	  if (effects & Fade)
	    set(el, { opacity: (hide ? 0 : 1) });

	  $el.one(transitionEventEnd, function() {
	      if (hide)
		el.style.display = display;

	      restore(el, elStyle);

	      onEnd();
	    });
	}, 50); // If this timeout is too small or 0, this will cause some browsers, like
                // Chrome to sometimes not perform the animation at all.
    }

    function animateAbsoluteVertical() {
      animateAbsolute("height", effect == SlideInFromTop ? "top" : "bottom",
		      effect == SlideInFromTop, "Y");
    }

    function animateAbsoluteHorizontal() {
      animateAbsolute("width", effect == SlideInFromLeft ? "left" : "right",
		      effect == SlideInFromLeft, "X");
    }

    function animateTransition() {
      set(el, { animationDuration: duration + 'ms' }, elStyle);

      if (hide)
	$el.removeClass("in");

      var cl;

      switch (effect) {
      case Pop: cl = "pop"; break;
      case SlideInFromLeft: cl = hide ? "slide" : "slide reverse"; break;
      case SlideInFromRight: cl = hide ? "slide reverse" : "slide"; break;
      }

      cl += hide ? " out" : " in";

      if (effects & Fade)
	cl += " fade";

      if (!hide)
	show();

      $el.addClass(cl);
      $el.one(animationEventEnd, function() {
          if (!hide)
	    cl = cl.replace(' in', '');
	  $el.removeClass(cl);
	  if (hide)
	    el.style.display = display;
	  restore(el, elStyle);
	  onEnd();
	});
    }

    setTimeout(function() { 
     var position = $el.css('position'),
       absolute = (position === 'absolute' || position === 'fixed');

     switch (effect) {
     case SlideInFromTop:
     case SlideInFromBottom:
       if (!absolute)
	 animateStaticVertical();
       else
	 animateAbsoluteVertical();
       break;
     case SlideInFromLeft:
     case SlideInFromRight:
       if (absolute)
	 animateAbsoluteHorizontal();
       else
	 animateTransition();
       break;
     case NoEffect:
     case Pop:
       animateTransition();
       break;
     }
   } , 0);
  }
  };

 doAnimateDisplay(id, effects, timing, duration, display);
 }
);

WT_DECLARE_WT_MEMBER
(2, JavaScriptFunction, "animateVisible",
 function(id, effects, timing, duration, visibility, position, top, left) {
 }
);
