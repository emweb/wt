/**
 * Based on:
 *  Copyright Marc J. Schmidt. See the LICENSE file at the top-level
 *  directory of this distribution and at
 *  https://github.com/marcj/css-element-queries/blob/master/LICENSE.

Copyright (c) 2013 Marc J. Schmidt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "ResizeSensor",
function(WT, element) {
    var requestAnimationFrame = window.requestAnimationFrame ||
        window.mozRequestAnimationFrame ||
        window.webkitRequestAnimationFrame ||
        function (fn) {
            return window.setTimeout(fn, 20);
        };

    element.resizeSensor = document.createElement('div');
    element.resizeSensor.className = 'resize-sensor';
    var style = 'position: absolute; left: 0; top: 0; right: 0; bottom: 0; overflow: hidden; z-index: -1; visibility: hidden;';
    var styleChild = 'position: absolute; left: 0; top: 0; transition: 0s;';

    element.resizeSensor.style.cssText = style;
    element.resizeSensor.innerHTML =
        '<div class="resize-sensor-expand" style="' + style + '">' +
          '<div style="' + styleChild + '"></div>' +
        '</div>' +
        '<div class="resize-sensor-shrink" style="' + style + '">' +
          '<div style="' + styleChild + ' width: 200%; height: 200%"></div>' +
        '</div>';
    element.appendChild(element.resizeSensor);

    if (WT.css(element, 'position') == 'static') {
        element.style.position = 'relative';
    }

    var expand = element.resizeSensor.childNodes[0];
    var expandChild = expand.childNodes[0];
    var shrink = element.resizeSensor.childNodes[1];

    var initialHiddenCheck = true;
    var lastAnimationFrameForInvisibleCheck = 0;

    var reset = function() {
        // Check if element is hidden
        if (initialHiddenCheck) {
            var invisible = element.offsetWidth === 0 && element.offsetHeight === 0;
            if (invisible) {
                // Check in next frame
                if (!lastAnimationFrameForInvisibleCheck) {
                    lastAnimationFrameForInvisibleCheck = requestAnimationFrame(function(){
                        lastAnimationFrameForInvisibleCheck = 0;
                        reset();
                    });
                }
            } else {
                // Stop checking
                initialHiddenCheck = false;
            }
        }
        expandChild.style.width  = 100000 + 'px';
        expandChild.style.height = 100000 + 'px';

        expand.scrollLeft = 100000;
        expand.scrollTop = 100000;

        shrink.scrollLeft = 100000;
        shrink.scrollTop = 100000;
    };

    element.resizeSensor.trigger = function() {
      var w = lastWidth;
      var h = lastHeight;

      if (!WT.boxSizing(element)) {
        h -= WT.px(element, 'borderTopWidth');
        h -= WT.px(element, 'borderBottomWidth');
        h -= WT.px(element, 'paddingTop');
        h -= WT.px(element, 'paddingBottom');

        w -= WT.px(element, 'borderLeftWidth');
        w -= WT.px(element, 'borderRightWidth');
        w -= WT.px(element, 'paddingLeft');
        w -= WT.px(element, 'paddingRight');
      }

      if (element.wtResize)
        element.wtResize(element, w, h, false);
    };
    
    reset();
    var dirty = false;
    var lastWidth, lastHeight;

    var dirtyChecking = function() {
        if (dirty) {
	  element.resizeSensor.trigger();
	  dirty = false;
        }

        requestAnimationFrame(dirtyChecking);
    };

    requestAnimationFrame(dirtyChecking);
    var cachedWidth, cachedHeight; //useful to not query offsetWidth twice

    var onScroll = function() {
        if ((cachedWidth = element.offsetWidth) != lastWidth || 
	    (cachedHeight = element.offsetHeight) != lastHeight) {
            dirty = true;

            lastWidth = cachedWidth;
            lastHeight = cachedHeight;
        }
        reset();
    };

    var addEvent = function(el, name, cb) {
        if (el.attachEvent) {
            el.attachEvent('on' + name, cb);
        } else {
            el.addEventListener(name, cb);
        }
    };

    addEvent(expand, 'scroll', onScroll);
    addEvent(shrink, 'scroll', onScroll);

    lastAnimationFrameForInvisibleCheck = requestAnimationFrame(function(){
        lastAnimationFrameForInvisibleCheck = 0;
        reset();
    });
});
