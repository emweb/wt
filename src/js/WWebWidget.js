/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptFunction,
  "animateDisplay", /**
   * @param {Object} APP
   * @param {Object} APP.WT
   * @param {Object} APP.layouts2
   * @param {string} id
   * @param {number} effects
   * @param {number} timing
   * @param {number} duration
   * @param {string} display
   */
  function(APP, id, effects, timing, duration, display) {
    const WT = APP.WT;

    /**
     * @param {string} id
     * @param {number} effects
     * @param {number} timing
     * @param {number} duration
     * @param {string} display
     */
    function doAnimateDisplay(id, effects, timing, duration, display) {
      const Effect = {
        None: 0x0,
        SlideInFromLeft: 0x1,
        SlideInFromRight: 0x2,
        SlideInFromBottom: 0x3,
        SlideInFromTop: 0x4,
        Pop: 0x5,
        Fade: 0x100,
      };

      const Timing = {
        Ease: 0,
        Linear: 1,
        EaseIn: 2,
        EaseOut: 3,
        EaseInOut: 4,
        CubicBezier: 5,
      };

      const timings = [
        "ease",
        "linear",
        "ease-in",
        "ease-out",
        "ease-in-out",
      ];

      const inverseTiming = [
        Timing.Ease,
        Timing.Linear,
        Timing.EaseOut,
        Timing.EaseIn,
        Timing.EaseInOut,
        Timing.CubicBezier,
      ];

      const el = document.getElementById(id);
      const elc = el.childElementCount === 1 ? el.firstElementChild : null;

      /**
       * @param {HTMLElement} el
       * @param {Object} style
       * @param {Object} [savedStyle]
       */
      function set(el, style, savedStyle) {
        for (const i of Object.keys(style)) {
          if (savedStyle && typeof savedStyle[i] === "undefined") {
            savedStyle[i] = el.style[i];
          }
          el.style[i] = style[i];
        }
      }

      const restore = set;

      const displayChanged = (() => {
        // Experimentally set display to the given display option, so
        // we can check if the display value actually changed.
        const elStyle = {};
        const prevDisplay = WT.css(el, "display");
        set(el, { display }, elStyle);
        const newDisplay = WT.css(el, "display");
        restore(el, elStyle);
        return prevDisplay !== newDisplay;
      })();

      if (displayChanged) {
        const p = el.parentNode;

        if (p.wtAnimateChild) {
          p.wtAnimateChild(WT, el, effects, timing, duration, { display });
          return;
        }

        if (el.classList.contains("animating")) {
          // already animating, wait until animation is done
          Promise.all(
            el.getAnimations()
              .map((animation) => animation.finished)
          ).then(() => {
            doAnimateDisplay(id, effects, timing, duration, display);
          });
          return;
        }

        el.classList.add("animating");

        const effect = effects & 0xFF,
          hide = display === "none",
          easing = timings[hide ? inverseTiming[timing] : timing],
          elStyle = {},
          elcStyle = {};

        const animationOptions = {
          duration,
          iterations: 1,
          easing,
        };

        function onEnd() {
          if (el.wtAnimatedHidden) {
            el.wtAnimatedHidden(hide);
          }

          if (el.getAnimations().length === 0) {
            el.classList.remove("animating");
          }

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
          if (!hide) {
            show();
          }

          set(el, { overflow: "hidden" }, elStyle);

          const animations = [];

          const elcPos = (() => {
            if (!elc) {
              return null;
            }
            const elRect = el.getBoundingClientRect();
            const elcRect = elc.getBoundingClientRect();
            return {
              left: elcRect.x - elRect.x - WT.px(el, "border-left-width"),
              top: elcRect.y - elRect.y - WT.px(el, "border-top-width"),
            };
          })();

          if (elc && effect === Effect.SlideInFromTop) {
            // Content slides along
            const visibleKeyframe = {
              transform: "translateY(0px)",
            };
            const hiddenKeyframe = {
              transform: `translateY(-${el.clientHeight}px)`,
            };
            const keyframes = hide ? [visibleKeyframe, hiddenKeyframe] : [hiddenKeyframe, visibleKeyframe];
            animations.push(elc.animate(keyframes, animationOptions));
          }

          {
            const hiddenKeyframe = {
              height: "0px",
              paddingTop: "0px",
              paddingBottom: "0px",
              borderTopWidth: "0px",
              borderBottomWidth: "0px",
            };
            const visibleKeyframe = {
              height: WT.css(el, "height"),
              paddingTop: WT.css(el, "padding-top"),
              paddingBottom: WT.css(el, "padding-bottom"),
              borderTopWidth: WT.css(el, "border-top-width"),
              borderBottomWidth: WT.css(el, "border-bottom-width"),
            };

            if (effects & Effect.Fade) {
              hiddenKeyframe.opacity = 0;
              visibleKeyframe.opacity = 1;
            }

            const keyframes = hide ? [visibleKeyframe, hiddenKeyframe] : [hiddenKeyframe, visibleKeyframe];
            animations.push(el.animate(keyframes, animationOptions));
          }

          if (elc) {
            // Fix content position
            set(el, { position: "relative" }, elStyle);
            const style = {
              position: "absolute",
              left: `${elcPos.left}px`,
              top: `${elcPos.top}px`,
            };
            set(elc, style, elcStyle);
          }

          Promise.all(animations.map((animation) => animation.finished)).then(() => {
            if (hide) {
              el.style.display = display;
            }

            restore(el, elStyle);
            restore(elc, elcStyle);

            onEnd();
          });
        }

        /**
         * @param {string} cssSize
         * @param {string} cssOffset
         * @param {boolean} topleft
         * @param {string} U
         */
        function animateAbsolute(cssSize, cssOffset, topleft, U) {
          if (!hide) {
            show();
          }

          const size = WT.px(el, cssSize),
            hiddenU = (WT.px(el, cssOffset) + size) * (topleft ? -1 : 1);

          const hiddenKeyframe = {
            transform: `translate${U}(${hiddenU}px)`,
          };
          const visibleKeyframe = {
            transform: `translate${U}(0px)`,
          };

          if (effects & Effect.Fade) {
            hiddenKeyframe.opacity = 0;
            visibleKeyframe.opacity = 1;
          }

          const keyframes = hide ? [visibleKeyframe, hiddenKeyframe] : [hiddenKeyframe, visibleKeyframe];
          const animation = el.animate(keyframes, animationOptions);

          animation.finished.then(() => {
            if (hide) {
              el.style.display = display;
            }

            onEnd();
          });
        }

        function animateAbsoluteVertical() {
          animateAbsolute(
            "height",
            effect === Effect.SlideInFromTop ? "top" : "bottom",
            effect === Effect.SlideInFromTop,
            "Y"
          );
        }

        function animateAbsoluteHorizontal() {
          animateAbsolute(
            "width",
            effect === Effect.SlideInFromLeft ? "left" : "right",
            effect === Effect.SlideInFromLeft,
            "X"
          );
        }

        function animateTransition() {
          if (!hide) {
            show();
          }

          const hiddenKeyframe = {};
          const visibleKeyframe = {};
          switch (effect) {
            case Effect.Pop:
              set(el, { transformOrigin: "50% 50%" }, elStyle);
              hiddenKeyframe.transform = "scale(.2)";
              visibleKeyframe.transform = "scale(1)";
              break;
            case Effect.SlideInFromRight:
              hiddenKeyframe.transform = "translateX(100%)";
              // hiddenKeyframe.clipPath = 'polygon(-100% 0%, 0% 0%, 0% 100%, -100% 100%)';
              visibleKeyframe.transform = "translateX(0)";
              // visibleKeyframe.clipPath = 'polygon(-100% 0%, 100% 0%, 100% 100%, -100% 100%)';
              break;
            case Effect.SlideInFromLeft:
              hiddenKeyframe.transform = "translateX(-100%)";
              // hiddenKeyframe.clipPath = 'polygon(100% 0%, 200% 0%, 200% 100%, 100% 100%)';
              visibleKeyframe.transform = "translateX(0)";
              // visibleKeyframe.clipPath = 'polygon(0% 0%, 200% 0%, 200% 100%, 0% 100%)';
              break;
          }
          if (effects & Effect.Fade) {
            hiddenKeyframe.opacity = 0;
            visibleKeyframe.opacity = WT.css(el, "opacity");
          }

          const keyframes = hide ? [visibleKeyframe, hiddenKeyframe] : [hiddenKeyframe, visibleKeyframe];
          const animation = el.animate(keyframes, animationOptions);

          animation.finished.then(() => {
            if (hide) {
              el.style.display = display;
            }

            restore(el, elStyle);

            onEnd();
          });
        }

        const absolute = ["absolute", "fixed"].includes(WT.css(el, "position"));

        switch (effect) {
          case Effect.SlideInFromTop:
          case Effect.SlideInFromBottom:
            if (absolute) {
              animateAbsoluteVertical();
            } else {
              animateStaticVertical();
            }
            break;
          case Effect.SlideInFromLeft:
          case Effect.SlideInFromRight:
            if (absolute) {
              animateAbsoluteHorizontal();
            } else {
              animateTransition();
            }
            break;
          case Effect.None:
          case Effect.Pop:
            animateTransition();
            break;
        }
      }
    }

    doAnimateDisplay(id, effects, timing, duration, display);
  }
);

WT_DECLARE_WT_MEMBER(
  2,
  JavaScriptFunction,
  "animateVisible",
  function(_id, _effects, _timing, _duration, _visibility, _position, _top, _left) {
  }
);
