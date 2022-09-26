/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(2, JavaScriptConstructor, "ChartCommon", function(APP) {
  const _MOVE_TO = 0,
    _LINE_TO = 1,
    CUBIC_C1 = 2,
    CUBIC_C2 = 3,
    _CUBIC_END = 4,
    QUAD_C = 5,
    _QUAD_END = 6,
    _ARC_C = 7,
    _ARC_R = 8,
    _ARC_ANGLE_SWEEP = 9;

  const X = 0, Y = 1;
  const WT = APP.WT;

  const self = this;

  const utils = WT.gfxUtils;
  const top = utils.rect_top;
  const bottom = utils.rect_bottom;
  const left = utils.rect_left;
  const right = utils.rect_right;
  const mult = utils.transform_mult;

  // Find the anchor point (not a curve control point!)
  // with X coordinate nearest to the given x,
  // and smaller than the given x, and return
  // its index in the given series.
  function binarySearch(x, series, ascending, isHorizontal) {
    let axis = X;
    if (isHorizontal) {
      axis = Y;
    }
    const len = series.length;
    function s(i) {
      if (ascending) {
        return series[i];
      } else {
        return series[len - 1 - i];
      }
    }
    // Move back to a non-control point.
    function moveBack(i) {
      while (s(i)[2] === CUBIC_C1 || s(i)[2] === CUBIC_C2) {
        i--;
      }
      return i;
    }
    let i = Math.floor(len / 2);
    i = moveBack(i);
    let lower_bound = 0;
    let upper_bound = len;
    let found = false;
    if (s(0)[axis] > x) {
      return ascending ? -1 : len;
    }
    if (s(len - 1)[axis] < x) {
      return ascending ? len : -1;
    }
    while (!found) {
      let next_i = i + 1;
      if (next_i < len && (s(next_i)[2] === CUBIC_C1 || s(next_i)[2] === CUBIC_C2)) {
        next_i += 2;
      }
      if (s(i)[axis] > x) {
        upper_bound = i;
        i = Math.floor((upper_bound + lower_bound) / 2);
        i = moveBack(i);
      } else {
        if (s(i)[axis] === x) {
          found = true;
        } else {
          if (next_i < len && s(next_i)[axis] > x) {
            found = true;
          } else if (next_i < len && s(next_i)[axis] === x) {
            i = next_i;
            found = true;
          } else {
            lower_bound = i;
            i = Math.floor((upper_bound + lower_bound) / 2);
            i = moveBack(i);
          }
        }
      }
    }
    return ascending ? i : len - 1 - i;
  }

  function isAscending(axis, series) {
    return series[0][axis] < series[series.length - 1][axis];
  }

  this.findClosestPoint = function(x, series, isHorizontal) {
    let axis = X;
    if (isHorizontal) {
      axis = Y;
    }
    const ascending = isAscending(axis, series);
    let i = binarySearch(x, series, ascending, isHorizontal);
    if (i < 0) {
      i = 0;
    }
    if (i >= series.length) {
      return [series[series.length - 1][X], series[series.length - 1][Y]];
    }
    if (i >= series.length) {
      i = series.length - 2;
    }
    if (series[i][axis] === x) {
      return [series[i][X], series[i][Y]];
    }
    let next_i = ascending ? i + 1 : i - 1;
    if (ascending && series[next_i][2] === CUBIC_C1) {
      next_i += 2;
    }
    if (!ascending && next_i < 0) {
      return [series[i][X], series[i][Y]];
    }
    if (!ascending && next_i > 0 && series[next_i][2] === CUBIC_C2) {
      next_i -= 2;
    }
    const d1 = Math.abs(x - series[i][axis]);
    const d2 = Math.abs(series[next_i][axis] - x);
    if (d1 < d2) {
      return [series[i][X], series[i][Y]];
    } else {
      return [series[next_i][X], series[next_i][Y]];
    }
  };

  this.minMaxY = function(series, isHorizontal) {
    const yAxis = isHorizontal ? X : Y;
    let min = series[0][yAxis];
    let max = series[0][yAxis];
    for (let i = 1; i < series.length; ++i) {
      if (
        series[i][2] !== CUBIC_C1 && series[i][2] !== CUBIC_C2 &&
        series[i][2] !== QUAD_C
      ) {
        if (series[i][yAxis] > max) {
          max = series[i][yAxis];
        }
        if (series[i][yAxis] < min) {
          min = series[i][yAxis];
        }
      }
    }
    return [min, max];
  };

  // Get projection matrix to project any point
  // to a line through m at angle theta
  this.projection = function(theta, m) {
    const c = Math.cos(theta);
    const s = Math.sin(theta);
    const c2 = c * c;
    const s2 = s * s;
    const cs = c * s;
    const h = -m[0] * c - m[1] * s;
    return [c2, cs, cs, s2, c * h + m[0], s * h + m[1]];
  };

  this.distanceSquared = function(p1, p2) {
    const d = [p2[X] - p1[X], p2[Y] - p1[Y]];
    return d[X] * d[X] + d[Y] * d[Y];
  };

  this.distanceLessThanRadius = function(p1, p2, radius) {
    return radius * radius >= self.distanceSquared(p1, p2);
  };

  this.toZoomLevel = function(zoomFactor) {
    return Math.floor(Math.log(zoomFactor) / Math.LN2 + 0.5) + 1;
  };

  // Check if a point is inside of the given rect
  /**
   * @param {[number]|{x:number,y:number}} point
   * @param {[number]} rect
   */
  this.isPointInRect = function(point, rect) {
    const x = point.x ?? point[0];
    const y = point.y ?? point[1];
    return x >= left(rect) && x <= right(rect) &&
      y >= top(rect) && y <= bottom(rect);
  };

  this.toDisplayCoord = function(p, transform, isHorizontal, area, modelArea) {
    let u, res;
    if (isHorizontal) {
      u = [(p[X] - modelArea[0]) / modelArea[2], (p[Y] - modelArea[1]) / modelArea[3]];
      res = [area[0] + u[Y] * area[2], area[1] + u[X] * area[3]];
    } else {
      u = [(p[X] - modelArea[0]) / modelArea[2], 1 - (p[Y] - modelArea[1]) / modelArea[3]];
      res = [area[0] + u[X] * area[2], area[1] + u[Y] * area[3]];
    }
    return mult(transform, res);
  };

  this.findYRange = function(
    series,
    seriesAxis,
    lowerBound,
    upperBound,
    horizontal,
    area,
    modelArea,
    minZoom,
    maxZoom
  ) {
    if (series.length === 0) {
      return null; // This would be weird?
    }
    const p0 = self.toDisplayCoord([lowerBound, 0], [1, 0, 0, 1, 0, 0], horizontal, area, modelArea);
    const p1 = self.toDisplayCoord([upperBound, 0], [1, 0, 0, 1, 0, 0], horizontal, area, modelArea);
    const axis = horizontal ? Y : X;
    const otherAxis = horizontal ? X : Y;
    const ascending = isAscending(axis, series);
    let i0 = binarySearch(p0[axis], series, ascending, horizontal);
    let i_n = binarySearch(p1[axis], series, ascending, horizontal);
    let i, u, y, before_i0, after_i_n;
    let min_y = Infinity;
    let max_y = -Infinity;
    const outsideRange = (i0 === i_n && i0 === series.length) || (i0 === -1 && i_n === -1);
    if (!outsideRange) {
      if (ascending) {
        if (i0 < 0) {
          i0 = 0;
        } else {
          i0++;
          if (series[i0] && series[i0][2] === CUBIC_C1) {
            i0 += 2;
          }
        }
      } else if (i0 >= series.length - 1) {
        i0 = series.length - 2;
      }
      if (!ascending && i_n < 0) {
        i_n = 0;
      }
      for (i = Math.min(i0, i_n); i <= Math.max(i0, i_n) && i < series.length; ++i) {
        if (series[i][2] !== CUBIC_C1 && series[i][2] !== CUBIC_C2) {
          if (series[i][otherAxis] < min_y) {
            min_y = series[i][otherAxis];
          }
          if (series[i][otherAxis] > max_y) {
            max_y = series[i][otherAxis];
          }
        }
      }
      if (ascending && i0 > 0 || !ascending && i0 < series.length - 1) {
        // Interpolate on the lower X end
        if (ascending) {
          before_i0 = i0 - 1;
          if (series[before_i0] && series[before_i0][2] === CUBIC_C2) {
            before_i0 -= 2;
          }
        } else {
          before_i0 = i0 + 1;
          if (series[before_i0] && series[before_i0][2] === CUBIC_C1) {
            before_i0 += 2;
          }
        }
        u = (p0[axis] - series[before_i0][axis]) / (series[i0][axis] - series[before_i0][axis]);
        y = series[before_i0][otherAxis] + u * (series[i0][otherAxis] - series[before_i0][otherAxis]);
        if (y < min_y) {
          min_y = y;
        }
        if (y > max_y) {
          max_y = y;
        }
      }
      if (ascending && i_n < series.length - 1 || !ascending && i_n > 0) {
        // Interpolate on the upper X end
        if (ascending) {
          after_i_n = i_n + 1;
          if (series[after_i_n][2] === CUBIC_C1) {
            after_i_n += 2;
          }
        } else {
          after_i_n = i_n - 1;
          if (series[after_i_n][2] === CUBIC_C2) {
            after_i_n -= 2;
          }
        }
        u = (p1[axis] - series[i_n][axis]) / (series[after_i_n][axis] - series[i_n][axis]);
        y = series[i_n][otherAxis] + u * (series[after_i_n][otherAxis] - series[i_n][otherAxis]);
        if (y < min_y) {
          min_y = y;
        }
        if (y > max_y) {
          max_y = y;
        }
      }
    }
    let yZoom, yMargin;
    const xZoom = modelArea[2] / (upperBound - lowerBound);
    const H = horizontal ? 2 : 3;
    if (!outsideRange) {
      yZoom = area[H] / (max_y - min_y);
      yMargin = 10;
      yZoom = area[H] / (area[H] / yZoom + yMargin * 2); // Give it 10 px extra on each side
      if (yZoom > maxZoom.y[seriesAxis]) {
        yZoom = maxZoom.y[seriesAxis];
      }
      if (yZoom < minZoom.y[seriesAxis]) {
        yZoom = minZoom.y[seriesAxis];
      }
    }
    let panPoint;
    if (horizontal) {
      panPoint = [p0[Y] - top(area), !outsideRange ? ((min_y + max_y) / 2 - (area[2] / yZoom) / 2 - left(area)) : 0];
    } else {
      panPoint = [
        p0[X] - left(area),
        !outsideRange ? -((min_y + max_y) / 2 + (area[3] / yZoom) / 2 - bottom(area)) : 0,
      ];
    }
    return { xZoom: xZoom, yZoom: yZoom, panPoint: panPoint };
  };

  this.matchXAxis = function(x, y, area, xAxes, isHorizontal) {
    function xAxisCount() {
      return xAxes.length;
    }
    function xAxisSide(ax) {
      return xAxes[ax].side;
    }
    function xAxisWidth(ax) {
      return xAxes[ax].width;
    }
    function xAxisMinOffset(ax) {
      return xAxes[ax].minOffset;
    }
    function xAxisMaxOffset(ax) {
      return xAxes[ax].maxOffset;
    }
    // Check if the given x, y position (in pixels) matches an axis.
    // If so, this returns the axis id, otherwise this returns -1.
    if (isHorizontal) {
      if (y < top(area) || y > bottom(area)) {
        return -1;
      }
    } else {
      if (x < left(area) || x > right(area)) {
        return -1;
      }
    }
    for (let xAx = 0; xAx < xAxisCount(); ++xAx) {
      if (isHorizontal) {
        if (
          (xAxisSide(xAx) === "min" || xAxisSide(xAx) === "both") &&
          x >= left(area) - xAxisMinOffset(xAx) - xAxisWidth(xAx) &&
          x <= left(area) - xAxisMinOffset(xAx)
        ) {
          return xAx;
        } else if (
          (xAxisSide(xAx) === "max" || xAxisSide(xAx) === "both") &&
          x >= right(area) + xAxisMaxOffset(xAx) &&
          x <= right(area) + xAxisMaxOffset(xAx) + xAxisWidth(xAx)
        ) {
          return xAx;
        }
      } else {
        if (
          (xAxisSide(xAx) === "min" || xAxisSide(xAx) === "both") &&
          y <= bottom(area) + xAxisMinOffset(xAx) + xAxisWidth(xAx) &&
          y >= bottom(area) + xAxisMinOffset(xAx)
        ) {
          return xAx;
        } else if (
          (xAxisSide(xAx) === "max" || xAxisSide(xAx) === "both") &&
          y <= top(area) - xAxisMaxOffset(xAx) &&
          y >= top(area) - xAxisMaxOffset(xAx) - xAxisWidth(xAx)
        ) {
          return xAx;
        }
      }
    }
    return -1;
  };

  this.matchYAxis = function(x, y, area, yAxes, isHorizontal) {
    function yAxisCount() {
      return yAxes.length;
    }
    function yAxisSide(ax) {
      return yAxes[ax].side;
    }
    function yAxisWidth(ax) {
      return yAxes[ax].width;
    }
    function yAxisMinOffset(ax) {
      return yAxes[ax].minOffset;
    }
    function yAxisMaxOffset(ax) {
      return yAxes[ax].maxOffset;
    }
    // Check if the given x, y position (in pixels) matches an axis.
    // If so, this returns the axis id, otherwise this returns -1.
    if (isHorizontal) {
      if (x < left(area) || x > right(area)) {
        return -1;
      }
    } else {
      if (y < top(area) || y > bottom(area)) {
        return -1;
      }
    }
    for (let yAx = 0; yAx < yAxisCount(); ++yAx) {
      if (isHorizontal) {
        if (
          (yAxisSide(yAx) === "min" || yAxisSide(yAx) === "both") &&
          y >= top(area) - yAxisMinOffset(yAx) - yAxisWidth(yAx) &&
          y <= top(area) - yAxisMinOffset(yAx)
        ) {
          return yAx;
        } else if (
          (yAxisSide(yAx) === "max" || yAxisSide(yAx) === "both") &&
          y >= bottom(area) + yAxisMaxOffset(yAx) &&
          y <= bottom(area) + yAxisMaxOffset(yAx) + yAxisWidth(yAx)
        ) {
          return yAx;
        }
      } else {
        if (
          (yAxisSide(yAx) === "min" || yAxisSide(yAx) === "both") &&
          x >= left(area) - yAxisMinOffset(yAx) - yAxisWidth(yAx) &&
          x <= left(area) - yAxisMinOffset(yAx)
        ) {
          return yAx;
        } else if (
          (yAxisSide(yAx) === "max" || yAxisSide(yAx) === "both") &&
          x >= right(area) + yAxisMaxOffset(yAx) &&
          x <= right(area) + yAxisMaxOffset(yAx) + yAxisWidth(yAx)
        ) {
          return yAx;
        }
      }
    }
    return -1;
  };
});
