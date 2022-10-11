/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptFunction, "animateDisplay", function(APP, id, effects, timing, duration, display) {
  const WT = APP.WT;

  const doAnimateDisplay = function(id, effects, timing, duration, display) {
    const NoEffect = 0x0;
    const SlideInFromLeft = 0x1;
    const SlideInFromRight = 0x2;
    const SlideInFromBottom = 0x3;
    const SlideInFromTop = 0x4;
    const Pop = 0x5;
    const Fade = 0x100;

    // const Ease = 0;
    // const Linear = 1;
    // const EaseIn = 2;
    // const EaseOut = 3;
    // const EaseInOut = 4;
    // const CubicBezier = 5;

    const timings = ["ease", "linear", "ease-in", "ease-out", "ease-in-out"],
      inverseTiming = [0, 1, 3, 2, 4, 5];

    const animationPrefix = WT.vendorPrefix(WT.styleAttribute("animation"));
    const transitionPrefix = WT.vendorPrefix(WT.styleAttribute("transition"));
    const transformPrefix = WT.vendorPrefix(WT.styleAttribute("transform"));
    const el = WT.$(id),
      animationEventEnd = animationPrefix === "Webkit" ?
        "webkitAnimationEnd" :
        "animationend",
      transitionEventEnd = transitionPrefix === "Webkit" ?
        "webkitTransitionEnd" :
        "transitionend";

    if (WT.css(el, "display") !== display) {
      const p = el.parentNode;

      if (p.wtAnimateChild) {
        p.wtAnimateChild(WT, el, effects, timing, duration, { display: display });
        return;
      }

      if (el.classList.contains("animating")) {
        el.addEventListener(transitionEventEnd, function() {
          doAnimateDisplay(id, effects, timing, duration, display);
        }, { once: true });
        return;
      }

      el.classList.add("animating");

      const effect = effects & 0xFF,
        hide = display === "none",
        cssTiming = timings[hide ? inverseTiming[timing] : timing],
        elStyle = {};

      function set(el, style, savedStyle) {
        for (const i of Object.keys(style)) {
          let k = i;

          if (k === "animationDuration" && animationPrefix !== "") {
            k = animationPrefix + k.substring(0, 1).toUpperCase() +
              k.substring(1);
          } else if (k === "transform" && transformPrefix !== "") {
            k = transformPrefix + k.substring(0, 1).toUpperCase() +
              k.substring(1);
          } else if (k === "transition" && transitionPrefix !== "") {
            k = transitionPrefix + k.substring(0, 1).toUpperCase() +
              k.substring(1);
          }

          if (savedStyle && typeof (savedStyle[k]) === "undefined") {
            savedStyle[k] = el.style[k];
          }
          el.style[k] = style[i];
        }
      }

      const restore = set;

      function onEnd() {
        if (el.wtAnimatedHidden) {
          el.wtAnimatedHidden(hide);
        }

        el.classList.remove("animating");

        if (APP.layouts2) {
          APP.layouts2.setElementDirty(el);
        }
      }

      function show() {
        el.style.display = display;
        if (el.wtPosition) {
          el.wtPosition();
        }
        if (window.onshow) {
          window.onshow();
        }
      }

      function animateStaticVertical() {
        let targetHeight, currentHeight;
        const elcStyle = {};
        let elc;

        if (hide) {
          currentHeight = WT.css(el, "height");
          set(el, { height: currentHeight, overflow: "hidden" }, elStyle);

          if (effect === SlideInFromTop && el.childNodes.length === 1) {
            elc = el.firstChild;
            set(elc, { transform: "translateY(0)" }, elcStyle);
            if (!WT.hasTag(elc, "TABLE")) {
              set(elc, { display: "block" }, elcStyle);
            }
          }

          targetHeight = "0px";
        } else {
          const pStyle = {};

          set(p, { height: WT.css(p, "height"), overflow: "hidden" }, pStyle);

          show();

          if (WT.css(el, "height") === "0px") {
            el.style.height = "auto";
          }

          targetHeight = WT.css(el, "height");
          set(el, { height: "0px", overflow: "hidden" }, elStyle);
          restore(p, pStyle);

          if (effect === SlideInFromTop) {
            set(el, { WebkitBackfaceVisibility: "visible" }, elStyle);
            el.scrollTop = 1000;
          }
        }

        if (effects & Fade) {
          set(el, { opacity: (hide ? 1 : 0) }, elStyle);
        }

        currentHeight = el.clientHeight; // force 'absorbing' set height

        setTimeout(function() {
          set(el, { transition: "all " + duration + "ms " + cssTiming, height: targetHeight }, elStyle);

          if (effects & Fade) {
            set(el, { opacity: (hide ? 0 : 1) });
          }

          if (elc) {
            set(elc, {
              transition: "all " + duration + "ms " + cssTiming,
              transform: "translateY(-" + currentHeight + ")",
            }, elcStyle);
          }

          el.addEventListener(transitionEventEnd, function() {
            if (hide) {
              el.style.display = display;
            }

            restore(el, elStyle);
            if (effect === SlideInFromTop) {
              el.scrollTop = 0;
              if (elc) {
                restore(elc, elcStyle);
              }
            }

            onEnd();
          }, { once: true });
        }, 0);
      }

      function animateAbsolute(cssSize, cssOffset, topleft, U) {
        if (!hide) {
          show();
        }

        const size = WT.px(el, cssSize),
          hiddenU = (WT.px(el, cssOffset) + size) * (topleft ? -1 : 1);

        let targetU;
        if (hide) {
          set(el, { transform: "translate" + U + "(0px)" }, elStyle);
          targetU = hiddenU;
        } else {
          set(el, { transform: "translate" + U + "(" + hiddenU + "px)" }, elStyle);
          targetU = 0;
        }

        if (effects & Fade) {
          set(el, { opacity: (hide ? 1 : 0) }, elStyle);
        }

        setTimeout(function() {
          set(el, {
            transition: "all " + duration + "ms " + cssTiming,
            transform: "translate" + U + "(" + targetU + "px)",
          }, elStyle);

          if (effects & Fade) {
            set(el, { opacity: (hide ? 0 : 1) });
          }

          el.addEventListener(transitionEventEnd, function() {
            if (hide) {
              el.style.display = display;
            }

            restore(el, elStyle);

            onEnd();
          }, { once: true });
        }, 50); // If this timeout is too small or 0, this will cause some browsers, like
        // Chrome to sometimes not perform the animation at all.
      }

      function animateAbsoluteVertical() {
        animateAbsolute("height", effect === SlideInFromTop ? "top" : "bottom", effect === SlideInFromTop, "Y");
      }

      function animateAbsoluteHorizontal() {
        animateAbsolute("width", effect === SlideInFromLeft ? "left" : "right", effect === SlideInFromLeft, "X");
      }

      function animateTransition() {
        set(el, { animationDuration: duration + "ms" }, elStyle);

        if (hide) {
          el.classList.remove("in");
        }

        let cl;

        switch (effect) {
          case Pop:
            cl = "pop";
            break;
          case SlideInFromLeft:
            cl = hide ? "slide" : "slide reverse";
            break;
          case SlideInFromRight:
            cl = hide ? "slide reverse" : "slide";
            break;
        }

        cl += hide ? " out" : " in";

        if (effects & Fade) {
          cl += " fade";
        }

        if (!hide) {
          show();
        }

        el.classList.add(...cl.split(" ").filter((el) => el !== ""));
        el.addEventListener(animationEventEnd, function() {
          if (!hide) {
            cl = cl.replace(" in", "");
          }
          el.classList.remove(...cl.split(" ").filter((el) => el !== ""));
          if (hide) {
            el.style.display = display;
          }
          restore(el, elStyle);
          onEnd();
        }, { once: true });
      }

      setTimeout(function() {
        const position = WT.css(el, "position"),
          absolute = (position === "absolute" || position === "fixed");

        switch (effect) {
          case SlideInFromTop:
          case SlideInFromBottom:
            if (!absolute) {
              animateStaticVertical();
            } else {
              animateAbsoluteVertical();
            }
            break;
          case SlideInFromLeft:
          case SlideInFromRight:
            if (absolute) {
              animateAbsoluteHorizontal();
            } else {
              animateTransition();
            }
            break;
          case NoEffect:
          case Pop:
            animateTransition();
            break;
        }
      }, 0);
    }
  };

  doAnimateDisplay(id, effects, timing, duration, display);
});

WT_DECLARE_WT_MEMBER(
  2,
  JavaScriptFunction,
  "animateVisible",
  function(_id, _effects, _timing, _duration, _visibility, _position, _top, _left) {
  }
);
