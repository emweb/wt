/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  10,
  JavaScriptFunction,
  "toolTip",
  function(APP, id, text, isForceShow, deferred, isShowOnHover, ToolTipInnerStyle, ToolTipOuterStyle) {
    const el = document.getElementById(id);
    const WT = APP.WT;

    const obj = el.toolTip;

    if (!obj) {
      el.toolTip = new function() {
        let showTimer = null, checkInt = null, coords = null, toolTipEl = null;
        const MouseDistance = 10;
        const Delay = 500;
        const HideDelay = 200;
        let waitingForText = false, toolTipText = text;
        let forceShow = isForceShow;
        let showOnHover = isShowOnHover;

        let overTooltip = false;

        function checkIsOver() {
          if (!document.querySelectorAll(`#${id}:hover`).length) {
            tryHideToolTip();
          }
        }

        function loadToolTipText() {
          waitingForText = true;
          APP.emit(el, "Wt-loadToolTip");
        }

        this.setToolTipText = function(text) {
          toolTipText = text;
          if (waitingForText || forceShow) {
            this.showToolTip();
            clearTimeout(showTimer);
            waitingForText = false;
          }
        };

        this.setVisibilityParams = function(isForceShow, isShowOnHover) {
          forceShow = isForceShow;
          showOnHover = isShowOnHover;
          hideToolTip();
        };

        this.showToolTip = function() {
          if (!showOnHover && !forceShow) {
            return;
          }

          if (deferred && !toolTipText && !waitingForText) {
            loadToolTipText();
          }

          if (toolTipText) {
            hideToolTip(); // delete previous toolTipEl before creating a new one.

            toolTipEl = document.createElement("div");
            toolTipEl.className = ToolTipInnerStyle;
            toolTipEl.innerHTML = toolTipText;

            const outerDiv = document.createElement("div");
            outerDiv.className = ToolTipOuterStyle;

            document.body.appendChild(outerDiv);
            outerDiv.appendChild(toolTipEl);

            if (forceShow) {
              const x = el.offsetLeft + el.offsetWidth / 2, y = el.offsetTop + el.offsetHeight;
              WT.fitToWindow(outerDiv, x + MouseDistance, y + 2 * MouseDistance, x - MouseDistance, y);
            } else {
              const x = coords.x, y = coords.y;
              WT.fitToWindow(outerDiv, x + MouseDistance, y + MouseDistance, x - MouseDistance, y - MouseDistance);
            }

            // bring tooltip to front if there are dialogs
            let maxz = 0;
            const oldZIndex = parseInt(WT.css(outerDiv, "zIndex"), 10);
            document.querySelectorAll(".Wt-dialog, .modal, .modal-dialog").forEach(function(elem) {
              maxz = Math.max(maxz, parseInt(WT.css(elem, "zIndex"), 10));
              if (maxz > oldZIndex) {
                const newZIndex = maxz + 1000;
                outerDiv.style["zIndex"] = newZIndex;
              }
            });

            toolTipEl.addEventListener("mouseenter", function() {
              overTooltip = true;
            });
            toolTipEl.addEventListener("mouseleave", function() {
              overTooltip = false;
            });
          }

          clearInterval(checkInt);
          checkInt = null;
          checkInt = setInterval(function() {
            checkIsOver();
          }, 200);
        };

        function tryHideToolTip() {
          clearTimeout(showTimer);
          setTimeout(function() {
            if ((overTooltip && showOnHover) || forceShow) {
              return;
            }
            hideToolTip();
          }, HideDelay);
        }

        function hideToolTip() {
          if (toolTipEl) {
            toolTipEl.parentElement.remove();
            toolTipEl = null;
            clearInterval(checkInt);
            checkInt = null;
            overTooltip = false;
          }
        }

        function resetTimer(e) {
          clearTimeout(showTimer);
          coords = WT.pageCoordinates(e);

          if (!toolTipEl) {
            showTimer = setTimeout(function() {
              el.toolTip.showToolTip();
            }, Delay);
          }
        }

        el.addEventListener("mouseenter", resetTimer);
        el.addEventListener("mousemove", resetTimer);
        el.addEventListener("mouseleave", tryHideToolTip);

        if (forceShow) {
          this.showToolTip();
        }
      }();
    }

    if (obj) {
      obj.setVisibilityParams(isForceShow, isShowOnHover);
      obj.setToolTipText(text);
    }
  }
);
