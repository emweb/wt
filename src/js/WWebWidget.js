WT_DECLARE_WT_MEMBER
(1, JavaScriptFunction, "animateDisplay",
 function(id, effects, timing, duration, display) {

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

  var timings = [ "ease", "linear", "ease-in", "ease-out", "ease-in-out" ];
  var inverseTiming = [ 0, 1, 3, 2, 4, 5 ];

  var WT = this, $el = $("#" + id), el = $el.get(0);

  if ($el.css("display") !== display) {
    var p = $el.get(0).parentNode;

    if (p.wtAnimateChild) {
      p.wtAnimateChild($el.get(0), effects, timing, duration,
		       { display: display });
      return;
    }

    var effect = effects & 0xFF,
      hide = display === "none",
      cssTiming = timings[hide ? inverseTiming[timing] : timing],
      elStyle = {};

    function set(el, style, savedStyle) {
      var i, il;

      for (i in style) {
	if (savedStyle && typeof(savedStyle[i]) === "undefined")
	  savedStyle[i] = el.style[i];
	el.style[i] = style[i];
      }
    }

    var restore = set;

    function show() {
      el.style.display = display;
      if (el.wtPosition)
	el.wtPosition();
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
	  set(elc, { webkitTransform: "translateY(0)" }, elcStyle);
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
	targetHeight = $el.height() + "px";

	set(el,
	    {   height: "0px",
	      overflow: "hidden" },
	    elStyle);
	restore(p, pStyle);

	if (effect == SlideInFromTop)
	  el.scrollTop = 1000;
      }

      if (effects & Fade)
	set(el, { opacity: (hide ? 1 : 0) }, elStyle);

      setTimeout(function() {
	  set(el,
	      { webkitTransition: "all " + duration + "ms " + cssTiming,
		          height: targetHeight },
	      elStyle);

	  if (effects & Fade)
	    set(el, { opacity: (hide ? 0 : 1) });

	  if (elc) {
	    set(elc,
		{ webkitTransition: "-webkit-transform "
		                    + duration + "ms " + cssTiming,
		    webkitTransform: "translateY(-" + currentHeight + ")" },
		elcStyle);
	  }

	  $el.one("webkitTransitionEnd", function() {
	      if (hide)
		el.style.display = display;

	      restore(el, elStyle);
	      if (effect == SlideInFromTop) {
		el.scrollTop = 0;
		if (elc)
		  restore(elc, elcStyle);
	      }
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
	set(el, { webkitTransform: "translate" + U + "(0px)" }, elStyle);
	targetU = hiddenU;
      } else {
	set(el, { webkitTransform: "translate" + U + "(" + hiddenU + "px)" },
	    elStyle);
	targetU = 0;
      }

      if (effects & Fade)
	set(el, { opacity: (hide ? 1 : 0) }, elStyle);

      setTimeout(function() {
	  set(el,
	      { webkitTransition: "all " + duration + "ms " + cssTiming,
		 webkitTransform: "translate" + U + "(" + targetU + "px)" },
	      elStyle);

	  if (effects & Fade)
	    set(el, { opacity: (hide ? 0 : 1) });

	  $el.one("webkitTransitionEnd", function() {
	      if (hide)
		el.style.display = display;

	      restore(el, elStyle);
	    });
	}, 0);
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
      set(el, { webkitAnimationDuration: duration + 'ms' }, elStyle);

      var cl = (effect == Pop ? "pop " : "") + (hide ? "out" : "in");
      if (effects & Fade)
	cl += " fade";

      if (!hide)
	show();

      $el.addClass(cl);
      $el.one('webkitAnimationEnd', function() {
	  $el.removeClass(cl);
	  el.style.display = display;
	  restore(el, elStyle);
	});
    }

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
   }
 }
);

WT_DECLARE_WT_MEMBER
(2, JavaScriptFunction, "animateVisible",
 function(id, effects, timing, duration, visibility, position, top, left) {
 }
);
