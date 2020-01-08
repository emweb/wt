/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER_BIG
(1, JavaScriptConstructor, "WCartesianChart",
  // target: the WPaintedWidget JavaScript obj, with:
  //   repaint
  //   canvas
  //   combinedTransform is set by WCartesianChart
  // config: the initial configuration (can be overridden with updateConfig)
  //   curveManipulation (determines whether series manipulation is enabled)
  //   seriesSelection (determines whether series selection is enabled)
  //   selectedCurve (the curve that is currently selected)
  //   isHorizontal (if orientation() == Horizontal)
  //   xTransform
  //   yTransforms[]
  //   area (WRectF describing render area)
  //   insideArea (WRectF describing render area without axis margins)
  //   modelAreas ([WRectF] describing model areas per Y axis)
  //   minZoom {x: float, y: [float]}
  //   maxZoom {x: float, y: [float]}
  //   rubberBand
  //   zoom (bool)
  //   pan (bool)
  //   crosshair (bool)
  //   crosshairAxis (int)
  //   crosshairColor (css text)
  //   followCurve (int, -1 for disabled)
  //   notifyTransform {x: bool, y: [bool]} // Whether we should emit a signal on X or Y transform change
  //   series {modelColumn: {curve: curve ref, transform: transform ref, axis: int},...}
  //   ToolTipInnerStyle // Style for the tooltip
  //   ToolTipOuterStyle // Style for the tooltip
  //   hasToolTips // whether there are any tooltips
  //   pens // {x: [linePen,textPen,gridPen], y: [[linePen,textPen,gridPen],...] }
  //   penAlpha: {x: [linePenAlpha,textPenAlpha,gridPenAlpha], y: [[linePenAlpha,textPenAlpha,gridPenAlpha],...] }
  //   xAxis: {width: float, side: side} side = min max zero both
  //   yAxes: [{width: float, side: side, minOffset: float, maxOffset: float}] side = min max zero both
  //
  function(APP, widget, target, config) {

    widget.wtCObj = this;

    var self = this;
    var WT = APP.WT;

    self.config = config;

    var utils = WT.gfxUtils;
    var mult = utils.transform_mult;
    var inverted = utils.transform_inverted;
    var assign = utils.transform_assign;
    var equal = utils.transform_equal;
    var apply = utils.transform_apply;
    var top = utils.rect_top;
    var bottom = utils.rect_bottom;
    var left = utils.rect_left;
    var right = utils.rect_right;
    var intersection = utils.rect_intersection;

    var chartCommon = WT.chartCommon;
    var minMaxY = chartCommon.minMaxY;
    var findClosestPoint = chartCommon.findClosestPoint;
    var projection = chartCommon.projection;
    var distanceLessThanRadius = chartCommon.distanceLessThanRadius;
    var toZoomLevel = chartCommon.toZoomLevel;
    var isPointInRect = chartCommon.isPointInRect;
    var findYRange = chartCommon.findYRange;
    var matchesXAxis = function(x,y) {
      return chartCommon.matchesXAxis(x, y, configArea(), config.xAxis, isHorizontal());
    };
    var matchYAxis = function(x,y) {
      return chartCommon.matchYAxis(x, y, configArea(), config.yAxes, isHorizontal());
    };

    // Functions that help in making minification more effective
    function isUndefined(x) {
      return x === undefined;
    }
    function modelArea(ax) { return config.modelAreas[ax]; }
    function followCurve() { return config.followCurve; }
    function showCrosshair() {
      return config.crosshair || followCurve() !== -1;
    }
    function isHorizontal() { return config.isHorizontal; }
    function xTransform() {
      return config.xTransform;
    }
    function yTransform(ax) {
      return config.yTransforms[ax];
    }
    function configArea() { return config.area; }
    function insideArea() { return config.insideArea; }
    function configSeries(seriesNb) {
      if (!isUndefined(seriesNb)) {
        return config.series[seriesNb];
      } else {
        return config.series;
      }
    }
    function seriesTransform(seriesNb) {
      return configSeries(seriesNb).transform;
    }
    function curveTransform(seriesNb) {
      if (isHorizontal()) {
        return mult([0,1,1,0,0,0], mult(seriesTransform(seriesNb), [0,1,1,0,0,0]));
      } else {
        return seriesTransform(seriesNb);
      }
    }
    function seriesCurve(seriesNb) {
      return configSeries(seriesNb).curve;
    }
    function seriesAxis(seriesNb) {
      return configSeries(seriesNb).axis;
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
    function minXZoom() {
      return config.minZoom.x;
    }
    function minYZoom(ax) {
      return config.minZoom.y[ax];
    }
    function maxXZoom() {
      return config.maxZoom.x;
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
      if (e.preventDefault)
        e.preventDefault();
    }
    function addEventListener(e,l) {
      widget.addEventListener(e,l);
    }
    function removeEventListener(e,l) {
      widget.removeEventListener(e,l);
    }
    function len(ar) {
      return ar.length;
    }
    function yAxisCount() {
      return len(config.yTransforms);
    }
    function notifyAnyTransform() {
      if (config.notifyTransform.x)
        return true;
      for (var i = 0; i < yAxisCount(); ++i) {
        if (config.notifyTransform.y[i])
          return true;
      }
      return false;
    }
    function crosshairAxis() {
      return config.crosshairAxis;
    }

    /* const */ var ANIMATION_INTERVAL = 17;
    var rqAnimFrame = (function(){
      return window.requestAnimationFrame       ||
        window.webkitRequestAnimationFrame ||
        window.mozRequestAnimationFrame    ||
        function(callback) {
          window.setTimeout(callback, ANIMATION_INTERVAL);
        };
    })();
    var framePending = false;
    var rqAnimFrameThrottled = function(cb) {
      if (framePending) return;
      framePending = true;
      rqAnimFrame(function() {
        cb();
        framePending = false;
      });
    };

    if (window.MSPointerEvent || window.PointerEvent) {
      widget.style.touchAction = 'none';
      target.canvas.style.msTouchAction = 'none';
      target.canvas.style.touchAction = 'none';
    }

    /*const*/ var MOVE_TO = 0, LINE_TO = 1, CUBIC_C1 = 2, CUBIC_C2 = 3, CUBIC_END = 4,
                  QUAD_C = 5, QUAD_END = 6, ARC_C = 7, ARC_R = 8, ARC_ANGLE_SWEEP = 9;
    /*const*/ var NO_LIMIT = 1, DAMPEN = 2; // bit flags
    /*const*/ var X_ONLY = 1, Y_ONLY = 2; // bit flags
    /*const*/ var X = 0, Y = 1;
    /*const*/ var LOOK_MODE = 0, CROSSHAIR_MODE = 1;
    /*const*/ var WHEEL_ZOOM_X = 0, WHEEL_ZOOM_Y = 1, WHEEL_ZOOM_XY = 2,
                  WHEEL_ZOOM_MATCHING = 3, WHEEL_PAN_X = 4, WHEEL_PAN_Y = 5,
                  WHEEL_PAN_MATCHING = 6;

    /*const*/ var SERIES_SELECTION_TIMEOUT = 200; // ms
    /*const*/ var TRANSFORM_CHANGED_TIMEOUT = 250; // ms
    /*const*/ var TOOLTIP_TIMEOUT = 500; // ms
    /*const*/ var TOOLTIP_HIDE_DELAY = 200; // ms

    /*const*/ var FRICTION_FACTOR = 0.003, // Determines how strongly the speed decreases, when animating
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
    var eobj2 = widget.wtEObj2;
    if (!eobj2) {
      eobj2 = {};
      eobj2.contextmenuListener = function(e) {
        preventDefault(e);
        removeEventListener('contextmenu', eobj2.contextmenuListener);
      };
    }
    widget.wtEObj2 = eobj2;

    var touchHandlers = {};

    function isTouchEvent(event) {
      return event.pointerType === 2 || event.pointerType === 3 ||
        event.pointerType === 'pen' || event.pointerType === 'touch';
    }

    var pointerActive = false;

    if (window.MSPointerEvent || window.PointerEvent) {
      (function(){
        var pointers = []

          function updatePointerActive() {
            pointerActive = len(pointers) > 0;
          }

        function pointerDown(event) {
          if (!isTouchEvent(event)) return;
          preventDefault(event);
          pointers.push(event);

          updatePointerActive();
          touchHandlers.start(widget, {touches:pointers.slice(0)});
        }

        function pointerUp(event) {
          if (!pointerActive) return;
          if (!isTouchEvent(event)) return;
          preventDefault(event);
          var i;
          for (i = 0; i < len(pointers); ++i) {
            if (pointers[i].pointerId === event.pointerId) {
              pointers.splice(i, 1);
              break;
            }
          }

          updatePointerActive();
          touchHandlers.end(widget, {touches:pointers.slice(0),changedTouches:[]});
        }

        function pointerMove(event) {
          if (!isTouchEvent(event)) return;
          preventDefault(event);
          var i;
          for (i = 0; i < len(pointers); ++i) {
            if (pointers[i].pointerId === event.pointerId) {
              pointers[i] = event;
              break;
            }
          }

          updatePointerActive();
          touchHandlers.moved(widget, {touches:pointers.slice(0)});
        }

        // eobj: an object for holding the handlers so we can properly register/unregister them,
        //       even when the chart gets updated
        var o = widget.wtEObj;
        if (o) {
          if (!window.PointerEvent) {
            removeEventListener('MSPointerDown', o.pointerDown);
            removeEventListener('MSPointerUp', o.pointerUp);
            removeEventListener('MSPointerOut', o.pointerUp);
            removeEventListener('MSPointerMove', o.pointerMove);
          } else {
            removeEventListener('pointerdown', o.pointerDown);
            removeEventListener('pointerup', o.pointerUp);
            removeEventListener('pointerout', o.pointerUp);
            removeEventListener('pointermove', o.pointerMove);
          }
        }
        widget.wtEObj = {
          pointerDown: pointerDown,
          pointerUp: pointerUp,
          pointerMove: pointerMove
        };
        if (!window.PointerEvent) {
          addEventListener('MSPointerDown', pointerDown);
          addEventListener('MSPointerUp', pointerUp);
          addEventListener('MSPointerOut', pointerUp);
          addEventListener('MSPointerMove', pointerMove);
        } else {
          addEventListener('pointerdown', pointerDown);
          addEventListener('pointerup', pointerUp);
          addEventListener('pointerout', pointerUp);
          addEventListener('pointermove', pointerMove);
        }
      })();
    }

    // oobj: the <canvas> for drawing the crosshair
    var overlay = widget.wtOObj;

    var crosshair = null;

    var paintEnabled = true; 
    var dragPreviousXY = null;
    var dragXAxis = false;
    var dragCurrentYAxis = -1;
    var touches = [];
    var singleTouch = false;
    var doubleTouch = false;
    var zoomAngle = null;
    var zoomMiddle = null;
    var zoomProjection = null;

    var v = {x: 0, y: 0};

    var seriesSelectionTimeout = null;
    var lastDate = null;

    var tobj = widget.wtTObj;
    if (!tobj) {
      tobj = {overTooltip:false};
      widget.wtTObj = tobj;
    }

    function hideTooltip() {
      if (!tobj)
        return;
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

    var mode = null;


    var animating = false;

    var transformChangedTimeout = null;
    var oldXTransform = [0,0,0,0,0,0];
    assign(oldXTransform, xTransform());
    var oldYTransforms = [];
    for (var i = 0; i < yAxisCount(); ++i) {
      oldYTransforms.push([0,0,0,0,0,0]);
      assign(oldYTransforms[i], yTransform(i));
    }
    function setTransformChangedTimeout() {
      if (!notifyAnyTransform())
        return;
      if (transformChangedTimeout) {
        window.clearTimeout(transformChangedTimeout);
        transformChangedTimeout = null;
      }
      transformChangedTimeout = setTimeout(function(){
        if (config.notifyTransform.x && !equal(oldXTransform, xTransform())) {
          APP.emit(target.widget, "xTransformChanged");
          assign(oldXTransform, xTransform());
        }
        for (var i = 0; i < yAxisCount(); ++i) {
          if (config.notifyTransform.y[i] && !equal(oldYTransforms[i], yTransform(i))) {
            APP.emit(target.widget, "yTransformChanged" + i);
            assign(oldYTransforms[i], yTransform(i));
          }
        }
      }, TRANSFORM_CHANGED_TIMEOUT);
    }
    var tAssign = function(a, b) {
      assign(a,b);
      setTransformChangedTimeout();
    }


    function combinedTransform(ax) {
      if (ax === undefined) {
        ax = 0;
      }
      var l, b, t;
      if (isHorizontal()) {
        l = left(configArea());
        t = top(configArea());
        return mult([0,1,1,0,l,t], mult(xTransform(), mult(yTransform(ax), [0,1,1,0,-t,-l])));
      } else {
        l = left(configArea());
        b = bottom(configArea());
        return mult([1,0,0,-1,l,b], mult(xTransform(), mult(yTransform(ax), [1,0,0,-1,-l,b])));
      }
    }
    target.combinedTransform = combinedTransform;

    function transformedInsideChartArea(ax) {
      return mult(combinedTransform(ax), insideArea());
    }


    function toModelCoord(p, ax, noTransform) {
      if (isUndefined(noTransform)) noTransform = false;
      var res;
      if (noTransform) {
        res = p;
      } else {
        res = mult(inverted(combinedTransform(ax)), p);
      }
      var u;
      if (isHorizontal()) {
        u = [(res[Y] - configArea()[1]) / configArea()[3],
          (res[X] - configArea()[0]) / configArea()[2]];
      } else {
        u = [(res[X] - configArea()[0]) / configArea()[2],
          1 - (res[Y] - configArea()[1]) / configArea()[3]];
      }
      return [modelArea(ax)[0] + u[X] * modelArea(ax)[2],
      modelArea(ax)[1] + u[Y] * modelArea(ax)[3]];
    }

    function toDisplayCoord(p, ax, noTransform) {
      if (isUndefined(noTransform))
        noTransform = false;
      return chartCommon.toDisplayCoord(p, noTransform ? [1,0,0,1,0,0] : combinedTransform(ax), isHorizontal(), configArea(), modelArea(ax));
    }

    function notifyAreaChanged() {
      var u,v;
      if (isHorizontal()) {
        u = (toModelCoord([0, top(configArea())], 0)[0] - modelArea(0)[0]) / modelArea(0)[2];
        v = (toModelCoord([0, bottom(configArea())], 0)[0] - modelArea(0)[0]) / modelArea(0)[2];
      } else {
        u = (toModelCoord([left(configArea()), 0], 0)[0] - modelArea(0)[0]) / modelArea(0)[2];
        v = (toModelCoord([right(configArea()), 0], 0)[0] - modelArea(0)[0]) / modelArea(0)[2];
      }
      var i;
      for (i = 0; i < len(sliders()); ++i) {
        var o = $('#' + sliders()[i]);
        if (o) {
          var sobj = o.data('sobj');
          if (sobj) {
            sobj.changeRange(u, v);
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
      if (!paintEnabled) return;
      rqAnimFrameThrottled(function(){
        target.repaint();
        if (showCrosshair()) {
          repaintOverlay();
        }
      });
    }

    function repaintOverlay() {
      if (!paintEnabled) return;
      var ctx = overlay.getContext('2d');

      ctx.clearRect(0, 0, overlay.width, overlay.height);

      ctx.save();

      ctx.beginPath();
      ctx.moveTo(left(configArea()), top(configArea()));
      ctx.lineTo(right(configArea()), top(configArea()));
      ctx.lineTo(right(configArea()), bottom(configArea()));
      ctx.lineTo(left(configArea()), bottom(configArea()));
      ctx.closePath();
      ctx.clip();

      var p = mult(inverted(combinedTransform(crosshairAxis())), crosshair);
      var x = crosshair[X];
      var y = crosshair[Y];
      if (followCurve() !== -1) {
        p = findClosestPoint(isHorizontal() ? p[Y] : p[X],
            seriesCurve(followCurve()), isHorizontal());
        var tp = mult(combinedTransform(seriesAxis(followCurve())), mult(curveTransform(followCurve()), p));
        x = tp[X];
        y = tp[Y];
        crosshair[X] = x;
        crosshair[Y] = y;
      }
      var u;
      if (isHorizontal()) {
        u = [(p[Y] - configArea()[1]) / configArea()[3],
          (p[X] - configArea()[0]) / configArea()[2]];
      } else {
        u = [(p[X] - configArea()[0]) / configArea()[2],
          1 - (p[Y] - configArea()[1]) / configArea()[3]];
      }
      if (followCurve() !== -1) {
        p = [modelArea(seriesAxis(followCurve()))[0] + u[X] * modelArea(seriesAxis(followCurve()))[2],
          modelArea(seriesAxis(followCurve()))[1] + u[Y] * modelArea(seriesAxis(followCurve()))[3]];
      } else {
        p = [modelArea(crosshairAxis())[0] + u[X] * modelArea(crosshairAxis())[2],
          modelArea(crosshairAxis())[1] + u[Y] * modelArea(crosshairAxis())[3]];
      }

      ctx.fillStyle = ctx.strokeStyle = config.crosshairColor;
      ctx.font = '16px sans-serif';
      ctx.textAlign = 'right';
      ctx.textBaseline = 'top';
      var textX = p[0].toFixed(2);
      var textY = p[1].toFixed(2);
      if (textX === '-0.00') textX = '0.00';
      if (textY === '-0.00') textY = '0.00';
      ctx.fillText("("+textX+","+textY+")", right(configArea()) - coordinateOverlayPadding()[0],
          top(configArea()) + coordinateOverlayPadding()[1]);

      if (ctx.setLineDash) {
        ctx.setLineDash([1,2]);
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

    function enforceLimits(flags) {
      for (var i = 0; i < yAxisCount(); ++i) {
        var newChartArea = transformedInsideChartArea(i);
        if (isHorizontal()) {
          if (flags === X_ONLY) flags = Y_ONLY;
          else if (flags === Y_ONLY) flags = X_ONLY;
        }
        var diff;
        if (isUndefined(flags) || flags === X_ONLY) {
          if (xTransform()[0] < 1) {
            xTransform()[0] = 1;
            newChartArea = transformedInsideChartArea(i);
          }
        }
        if (isUndefined(flags) || flags === Y_ONLY) {
          if (yTransform(i)[3] < 1) {
            yTransform(i)[3] = 1;
            newChartArea = transformedInsideChartArea(i);
          }
        }
        if (isUndefined(flags) || flags === X_ONLY) {
          if (left(newChartArea) > left(insideArea())) {
            diff = left(insideArea()) - left(newChartArea);
            if (isHorizontal())
              yTransform(i)[5] = yTransform(i)[5] + diff;
            else
              xTransform()[4] = xTransform()[4] + diff;
            newChartArea = transformedInsideChartArea(i);
          }
          if (right(newChartArea) < right(insideArea())) {
            diff = right(insideArea()) - right(newChartArea);
            if (isHorizontal())
              yTransform(i)[5] = yTransform(i)[5] + diff;
            else
              xTransform()[4] = xTransform()[4] + diff;
            newChartArea = transformedInsideChartArea(i);
          }
        }
        if (isUndefined(flags) || flags === Y_ONLY) {
          if (top(newChartArea) > top(insideArea())) {
            diff = top(insideArea()) - top(newChartArea);
            if (isHorizontal())
              xTransform()[4] = xTransform()[4] + diff;
            else
              yTransform(i)[5] = yTransform(i)[5] - diff;
            newChartArea = transformedInsideChartArea(i);
          }
          if (bottom(newChartArea) < bottom(insideArea())) {
            diff = bottom(insideArea()) - bottom(newChartArea);
            if (isHorizontal())
              xTransform()[4] = xTransform()[4] + diff;
            else
              yTransform(i)[5] = yTransform(i)[5] - diff;
            newChartArea = transformedInsideChartArea(i);
          }
        }
      }
      setTransformChangedTimeout();
    }

    function loadTooltip() {
      if (tobj.toolTipEl)
        return;
      APP.emit(target.widget, "loadTooltip", tobj.tooltipPosition[X], tobj.tooltipPosition[Y]);
    }

    /* const */ var MouseDistance = 10;

    this.updateTooltip = function(contents) {
      hideTooltip();
      if (contents) {
        if (!tobj.tooltipPosition) {
          return;
        }
        tobj.toolTipEl = document.createElement('div');
        tobj.toolTipEl.className = config.ToolTipInnerStyle;
        tobj.toolTipEl.innerHTML = contents;

        tobj.tooltipOuterDiv = document.createElement('div');
        tobj.tooltipOuterDiv.className = config.ToolTipOuterStyle;

        document.body.appendChild(tobj.tooltipOuterDiv);
        tobj.tooltipOuterDiv.appendChild(tobj.toolTipEl);
        var c = WT.widgetPageCoordinates(target.canvas);

        var x = tobj.tooltipPosition[X] + c.x;
        var y = tobj.tooltipPosition[Y] + c.y;
        WT.fitToWindow(tobj.tooltipOuterDiv, x + MouseDistance, y + MouseDistance,
            x - MouseDistance, y - MouseDistance);

        $(tobj.toolTipEl).mouseenter(function() {
          tobj.overTooltip = true;
        });
        $(tobj.toolTipEl).mouseleave(function() {
          tobj.overTooltip = false;
        });
      }
    }

    this.mouseMove = function(o, event) {
      // Delay mouse move, because IE reacts to
      // mousemove first, but we actually want to
      // handle it after pointer events.
      setTimeout(function() {
        setTimeout(hideTooltip, TOOLTIP_HIDE_DELAY);
        if (pointerActive) return;
        var c = WT.widgetCoordinates(target.canvas, event);
        if (!isPointInRect(c, configArea())) return;

        if (hasToolTips()) {
          tobj.tooltipPosition = [c.x,c.y];
          tobj.tooltipTimeout = setTimeout(function() {
            loadTooltip();
          }, TOOLTIP_TIMEOUT);
        }

        if (dragPreviousXY === null && showCrosshair() && paintEnabled) {
          crosshair = [c.x,c.y];
          rqAnimFrameThrottled(repaintOverlay);
        }
      }, 0);
    }

    this.mouseOut = function(o, event) {
      setTimeout(hideTooltip, TOOLTIP_HIDE_DELAY);
    }

    this.mouseDown = function(o, event) {
      if (pointerActive)
        return;
      var c = WT.widgetCoordinates(target.canvas, event);
      var matchedYAxis = matchYAxis(c.x, c.y);
      var inRect = isPointInRect(c, configArea());
      var matchedXAxis = matchesXAxis(c.x, c.y);
      if (matchedYAxis === -1 && !matchedXAxis && !inRect)
        return;

      dragPreviousXY = c;
      dragXAxis = matchedXAxis;
      dragCurrentYAxis = matchedYAxis;
    };

    this.mouseUp = function(o, event) {
      if (pointerActive)
        return;
      dragPreviousXY = null;
      dragXAxis = false;
      dragCurrentYAxis = -1;
    };

    this.mouseDrag = function(o, event) {
      if (pointerActive)
        return;
      if (dragPreviousXY === null) {
        self.mouseDown(o, event);
        return;
      }
      var c = WT.widgetCoordinates(target.canvas, event);
      if (WT.buttons === 1) {
        if (dragCurrentYAxis === -1 && !dragXAxis &&
            curveManipulation() && configSeries(configSelectedCurve())) {
          var curve = configSelectedCurve();
          var dy;
          if (isHorizontal()) {
            dy = c.x - dragPreviousXY.x;
          } else {
            dy = c.y - dragPreviousXY.y;
          }
          assign(seriesTransform(curve),
              mult([1,0,0,1,0,dy / yTransform(seriesAxis(configSelectedCurve()))[3]],
                seriesTransform(curve)));
          repaint();
        } else if (config.pan) {
          translate({
            x: c.x - dragPreviousXY.x,
            y: c.y - dragPreviousXY.y
          }, 0, dragCurrentYAxis, dragXAxis);
        }
      }
      dragPreviousXY = c;
    };

    this.clicked = function(o, event) {
      if (pointerActive) return;
      if (dragPreviousXY !== null) return;
      if (!seriesSelection()) return;
      var c = WT.widgetCoordinates(target.canvas, event);
      APP.emit(target.widget, 'seriesSelected', c.x, c.y);
    };

    function init() {
      if (showCrosshair() && (isUndefined(overlay) || target.canvas.width !== overlay.width || target.canvas.height !== overlay.height)) {
        if (overlay) {
          overlay.parentNode.removeChild(overlay);
          delete widget.wtOObj;
          overlay = undefined;
        }
        var c = document.createElement("canvas");
        c.setAttribute("width", target.canvas.width);
        c.setAttribute("height", target.canvas.height);
        c.style.position = 'absolute';
        c.style.display = 'block';
        c.style.left = '0';
        c.style.top = '0';
        if (window.MSPointerEvent || window.PointerEvent) {
          c.style.msTouchAction = 'none';
          c.style.touchAction = 'none';
        }
        target.canvas.parentNode.appendChild(c);
        overlay = c;
        widget.wtOObj = overlay;
      } else if (!isUndefined(overlay) && !showCrosshair()) {
        // If the mouse handler is not reinitialized, we don't actually get here!
        overlay.parentNode.removeChild(overlay);
        delete widget.wtOObj;
        overlay = undefined;
      }

      crosshair = [(left(configArea()) + right(configArea())) / 2,
      (top(configArea()) + bottom(configArea())) / 2];
    }

    this.mouseWheel = function(o, event) {
      var modifiers = (event.metaKey << 3) + (event.altKey << 2) + (event.ctrlKey << 1) + event.shiftKey;
      var action = config.wheelActions[modifiers];
      if (isUndefined(action)) return;

      var c = WT.widgetCoordinates(target.canvas, event);
      var matchedXAxis = matchesXAxis(c.x, c.y);
      var matchedYAxis = matchYAxis(c.x, c.y);
      var inRect = isPointInRect(c, configArea());
      if (!matchedXAxis && matchedYAxis === -1 && !inRect)
        return;
      var w = WT.normalizeWheel(event);
      if (inRect && modifiers === 0 && curveManipulation()) {
        // Scale the curve around its middle
        var curve = configSelectedCurve();
        var d = -w.spinY;
        if (configSeries(curve)) {
          var t = curveTransform(curve);
          var s = apply(t,seriesCurve(curve));
          var minMax = minMaxY(s, isHorizontal());
          var middle = (minMax[0] + minMax[1]) / 2;
          WT.cancelEvent(event);
          var s_y = Math.pow(1.2, d);
          assign(seriesTransform(curve),
              mult([1,0,0,s_y,0,middle-s_y*middle],
                seriesTransform(curve)));
          repaint();
          return;
        }
      }
      if ((action === WHEEL_PAN_X || action === WHEEL_PAN_Y || action === WHEEL_PAN_MATCHING) && config.pan) {
        var xBefore = xTransform()[4];
        var yBefore = [];
        for (var yAx = 0; yAx < yAxisCount(); ++yAx) {
          yBefore.push(yTransform(yAx)[5]);
        }
        if (action === WHEEL_PAN_MATCHING)
          translate({x:-w.pixelX,y:-w.pixelY}, 0, matchedYAxis, matchedXAxis);
        else if (action === WHEEL_PAN_Y)
          translate({x:0,y:-w.pixelX - w.pixelY}, 0, matchedYAxis, matchedXAxis);
        else if (action === WHEEL_PAN_X)
          translate({x:-w.pixelX - w.pixelY,y:0}, 0, matchedYAxis, matchedXAxis);
        if (xBefore !== xTransform()[4]) {
          WT.cancelEvent(event);
        }
        for (var yAx = 0; yAx < yAxisCount(); ++yAx) {
          if (yBefore[yAx] !== yTransform(yAx)[5])
            WT.cancelEvent(event);
        }
      } else if (config.zoom) {
        WT.cancelEvent(event);
        var d = -w.spinY;
        // Some browsers scroll horizontally when shift key pressed
        if (d === 0) d = -w.spinX;
        if (action === WHEEL_ZOOM_Y) {
          zoom(c, 0, d, matchedYAxis, matchedXAxis);
        } else if (action === WHEEL_ZOOM_X) {
          zoom(c, d, 0, matchedYAxis, matchedXAxis);
        } else if (action === WHEEL_ZOOM_XY) {
          zoom(c, d, d, matchedYAxis, matchedXAxis);
        } else if (action === WHEEL_ZOOM_MATCHING) {
          if (w.pixelX !== 0)
            zoom(c, d, 0, matchedYAxis, matchedXAxis);
          else
            zoom(c, 0, d, matchedYAxis, matchedXAxis);
        }
      }
    };

    var CROSSHAIR_RADIUS = 30;

    var seriesSelected = function() {
      if (!seriesSelection())
        return;
      APP.emit(target.widget, 'seriesSelected', dragPreviousXY.x, dragPreviousXY.y);
    }

    function topElement() {
      if (overlay)
        return overlay;
      else
        return target.canvas;
    }

    // fromDoubleTouch: indicates that this start of a touch comes from releasing of a double touch,
    //                  so should not be interpreted for series selection
    touchHandlers.start = function(o, event, fromDoubleTouch) {
      singleTouch = len(event.touches) === 1;
      doubleTouch = len(event.touches) === 2;

      if (singleTouch) {
        animating = false;
        var c = WT.widgetCoordinates(target.canvas, event.touches[0]);
        var matchedYAxis = matchYAxis(c.x, c.y);
        var inRect = isPointInRect(c, configArea());
        var matchedXAxis = matchesXAxis(c.x, c.y);
        if (matchedYAxis === -1 && !matchedXAxis && !inRect)
          return;
        if (matchedYAxis === -1 && !matchedXAxis && showCrosshair() && distanceLessThanRadius(crosshair, [c.x,c.y], CROSSHAIR_RADIUS)) {
          mode = CROSSHAIR_MODE;
        } else {
          mode = LOOK_MODE;
        }
        lastDate = Date.now();
        dragPreviousXY = c;
        dragCurrentYAxis = matchedYAxis;
        dragXAxis = matchedXAxis;
        if (mode !== CROSSHAIR_MODE) {
          if (!fromDoubleTouch && inRect) {
            seriesSelectionTimeout = window.setTimeout(seriesSelected, SERIES_SELECTION_TIMEOUT);
          }
          addEventListener('contextmenu', eobj2.contextmenuListener);
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
          WT.widgetCoordinates(target.canvas,event.touches[0]),
          WT.widgetCoordinates(target.canvas,event.touches[1])
        ].map(function(t){return [t.x,t.y];});
        var matchedXAxis = false;
        var matchedYAxis = -1;
        if (!touches.every(function(p){return isPointInRect(p,configArea());})) {
          matchedXAxis = matchesXAxis(touches[0][X], touches[0][Y]) &&
                         matchesXAxis(touches[1][X], touches[1][Y]);
          if (!matchedXAxis)
            matchedYAxis = matchYAxis(touches[0][X], touches[0][Y]);
          if (!matchedXAxis &&
              (matchedYAxis === -1 ||
               matchYAxis(touches[1][X], touches[1][Y]) !== matchedYAxis)) {
            doubleTouch = null;
            return;
          }
          dragXAxis = matchedXAxis;
          dragCurrentYAxis = matchedYAxis;
        }
        WT.capture(null);
        WT.capture(topElement());
        zoomAngle = Math.atan2(touches[1][1] - touches[0][1], touches[1][0] - touches[0][0]);
        zoomMiddle = [
          (touches[0][0] + touches[1][0]) / 2,
          (touches[0][1] + touches[1][1]) / 2];
        var sin = Math.abs(Math.sin(zoomAngle));
        var cos = Math.abs(Math.cos(zoomAngle));
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
        dragXAxis = matchedXAxis;
        dragCurrentYAxis = matchedYAxis;
      } else {
        return;
      }
      preventDefault(event);
    };

    function animate(ts, dt) {
      if (!animating)
        return;
      var now = Date.now();
      if (isUndefined(dt)) {
        dt = now - lastDate;
      }
      var d = {x: 0, y: 0};
      var area;
      if (dragXAxis) {
        area = transformedInsideChartArea(0);
      } else if (dragCurrentYAxis === -1) {
        area = transformedInsideChartArea(0);
        for (var yAx = 1; yAx < yAxisCount(); ++yAx) {
          area = intersection(area, transformedInsideChartArea(yAx));
        }
      } else {
        area = transformedInsideChartArea(dragCurrentYAxis);
      }
      var k = SPRING_CONSTANT;

      if (dt > 2 * ANIMATION_INTERVAL) {
        paintEnabled = false;
        var i = Math.floor(dt / ANIMATION_INTERVAL - 1);
        var j;
        for (j = 0; j < i; ++j) {
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
        if (v.x > 0) v.x = MAX_SPEED;
        else v.x = -MAX_SPEED;
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
        if (Math.abs(v.x) > MAX_SPEED) v.x = (v.x > 0 ? 1 : -1) * MAX_SPEED;
        d.x = v.x * dt;
      }
      if (v.y === Infinity || v.y === -Infinity) {
        if (v.y > 0) v.y = MAX_SPEED;
        else v.y = -MAX_SPEED;
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
        if (Math.abs(v.y) > MAX_SPEED) v.y = (v.y > 0 ? 1 : -1) * MAX_SPEED;
        d.y = v.y * dt;
      }

      if (dragXAxis) {
        area = transformedInsideChartArea(0);
      } else if (dragCurrentYAxis === -1) {
        area = transformedInsideChartArea(0);
        for (var yAx = 1; yAx < yAxisCount(); ++yAx) {
          area = intersection(area, transformedInsideChartArea(yAx));
        }
      } else {
        area = transformedInsideChartArea(dragCurrentYAxis);
      }
      translate(d, NO_LIMIT, dragCurrentYAxis, dragXAxis);
      var newArea;
      if (dragXAxis) {
        newArea = transformedInsideChartArea(0);
      } else if (dragCurrentYAxis === -1) {
        newArea = transformedInsideChartArea(0);
        for (var yAx = 1; yAx < yAxisCount(); ++yAx) {
          newArea = intersection(newArea, transformedInsideChartArea(yAx));
        }
      } else {
        newArea = transformedInsideChartArea(dragCurrentYAxis);
      }
      if (left(area) > left(insideArea()) &&
          left(newArea) <= left(insideArea())) {
        v.x = 0;
        translate({x:-d.x,y:0}, NO_LIMIT, dragCurrentYAxis, dragXAxis);
        enforceLimits(X_ONLY);
      }
      if (right(area) < right(insideArea()) &&
          right(newArea) >= right(insideArea())) {
        v.x = 0;
        translate({x:-d.x,y:0}, NO_LIMIT, dragCurrentYAxis, dragXAxis);
        enforceLimits(X_ONLY);
      }
      if (top(area) > top(insideArea()) &&
          top(newArea) <= top(insideArea())) {
        v.y = 0;
        translate({x:0,y:-d.y}, NO_LIMIT, dragCurrentYAxis, dragXAxis);
        enforceLimits(Y_ONLY);
      }
      if (bottom(area) < bottom(insideArea()) &&
          bottom(newArea) >= bottom(insideArea())) {
        v.y = 0;
        translate({x:0,y:-d.y}, NO_LIMIT, dragCurrentYAxis, dragXAxis);
        enforceLimits(Y_ONLY);
      }
      if (Math.abs(v.x) < STOPPING_SPEED &&
          Math.abs(v.y) < STOPPING_SPEED &&
          isWithinBounds(newArea)) {
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
          rqAnimFrame(animate);
        }
      }
    }

    touchHandlers.end = function(o, event) {
      if (seriesSelectionTimeout) {
        window.clearTimeout(seriesSelectionTimeout);
        seriesSelectionTimeout = null;
      }
      window.setTimeout(function() {
        removeEventListener('contextmenu', eobj2.contextmenuListener);
      }, 0);
      var touches = Array.prototype.slice.call(event.touches);

      var noTouch = len(touches) === 0;

      if (!noTouch) {
        (function(){
          var i;
          for (i = 0; i < len(event.changedTouches); ++i) {
            (function(){
              var id = event.changedTouches[i].identifier;
              for (var j = 0; j < len(touches); ++j) {
                if (touches[j].identifier === id) {
                  touches.splice(j, 1);
                  return;
                }
              }
            })();
          }
        })();
      }

      noTouch     = len(touches) === 0;
      singleTouch = len(touches) === 1;
      doubleTouch = len(touches) === 2;

      if (noTouch) {
        moveTimeout = null;
        if (mode === LOOK_MODE && (isFinite(v.x) || isFinite(v.y)) && config.rubberBand) {
          lastDate = Date.now();
          animating = true;
          rqAnimFrame(animate);
        } else {
          if (mode === CROSSHAIR_MODE)
            self.mouseUp(null, null);
          touches = [];
          zoomAngle = null;
          zoomMiddle = null;
          zoomProjection = null;
          lastDate = null;
        }
        mode = null;
      } else if (singleTouch || doubleTouch)
        touchHandlers.start(o, event, true);
    };

    var moveTimeout = null;
    var c1 = null;
    var c2 = null;

    touchHandlers.moved = function(o, event) {
      if ( (!singleTouch) && (!doubleTouch) ) {
        return;
      }
      if (singleTouch && dragPreviousXY == null) return;
      preventDefault(event);
      c1 = WT.widgetCoordinates(target.canvas, event.touches[0]);
      // kind of breaks pinch-to-zoom?
      if (len(event.touches) > 1)
        c2 = WT.widgetCoordinates(target.canvas, event.touches[1]);
      if (!dragXAxis && dragCurrentYAxis === -1 && singleTouch && seriesSelectionTimeout && !distanceLessThanRadius([c1.x,c1.y],[dragPreviousXY.x,dragPreviousXY.y],3)) {
        window.clearTimeout(seriesSelectionTimeout);
        seriesSelectionTimeout = null;
      }
      // setTimeout prevents high animation velocity due to looking
      // at events that are further apart.
      if (!moveTimeout) moveTimeout = setTimeout(function(){
        if (!dragXAxis && dragCurrentYAxis === -1 && singleTouch && curveManipulation() && configSeries(configSelectedCurve())) {
          var curve = configSelectedCurve();
          if (configSeries(curve)) {
            var c = c1;
            var dy;
            if (isHorizontal()) {
              dy = (c.x - dragPreviousXY.x) / yTransform(seriesAxis(configSelectedCurve()))[3];
            } else {
              dy = (c.y - dragPreviousXY.y) / yTransform(seriesAxis(configSelectedCurve()))[3];
            }
            seriesTransform(curve)[5] += dy;
            dragPreviousXY = c;
            repaint();
          }
        } else if (singleTouch) {
          var c = c1;
          var now = Date.now();
          var d = {
            x: c.x - dragPreviousXY.x,
            y: c.y - dragPreviousXY.y
          };
          var dt = now - lastDate;
          lastDate = now;
          if (mode === CROSSHAIR_MODE) {
            crosshair[X] += d.x;
            crosshair[Y] += d.y;
            if (showCrosshair() && paintEnabled) {
              rqAnimFrame(repaintOverlay);
            }
          } else if (config.pan) {
            v.x = d.x / dt;
            v.y = d.y / dt;
            translate(d, config.rubberBand ? DAMPEN : 0, dragCurrentYAxis, dragXAxis);
          }
          dragPreviousXY = c;
        } else if (!dragXAxis && dragCurrentYAxis === -1 && doubleTouch && curveManipulation() && configSeries(configSelectedCurve())) {
          var yAxis = isHorizontal() ? X : Y;
          var newTouches = [ c1, c2 ].map(function(t){
            if (isHorizontal()) {
              return [t.x, myBefore];
            } else {
              return [mxBefore, t.y];
            }
          });
          var dyBefore = Math.abs(touches[1][yAxis] - touches[0][yAxis]);
          var dyAfter = Math.abs(newTouches[1][yAxis] - newTouches[0][yAxis]);
          var yScale = dyBefore > 0 ? dyAfter / dyBefore : 1;
          if (dyAfter === dyBefore) {
            yScale = 1;
          }
          var curve = configSelectedCurve();
          if (configSeries(curve)) {
            var myBefore = mult(inverted(combinedTransform(seriesAxis(curve))), [0, (touches[0][yAxis] + touches[1][yAxis]) / 2])[1];
            var myAfter = mult(inverted(combinedTransform(seriesAxis(curve))), [0, (newTouches[0][yAxis] + newTouches[1][yAxis]) / 2])[1];
            assign(seriesTransform(curve),
                mult(
                  [1,0,0,yScale,0,-yScale*myBefore+myAfter],
                  seriesTransform(curve)
                  )
                );
            dragPreviousXY = c;
            repaint();
            touches = newTouches;
          }
        } else if (doubleTouch && config.zoom) {
          var crosshairBefore = toModelCoord(crosshair, crosshairAxis());
          var mxBefore = (touches[0][0] + touches[1][0]) / 2;
          var myBefore = (touches[0][1] + touches[1][1]) / 2;
          var newTouches = [ c1, c2 ].map(function(t){
            if (zoomAngle === 0) {
              return [t.x, myBefore];
            } else if (zoomAngle === Math.PI / 2) {
              return [mxBefore, t.y];
            } else {
              return mult(zoomProjection,[t.x,t.y]);
            }
          });

          var dxBefore = Math.abs(touches[1][0] - touches[0][0]);
          var dxAfter = Math.abs(newTouches[1][0] - newTouches[0][0]);
          var xScale = dxBefore > 0 ? dxAfter / dxBefore : 1;
          if (dxAfter === dxBefore || zoomAngle === Math.PI / 2) {
            xScale = 1;
          }
          var mxAfter = (newTouches[0][0] + newTouches[1][0]) / 2;
          var dyBefore = Math.abs(touches[1][1] - touches[0][1]);
          var dyAfter = Math.abs(newTouches[1][1] - newTouches[0][1]);
          var yScale = dyBefore > 0 ? dyAfter / dyBefore : 1;
          if (dyAfter === dyBefore || zoomAngle === 0) {
            yScale = 1;
          }
          var myAfter = (newTouches[0][1] + newTouches[1][1]) / 2;

          if (isHorizontal()) {
            (function() {
              var tmp = xScale;
              xScale = yScale;
              yScale = tmp;
              tmp = mxAfter;
              mxAfter = myAfter;
              myAfter = tmp;
              tmp = mxBefore;
              mxBefore = myBefore;
              myBefore = tmp;
            })();
          }

          if (xTransform()[0] * xScale > maxXZoom()) {
            xScale = maxXZoom() / xTransform()[0];
          }
          if (xTransform()[0] * xScale < minXZoom()) {
            xScale = minXZoom() / xTransform()[0];
          }
          var yScalePerAxis = [];
          for (var yAx = 0; yAx < yAxisCount(); ++yAx) {
            yScalePerAxis.push(yScale);
          }
          for (var yAx = 0; yAx < yAxisCount(); ++yAx) {
            if (yTransform(yAx)[3] * yScalePerAxis[yAx] > maxYZoom(yAx)) {
              yScalePerAxis[yAx] = maxYZoom(yAx) / yTransform(yAx)[3];
            }
            if (yTransform(yAx)[3] * yScalePerAxis[yAx] < minYZoom(yAx)) {
              yScalePerAxis[yAx] = minYZoom(yAx) / yTransform(yAx)[3];
            }
          }
          if (dragXAxis) {
            if (xScale !== 1 &&
                (xScale < 1.0 || xTransform()[0] !== maxXZoom())) {
              tAssign(xTransform(),
                  mult(
                    [xScale,0,0,1,-xScale*mxBefore+mxAfter,0],
                    xTransform()
                    )
                  );
            }
          } else if (dragCurrentYAxis === -1) {
            if (xScale !== 1 &&
                (xScale < 1.0 || xTransform()[0] !== maxXZoom())) {
              tAssign(xTransform(),
                  mult(
                    [xScale,0,0,1,-xScale*mxBefore+mxAfter,0],
                    xTransform()
                    )
                  );
            }
            for (var yAx = 0; yAx < yAxisCount(); ++yAx) {
              if (yScalePerAxis[yAx] !== 1 &&
                  (yScalePerAxis[yAx] < 1.0 || yTransform(yAx)[3] !== maxYZoom(yAx))) {
                tAssign(yTransform(yAx),
                    mult(
                      [1,0,0,yScalePerAxis[yAx],0,-yScalePerAxis[yAx]*myBefore+myAfter],
                      yTransform(yAx)
                      )
                    );
              }
            }
          } else {
            if (yScalePerAxis[dragCurrentYAxis] !== 1 &&
                (yScalePerAxis[dragCurrentYAxis] < 1.0 || yTransform(dragCurrentYAxis)[3] !== maxYZoom(dragCurrentYAxis))) {
              tAssign(yTransform(dragCurrentYAxis),
                  mult(
                    [1,0,0,yScalePerAxis[dragCurrentYAxis],0,-yScalePerAxis[dragCurrentYAxis]*myBefore+myAfter],
                    yTransform(dragCurrentYAxis)
                    )
                  );
            }
          }
          enforceLimits();

          var crosshairAfter = toDisplayCoord(crosshairBefore, crosshairAxis());
          crosshair[X] = crosshairAfter[X];
          crosshair[Y] = crosshairAfter[Y];

          touches = newTouches;
          refreshPenColors();
          repaint();
          notifyAreaChanged();
        }
        moveTimeout = null;
      }, 1);
    };

    function refreshPenColors() {
      var i, j;
      var xLevel = toZoomLevel(xTransform()[0]) - 1;
      if (xTransform()[0] == maxXZoom())
        xLevel = len(pens().x) - 1;
      if (xLevel >= len(pens().x))
        xLevel = len(pens().x) - 1;
      for (i = 0; i < len(pens().x); ++i) {
        if (xLevel === i) {
          for (j = 0; j < len(pens().x[i]); ++j) {
            pens().x[i][j].color[3] = penAlpha().x[j];
          }
        } else {
          for (j = 0; j < len(pens().x[i]); ++j) {
            pens().x[i][j].color[3] = 0;
          }
        }
      }
      for (var yAx = 0; yAx < len(pens().y); ++yAx) {
        var yLevel = toZoomLevel(yTransform(yAx)[3]) - 1;
        if (yTransform(yAx)[3] == maxYZoom(yAx))
          yLevel = len(pens().y[yAx]) - 1;
        if (yLevel >= len(pens().y[yAx]))
          yLevel = len(pens().y[yAx]) - 1;
        for (i = 0; i < len(pens().y[yAx]); ++i) {
          if (yLevel === i) {
            for (j = 0; j < len(pens().y[yAx][i]); ++j) {
              pens().y[yAx][i][j].color[3] = penAlpha().y[yAx][j];
            }
          } else {
            for (j = 0; j < len(pens().y[yAx][i]); ++j) {
              pens().y[yAx][i][j].color[3] = 0;
            }
          }
        }
      }
    }

    function translate(d, flags, matchedYAxis, dragX) {
      if (isUndefined(flags))
        flags = 0;
      if (isUndefined(matchedYAxis))
        matchedYAxis = -1;
      if (isUndefined(dragX))
        dragX = false;
      var crosshairBefore = toModelCoord(crosshair, crosshairAxis());

      if (isHorizontal()) {
        d = {x:d.y,y:-d.x};
      }

      if (flags & NO_LIMIT) {
        if (dragX) {
          xTransform()[4] = xTransform()[4] + d.x;
        } else if (matchedYAxis === -1) {
          xTransform()[4] = xTransform()[4] + d.x;
          for (var yAx = 0; yAx < yAxisCount(); ++yAx)
            yTransform(yAx)[5] = yTransform(yAx)[5] - d.y;
        } else {
          yTransform(matchedYAxis)[5] = yTransform(matchedYAxis)[5] - d.y;
        }
        setTransformChangedTimeout();
      } else if (flags & DAMPEN) {
        var area;
        if (dragX) {
          area = transformedInsideChartArea(0);
        } else if (matchedYAxis === -1) {
          area = transformedInsideChartArea(0);
          for (var yAx = 1; yAx < yAxisCount(); ++yAx) {
            area = intersection(area, transformedInsideChartArea(yAx));
          }
        } else {
          area = transformedInsideChartArea(matchedYAxis);
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
        if (dragX) {
          xTransform()[4] = xTransform()[4] + d.x;
        } else if (matchedYAxis === -1) {
          xTransform()[4] = xTransform()[4] + d.x;
          for (var yAx = 0; yAx < yAxisCount(); ++yAx)
            yTransform(yAx)[5] = yTransform(yAx)[5] - d.y;
        } else {
          yTransform(matchedYAxis)[5] = yTransform(matchedYAxis)[5] - d.y;
        }
        if (matchedYAxis === -1)
          crosshair[X] = crosshair[X] + d.x;
        if (!dragX)
          crosshair[Y] = crosshair[Y] + d.y;
        setTransformChangedTimeout();
      } else {
        if (dragX) {
          xTransform()[4] = xTransform()[4] + d.x;
        } else if (matchedYAxis === -1) {
          xTransform()[4] = xTransform()[4] + d.x;
          for (var yAx = 0; yAx < yAxisCount(); ++yAx)
            yTransform(yAx)[5] = yTransform(yAx)[5] - d.y;
        } else {
          yTransform(matchedYAxis)[5] = yTransform(matchedYAxis)[5] - d.y;
        }
        if (matchedYAxis === -1)
          crosshair[X] = crosshair[X] + d.x;
        if (!dragX)
          crosshair[Y] = crosshair[Y] + d.y;

        enforceLimits();
      }

      var crosshairAfter = toDisplayCoord(crosshairBefore, crosshairAxis());

      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      repaint();
      notifyAreaChanged();
    }

    function zoom(coords, xDelta, yDelta, matchedYAxis, matchedXAxis) {
      if (isUndefined(matchedYAxis))
        matchedYAxis = -1;
      if (isUndefined(matchedXAxis))
        matchedXAxis = false;
      var crosshairBefore = toModelCoord(crosshair, crosshairAxis());
      var xy;
      if (isHorizontal()) {
        xy = [coords.y - top(configArea()), coords.x - left(configArea())];
      } else {
        xy = mult(
            inverted([1,0,0,-1,left(configArea()),bottom(configArea())]), [coords.x, coords.y]);
      }
      var x = xy[0];
      var y = xy[1];
      var s_x = Math.pow(1.2, isHorizontal() ? yDelta : xDelta);
      var s_y = Math.pow(1.2, isHorizontal() ? xDelta : yDelta);
      if (xTransform()[0] * s_x > maxXZoom()) {
        s_x = maxXZoom() / xTransform()[0];
      }
      if (xTransform()[0] * s_x < minXZoom()) {
        s_x = minXZoom() / xTransform()[0];
      }
      if (matchedXAxis) {
        if (s_x < 1.0 || xTransform()[0] !== maxXZoom()) {
          tAssign(xTransform(),
              mult([s_x,0,0,1,x-s_x*x,0], xTransform()));
        }
      } else if (matchedYAxis === -1) {
        if (s_x < 1.0 || xTransform()[0] !== maxXZoom()) {
          tAssign(xTransform(),
              mult([s_x,0,0,1,x-s_x*x,0], xTransform()));
        }
        for (var yAx = 0; yAx < yAxisCount(); ++yAx) {
          var s_specific_y = s_y;
          if (yTransform(yAx)[3] * s_y > maxYZoom(yAx)) {
            s_specific_y = maxYZoom(yAx) / yTransform(yAx)[3];
          }
          if (s_specific_y < 1.0 || yTransform(yAx)[3] !== maxYZoom(yAx)) {
            tAssign(yTransform(yAx), mult([1,0,0,s_specific_y,0,y-s_specific_y*y], yTransform(yAx)));
          }
        }
      } else {
        if (yTransform(matchedYAxis)[3] * s_y > maxYZoom(matchedYAxis)) {
          s_y = maxYZoom(matchedYAxis) / yTransform(matchedYAxis)[3];
        }
        if (s_y < 1.0 || yTransform(matchedYAxis)[3] != maxYZoom(matchedYAxis)) {
          tAssign(yTransform(matchedYAxis), mult([1,0,0,s_y,0,y-s_y*y], yTransform(matchedYAxis)));
        }
      }

      enforceLimits();

      var crosshairAfter = toDisplayCoord(crosshairBefore, crosshairAxis());
      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      refreshPenColors();
      repaint();
      notifyAreaChanged();
    }

    this.setXRange = function(seriesNb, lowerBound, upperBound, updateYAxis) {
      lowerBound = modelArea(0)[0] + modelArea(0)[2] * lowerBound;
      upperBound = modelArea(0)[0] + modelArea(0)[2] * upperBound;
      //Constrain given range
      if (left(modelArea(0)) > right(modelArea(0))) {
        if (lowerBound > left(modelArea(0)))
          lowerBound = left(modelArea(0));
        if (upperBound < right(modelArea(0)))
          upperBound = right(modelArea(0));
      } else {
        if (lowerBound < left(modelArea(0)))
          lowerBound = left(modelArea(0));
        if (upperBound > right(modelArea(0)))
          upperBound = right(modelArea(0));
      }
      // Set X range, and adjust Y!
      var series = seriesCurve(seriesNb);

      var res = findYRange(series, seriesAxis(seriesNb), lowerBound, upperBound, isHorizontal(), configArea(), modelArea(seriesAxis(seriesNb)), config.minZoom, config.maxZoom);
      var xZoom = res.xZoom;
      var yZoom = res.yZoom;
      var panPoint = res.panPoint;

      var crosshairBefore = toModelCoord(crosshair, crosshairAxis());

      xTransform()[0] = xZoom;
      if (yZoom && updateYAxis)
        yTransform(seriesAxis(seriesNb))[3] = yZoom;
      xTransform()[4] = -panPoint[X] * xZoom;
      if (yZoom && updateYAxis)
        yTransform(seriesAxis(seriesNb))[5] = -panPoint[Y] * yZoom;
      setTransformChangedTimeout();

      var crosshairAfter = toDisplayCoord(crosshairBefore, crosshairAxis());
      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      enforceLimits();
      refreshPenColors();
      repaint();
      notifyAreaChanged();
    }

    this.getSeries = function(seriesNb) {
      return seriesCurve(seriesNb);
    }

    this.rangeChangedCallbacks = [];

    this.updateConfig = function(newConfig) {
      for (var key in newConfig) {
        if (newConfig.hasOwnProperty(key)) {
          config[key] = newConfig[key];
        }
      }
      init();
      refreshPenColors();
      repaint();
      notifyAreaChanged();
    }

    this.updateConfig({});

    if (window.TouchEvent && !window.MSPointerEvent && !window.PointerEvent) {
      self.touchStart = touchHandlers.start;
      self.touchEnd = touchHandlers.end;
      self.touchMoved = touchHandlers.moved;
    } else {
      var nop = function(){};
      self.touchStart = nop;
      self.touchEnd = nop;
      self.touchMoved = nop;
    }
  });
