/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER_BIG(
  1,
  JavaScriptConstructor,
  "WCartesianChart", // target: the WPaintedWidget JavaScript obj, with:
  //   repaint
  //   canvas
  //   combinedTransform is set by WCartesianChart
  // config: the initial configuration (can be overridden with updateConfig)
  //   curveManipulation (determines whether series manipulation is enabled)
  //   seriesSelection (determines whether series selection is enabled)
  //   selectedCurve (the curve that is currently selected)
  //   isHorizontal (if orientation() == Horizontal)
  //   xTransforms[]
  //   yTransforms[]
  //   area (WRectF describing render area)
  //   insideArea (WRectF describing render area without axis margins)
  //   xModelAreas ([WRectF] describing model areas per X axis)
  //   yModelAreas ([WRectF] describing model areas per Y axis)
  //   minZoom {x: [float], y: [float]}
  //   maxZoom {x: [float], y: [float]}
  //   rubberBand
  //   zoom (bool)
  //   pan (bool)
  //   crosshair (bool)
  //   crosshairXAxis (int)
  //   crosshairYAxis (int)
  //   crosshairColor (css text)
  //   followCurve (int, -1 for disabled)
  //   notifyTransform {x: [bool], y: [bool]} // Whether we should emit a signal on X or Y transform change
  //   series {modelColumn: {curve: curve ref, transform: transform ref, xAxis: int, yAxis: int},...}
  //   ToolTipInnerStyle // Style for the tooltip
  //   ToolTipOuterStyle // Style for the tooltip
  //   hasToolTips // whether there are any tooltips
  //   pens // {x: [[linePen,textPen,gridPen],...], y: [[linePen,textPen,gridPen],...] }
  //   penAlpha: {x: [[linePenAlpha,textPenAlpha,gridPenAlpha],...], y: [[linePenAlpha,textPenAlpha,gridPenAlpha],...] }
  //   xAxes: [{width: float, side: side, minOffset: float, maxOffset: float}] side = min max zero both
  //   yAxes: [{width: float, side: side, minOffset: float, maxOffset: float}] side = min max zero both
  //
  function(APP, widget, target, config) {
    widget.wtCObj = this;

    const self = this;
    const WT = APP.WT;

    self.config = config;

    const utils = WT.gfxUtils;
    const mult = utils.transform_mult;
    const inverted = utils.transform_inverted;
    const assign = utils.transform_assign;
    const equal = utils.transform_equal;
    const apply = utils.transform_apply;
    const top = utils.rect_top;
    const bottom = utils.rect_bottom;
    const left = utils.rect_left;
    const right = utils.rect_right;
    const intersection = utils.rect_intersection;

    const chartCommon = WT.chartCommon;
    const minMaxY = chartCommon.minMaxY;
    const findClosestPoint = chartCommon.findClosestPoint;
    const projection = chartCommon.projection;
    const distanceLessThanRadius = chartCommon.distanceLessThanRadius;
    const toZoomLevel = chartCommon.toZoomLevel;
    const isPointInRect = chartCommon.isPointInRect;
    const findYRange = chartCommon.findYRange;
    const matchXAxis = function(x, y) {
      return chartCommon.matchXAxis(x, y, configArea(), config.xAxes, isHorizontal());
    };
    const matchYAxis = function(x, y) {
      return chartCommon.matchYAxis(x, y, configArea(), config.yAxes, isHorizontal());
    };

    // Functions that help in making minification more effective
    function xModelArea(ax) {
      return config.xModelAreas[ax];
    }
    function yModelArea(ax) {
      return config.yModelAreas[ax];
    }
    function modelArea(xAx, yAx) {
      const xArea = xModelArea(xAx);
      const yArea = yModelArea(yAx);
      if (isHorizontal()) {
        return [yArea[0], xArea[1], yArea[2], xArea[3]];
      } else {
        return [xArea[0], yArea[1], xArea[2], yArea[3]];
      }
    }
    function followCurve() {
      return config.followCurve;
    }
    function showCrosshair() {
      return config.crosshair || followCurve() !== -1;
    }
    function isHorizontal() {
      return config.isHorizontal;
    }
    function xTransform(ax) {
      return config.xTransforms[ax];
    }
    function yTransform(ax) {
      return config.yTransforms[ax];
    }
    function configArea() {
      return config.area;
    }
    function insideArea() {
      return config.insideArea;
    }
    function configSeries(seriesNb = null) {
      if (seriesNb === null) {
        return config.series;
      } else {
        return config.series[seriesNb];
      }
    }
    function seriesTransform(seriesNb) {
      return configSeries(seriesNb).transform;
    }
    function curveTransform(seriesNb) {
      if (isHorizontal()) {
        return mult([0, 1, 1, 0, 0, 0], mult(seriesTransform(seriesNb), [0, 1, 1, 0, 0, 0]));
      } else {
        return seriesTransform(seriesNb);
      }
    }
    function seriesCurve(seriesNb) {
      return configSeries(seriesNb).curve;
    }
    function seriesXAxis(seriesNb) {
      return configSeries(seriesNb).xAxis;
    }
    function seriesYAxis(seriesNb) {
      return configSeries(seriesNb).yAxis;
    }
    function seriesSelection() {
      return config.seriesSelection;
    }
    function sliders() {
      return config.sliders;
    }
    function hasToolTips() {
      return config.hasToolTips;
    }
    function coordinateOverlayPadding() {
      return config.coordinateOverlayPadding;
    }
    function curveManipulation() {
      return config.curveManipulation;
    }
    function minXZoom(ax) {
      return config.minZoom.x[ax];
    }
    function minYZoom(ax) {
      return config.minZoom.y[ax];
    }
    function maxXZoom(ax) {
      return config.maxZoom.x[ax];
    }
    function maxYZoom(ax) {
      return config.maxZoom.y[ax];
    }
    function pens() {
      return config.pens;
    }
    function penAlpha() {
      return config.penAlpha;
    }
    function configSelectedCurve() {
      return config.selectedCurve;
    }
    function preventDefault(e) {
      if (e.preventDefault) {
        e.preventDefault();
      }
    }
    function addEventListener(e, l) {
      widget.addEventListener(e, l);
    }
    function removeEventListener(e, l) {
      widget.removeEventListener(e, l);
    }
    function len(ar) {
      return ar.length;
    }
    function xAxisCount() {
      return len(config.xAxes);
    }
    function yAxisCount() {
      return len(config.yAxes);
    }
    function notifyAnyTransform() {
      for (let i = 0; i < xAxisCount(); ++i) {
        if (config.notifyTransform.x[i]) {
          return true;
        }
      }
      for (let i = 0; i < yAxisCount(); ++i) {
        if (config.notifyTransform.y[i]) {
          return true;
        }
      }
      return false;
    }
    function crosshairXAxis() {
      return config.crosshairXAxis;
    }
    function crosshairYAxis() {
      return config.crosshairYAxis;
    }

    const ANIMATION_INTERVAL = 17;
    let framePending = false;
    const rqAnimFrameThrottled = function(cb) {
      if (framePending) {
        return;
      }
      framePending = true;
      requestAnimationFrame(function() {
        cb();
        framePending = false;
      });
    };

    if (window.MSPointerEvent || window.PointerEvent) {
      widget.style.touchAction = "none";
      target.canvas.style.msTouchAction = "none";
      target.canvas.style.touchAction = "none";
    }

    const NO_LIMIT = 1, DAMPEN = 2; // bit flags
    const X_ONLY = 1, Y_ONLY = 2; // bit flags
    const X = 0, Y = 1;
    const LOOK_MODE = 0, CROSSHAIR_MODE = 1;
    const WHEEL_ZOOM_X = 0,
      WHEEL_ZOOM_Y = 1,
      WHEEL_ZOOM_XY = 2,
      WHEEL_ZOOM_MATCHING = 3,
      WHEEL_PAN_X = 4,
      WHEEL_PAN_Y = 5,
      WHEEL_PAN_MATCHING = 6;

    const SERIES_SELECTION_TIMEOUT = 200; // ms
    const TRANSFORM_CHANGED_TIMEOUT = 250; // ms
    const TOOLTIP_TIMEOUT = 500; // ms
    const TOOLTIP_HIDE_DELAY = 200; // ms

    const FRICTION_FACTOR = 0.003, // Determines how strongly the speed decreases, when animating
      SPRING_CONSTANT = 0.0002, // How strongly the spring pulls, relative to how extended it is
      RESISTANCE_FACTOR = 0.07, // How strongly the spring resists movement, when dragging
      BOUNDS_SLACK = 3, // The amount of slack to apply to determine whether an area is within bounds
      MIN_SPEED = 0.001, // The minimum speed that the animation should pan at when out of bounds,
      // ensures that the animation does not stop prematurely.
      MAX_SPEED = 1.5, // The maximum speed that we should cap to, to prevent glitchy stuff
      STOPPING_SPEED = 0.02; // If the speed is below the stopping speed, and we're inside of bounds,
    // then we can stop the animation.

    // eobj2: an object to hold the context menu listener, that simply prevents the default behaviour,
    //        so that a long press in order to select a series is not interpreted as a right click
    let eobj2 = widget.wtEObj2;
    if (!eobj2) {
      eobj2 = {};
      eobj2.contextmenuListener = function(e) {
        preventDefault(e);
        removeEventListener("contextmenu", eobj2.contextmenuListener);
      };
    }
    widget.wtEObj2 = eobj2;

    const touchHandlers = {};

    function isTouchEvent(event) {
      return event.pointerType === 2 || event.pointerType === 3 ||
        event.pointerType === "pen" || event.pointerType === "touch";
    }

    let pointerActive = false;

    if (window.MSPointerEvent || window.PointerEvent) {
      (function() {
        const pointers = [];

        function updatePointerActive() {
          pointerActive = len(pointers) > 0;
        }

        function pointerDown(event) {
          if (!isTouchEvent(event)) {
            return;
          }
          preventDefault(event);
          pointers.push(event);

          updatePointerActive();
          touchHandlers.start(widget, { touches: pointers.slice(0) });
        }

        function pointerUp(event) {
          if (!pointerActive) {
            return;
          }
          if (!isTouchEvent(event)) {
            return;
          }
          preventDefault(event);
          for (let i = 0; i < len(pointers); ++i) {
            if (pointers[i].pointerId === event.pointerId) {
              pointers.splice(i, 1);
              break;
            }
          }

          updatePointerActive();
          touchHandlers.end(widget, { touches: pointers.slice(0), changedTouches: [] });
        }

        function pointerMove(event) {
          if (!isTouchEvent(event)) {
            return;
          }
          preventDefault(event);
          for (let i = 0; i < len(pointers); ++i) {
            if (pointers[i].pointerId === event.pointerId) {
              pointers[i] = event;
              break;
            }
          }

          updatePointerActive();
          touchHandlers.moved(widget, { touches: pointers.slice(0) });
        }

        // eobj: an object for holding the handlers so we can properly register/unregister them,
        //       even when the chart gets updated
        const o = widget.wtEObj;
        if (o) {
          if (!window.PointerEvent) {
            removeEventListener("MSPointerDown", o.pointerDown);
            removeEventListener("MSPointerUp", o.pointerUp);
            removeEventListener("MSPointerOut", o.pointerUp);
            removeEventListener("MSPointerMove", o.pointerMove);
          } else {
            removeEventListener("pointerdown", o.pointerDown);
            removeEventListener("pointerup", o.pointerUp);
            removeEventListener("pointerout", o.pointerUp);
            removeEventListener("pointermove", o.pointerMove);
          }
        }
        widget.wtEObj = {
          pointerDown: pointerDown,
          pointerUp: pointerUp,
          pointerMove: pointerMove,
        };
        if (!window.PointerEvent) {
          addEventListener("MSPointerDown", pointerDown);
          addEventListener("MSPointerUp", pointerUp);
          addEventListener("MSPointerOut", pointerUp);
          addEventListener("MSPointerMove", pointerMove);
        } else {
          addEventListener("pointerdown", pointerDown);
          addEventListener("pointerup", pointerUp);
          addEventListener("pointerout", pointerUp);
          addEventListener("pointermove", pointerMove);
        }
      })();
    }

    // oobj: the <canvas> for drawing the crosshair
    /** @type {?HTMLCanvasElement} */
    let overlay = widget.wtOObj ?? null;

    let crosshair = null;

    let paintEnabled = true;
    let dragPreviousXY = null;
    let dragCurrentXAxis = -1;
    let dragCurrentYAxis = -1;
    let touches = [];
    let singleTouch = false;
    let doubleTouch = false;
    let zoomAngle = null;
    let zoomMiddle = null;
    let zoomProjection = null;

    const v = { x: 0, y: 0 };

    let seriesSelectionTimeout = null;
    let lastDate = null;

    let tobj = widget.wtTObj;
    if (!tobj) {
      tobj = { overTooltip: false };
      widget.wtTObj = tobj;
    }

    function hideTooltip() {
      if (!tobj) {
        return;
      }
      if (tobj.tooltipTimeout) {
        clearTimeout(tobj.tooltipTimeout);
        tobj.tooltipTimeout = null;
      }
      if (tobj.overTooltip) {
        return;
      }
      if (tobj.tooltipOuterDiv) {
        document.body.removeChild(tobj.tooltipOuterDiv);
        tobj.toolTipEl = null;
        tobj.tooltipOuterDiv = null;
      }
    }

    let mode = null;

    let animating = false;

    let transformChangedTimeout = null;
    const oldXTransforms = [];
    for (let i = 0; i < xAxisCount(); ++i) {
      oldXTransforms.push([0, 0, 0, 0, 0, 0]);
      assign(oldXTransforms[i], xTransform(i));
    }
    const oldYTransforms = [];
    for (let i = 0; i < yAxisCount(); ++i) {
      oldYTransforms.push([0, 0, 0, 0, 0, 0]);
      assign(oldYTransforms[i], yTransform(i));
    }
    function setTransformChangedTimeout() {
      if (!notifyAnyTransform()) {
        return;
      }
      if (transformChangedTimeout) {
        window.clearTimeout(transformChangedTimeout);
        transformChangedTimeout = null;
      }
      transformChangedTimeout = setTimeout(function() {
        for (let i = 0; i < xAxisCount(); ++i) {
          if (config.notifyTransform.x[i] && !equal(oldXTransforms[i], xTransform(i))) {
            APP.emit(target.widget, "xTransformChanged" + i);
            assign(oldXTransforms[i], xTransform(i));
          }
        }
        for (let i = 0; i < yAxisCount(); ++i) {
          if (config.notifyTransform.y[i] && !equal(oldYTransforms[i], yTransform(i))) {
            APP.emit(target.widget, "yTransformChanged" + i);
            assign(oldYTransforms[i], yTransform(i));
          }
        }
      }, TRANSFORM_CHANGED_TIMEOUT);
    }
    const tAssign = function(a, b) {
      assign(a, b);
      setTransformChangedTimeout();
    };

    function combinedTransform(xAx = 0, yAx = 0) {
      if (isHorizontal()) {
        const l = left(configArea());
        const t = top(configArea());
        return mult([0, 1, 1, 0, l, t], mult(xTransform(xAx), mult(yTransform(yAx), [0, 1, 1, 0, -t, -l])));
      } else {
        const l = left(configArea());
        const b = bottom(configArea());
        return mult([1, 0, 0, -1, l, b], mult(xTransform(xAx), mult(yTransform(yAx), [1, 0, 0, -1, -l, b])));
      }
    }
    target.combinedTransform = combinedTransform;

    function transformedInsideChartArea(xAx, yAx) {
      return mult(combinedTransform(xAx, yAx), insideArea());
    }

    function toModelCoord(p, xAx, yAx, noTransform = false) {
      let res;
      if (noTransform) {
        res = p;
      } else {
        res = mult(inverted(combinedTransform(xAx, yAx)), p);
      }
      let u;
      if (isHorizontal()) {
        u = [(res[Y] - configArea()[1]) / configArea()[3], (res[X] - configArea()[0]) / configArea()[2]];
      } else {
        u = [(res[X] - configArea()[0]) / configArea()[2], 1 - (res[Y] - configArea()[1]) / configArea()[3]];
      }
      return [
        modelArea(xAx, yAx)[0] + u[X] * modelArea(xAx, yAx)[2],
        modelArea(xAx, yAx)[1] + u[Y] * modelArea(xAx, yAx)[3],
      ];
    }

    function toDisplayCoord(p, xAx, yAx, noTransform = false) {
      return chartCommon.toDisplayCoord(
        p,
        noTransform ? [1, 0, 0, 1, 0, 0] : combinedTransform(xAx, yAx),
        isHorizontal(),
        configArea(),
        modelArea(xAx, yAx)
      );
    }

    function notifyAreaChanged() {
      for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
        let u, v;
        const area = modelArea(xAx, 0);
        if (isHorizontal()) {
          u = (toModelCoord([0, top(configArea())], xAx, 0)[0] - area[0]) / area[2];
          v = (toModelCoord([0, bottom(configArea())], xAx, 0)[0] - area[0]) / area[2];
        } else {
          u = (toModelCoord([left(configArea()), 0], xAx, 0)[0] - area[0]) / area[2];
          v = (toModelCoord([right(configArea()), 0], xAx, 0)[0] - area[0]) / area[2];
        }
        for (let i = 0; i < len(sliders()); ++i) {
          const o = WT.$(sliders()[i]);
          if (o) {
            const sobj = o.wtSObj;
            if (sobj && sobj.xAxis() === xAx) {
              sobj.changeRange(u, v);
            }
          }
        }
      }
    }

    function repaint() {
      hideTooltip();
      if (hasToolTips() && tobj.tooltipPosition) {
        tobj.tooltipTimeout = setTimeout(function() {
          loadTooltip();
        }, TOOLTIP_TIMEOUT);
      }
      if (!paintEnabled) {
        return;
      }
      rqAnimFrameThrottled(function() {
        target.repaint();
        if (showCrosshair()) {
          repaintOverlay();
        }
      });
    }

    function repaintOverlay() {
      if (!paintEnabled) {
        return;
      }
      const ctx = overlay.getContext("2d");

      ctx.clearRect(0, 0, overlay.width, overlay.height);

      ctx.save();

      ctx.beginPath();
      ctx.moveTo(left(configArea()), top(configArea()));
      ctx.lineTo(right(configArea()), top(configArea()));
      ctx.lineTo(right(configArea()), bottom(configArea()));
      ctx.lineTo(left(configArea()), bottom(configArea()));
      ctx.closePath();
      ctx.clip();

      let p = mult(inverted(combinedTransform(crosshairXAxis(), crosshairYAxis())), crosshair);
      let x = crosshair[X];
      let y = crosshair[Y];
      if (followCurve() !== -1) {
        p = findClosestPoint(isHorizontal() ? p[Y] : p[X], seriesCurve(followCurve()), isHorizontal());
        const tp = mult(
          combinedTransform(seriesXAxis(followCurve()), seriesYAxis(followCurve())),
          mult(curveTransform(followCurve()), p)
        );
        x = tp[X];
        y = tp[Y];
        crosshair[X] = x;
        crosshair[Y] = y;
      }
      let u;
      if (isHorizontal()) {
        u = [(p[Y] - configArea()[1]) / configArea()[3], (p[X] - configArea()[0]) / configArea()[2]];
      } else {
        u = [(p[X] - configArea()[0]) / configArea()[2], 1 - (p[Y] - configArea()[1]) / configArea()[3]];
      }
      if (followCurve() !== -1) {
        const area = modelArea(seriesXAxis(followCurve()), seriesYAxis(followCurve()));
        p = [area[0] + u[X] * area[2], area[1] + u[Y] * area[3]];
      } else {
        const area = modelArea(crosshairXAxis(), crosshairYAxis());
        p = [area[0] + u[X] * area[2], area[1] + u[Y] * area[3]];
      }

      ctx.fillStyle = ctx.strokeStyle = config.crosshairColor;
      ctx.font = "16px sans-serif";
      ctx.textAlign = "right";
      ctx.textBaseline = "top";
      let textX = p[0].toFixed(2);
      let textY = p[1].toFixed(2);
      if (textX === "-0.00") {
        textX = "0.00";
      }
      if (textY === "-0.00") {
        textY = "0.00";
      }
      ctx.fillText(
        "(" + textX + "," + textY + ")",
        right(configArea()) - coordinateOverlayPadding()[0],
        top(configArea()) + coordinateOverlayPadding()[1]
      );

      if (ctx.setLineDash) {
        ctx.setLineDash([1, 2]);
      }
      ctx.beginPath();
      ctx.moveTo(Math.floor(x) + 0.5, Math.floor(top(configArea())) + 0.5);
      ctx.lineTo(Math.floor(x) + 0.5, Math.floor(bottom(configArea())) + 0.5);
      ctx.moveTo(Math.floor(left(configArea())) + 0.5, Math.floor(y) + 0.5);
      ctx.lineTo(Math.floor(right(configArea())) + 0.5, Math.floor(y) + 0.5);
      ctx.stroke();

      ctx.restore();
    }

    // Check if the given area is within the bounds of the chart's area + some slack.
    function isWithinBounds(area) {
      return top(area) <= top(insideArea()) + BOUNDS_SLACK &&
        bottom(area) >= bottom(insideArea()) - BOUNDS_SLACK &&
        left(area) <= left(insideArea()) + BOUNDS_SLACK &&
        right(area) >= right(insideArea()) - BOUNDS_SLACK;
    }

    function enforceLimits(flags = null) {
      let diff;
      if (isHorizontal()) {
        if (flags === X_ONLY) {
          flags = Y_ONLY;
        } else if (flags === Y_ONLY) {
          flags = X_ONLY;
        }
      }
      for (let i = 0; i < xAxisCount(); ++i) {
        let transformedArea = transformedInsideChartArea(i, 0);
        if (!isHorizontal()) {
          if (flags === null || flags === X_ONLY) {
            if (xTransform(i)[0] < 1) {
              xTransform(i)[0] = 1;
              transformedArea = transformedInsideChartArea(i, 0);
            }
            if (left(transformedArea) > left(insideArea())) {
              diff = left(insideArea()) - left(transformedArea);
              xTransform(i)[4] = xTransform(i)[4] + diff;
            } else if (right(transformedArea) < right(insideArea())) {
              diff = right(insideArea()) - right(transformedArea);
              xTransform(i)[4] = xTransform(i)[4] + diff;
            }
          }
        } else {
          if (flags === null || flags === Y_ONLY) {
            if (xTransform(i)[0] < 1) {
              xTransform(i)[0] = 1;
              transformedArea = transformedInsideChartArea(i, 0);
            }
            if (top(transformedArea) > top(insideArea())) {
              diff = top(insideArea()) - top(transformedArea);
              xTransform(i)[4] = xTransform(i)[4] + diff;
            } else if (bottom(transformedArea) < bottom(insideArea())) {
              diff = bottom(insideArea()) - bottom(transformedArea);
              xTransform(i)[4] = xTransform(i)[4] + diff;
            }
          }
        }
      }
      for (let i = 0; i < yAxisCount(); ++i) {
        let transformedArea = transformedInsideChartArea(0, i);
        if (!isHorizontal()) {
          if (flags === null || flags === Y_ONLY) {
            if (yTransform(i)[3] < 1) {
              yTransform(i)[3] = 1;
              transformedArea = transformedInsideChartArea(0, i);
            }
            if (top(transformedArea) > top(insideArea())) {
              diff = top(insideArea()) - top(transformedArea);
              yTransform(i)[5] = yTransform(i)[5] - diff;
            } else if (bottom(transformedArea) < bottom(insideArea())) {
              diff = bottom(insideArea()) - bottom(transformedArea);
              yTransform(i)[5] = yTransform(i)[5] - diff;
            }
          }
        } else {
          if (flags === null || flags === X_ONLY) {
            if (yTransform(i)[3] < 1) {
              yTransform(i)[3] = 1;
              transformedArea = transformedInsideChartArea(0, i);
            }
            if (left(transformedArea) > left(insideArea())) {
              diff = left(insideArea()) - left(transformedArea);
              yTransform(i)[5] = yTransform(i)[5] + diff;
            } else if (right(transformedArea) < right(insideArea())) {
              diff = right(insideArea()) - right(transformedArea);
              yTransform(i)[5] = yTransform(i)[5] + diff;
            }
          }
        }
      }
      setTransformChangedTimeout();
    }

    function loadTooltip() {
      if (tobj.toolTipEl) {
        return;
      }
      APP.emit(target.widget, "loadTooltip", tobj.tooltipPosition[X], tobj.tooltipPosition[Y]);
    }

    const MouseDistance = 10;

    this.updateTooltip = function(contents) {
      hideTooltip();
      if (contents) {
        if (!tobj.tooltipPosition) {
          return;
        }
        tobj.toolTipEl = document.createElement("div");
        tobj.toolTipEl.className = config.ToolTipInnerStyle;
        tobj.toolTipEl.innerHTML = contents;

        tobj.tooltipOuterDiv = document.createElement("div");
        tobj.tooltipOuterDiv.className = config.ToolTipOuterStyle;

        document.body.appendChild(tobj.tooltipOuterDiv);
        tobj.tooltipOuterDiv.appendChild(tobj.toolTipEl);
        const c = WT.widgetPageCoordinates(target.canvas);

        const x = tobj.tooltipPosition[X] + c.x;
        const y = tobj.tooltipPosition[Y] + c.y;
        WT.fitToWindow(
          tobj.tooltipOuterDiv,
          x + MouseDistance,
          y + MouseDistance,
          x - MouseDistance,
          y - MouseDistance
        );

        tobj.toolTipEl.addEventListener("mouseenter", function() {
          tobj.overTooltip = true;
        });
        tobj.toolTipEl.addEventListener("mouseleave", function() {
          tobj.overTooltip = false;
        });
      }
    };

    this.mouseMove = function(o, event) {
      // Delay mouse move, because IE reacts to
      // mousemove first, but we actually want to
      // handle it after pointer events.
      setTimeout(function() {
        setTimeout(hideTooltip, TOOLTIP_HIDE_DELAY);
        if (pointerActive) {
          return;
        }
        const c = WT.widgetCoordinates(target.canvas, event);
        if (!isPointInRect(c, configArea())) {
          return;
        }

        if (hasToolTips()) {
          tobj.tooltipPosition = [c.x, c.y];
          tobj.tooltipTimeout = setTimeout(function() {
            loadTooltip();
          }, TOOLTIP_TIMEOUT);
        }

        if (dragPreviousXY === null && showCrosshair() && paintEnabled) {
          crosshair = [c.x, c.y];
          rqAnimFrameThrottled(repaintOverlay);
        }
      }, 0);
    };

    this.mouseOut = function(_o, _event) {
      setTimeout(hideTooltip, TOOLTIP_HIDE_DELAY);
    };

    this.mouseDown = function(o, event) {
      if (pointerActive) {
        return;
      }
      const c = WT.widgetCoordinates(target.canvas, event);
      const matchedYAxis = matchYAxis(c.x, c.y);
      const inRect = isPointInRect(c, configArea());
      const matchedXAxis = matchXAxis(c.x, c.y);
      if (matchedYAxis === -1 && matchedXAxis === -1 && !inRect) {
        return;
      }

      dragPreviousXY = c;
      dragCurrentXAxis = matchedXAxis;
      dragCurrentYAxis = matchedYAxis;
    };

    this.mouseUp = function(_o, _event) {
      if (pointerActive) {
        return;
      }
      dragPreviousXY = null;
      dragCurrentXAxis = -1;
      dragCurrentYAxis = -1;
    };

    this.mouseDrag = function(o, event) {
      if (pointerActive) {
        return;
      }
      if (dragPreviousXY === null) {
        self.mouseDown(o, event);
        return;
      }
      const c = WT.widgetCoordinates(target.canvas, event);
      if (WT.buttons === 1) {
        if (
          dragCurrentYAxis === -1 && dragCurrentXAxis === -1 &&
          curveManipulation() && configSeries(configSelectedCurve())
        ) {
          const curve = configSelectedCurve();
          let dy;
          if (isHorizontal()) {
            dy = c.x - dragPreviousXY.x;
          } else {
            dy = c.y - dragPreviousXY.y;
          }
          assign(
            seriesTransform(curve),
            mult([1, 0, 0, 1, 0, dy / yTransform(seriesYAxis(configSelectedCurve()))[3]], seriesTransform(curve))
          );
          repaint();
        } else if (config.pan) {
          translate(
            {
              x: c.x - dragPreviousXY.x,
              y: c.y - dragPreviousXY.y,
            },
            0,
            dragCurrentXAxis,
            dragCurrentYAxis
          );
        }
      }
      dragPreviousXY = c;
    };

    this.clicked = function(o, event) {
      if (pointerActive) {
        return;
      }
      if (dragPreviousXY !== null) {
        return;
      }
      if (!seriesSelection()) {
        return;
      }
      const c = WT.widgetCoordinates(target.canvas, event);
      APP.emit(target.widget, "seriesSelected", c.x, c.y);
    };

    function init() {
      if (
        showCrosshair() &&
        (overlay === null || target.canvas.width !== overlay.width || target.canvas.height !== overlay.height)
      ) {
        if (overlay) {
          overlay.parentNode.removeChild(overlay);
          delete widget.wtOObj;
          overlay = null;
        }
        const c = document.createElement("canvas");
        c.setAttribute("width", target.canvas.width);
        c.setAttribute("height", target.canvas.height);
        c.style.position = "absolute";
        c.style.display = "block";
        c.style.left = "0";
        c.style.top = "0";
        if (window.MSPointerEvent || window.PointerEvent) {
          c.style.msTouchAction = "none";
          c.style.touchAction = "none";
        }
        target.canvas.parentNode.appendChild(c);
        overlay = c;
        widget.wtOObj = overlay;
      } else if (overlay !== null && !showCrosshair()) {
        // If the mouse handler is not reinitialized, we don't actually get here!
        overlay.parentNode.removeChild(overlay);
        delete widget.wtOObj;
        overlay = null;
      }

      crosshair = [(left(configArea()) + right(configArea())) / 2, (top(configArea()) + bottom(configArea())) / 2];
    }

    this.mouseWheel = function(o, event) {
      const modifiers = (event.metaKey << 3) + (event.altKey << 2) + (event.ctrlKey << 1) + event.shiftKey;
      const action = config.wheelActions[modifiers];
      if (typeof action === "undefined") {
        return;
      }

      const c = WT.widgetCoordinates(target.canvas, event);
      const matchedXAxis = matchXAxis(c.x, c.y);
      const matchedYAxis = matchYAxis(c.x, c.y);
      const inRect = isPointInRect(c, configArea());
      if (matchedXAxis === -1 && matchedYAxis === -1 && !inRect) {
        return;
      }
      const w = WT.normalizeWheel(event);
      if (inRect && modifiers === 0 && curveManipulation()) {
        // Scale the curve around its middle
        const curve = configSelectedCurve();
        const d = -w.spinY;
        if (configSeries(curve)) {
          const t = curveTransform(curve);
          const s = apply(t, seriesCurve(curve));
          const minMax = minMaxY(s, isHorizontal());
          const middle = (minMax[0] + minMax[1]) / 2;
          WT.cancelEvent(event);
          const s_y = Math.pow(1.2, d);
          assign(seriesTransform(curve), mult([1, 0, 0, s_y, 0, middle - s_y * middle], seriesTransform(curve)));
          repaint();
          return;
        }
      }
      if ((action === WHEEL_PAN_X || action === WHEEL_PAN_Y || action === WHEEL_PAN_MATCHING) && config.pan) {
        const xBefore = [];
        for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
          xBefore.push(xTransform(xAx)[4]);
        }
        const yBefore = [];
        for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
          yBefore.push(yTransform(yAx)[5]);
        }
        if (action === WHEEL_PAN_MATCHING) {
          translate({ x: -w.pixelX, y: -w.pixelY }, 0, matchedXAxis, matchedYAxis);
        } else if (action === WHEEL_PAN_Y) {
          translate({ x: 0, y: -w.pixelX - w.pixelY }, 0, matchedXAxis, matchedYAxis);
        } else if (action === WHEEL_PAN_X) {
          translate({ x: -w.pixelX - w.pixelY, y: 0 }, 0, matchedXAxis, matchedYAxis);
        }
        for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
          if (xBefore[xAx] !== xTransform(xAx)[4]) {
            WT.cancelEvent(event);
          }
        }
        for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
          if (yBefore[yAx] !== yTransform(yAx)[5]) {
            WT.cancelEvent(event);
          }
        }
      } else if (config.zoom) {
        WT.cancelEvent(event);
        let d = -w.spinY;
        // Some browsers scroll horizontally when shift key pressed
        if (d === 0) {
          d = -w.spinX;
        }
        if (action === WHEEL_ZOOM_Y) {
          zoom(c, 0, d, matchedXAxis, matchedYAxis);
        } else if (action === WHEEL_ZOOM_X) {
          zoom(c, d, 0, matchedXAxis, matchedYAxis);
        } else if (action === WHEEL_ZOOM_XY) {
          zoom(c, d, d, matchedXAxis, matchedYAxis);
        } else if (action === WHEEL_ZOOM_MATCHING) {
          if (w.pixelX !== 0) {
            zoom(c, d, 0, matchedXAxis, matchedYAxis);
          } else {
            zoom(c, 0, d, matchedXAxis, matchedYAxis);
          }
        }
      }
    };

    const CROSSHAIR_RADIUS = 30;

    const seriesSelected = function() {
      if (!seriesSelection()) {
        return;
      }
      APP.emit(target.widget, "seriesSelected", dragPreviousXY.x, dragPreviousXY.y);
    };

    function topElement() {
      if (overlay) {
        return overlay;
      } else {
        return target.canvas;
      }
    }

    // fromDoubleTouch: indicates that this start of a touch comes from releasing of a double touch,
    //                  so should not be interpreted for series selection
    touchHandlers.start = function(o, event, fromDoubleTouch) {
      singleTouch = len(event.touches) === 1;
      doubleTouch = len(event.touches) === 2;

      if (singleTouch) {
        animating = false;
        const c = WT.widgetCoordinates(target.canvas, event.touches[0]);
        const matchedYAxis = matchYAxis(c.x, c.y);
        const inRect = isPointInRect(c, configArea());
        const matchedXAxis = matchXAxis(c.x, c.y);
        if (matchedYAxis === -1 && matchedXAxis === -1 && !inRect) {
          return;
        }
        if (
          matchedYAxis === -1 && matchedXAxis === -1 && showCrosshair() &&
          distanceLessThanRadius(crosshair, [c.x, c.y], CROSSHAIR_RADIUS)
        ) {
          mode = CROSSHAIR_MODE;
        } else {
          mode = LOOK_MODE;
        }
        lastDate = Date.now();
        dragPreviousXY = c;
        dragCurrentYAxis = matchedYAxis;
        dragCurrentXAxis = matchedXAxis;
        if (mode !== CROSSHAIR_MODE) {
          if (!fromDoubleTouch && inRect) {
            seriesSelectionTimeout = window.setTimeout(seriesSelected, SERIES_SELECTION_TIMEOUT);
          }
          addEventListener("contextmenu", eobj2.contextmenuListener);
        }
        WT.capture(null);
        WT.capture(topElement());
      } else if (doubleTouch && (config.zoom || curveManipulation())) {
        if (seriesSelectionTimeout) {
          window.clearTimeout(seriesSelectionTimeout);
          seriesSelectionTimeout = null;
        }
        animating = false;
        touches = [
          WT.widgetCoordinates(target.canvas, event.touches[0]),
          WT.widgetCoordinates(target.canvas, event.touches[1]),
        ].map(function(t) {
          return [t.x, t.y];
        });
        let matchedXAxis = -1;
        let matchedYAxis = -1;
        if (
          !touches.every(function(p) {
            return isPointInRect(p, configArea());
          })
        ) {
          matchedXAxis = matchXAxis(touches[0][X], touches[0][Y]);
          if (matchedXAxis !== -1) {
            if (matchedXAxis !== matchXAxis(touches[1][X], touches[1][Y])) {
              doubleTouch = null;
              return;
            }
          } else {
            matchedYAxis = matchYAxis(touches[0][X], touches[0][Y]);
            if (matchedYAxis !== 1) {
              if (matchedYAxis !== matchYAxis(touches[1][X], touches[1][Y])) {
                doubleTouch = null;
                return;
              }
            } else {
              doubleTouch = null;
              return;
            }
          }
        }
        WT.capture(null);
        WT.capture(topElement());
        zoomAngle = Math.atan2(touches[1][1] - touches[0][1], touches[1][0] - touches[0][0]);
        zoomMiddle = [
          (touches[0][0] + touches[1][0]) / 2,
          (touches[0][1] + touches[1][1]) / 2,
        ];
        const sin = Math.abs(Math.sin(zoomAngle));
        const cos = Math.abs(Math.cos(zoomAngle));
        if (sin < Math.sin(22.5 / 180 * Math.PI)) {
          zoomAngle = 0;
        } else if (cos < Math.cos(67.5 / 180 * Math.PI)) {
          zoomAngle = Math.PI / 2;
        } else if (Math.tan(zoomAngle) > 0) {
          zoomAngle = Math.PI / 4;
        } else {
          zoomAngle = -Math.PI / 4;
        }
        zoomProjection = projection(zoomAngle, zoomMiddle);
        dragCurrentXAxis = matchedXAxis;
        dragCurrentYAxis = matchedYAxis;
      } else {
        return;
      }
      preventDefault(event);
    };

    function animate(ts, dt = null) {
      if (!animating) {
        return;
      }
      const now = Date.now();
      dt = dt ?? now - lastDate;
      const d = { x: 0, y: 0 };
      let area;
      if (dragCurrentXAxis !== -1) {
        area = transformedInsideChartArea(dragCurrentXAxis, 0);
      } else if (dragCurrentYAxis === -1) {
        area = transformedInsideChartArea(0, 0);
        for (let xAx = 1; xAx < xAxisCount(); ++xAx) {
          area = intersection(area, transformedInsideChartArea(xAx, 0));
        }
        for (let yAx = 1; yAx < yAxisCount(); ++yAx) {
          area = intersection(area, transformedInsideChartArea(0, yAx));
        }
      } else {
        area = transformedInsideChartArea(0, dragCurrentYAxis);
      }
      const k = SPRING_CONSTANT;

      if (dt > 2 * ANIMATION_INTERVAL) {
        paintEnabled = false;
        const i = Math.floor(dt / ANIMATION_INTERVAL - 1);
        for (let j = 0; j < i; ++j) {
          animate(ts, ANIMATION_INTERVAL);
          if (!animating) {
            paintEnabled = true;
            repaint();
            return;
          }
        }
        dt -= i * ANIMATION_INTERVAL;
        paintEnabled = true;
      }

      // Calculate new area position and v.x, v.y
      if (v.x === Infinity || v.x === -Infinity) {
        if (v.x > 0) {
          v.x = MAX_SPEED;
        } else {
          v.x = -MAX_SPEED;
        }
      }
      if (isFinite(v.x)) {
        v.x = v.x / (1 + FRICTION_FACTOR * dt);
        area[0] += v.x * dt;
        if (left(area) > left(insideArea())) {
          v.x = v.x + (-k) * (left(area) - left(insideArea())) * dt;
          v.x *= 0.7;
        } else if (right(area) < right(insideArea())) {
          v.x = v.x + (-k) * (right(area) - right(insideArea())) * dt;
          v.x *= 0.7;
        }
        if (Math.abs(v.x) < MIN_SPEED) {
          if (left(area) > left(insideArea())) {
            v.x = MIN_SPEED;
          } else if (right(area) < right(insideArea())) {
            v.x = -MIN_SPEED;
          }
        }
        // cap speed
        if (Math.abs(v.x) > MAX_SPEED) {
          v.x = (v.x > 0 ? 1 : -1) * MAX_SPEED;
        }
        d.x = v.x * dt;
      }
      if (v.y === Infinity || v.y === -Infinity) {
        if (v.y > 0) {
          v.y = MAX_SPEED;
        } else {
          v.y = -MAX_SPEED;
        }
      }
      if (isFinite(v.y)) {
        v.y = v.y / (1 + FRICTION_FACTOR * dt);
        area[1] += v.y * dt;
        if (top(area) > top(insideArea())) {
          v.y = v.y + (-k) * (top(area) - top(insideArea())) * dt;
          v.y *= 0.7;
        } else if (bottom(area) < bottom(insideArea())) {
          v.y = v.y + (-k) * (bottom(area) - bottom(insideArea())) * dt;
          v.y *= 0.7;
        }
        if (Math.abs(v.y) < 0.001) {
          if (top(area) > top(insideArea())) {
            v.y = 0.001;
          } else if (bottom(area) < bottom(insideArea())) {
            v.y = -0.001;
          }
        }
        // cap speed
        if (Math.abs(v.y) > MAX_SPEED) {
          v.y = (v.y > 0 ? 1 : -1) * MAX_SPEED;
        }
        d.y = v.y * dt;
      }

      if (dragCurrentXAxis !== -1) {
        area = transformedInsideChartArea(dragCurrentXAxis, 0);
      } else if (dragCurrentYAxis === -1) {
        area = transformedInsideChartArea(0, 0);
        for (let xAx = 1; xAx < xAxisCount(); ++xAx) {
          area = intersection(area, transformedInsideChartArea(xAx, 0));
        }
        for (let yAx = 1; yAx < yAxisCount(); ++yAx) {
          area = intersection(area, transformedInsideChartArea(0, yAx));
        }
      } else {
        area = transformedInsideChartArea(0, dragCurrentYAxis);
      }
      translate(d, NO_LIMIT, dragCurrentXAxis, dragCurrentYAxis);
      let newArea;
      if (dragCurrentXAxis !== -1) {
        newArea = transformedInsideChartArea(dragCurrentXAxis, 0);
      } else if (dragCurrentYAxis === -1) {
        newArea = transformedInsideChartArea(0, 0);
        for (let xAx = 1; xAx < xAxisCount(); ++xAx) {
          newArea = intersection(newArea, transformedInsideChartArea(xAx, 0));
        }
        for (let yAx = 1; yAx < yAxisCount(); ++yAx) {
          newArea = intersection(newArea, transformedInsideChartArea(0, yAx));
        }
      } else {
        newArea = transformedInsideChartArea(0, dragCurrentYAxis);
      }
      if (
        left(area) > left(insideArea()) &&
        left(newArea) <= left(insideArea())
      ) {
        v.x = 0;
        translate({ x: -d.x, y: 0 }, NO_LIMIT, dragCurrentXAxis, dragCurrentYAxis);
        enforceLimits(X_ONLY);
      }
      if (
        right(area) < right(insideArea()) &&
        right(newArea) >= right(insideArea())
      ) {
        v.x = 0;
        translate({ x: -d.x, y: 0 }, NO_LIMIT, dragCurrentXAxis, dragCurrentYAxis);
        enforceLimits(X_ONLY);
      }
      if (
        top(area) > top(insideArea()) &&
        top(newArea) <= top(insideArea())
      ) {
        v.y = 0;
        translate({ x: 0, y: -d.y }, NO_LIMIT, dragCurrentXAxis, dragCurrentYAxis);
        enforceLimits(Y_ONLY);
      }
      if (
        bottom(area) < bottom(insideArea()) &&
        bottom(newArea) >= bottom(insideArea())
      ) {
        v.y = 0;
        translate({ x: 0, y: -d.y }, NO_LIMIT, dragCurrentXAxis, dragCurrentYAxis);
        enforceLimits(Y_ONLY);
      }
      if (
        Math.abs(v.x) < STOPPING_SPEED &&
        Math.abs(v.y) < STOPPING_SPEED &&
        isWithinBounds(newArea)
      ) {
        enforceLimits();
        animating = false;
        dragPreviousXY = null;
        v.x = 0;
        // Calculate new area p
        v.y = 0;
        lastDate = null;
        touches = [];
      } else {
        lastDate = now;
        if (paintEnabled) {
          requestAnimationFrame(animate);
        }
      }
    }

    touchHandlers.end = function(o, event) {
      if (seriesSelectionTimeout) {
        window.clearTimeout(seriesSelectionTimeout);
        seriesSelectionTimeout = null;
      }
      window.setTimeout(function() {
        removeEventListener("contextmenu", eobj2.contextmenuListener);
      }, 0);
      let touches = Array.prototype.slice.call(event.touches);

      let noTouch = len(touches) === 0;

      if (!noTouch) {
        for (let i = 0; i < len(event.changedTouches); ++i) {
          const id = event.changedTouches[i].identifier;
          for (let j = 0; j < len(touches); ++j) {
            if (touches[j].identifier === id) {
              touches.splice(j, 1);
              return;
            }
          }
        }
      }

      noTouch = len(touches) === 0;
      singleTouch = len(touches) === 1;
      doubleTouch = len(touches) === 2;

      if (noTouch) {
        moveTimeout = null;
        if (mode === LOOK_MODE && (isFinite(v.x) || isFinite(v.y)) && config.rubberBand) {
          lastDate = Date.now();
          animating = true;
          requestAnimationFrame(animate);
        } else {
          if (mode === CROSSHAIR_MODE) {
            self.mouseUp(null, null);
          }
          touches = [];
          zoomAngle = null;
          zoomMiddle = null;
          zoomProjection = null;
          lastDate = null;
        }
        mode = null;
      } else if (singleTouch || doubleTouch) {
        touchHandlers.start(o, event, true);
      }
    };

    let moveTimeout = null;
    let c1 = null;
    let c2 = null;

    touchHandlers.moved = function(o, event) {
      if ((!singleTouch) && (!doubleTouch)) {
        return;
      }
      if (singleTouch && dragPreviousXY === null) {
        return;
      }
      preventDefault(event);
      c1 = WT.widgetCoordinates(target.canvas, event.touches[0]);
      // kind of breaks pinch-to-zoom?
      if (len(event.touches) > 1) {
        c2 = WT.widgetCoordinates(target.canvas, event.touches[1]);
      }
      if (
        dragCurrentXAxis === -1 && dragCurrentYAxis === -1 && singleTouch && seriesSelectionTimeout &&
        !distanceLessThanRadius([c1.x, c1.y], [dragPreviousXY.x, dragPreviousXY.y], 3)
      ) {
        window.clearTimeout(seriesSelectionTimeout);
        seriesSelectionTimeout = null;
      }
      // setTimeout prevents high animation velocity due to looking
      // at events that are further apart.
      if (!moveTimeout) {
        moveTimeout = setTimeout(function() {
          if (
            dragCurrentXAxis === -1 && dragCurrentYAxis === -1 && singleTouch && curveManipulation() &&
            configSeries(configSelectedCurve())
          ) {
            const curve = configSelectedCurve();
            if (configSeries(curve)) {
              const c = c1;
              let dy;
              if (isHorizontal()) {
                dy = (c.x - dragPreviousXY.x) / yTransform(seriesYAxis(configSelectedCurve()))[3];
              } else {
                dy = (c.y - dragPreviousXY.y) / yTransform(seriesYAxis(configSelectedCurve()))[3];
              }
              seriesTransform(curve)[5] += dy;
              dragPreviousXY = c;
              repaint();
            }
          } else if (singleTouch) {
            const c = c1;
            const now = Date.now();
            const d = {
              x: c.x - dragPreviousXY.x,
              y: c.y - dragPreviousXY.y,
            };
            const dt = now - lastDate;
            lastDate = now;
            if (mode === CROSSHAIR_MODE) {
              crosshair[X] += d.x;
              crosshair[Y] += d.y;
              if (showCrosshair() && paintEnabled) {
                requestAnimationFrame(repaintOverlay);
              }
            } else if (config.pan) {
              v.x = d.x / dt;
              v.y = d.y / dt;
              translate(d, config.rubberBand ? DAMPEN : 0, dragCurrentXAxis, dragCurrentYAxis);
            }
            dragPreviousXY = c;
          } else if (
            dragCurrentXAxis === -1 && dragCurrentYAxis === -1 && doubleTouch && curveManipulation() &&
            configSeries(configSelectedCurve())
          ) {
            const curve = configSelectedCurve();
            if (configSeries(curve)) {
              const yAxis = isHorizontal() ? X : Y;
              const newTouches = [c1, c2].map(function(t) {
                return [t.x, t.y];
              });
              const dyBefore = Math.abs(touches[1][yAxis] - touches[0][yAxis]);
              const dyAfter = Math.abs(newTouches[1][yAxis] - newTouches[0][yAxis]);
              let yScale = dyBefore > 0 ? dyAfter / dyBefore : 1;
              if (dyAfter === dyBefore) {
                yScale = 1;
              }
              const myBefore = mult(inverted(combinedTransform(seriesXAxis(curve), seriesYAxis(curve))), [
                0,
                (touches[0][yAxis] + touches[1][yAxis]) / 2,
              ])[1];
              const myAfter = mult(inverted(combinedTransform(seriesXAxis(curve), seriesYAxis(curve))), [
                0,
                (newTouches[0][yAxis] + newTouches[1][yAxis]) / 2,
              ])[1];
              assign(
                seriesTransform(curve),
                mult(
                  [1, 0, 0, yScale, 0, -yScale * myBefore + myAfter],
                  seriesTransform(curve)
                )
              );
              repaint();
              dragPreviousXY = null;
              touches = newTouches;
            }
          } else if (doubleTouch && config.zoom) {
            const crosshairBefore = toModelCoord(crosshair, crosshairXAxis(), crosshairYAxis());
            let mxBefore = (touches[0][0] + touches[1][0]) / 2;
            let myBefore = (touches[0][1] + touches[1][1]) / 2;
            const newTouches = [c1, c2].map(function(t) {
              if (zoomAngle === 0) {
                return [t.x, myBefore];
              } else if (zoomAngle === Math.PI / 2) {
                return [mxBefore, t.y];
              } else {
                return mult(zoomProjection, [t.x, t.y]);
              }
            });

            const dxBefore = Math.abs(touches[1][0] - touches[0][0]);
            const dxAfter = Math.abs(newTouches[1][0] - newTouches[0][0]);
            let xScale = dxBefore > 0 ? dxAfter / dxBefore : 1;
            if (dxAfter === dxBefore || zoomAngle === Math.PI / 2) {
              xScale = 1;
            }
            let mxAfter = (newTouches[0][0] + newTouches[1][0]) / 2;
            const dyBefore = Math.abs(touches[1][1] - touches[0][1]);
            const dyAfter = Math.abs(newTouches[1][1] - newTouches[0][1]);
            let yScale = dyBefore > 0 ? dyAfter / dyBefore : 1;
            if (dyAfter === dyBefore || zoomAngle === 0) {
              yScale = 1;
            }
            let myAfter = (newTouches[0][1] + newTouches[1][1]) / 2;

            if (isHorizontal()) {
              [xScale, yScale] = [yScale, xScale];
              [mxBefore, myBefore] = [myBefore, mxBefore];
              [mxAfter, myAfter] = [myAfter, mxAfter];
            }

            const xScalePerAxis = [];
            for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
              xScalePerAxis.push(xScale);
            }
            for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
              if (xTransform(xAx)[0] * xScalePerAxis[xAx] > maxXZoom(xAx)) {
                xScalePerAxis[xAx] = maxXZoom(xAx) / xTransform(xAx)[0];
              }
              if (xTransform(xAx)[0] * xScalePerAxis[xAx] < minXZoom(xAx)) {
                xScalePerAxis[xAx] = minXZoom(xAx) / xTransform(xAx)[0];
              }
            }
            const yScalePerAxis = [];
            for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
              yScalePerAxis.push(yScale);
            }
            for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
              if (yTransform(yAx)[3] * yScalePerAxis[yAx] > maxYZoom(yAx)) {
                yScalePerAxis[yAx] = maxYZoom(yAx) / yTransform(yAx)[3];
              }
              if (yTransform(yAx)[3] * yScalePerAxis[yAx] < minYZoom(yAx)) {
                yScalePerAxis[yAx] = minYZoom(yAx) / yTransform(yAx)[3];
              }
            }
            if (dragCurrentXAxis !== -1) {
              if (
                xScalePerAxis[dragCurrentXAxis] !== 1 &&
                (xScalePerAxis[dragCurrentXAxis] < 1.0 ||
                  xTransform(dragCurrentXAxis)[0] !== maxXZoom(dragCurrentXAxis))
              ) {
                tAssign(
                  xTransform(dragCurrentXAxis),
                  mult(
                    [
                      xScalePerAxis[dragCurrentXAxis],
                      0,
                      0,
                      1,
                      -xScalePerAxis[dragCurrentXAxis] * mxBefore + mxAfter,
                      0,
                    ],
                    xTransform(dragCurrentXAxis)
                  )
                );
              }
            } else if (dragCurrentYAxis === -1) {
              for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
                if (
                  xScalePerAxis[xAx] !== 1 &&
                  (xScalePerAxis[xAx] < 1.0 || xTransform(xAx)[0] !== maxXZoom(xAx))
                ) {
                  tAssign(
                    xTransform(xAx),
                    mult(
                      [xScalePerAxis[xAx], 0, 0, 1, -xScalePerAxis[xAx] * mxBefore + mxAfter, 0],
                      xTransform(xAx)
                    )
                  );
                }
              }
              for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
                if (
                  yScalePerAxis[yAx] !== 1 &&
                  (yScalePerAxis[yAx] < 1.0 || yTransform(yAx)[3] !== maxYZoom(yAx))
                ) {
                  tAssign(
                    yTransform(yAx),
                    mult(
                      [1, 0, 0, yScalePerAxis[yAx], 0, -yScalePerAxis[yAx] * myBefore + myAfter],
                      yTransform(yAx)
                    )
                  );
                }
              }
            } else {
              if (
                yScalePerAxis[dragCurrentYAxis] !== 1 &&
                (yScalePerAxis[dragCurrentYAxis] < 1.0 ||
                  yTransform(dragCurrentYAxis)[3] !== maxYZoom(dragCurrentYAxis))
              ) {
                tAssign(
                  yTransform(dragCurrentYAxis),
                  mult(
                    [
                      1,
                      0,
                      0,
                      yScalePerAxis[dragCurrentYAxis],
                      0,
                      -yScalePerAxis[dragCurrentYAxis] * myBefore + myAfter,
                    ],
                    yTransform(dragCurrentYAxis)
                  )
                );
              }
            }
            enforceLimits();

            const crosshairAfter = toDisplayCoord(crosshairBefore, crosshairXAxis(), crosshairYAxis());
            crosshair[X] = crosshairAfter[X];
            crosshair[Y] = crosshairAfter[Y];

            touches = newTouches;
            refreshPenColors();
            repaint();
            notifyAreaChanged();
          }
          moveTimeout = null;
        }, 1);
      }
    };

    function refreshPenColors() {
      for (let xAx = 0; xAx < len(pens().x); ++xAx) {
        let xLevel = toZoomLevel(xTransform(xAx)[0]) - 1;
        if (xTransform(xAx)[0] === maxXZoom(xAx)) {
          xLevel = len(pens().x[xAx]) - 1;
        }
        if (xLevel >= len(pens().x[xAx])) {
          xLevel = len(pens().x[xAx]) - 1;
        }
        for (let i = 0; i < len(pens().x[xAx]); ++i) {
          if (xLevel === i) {
            for (let j = 0; j < len(pens().x[xAx][i]); ++j) {
              pens().x[xAx][i][j].color[3] = penAlpha().x[xAx][j];
            }
          } else {
            for (let j = 0; j < len(pens().x[xAx][i]); ++j) {
              pens().x[xAx][i][j].color[3] = 0;
            }
          }
        }
      }
      for (let yAx = 0; yAx < len(pens().y); ++yAx) {
        let yLevel = toZoomLevel(yTransform(yAx)[3]) - 1;
        if (yTransform(yAx)[3] === maxYZoom(yAx)) {
          yLevel = len(pens().y[yAx]) - 1;
        }
        if (yLevel >= len(pens().y[yAx])) {
          yLevel = len(pens().y[yAx]) - 1;
        }
        for (let i = 0; i < len(pens().y[yAx]); ++i) {
          if (yLevel === i) {
            for (let j = 0; j < len(pens().y[yAx][i]); ++j) {
              pens().y[yAx][i][j].color[3] = penAlpha().y[yAx][j];
            }
          } else {
            for (let j = 0; j < len(pens().y[yAx][i]); ++j) {
              pens().y[yAx][i][j].color[3] = 0;
            }
          }
        }
      }
    }

    function translate(d, flags = 0, matchedXAxis = -1, matchedYAxis = -1) {
      const crosshairBefore = toModelCoord(crosshair, crosshairXAxis(), crosshairYAxis());

      if (isHorizontal()) {
        d = { x: d.y, y: -d.x };
      }

      if (flags & NO_LIMIT) {
        if (matchedXAxis !== -1) {
          xTransform(matchedXAxis)[4] = xTransform(matchedXAxis)[4] + d.x;
        } else if (matchedYAxis === -1) {
          for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
            xTransform(xAx)[4] = xTransform(xAx)[4] + d.x;
          }
          for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
            yTransform(yAx)[5] = yTransform(yAx)[5] - d.y;
          }
        } else {
          yTransform(matchedYAxis)[5] = yTransform(matchedYAxis)[5] - d.y;
        }
        setTransformChangedTimeout();
      } else if (flags & DAMPEN) {
        let area;
        if (matchedXAxis !== -1) {
          area = transformedInsideChartArea(matchedXAxis, 0);
        } else if (matchedYAxis === -1) {
          area = transformedInsideChartArea(0, 0);
          for (let xAx = 1; xAx < xAxisCount(); ++xAx) {
            area = intersection(area, transformedInsideChartArea(xAx, 0));
          }
          for (let yAx = 1; yAx < yAxisCount(); ++yAx) {
            area = intersection(area, transformedInsideChartArea(0, yAx));
          }
        } else {
          area = transformedInsideChartArea(0, matchedYAxis);
        }
        if (left(area) > left(insideArea())) {
          if (d.x > 0) {
            d.x = d.x / (1 + ((left(area) - left(insideArea())) * RESISTANCE_FACTOR));
          }
        } else if (right(area) < right(insideArea())) {
          if (d.x < 0) {
            d.x = d.x / (1 + ((right(insideArea()) - right(area)) * RESISTANCE_FACTOR));
          }
        }
        if (top(area) > top(insideArea())) {
          if (d.y > 0) {
            d.y = d.y / (1 + ((top(area) - top(insideArea())) * RESISTANCE_FACTOR));
          }
        } else if (bottom(area) < bottom(insideArea())) {
          if (d.y < 0) {
            d.y = d.y / (1 + ((bottom(insideArea()) - bottom(area)) * RESISTANCE_FACTOR));
          }
        }
        if (matchedXAxis !== -1) {
          xTransform(matchedXAxis)[4] = xTransform(matchedXAxis)[4] + d.x;
        } else if (matchedYAxis === -1) {
          for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
            xTransform(xAx)[4] = xTransform(xAx)[4] + d.x;
          }
          for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
            yTransform(yAx)[5] = yTransform(yAx)[5] - d.y;
          }
        } else {
          yTransform(matchedYAxis)[5] = yTransform(matchedYAxis)[5] - d.y;
        }
        if (matchedYAxis === -1) {
          crosshair[X] = crosshair[X] + d.x;
        }
        if (matchedXAxis === -1) {
          crosshair[Y] = crosshair[Y] + d.y;
        }
        setTransformChangedTimeout();
      } else {
        if (matchedXAxis !== -1) {
          xTransform(matchedXAxis)[4] = xTransform(matchedXAxis)[4] + d.x;
        } else if (matchedYAxis === -1) {
          for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
            xTransform(xAx)[4] = xTransform(xAx)[4] + d.x;
          }
          for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
            yTransform(yAx)[5] = yTransform(yAx)[5] - d.y;
          }
        } else {
          yTransform(matchedYAxis)[5] = yTransform(matchedYAxis)[5] - d.y;
        }
        if (matchedYAxis === -1) {
          crosshair[X] = crosshair[X] + d.x;
        }
        if (matchedXAxis === -1) {
          crosshair[Y] = crosshair[Y] + d.y;
        }

        enforceLimits();
      }

      const crosshairAfter = toDisplayCoord(crosshairBefore, crosshairXAxis(), crosshairYAxis());

      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      repaint();
      notifyAreaChanged();
    }

    function zoom(coords, xDelta, yDelta, matchedXAxis = -1, matchedYAxis = -1) {
      const crosshairBefore = toModelCoord(crosshair, crosshairXAxis(), crosshairYAxis());
      let xy;
      if (isHorizontal()) {
        xy = [coords.y - top(configArea()), coords.x - left(configArea())];
      } else {
        xy = mult(
          inverted([1, 0, 0, -1, left(configArea()), bottom(configArea())]),
          [coords.x, coords.y]
        );
      }
      const x = xy[0];
      const y = xy[1];
      let s_x = Math.pow(1.2, isHorizontal() ? yDelta : xDelta);
      let s_y = Math.pow(1.2, isHorizontal() ? xDelta : yDelta);
      if (matchedXAxis !== -1) {
        if (xTransform(matchedXAxis)[0] * s_x > maxXZoom(matchedXAxis)) {
          s_x = maxXZoom(matchedXAxis) / xTransform(matchedXAxis)[0];
        }
        if (s_x < 1.0 || xTransform(matchedXAxis)[0] !== maxXZoom(matchedXAxis)) {
          tAssign(xTransform(matchedXAxis), mult([s_x, 0, 0, 1, x - s_x * x, 0], xTransform(matchedXAxis)));
        }
      } else if (matchedYAxis === -1) {
        for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
          let s_specific_x = s_x;
          if (xTransform(xAx)[0] * s_x > maxXZoom(xAx)) {
            s_specific_x = maxXZoom(xAx) / xTransform(xAx)[0];
          }
          if (s_specific_x < 1.0 || xTransform(xAx)[0] !== maxXZoom(xAx)) {
            tAssign(xTransform(xAx), mult([s_specific_x, 0, 0, 1, x - s_specific_x * x, 0], xTransform(xAx)));
          }
        }
        for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
          let s_specific_y = s_y;
          if (yTransform(yAx)[3] * s_y > maxYZoom(yAx)) {
            s_specific_y = maxYZoom(yAx) / yTransform(yAx)[3];
          }
          if (s_specific_y < 1.0 || yTransform(yAx)[3] !== maxYZoom(yAx)) {
            tAssign(yTransform(yAx), mult([1, 0, 0, s_specific_y, 0, y - s_specific_y * y], yTransform(yAx)));
          }
        }
      } else {
        if (yTransform(matchedYAxis)[3] * s_y > maxYZoom(matchedYAxis)) {
          s_y = maxYZoom(matchedYAxis) / yTransform(matchedYAxis)[3];
        }
        if (s_y < 1.0 || yTransform(matchedYAxis)[3] !== maxYZoom(matchedYAxis)) {
          tAssign(yTransform(matchedYAxis), mult([1, 0, 0, s_y, 0, y - s_y * y], yTransform(matchedYAxis)));
        }
      }

      enforceLimits();

      const crosshairAfter = toDisplayCoord(crosshairBefore, crosshairXAxis(), crosshairYAxis());
      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      refreshPenColors();
      repaint();
      notifyAreaChanged();
    }

    this.setXRange = function(seriesNb, lowerBound, upperBound, updateYAxis) {
      const xAx = seriesXAxis(seriesNb);
      const area = modelArea(xAx, 0);
      lowerBound = area[0] + area[2] * lowerBound;
      upperBound = area[0] + area[2] * upperBound;
      // Constrain given range
      if (left(area) > right(area)) {
        if (lowerBound > left(area)) {
          lowerBound = left(area);
        }
        if (upperBound < right(area)) {
          upperBound = right(area);
        }
      } else {
        if (lowerBound < left(area)) {
          lowerBound = left(area);
        }
        if (upperBound > right(area)) {
          upperBound = right(area);
        }
      }
      // Set X range, and adjust Y!
      const series = seriesCurve(seriesNb);

      const res = findYRange(
        series,
        seriesYAxis(seriesNb),
        lowerBound,
        upperBound,
        isHorizontal(),
        configArea(),
        modelArea(seriesXAxis(seriesNb), seriesYAxis(seriesNb)),
        config.minZoom,
        config.maxZoom
      );
      const xZoom = res.xZoom;
      const yZoom = res.yZoom;
      const panPoint = res.panPoint;

      const crosshairBefore = toModelCoord(crosshair, crosshairXAxis(), crosshairYAxis());

      xTransform(seriesXAxis(seriesNb))[0] = xZoom;
      if (yZoom && updateYAxis) {
        yTransform(seriesYAxis(seriesNb))[3] = yZoom;
      }
      xTransform(seriesXAxis(seriesNb))[4] = -panPoint[X] * xZoom;
      if (yZoom && updateYAxis) {
        yTransform(seriesYAxis(seriesNb))[5] = -panPoint[Y] * yZoom;
      }
      setTransformChangedTimeout();

      const crosshairAfter = toDisplayCoord(crosshairBefore, crosshairXAxis(), crosshairYAxis());
      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      enforceLimits();
      refreshPenColors();
      repaint();
      notifyAreaChanged();
    };

    this.getSeries = function(seriesNb) {
      return seriesCurve(seriesNb);
    };

    this.rangeChangedCallbacks = [];

    this.updateConfig = function(newConfig) {
      for (const [key, value] of Object.entries(newConfig)) {
        config[key] = value;
      }
      init();
      refreshPenColors();
      repaint();
      notifyAreaChanged();
    };

    this.updateConfig({});

    if (window.TouchEvent && !window.MSPointerEvent && !window.PointerEvent) {
      self.touchStart = touchHandlers.start;
      self.touchEnd = touchHandlers.end;
      self.touchMoved = touchHandlers.moved;
    } else {
      const nop = function() {};
      self.touchStart = nop;
      self.touchEnd = nop;
      self.touchMoved = nop;
    }
  }
);
