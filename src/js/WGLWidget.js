/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WGLWidget",
 function(APP, canvas) {
   jQuery.data(canvas, 'obj', this);

   var self = this;
   var WT = APP.WT;

   var dragPreviousXY = null;
   var lookAtCenter = null;
   var lookAtUpDir = null;
   var lookAtPitchRate = 0;
   var lookAtYawRate = 0;
   var cameraMatrix = null;

   this.setLookAtParams = function(matrix, center, up, pitchRate, yawRate) {
     cameraMatrix = matrix;
     lookAtCenter = center;
     lookAtUpDir = up;
     lookAtPitchRate = pitchRate;
     lookAtYawRate = yawRate;
   };

   this.mouseDragLookAt = function(o, event) {
     var c = WT.pageCoordinates(event);
     var dx=(c.x - dragPreviousXY.x);
     var dy=(c.y - dragPreviousXY.y);
     console.log('mouseDragLookAt: ' + dx + ',' + dy);
     console.log('mouseDragLookAt: ' + mat4.str(cameraMatrix));
     o.mouseDownCoordinates = c;
     var s=vec3.create();
     s[0]=cameraMatrix[0];
     s[1]=cameraMatrix[4];
     s[2]=cameraMatrix[8];
     var r=mat4.create();
     mat4.identity(r);
     mat4.translate(r, lookAtCenter);
     mat4.rotate(r, dy * lookAtPitchRate, s);
     mat4.rotate(r, dx * lookAtYawRate, lookAtUpDir);
     vec3.negate(lookAtCenter);
     mat4.translate(r, lookAtCenter);
     vec3.negate(lookAtCenter);
     mat4.multiply(cameraMatrix,r,cameraMatrix);
     console.log('mouseDragLookAt after: ' + mat4.str(cameraMatrix));
     // Repaint!
     console.log('mouseDragLookAt: repaint');
     //alert('mouseDragLookAt: repaint');
     ctx.paintGl();
     // store mouse coord for next action
     dragPreviousXY = WT.pageCoordinates(event);
   };

   // Mouse wheel = zoom in/out
   this.mouseWheelLookAt = function(o, event) {
     //alert('foo');
     var d = WT.wheelDelta(event);
     var s = Math.pow(1.2, d);
     console.log('mouseWheelLookAt: ' + d);
     mat4.translate(cameraMatrix, lookAtCenter);
     mat4.scale(cameraMatrix, [s, s, s]);
     vec3.negate(lookAtCenter);
     mat4.translate(cameraMatrix, lookAtCenter);
     vec3.negate(lookAtCenter);
     // Repaint!
     console.log('mouseWheelLookAt: repaint');
     ctx.paintGl();
   };

   this.mouseDown = function(o, event) {
     WT.capture(null);
     WT.capture(canvas);

     dragPreviousXY = WT.pageCoordinates(event);
   };

   this.mouseUp = function(o, event) {
     if (dragPreviousXY != null)
       dragPreviousXY = null;
   };

 });
