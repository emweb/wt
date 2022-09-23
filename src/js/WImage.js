/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WImage", function(APP, el, target) {
  el.wtObj = this;

  const self = this;

  const WAIT_TIMEOUT = 100;
  const ROW_QUANTUM = 50;
  let areaCoordsJSON = null;

  let waitStarted = 0;
  let currentTransform = "";
  let timeoutId = null;
  let updateRow = 0;

  this.setAreaCoordsJSON = function(newAreaCoordsJSON) {
    areaCoordsJSON = newAreaCoordsJSON;
    currentTransform = "";
    this.updateAreas();
  };

  this.updateAreas = function() {
    const combinedTransformJS = target.combinedTransform;
    if (typeof combinedTransformJS === "undefined" || areaCoordsJSON === null) {
      return;
    }

    const newTransform = combinedTransformJS();
    const transformChanged = (newTransform.toString() !== currentTransform.toString());

    /* if idle state */
    if (waitStarted === 0 && updateRow === 0) {
      /* ...and transform unchanged, we are done...  */
      if (!transformChanged) {
        return;
      } /* Otherwise, wait to see if more transform changes occur... */
      else {
        currentTransform = newTransform;
        waitStarted = new Date().getTime();
        timeoutId = setTimeout(this.updateAreas, WAIT_TIMEOUT);
        return;
      }
    }

    const timeNow = new Date().getTime();

    /* if the transform changed, enter wait state */
    if (transformChanged) {
      currentTransform = newTransform;
      waitStarted = timeNow;
      updateRow = 0;
      if (timeoutId) {
        clearTimeout(timeoutId);
      }
      timeoutId = setTimeout(this.updateAreas, WAIT_TIMEOUT);
      return;
    }

    /* if already updating or wait timer expired, continue updating */
    if (updateRow > 0 || (waitStarted && (timeNow - waitStarted > WAIT_TIMEOUT))) {
      /* update some rows, and enter idle state if done */
      if (self.updateAreaCoords(ROW_QUANTUM)) {
        // console.log('updateAreaCoords: complete');
        waitStarted = 0;
        updateRow = 0;
        if (timeoutId) {
          clearTimeout(timeoutId);
        }
        timeoutId = null;
        return;
      }

      /* some rows updated - update more after processing event loop */
      if (timeoutId) {
        clearTimeout(timeoutId);
      }
      timeoutId = setTimeout(self.updateAreas, 0);
      return;
    } /* new call or timeout with quantum not expired ... */
    else {
      let timeRemaining = timeNow - waitStarted;
      if (timeRemaining > WAIT_TIMEOUT) {
        timeRemaining = WAIT_TIMEOUT;
      }
      if (timeoutId) {
        clearTimeout(timeoutId);
      }
      timeoutId = setTimeout(self.updateAreas, timeRemaining);
    }
  };

  this.updateAreaCoords = function(quantum) {
    // console.log('updateAreaCoords: updateRow: ' + updateRow + ", transform: " + currentTransform);
    const mult = APP.WT.gfxUtils.transform_mult;

    let endIndex = updateRow + quantum;
    if (endIndex > areaCoordsJSON.length) {
      endIndex = areaCoordsJSON.length;
    }

    while (updateRow < endIndex) {
      const coordEntry = areaCoordsJSON[updateRow];
      const areaEl = coordEntry[0];
      const points = coordEntry[1];

      const len = points.length;
      let new_coords = "";
      let i = 0;
      for (; i + 1 < len; i += 2) {
        if (i > 0) {
          new_coords += ",";
        }
        const p = mult(currentTransform, points.slice(i, i + 2));
        new_coords += Math.round(p[0]).toString() + "," + Math.round(p[1]).toString();
      }

      if (i < len) { // a circle -- copy radius unchanged
        new_coords += "," + points[i].toString();
      }
      if (areaEl) {
        areaEl.coords = new_coords;
      }
      updateRow++;
    }
    return (endIndex === areaCoordsJSON.length);
  };
});
