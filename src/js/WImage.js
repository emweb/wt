/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WImage",
 function(APP, el, target) {
   el.wtObj = this;

   var self = this;
   var WT = APP.WT;

   var WAIT_TIMEOUT = 100;
   var ROW_QUANTUM = 50;
   var areaCoordsJSON = null;

   var waitStarted = 0;
   var currentTransform = "";
   var timeoutId = null;
   var updateRow = 0;

   this.setAreaCoordsJSON = function(newAreaCoordsJSON) {
     areaCoordsJSON = newAreaCoordsJSON;
     currentTransform = "";
     this.updateAreas();
   }

   this.updateAreas = function() {
     var combinedTransformJS = target.combinedTransform;
     if (combinedTransformJS === undefined || areaCoordsJSON === null)
       return;

     var newTransform = combinedTransformJS();
     var transformChanged = (newTransform.toString() !== currentTransform.toString());

// console.log('updateAreas: currentTransform: ' + currentTransform + ', newTransform: ' + newTransform + ', changed: ' + transformChanged);
     /* if idle state */
     if (waitStarted === 0 && updateRow === 0) {

       /* ...and transform unchanged, we are done...  */
       if (!transformChanged)
         return;

       /* Otherwise, wait to see if more transform changes occur... */
       else {
         currentTransform = newTransform;
         waitStarted = new Date().getTime();
         timeoutId = setTimeout(this.updateAreas, WAIT_TIMEOUT);
         return;
       }
     }

     var timeNow = new Date().getTime();

     /* if the transform changed, enter wait state */
     if (transformChanged) {
       currentTransform = newTransform;
       waitStarted = timeNow;
       updateRow = 0;
       if (timeoutId)
         clearTimeout(timeoutId);
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
         if (timeoutId)
           clearTimeout(timeoutId);
         timeoutId = null;
         return;
       }

       /* some rows updated - update more after processing event loop */
       if (timeoutId)
         clearTimeout(timeoutId);
       timeoutId = setTimeout(self.updateAreas, 0);
       return;
     }
     /* new call or timeout with quantum not expired ... */
     else {
       var timeRemaining = timeNow - waitStarted;
       if (timeRemaining > WAIT_TIMEOUT) {
         timeRemaining = WAIT_TIMEOUT;
       }
       if (timeoutId)
         clearTimeout(timeoutId);
       timeoutId = setTimeout(self.updateAreas, timeRemaining);
     }
   }

   this.updateAreaCoords = function(quantum) {
// console.log('updateAreaCoords: updateRow: ' + updateRow + ", transform: " + currentTransform);
     var mult = APP.WT.gfxUtils.transform_mult;

     var endIndex = updateRow + quantum;
     if (endIndex > areaCoordsJSON.length)
       endIndex = areaCoordsJSON.length;

     while (updateRow < endIndex) {
       var coordEntry = areaCoordsJSON[updateRow];
       var areaEl = coordEntry[0];
       var points = coordEntry[1];

       var len = points.length;
       var new_coords = "";
       for (var i = 0; i + 1 < len; i += 2) {
         if (i > 0)
           new_coords += ",";
         var p = mult(currentTransform, points.slice(i, i + 2));
         new_coords += Math.round(p[0]).toString() + "," + Math.round(p[1]).toString();
       }

       if (i < len) // a circle -- copy radius unchanged
         new_coords += ("," + points[i].toString());
       if (areaEl)
         areaEl.coords = new_coords;
       updateRow++;
     }
     return (endIndex === areaCoordsJSON.length);
   }

 });
