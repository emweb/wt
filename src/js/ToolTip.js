/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(10, JavaScriptFunction, "toolTip",
 function(WT, id, text) {
   var $el = $("#" + id), el = $el.get(0);
   var eventsBound = el.toolTip;
   el.toolTip = text;

   if (!eventsBound) {
     (new (function() {
	var showTimer = null, coords = null, toolTipEl = null;
	/* const */ var MouseDistance = 10;
	/* const */ var Delay = 500;

	function showToolTip() {
	  toolTipEl = document.createElement('div');
	  toolTipEl.className = 'Wt-tooltip';
	  toolTipEl.innerHTML = text;
	  document.body.appendChild(toolTipEl);
	  var x = coords.x, y = coords.y;
	  WT.fitToWindow(toolTipEl, x + MouseDistance, y + MouseDistance,
			 x - MouseDistance, y - MouseDistance);
	}

	function resetTimer(e) {
	  clearTimeout(showTimer);
	  coords = WT.pageCoordinates(e);

	  if (!toolTipEl)
	    showTimer = setTimeout(function() { showToolTip(); }, Delay);
	}


	$el.mouseenter(resetTimer);
	$el.mousemove(resetTimer);

	$el.mouseleave(function() {
			 clearTimeout(showTimer);
			 if (toolTipEl) {
			   $(toolTipEl).remove();
			   toolTipEl = null;
			 }
		       });
      })());
   }
 }
);