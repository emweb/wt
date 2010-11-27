/**
 * @preserve Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * For terms of use, see LICENSE.
 */
_$_$if_DYNAMIC_JS_$_();
window.WT_DECLARE_WT_MEMBER = function(i, name, fn)
{
  var proto = name.indexOf('.prototype');
  if (proto == -1)
    _$_WT_CLASS_$_[name] = fn;
  else
    _$_WT_CLASS_$_[name.substr(0, proto)]
      .prototype[name.substr(proto + '.prototype.'.length)] = fn;
};

window.WT_DECLARE_APP_MEMBER = function(i, name, fn)
{
  var proto = name.indexOf('.prototype');
  if (proto == -1)
    _$_APP_CLASS_$_[name] = fn;
  else
    _$_APP_CLASS_$_[name.substr(0, proto)]
      .prototype[name.substr(proto + '.prototype.'.length)] = fn;
};

_$_$endif_$_();

window._$_WT_CLASS_$_ = new (function()
{
var WT = this;

// buttons currently down
this.buttons = 0;

this.mouseDown = function(e) {
  WT.buttons |= WT.button(e);
};

this.mouseUp = function(e) {
  WT.buttons ^= WT.button(e);
};

/**
 * @preserve Includes Array Remove - By John Resig (MIT Licensed)
 */
this.arrayRemove = function(a, from, to) {
  var rest = a.slice((to || from) + 1 || a.length);
  a.length = from < 0 ? a.length + from : from;
  return a.push.apply(a, rest);
};

this.addAll = function(a1, a2) {
  for (var i = 0, il = a2.length; i < il; ++i)
    a1.push(a2[i]);
}

var ie = (function(){
    var undef,
        v = 3,
        div = document.createElement('div'),
        all = div.getElementsByTagName('i');

    while (
        div.innerHTML = '<!--[if gt IE ' + (++v) + ']><i></i><![endif]-->',
        all[0]
    ) {}

    return v > 4 ? v : undef;
}());

this.isIE = ie !== undefined;
this.isIE6 = ie === 6;
this.isIElt9 = ie < 9;
this.isGecko = navigator.userAgent.toLowerCase().indexOf("gecko") != -1;
this.isIEMobile = navigator.userAgent.toLowerCase().indexOf("msie 4") != -1
  || navigator.userAgent.toLowerCase().indexOf("msie 5") != -1;
this.isOpera = typeof window.opera !== 'undefined';
this.updateDelay = this.isIE ? 10 : 51;

this.setHtml = function (el, html, add) {
  function myImportNode(e, deep) {
    var newNode, i, il;
    switch (e.nodeType) {
    case 1: // element
      if (e.namespaceURI == null)
	newNode = document.createElement(e.nodeName);
      else
	newNode = document.createElementNS(e.namespaceURI, e.nodeName);
      if (e.attributes && e.attributes.length > 0)
	for (i = 0, il = e.attributes.length; i < il;)
	  newNode.setAttribute(e.attributes[i].nodeName,
			       e.getAttribute(e.attributes[i++].nodeName));
      if (deep && e.childNodes.length > 0)
	for (i = 0, il = e.childNodes.length; i < il;) {
	  var c = myImportNode(e.childNodes[i++], deep);
	  if (c)
	    newNode.appendChild(c);
	}
      return newNode;
      break;
    case 3: // text
    case 4: // cdata
    case 5: // comment
      return document.createTextNode(e.nodeValue);
      break;
    }

    return null;
  }

  if (WT.isIE || (_$_INNER_HTML_$_ && !add)) {
    if (add)
      el.innerHTML += html;
    else
      el.innerHTML = html;
  } else {
    var d, b;
    d = new DOMParser();
    b = d.parseFromString('<div>'+html+'<\/div>','application/xhtml+xml');
    d = b.documentElement;
    if (d.nodeType != 1) // element
      d = d.nextSibling;

    if (!add)
      el.innerHTML = '';

    for (var i = 0, il = d.childNodes.length; i < il;)
      el.appendChild(myImportNode(d.childNodes[i++], true));
  }
};

this.hasTag = function(e, s) {
  return e.nodeType == 1 && e.tagName.toUpperCase() === s;
};

this.insertAt = function(p, c, i) {
  if (p.childNodes.length == 0)
    p.appendChild(c);
  else
    p.insertBefore(c, p.childNodes[i]);
};

this.remove = function(id)
{
  var e = WT.getElement(id);
  if (e)
    e.parentNode.removeChild(e);
};

this.contains = function(w1, w2) {
  var p = w2.parentNode;

  while (p != null && p.tagName.toLowerCase() != "body") {
    if (p == w1)
      return true;
    p = p.parentNode;
  }

  return false;
}

this.unstub = function(from, to, methodDisplay) {
  if (methodDisplay == 1) {
    if (from.style.display != 'none')
      to.style.display = from.style.display;
  } else {
    to.style.position = from.style.position;
    to.style.left = from.style.left;
    to.style.visibility = from.style.visibility;
  }

  if (from.style.height)
    to.style.height = from.style.height;
  if (from.style.width)
    to.style.width = from.style.width;
};

this.unwrap = function(e) {
  e = WT.getElement(e);
  if (e.parentNode.className.indexOf('Wt-wrap') == 0) {
    var wrapped = e;
    e = e.parentNode;
    if (e.className.length >= 8)
      wrapped.className = e.className.substring(8);
    if (WT.isIE)
      wrapped.style.setAttribute('cssText', e.getAttribute('style'));
    else
      wrapped.setAttribute('style', e.getAttribute('style'));
    e.parentNode.replaceChild(wrapped, e);
  } else {
    if (e.getAttribute('type') == 'submit') {
      e.setAttribute('type', 'button');
      e.removeAttribute('name');
    } if (WT.hasTag(e, 'INPUT') && e.getAttribute('type') == 'image') {
      // change <input> to <image>
      var img = document.createElement('img');
      if (img.mergeAttributes) {
	img.mergeAttributes(e, false);
	img.src = e.src;
      } else {
	if (e.attributes && e.attributes.length > 0) {
	  var i, il;
	  for (i = 0, il = e.attributes.length; i < il; i++) {
	    var n = e.attributes[i].nodeName;
	    if (n != 'type' && n != 'name')
	      img.setAttribute(n, e.getAttribute(n));
	  }
	}
      }
      e.parentNode.replaceChild(img, e);
    }
  }
};

var delegating = false;

this.CancelPropagate = 0x1;
this.CancelDefaultAction = 0x2;
this.CancelAll = 0x3;

this.cancelEvent = function(e, cancelType) {
  if (delegating)
    return;

  var ct = cancelType === undefined ? WT.CancelAll : cancelType;

  if (ct & WT.CancelDefaultAction)
    if (e.preventDefault)
      e.preventDefault();
    else
      e.returnValue=false;

  if (ct & WT.CancelPropagate) {
    if (e.stopPropagation)
      e.stopPropagation();
    else
      e.cancelBubble=true;

    try {
      if (document.activeElement && document.activeElement.blur)
	if (WT.hasTag(document.activeElement, "TEXTAREA"))
	  document.activeElement.blur();
    } catch(e) { }
  }
};

this.getElement = function(id) {
  var el = document.getElementById(id);
  if (!el)
    for (var i = 0; i < window.frames.length; ++i) {
      try {
        el = window.frames[i].document.getElementById(id);
        if (el)
          return el;
      } catch (e) {
      }
    }
  return el;
};

// Get coordinates of element relative to page origin.
this.widgetPageCoordinates = function(obj) {
  var objX = 0, objY = 0, op;

  // bug in safari, according to W3C, offsetParent for an area element should
  // be the map element, but safari returns null.
  if (WT.hasTag(obj, "AREA"))
    obj = obj.parentNode.nextSibling; // img after map

  while (obj) {
    objX += obj.offsetLeft;
    objY += obj.offsetTop;

    var f = css(obj, 'position');
    if (f == 'fixed') {
      objX += document.body.scrollLeft
	+ document.documentElement.scrollLeft;
      objY += document.body.scrollTop
	+ document.documentElement.scrollTop;
      break;
    }

    op = obj.offsetParent;

    if (op == null)
      obj = null;
    else {
      do {
	obj = obj.parentNode;
	if (WT.hasTag(obj, "DIV")) {
	  objX -= obj.scrollLeft;
	  objY -= obj.scrollTop;
	}
      } while (obj != null && obj != op);
    }
  }

  return { x: objX, y: objY };
};

// Get coordinates of (mouse) event relative to a element.
this.widgetCoordinates = function(obj, e) {
  var p = WT.pageCoordinates(e);
  var w = WT.widgetPageCoordinates(obj);
  return { x: p.x - w.x, y: p.y - w.y };
};

// Get coordinates of (mouse) event relative to page origin.
this.pageCoordinates = function(e) {
  if (!e) e = window.event;
  var posX = 0, posY = 0;
  if (e.pageX || e.pageY) {
    posX = e.pageX; posY = e.pageY;
  } else if (e.clientX || e.clientY) {
    posX = e.clientX + document.body.scrollLeft
      + document.documentElement.scrollLeft;
    posY = e.clientY + document.body.scrollTop
      + document.documentElement.scrollTop;
  }

  return { x: posX, y: posY };
};

this.windowCoordinates = function(e) {
  var p = WT.pageCoordinates(e);
  var cx = p.x - document.body.scrollLeft - document.documentElement.scrollLeft;
  var cy = p.y - document.body.scrollTop - document.documentElement.scrollTop;

  return { x: cx, y: cy };
};

this.wheelDelta = function(e) {
  var delta = 0;
  if (e.wheelDelta) { /* IE/Opera. */
    delta = e.wheelDelta > 0 ? 1 : -1;
    /* if (window.opera)
       delta = -delta; */
  } else if (e.detail) {
    delta = e.detail < 0 ? 1 : -1;
  }
  return delta;
};

this.scrollIntoView = function(id) {
  var obj = document.getElementById(id);
  if (obj && obj.scrollIntoView)
    obj.scrollIntoView(true);
};

this.getSelectionRange = function(elem) {
  if (document.selection) { // IE
    if (WT.hasTag(elem, 'TEXTAREA')) {
      var sel = document.selection.createRange();
      var sel2 = sel.duplicate();
      sel2.moveToElementText(elem);

      var pos = 0;
      if(sel.text.length > 1) {
	pos = pos - sel.text.length;
	if(pos < 0) {
	  pos = 0;
	}
      }

      var caretPos = -1 + pos;
      sel2.moveStart('character', pos);

      while (sel2.inRange(sel)) {
	sel2.moveStart('character');
	caretPos++;
      }

      var selStr = sel.text.replace(/\r/g, "");

      return {start: caretPos, end: selStr.length + caretPos};
    } else {
      var start, end;
      var val = $(elem).val();
      var range = document.selection.createRange().duplicate();
      range.moveEnd("character", val.length);
      start = (range.text == "" ? val.length : val.lastIndexOf(range.text));

      range = document.selection.createRange().duplicate();
      range.moveStart("character", -val.length);
      end = range.text.length;

      return {start: start, end: end};
    }
  } else if (elem.selectionStart || elem.selectionStart == 0) {
    return {start: elem.selectionStart, end: elem.selectionEnd};
  } else
    return {start: -1, end: -1};
};

this.setSelectionRange = function(elem, start, end) {
  /**
   * @preserve Includes jQuery Caret Range plugin
   * Copyright (c) 2009 Matt Zabriskie
   * Released under the MIT and GPL licenses.
   */
  var val = $(elem).val();

  if (typeof start != "number") start = -1;
  if (typeof end != "number") end = -1;
  if (start < 0) start = 0;
  if (end > val.length) end = val.length;
  if (end < start) end = start;
  if (start > end) start = end;

  elem.focus();

  if (typeof elem.selectionStart !== 'undefined') {
    elem.selectionStart = start;
    elem.selectionEnd = end;
  }
  else if (document.selection) {
    var range = elem.createTextRange();
    range.collapse(true);
    range.moveStart("character", start);
    range.moveEnd("character", end - start);
    range.select();
  }
};

this.isKeyPress = function(e) {
  if (!e) e = window.event;

  if (e.altKey || e.ctrlKey || e.metaKey)
    return false;

  var charCode = (typeof e.charCode !== 'undefined') ? e.charCode : 0;

  if (charCode > 0 || WT.isIE)
    return true;
  else {
    if (WT.isOpera)
      return (e.keyCode == 13 || e.keyCode == 27
	      || (e.keyCode >= 32 && e.keyCode < 125));
    else
      return (e.keyCode == 13 || e.keyCode == 27 || e.keyCode == 32
	      || (e.keyCode > 46 && e.keyCode < 112));

  }
};

function css(c, s) {
  if (c.style[s])
    return c.style[s];
  else if (document.defaultView && document.defaultView.getComputedStyle)
    return document.defaultView.getComputedStyle(c, null)[s];
  else if (c.currentStyle)
    return c.currentStyle[s];
  else
    return null;
}

function parseCss(value, regex, defaultvalue) {
  if (value == 'auto' || value == null)
    return defaultvalue;
  var m = regex.exec(value),
      v = m && m.length == 2 ? m[1] : null;
  return v ? parseFloat(v) : defaultvalue;
}

function parsePx(v) {
  return parseCss(v, /^\s*(-?\d+(?:\.\d+)?)\s*px\s*$/i, 0);
}

function parsePct(v, defaultValue) {
  return parseCss(v, /^\s*(-?\d+(?:\.\d+)?)\s*\%\s*$/i, defaultValue);
}

// Get an element metric in pixels
this.px = function(c, s) {
  return parsePx(css(c, s));
};

// Get a widget style in pixels, when set directly
this.pxself = function(c, s) {
  return parsePx(c.style[s]);
};

this.pctself = function(c, s) {
  return parsePct(c.style[s], 0);
};

// Return if an element (or one of its ancestors) is hidden
this.isHidden = function(w) {
  if (w.style.display == 'none')
    return true;
  else {
    w = w.parentNode;
    if (w != null && w.tagName.toLowerCase() != "body")
      return WT.isHidden(w);
    else
      return false;
  }
};

this.IEwidth = function(c, min, max) {
  if (c.parentNode) {
    var r = c.parentNode.clientWidth
    - WT.px(c, 'marginLeft')
    - WT.px(c, 'marginRight')
    - WT.px(c, 'borderLeftWidth')
    - WT.px(c, 'borderRightWidth')
    - WT.px(c.parentNode, 'paddingLeft')
    - WT.px(c.parentNode, 'paddingRight');

    min = parsePct(min, 0);
    max = parsePct(max, 100000);

    if (r < min)
      return min-1;
    else if (r > max)
      return max+1;
    else if (c.style["styleFloat"] != "")
      return min-1;
    else
      return "auto";
  } else
    return "auto";
};

this.hide = function(o) { WT.getElement(o).style.display = 'none'; };
this.inline = function(o) { WT.getElement(o).style.display = 'inline'; };
this.block = function(o) { WT.getElement(o).style.display = 'block'; };
this.show = function(o) { WT.getElement(o).style.display = ''; };

var captureElement = null;
this.firedTarget = null;

this.target = function(event) {
  return WT.firedTarget || event.target || event.srcElement;
};

function delegateCapture(e) {
  if (captureElement == null)
    return null;

  if (!e) e = window.event;

  if (e) {
    var t = WT.target(e), p = t;

    while (p && p != captureElement)
      p = p.parentNode;

    /*
     * We don't need to capture	the event when the event falls inside the
     * capture element. In this way, more specific widgets inside may still
     * handle (and cancel) the event if they want.
     *
     * On IE this means that we need to delegate the event to the event
     * target; on other browsers we can just rely on event bubbling.
     */
    if (p == captureElement)
      return WT.isIElt9 ? t : null;
    else
      return captureElement;
  } else
    return captureElement;
}

function mouseMove(e) {
  var d = delegateCapture(e);

  if (d && !delegating) {
    if (!e) e = window.event;
    delegating = true;
    if (WT.isIElt9) {
      WT.firedTarget = e.srcElement || d;
      d.fireEvent('onmousemove', e);
      WT.firedTarget = null;
    } else
      WT.condCall(d, 'onmousemove', e);
    delegating = false;
    return false;
  } else
    return true;
}

function mouseUp(e) {
  var d = delegateCapture(e);
  WT.capture(null);

  if (d) {
    if (!e) e = window.event;

    if (WT.isIElt9) {
      WT.firedTarget = e.srcElement || d;
      d.fireEvent('onmouseup', e);
      WT.firedTarget = null;
    } else
      WT.condCall(d, 'onmouseup', e);

    WT.cancelEvent(e, WT.CancelPropagate);

    return false;
  } else
    return true;
}

var captureInitialized = false;

function initCapture() {
  if (captureInitialized)
    return;

  captureInitialized = true;

  if (document.body.addEventListener) {
    var db = document.body;
    db.addEventListener('mousemove', mouseMove, true);
    db.addEventListener('mouseup', mouseUp, true);

    if (WT.isGecko) {
      window.addEventListener('mouseout', function(e) {
				if (!e.relatedTarget
				    && WT.hasTag(e.target, "HTML"))
				  mouseUp(e);
			      }, true);
    }
  } else {
    var db = document.body;
    db.attachEvent('onmousemove', mouseMove);
    db.attachEvent('onmouseup', mouseUp);
  }
}

this.capture = function(obj) {
  initCapture();

  if (captureElement && obj)
    return;

  captureElement = obj;

  var db = document.body;
  if (db.setCapture)
    if (obj != null)
      db.setCapture();
    else
      db.releaseCapture();

  if (obj != null) {
    $(db).addClass('unselectable');
    db.setAttribute('unselectable', 'on');
    db.onselectstart = 'return false;';
  } else {
    $(db).removeClass('unselectable');
    db.setAttribute('unselectable', 'off');
    db.onselectstart = '';
  }
};

this.checkReleaseCapture = function(obj, e) {
  if (captureElement && (obj == captureElement) && e.type == "mouseup")
    this.capture(null);
};

this.getElementsByClassName = function(className, parentElement) {
  if (document.getElementsByClassName) {
    return parentElement.getElementsByClassName(className);
  } else {
    var cc = parentElement.getElementsByTagName('*');
    var els = [], c;
    for (var i = 0, length = cc.length; i < length; i++) {
      c = cc[i];
      if (c.className.indexOf(className) != -1)
	els.push(c);
    }
    return els;
  }
};

this.addCss = function(selector, style) {
  var s = document.styleSheets[0];
  s.insertRule(selector + ' { ' + style + ' }', s.cssRules.length);
};

this.addCssText = function(cssText) {
  var s = document.getElementById('Wt-inline-css');

  if (!s) {
    s = document.createElement('style');
    document.getElementsByTagName('head')[0].appendChild(s);
  }

  if (!s.styleSheet) { // Konqueror
    var t = document.createTextNode(cssText);
    s.appendChild(t);
  } else {
    var ss = document.createElement('style');
    if (s)
      s.parentNode.insertBefore(ss, s);
    else {
      ss.id = 'Wt-inline-css';
      document.getElementsByTagName('head')[0].appendChild(ss);
    }
    ss.styleSheet.cssText = cssText;
  }
};

// from: http://www.hunlock.com/blogs/Totally_Pwn_CSS_with_Javascript
this.getCssRule = function(selector, deleteFlag) {
  selector=selector.toLowerCase();

  if (document.styleSheets) {
    for (var i=0; i<document.styleSheets.length; i++) {
      var styleSheet = document.styleSheets[i];
      var ii = 0;
      var cssRule;
      do {
	cssRule = null;
	if (styleSheet.cssRules)
	  cssRule = styleSheet.cssRules[ii];
	else if (styleSheet.rules)
	  cssRule = styleSheet.rules[ii];
	if (cssRule && cssRule.selectorText) {
	  if (cssRule.selectorText.toLowerCase()==selector) {
	    if (deleteFlag == 'delete') {
	      if (styleSheet.cssRules)
		styleSheet.deleteRule(ii);
	      else
		styleSheet.removeRule(ii);
	      return true;
	    } else
	      return cssRule;
	  }
	}
	++ii;
      } while (cssRule);
    }
  }

  return false;
};

this.removeCssRule = function(selector) {
  return WT.getCssRule(selector, 'delete');
};

this.addStyleSheet = function(uri, media) {
  if (document.createStyleSheet) {
    setTimeout(function() { document.createStyleSheet(uri); }, 15);
  } else {
    var s = document.createElement('link');
    s.setAttribute('type', 'text/css');
    s.setAttribute('href', uri);
    s.setAttribute('type','text/css');
    s.setAttribute('rel','stylesheet');
    if (media != '' && media != 'all')
      s.setAttribute('media', media);
    var h = document.getElementsByTagName('head')[0];
    h.appendChild(s);
  }
};

this.windowSize = function() {
  var x, y;

  if (typeof (window.innerWidth) === 'number') {
    x = window.innerWidth;
    y = window.innerHeight;
  } else {
    x = document.documentElement.clientWidth;
    y = document.documentElement.clientHeight;
  }

  return { x: x, y: y};
};

/*
 * position right to (x) or left from (rightx) and
 * bottom of (y) or top from (bottomy)
 */
this.fitToWindow = function(e, x, y, rightx, bottomy) {
  var ws = WT.windowSize();

  var wx = document.body.scrollLeft + document.documentElement.scrollLeft;
  var wy = document.body.scrollTop + document.documentElement.scrollTop;

  var ow = WT.widgetPageCoordinates(e.offsetParent);

  var hsides = [ 'left', 'right' ],
      vsides = ['top', 'bottom' ],
      ew = WT.px(e, 'maxWidth') || e.offsetWidth,
      eh = WT.px(e, 'maxHeight') || e.offsetHeight,
      hside, vside;

  if (x + ew > wx + ws.x) { // too far right, chose other side
    rightx -= ow.x;
    x = e.offsetParent.offsetWidth - (rightx + WT.px(e, 'marginRight'));
    hside = 1;
  } else {
    x -= ow.x;
    x = x - WT.px(e, 'marginLeft');
    hside = 0;
  }

  if (y + eh > wy + ws.y) { // too far below, chose other side
    if (bottomy > wy + ws.y)
      bottomy = wy + ws.y;
    bottomy -= ow.y;
    y = e.offsetParent.offsetHeight - (bottomy + WT.px(e, 'marginBottom'));
    vside = 1;
  } else {
    y -= ow.y;
    y = y - WT.px(e, 'marginTop');
    vside = 0;
  }

  /*
  if (x < wx)
    x = wx + ws.x - e.offsetWidth - 3;
  if (y < wy)
    y = wy + ws.y - e.offsetHeight - 3;
  */

  e.style[hsides[hside]] = x + 'px';
  e.style[hsides[1 - hside]] = '';
  e.style[vsides[vside]] = y + 'px';
  e.style[vsides[1 - vside]] = '';
};

this.positionXY = function(id, x, y) {
  var w = WT.getElement(id);

  if (!WT.isHidden(w))
    WT.fitToWindow(w, x, y, x, y);
};

this.Horizontal = 0x1;
this.Vertical = 0x2;

this.positionAtWidget = function(id, atId, orientation, parentInRoot) {
  var w = WT.getElement(id),
    atw = WT.getElement(atId),
    xy = WT.widgetPageCoordinates(atw),
    x, y, rightx, bottomy;

  if (parentInRoot) {
    w.parentNode.removeChild(w);
    $('.Wt-domRoot').get(0).appendChild(w);
  }

  w.style.position='absolute';
  w.style.display='block';

  if (orientation == WT.Horizontal) {
    x = xy.x + atw.offsetWidth;
    y = xy.y;
    rightx = xy.x,
    bottomy = xy.y + atw.offsetHeight;
  } else {
    x = xy.x;
    y = xy.y + atw.offsetHeight;
    rightx = xy.x + atw.offsetWidth;
    bottomy = xy.y;
  }

  WT.fitToWindow(w, x, y, rightx, bottomy);

  w.style.visibility='';
};

this.hasFocus = function(el) {
  return el == document.activeElement;
};

this.history = (function()
{
  /**
   * @preserve
   * Includes Yahoo History Frameowork
   * Copyright (c) 2008, Yahoo! Inc. All rights reserved.
   * Code licensed under the BSD License:
   * http://developer.yahoo.net/yui/license.txt
   * version: 2.5.2
   */
  var _UAwebkit = false;
  var _UAie = WT.isIElt9;
  var _UAopera = false;
  var _onLoadFn = null;
  var _histFrame = null;
  var _stateField = null;
  var _initialized = false;
  var _interval = null;
  var _fqstates = [];
  var _initialState, _currentState;
  var _onStateChange = function(){};
  function _getHash() {
    var i, href;
    href = location.href;
    i = href.indexOf("#");
    return i >= 0 ? href.substr(i + 1) : null;
  }
  function _storeStates() {
    _stateField.value = _initialState + "|" + _currentState;
    if (_UAwebkit) {
      _stateField.value += "|" + _fqstates.join(",");
    }
  }
  function _handleFQStateChange(fqstate) {
    var currentState;
    if (!fqstate) {
      _currentState = _initialState;
      _onStateChange(unescape(_currentState));
      return;
    }
    currentState = fqstate;
    if (!currentState || _currentState !== currentState) {
      _currentState = currentState || _initialState;
      _onStateChange(unescape(_currentState));
    }
  }
  function _updateIFrame (fqstate) {
    var html, doc;
    html = '<html><body><div id="state">' + fqstate
      + '</div></body></html>';
    try {
      doc = _histFrame.contentWindow.document;
      doc.open();
      doc.write(html);
      doc.close();
      return true;
    } catch (e) {
      return false;
    }
  }
  function _checkIframeLoaded() {
    var doc, elem, fqstate, hash;
    if (!_histFrame.contentWindow || !_histFrame.contentWindow.document) {
      setTimeout(_checkIframeLoaded, 10);
      return;
    }
    doc = _histFrame.contentWindow.document;
    elem = doc.getElementById("state");
    fqstate = elem ? elem.innerText : null;
    hash = _getHash();
    setInterval(function () {
	var newfqstate, newHash;
	doc = _histFrame.contentWindow.document;
	elem = doc.getElementById("state");
	newfqstate = elem ? elem.innerText : null;
	newHash = _getHash();
	if (newfqstate !== fqstate) {
	  fqstate = newfqstate;
	  _handleFQStateChange(fqstate);
	  if (!fqstate) {
	    newHash = _initialState;
	  } else {
	    newHash = fqstate;
	  }
	  location.hash = newHash;
	  hash = newHash;
	  _storeStates();
	} else if (newHash !== hash) {
	  hash = newHash;
	  _updateIFrame(newHash);
	}
      }, 50);
    _initialized = true;
    if (_onLoadFn != null)
      _onLoadFn();
  }

  function _initTimeout() {
    if (_UAie)
      return;

    var hash = _getHash(), counter = history.length;

    if (_interval)
      clearInterval(_interval);
    _interval = setInterval(function () {
	var state, newHash, newCounter;
	newHash = _getHash();
	newCounter = history.length;
	if (newHash !== hash) {
	  hash = newHash;
	  counter = newCounter;
	  _handleFQStateChange(hash);
	  _storeStates();
	} else if (newCounter !== counter && _UAwebkit) {
	  hash = newHash;
	  counter = newCounter;
	  state = _fqstates[counter - 1];
	  _handleFQStateChange(state);
	  _storeStates();
	}
      }, 50);
  }

  function _initialize() {
    var parts;
    parts = _stateField.value.split("|");
    if (parts.length > 1) {
      _initialState = parts[0];
      _currentState = parts[1];
    }
    if (parts.length > 2) {
      _fqstates = parts[2].split(",");
    }
    if (_UAie) {
      _checkIframeLoaded();
    } else {
      _initTimeout();
      _initialized = true;
      if (_onLoadFn != null)
	_onLoadFn();
    }
  }
  return {
  onReady: function (fn) {
    if (_initialized) {
      setTimeout(function () { fn(); }, 0);
    } else {
      _onLoadFn = fn;
    }
  },
  _initialize: function() {
    if (_stateField != null)
      _initialize();
  },
  _initTimeout: function() {
      _initTimeout();
  },
  register: function (initialState, onStateChange) {
    if (_initialized) {
      return;
    }
    _initialState = escape(initialState);
    _currentState = _initialState;
    _onStateChange = onStateChange;
  },
  initialize: function (stateField, histFrame) {
    if (_initialized) {
      return;
    }
    var vendor = navigator.vendor || "";
    if (vendor === "KDE") {
    } else if (typeof window.opera !== "undefined")
      _UAopera = true;
    else if (!_UAie && vendor.indexOf("Apple Computer, Inc.") > -1)
      _UAwebkit = true;

    /*
    if (_UAopera && typeof history.navigationMode !== "undefined")
      history.navigationMode = "compatible";
    */

    if (typeof stateField === "string")
      stateField = document.getElementById(stateField);
    if (!stateField ||
	stateField.tagName.toUpperCase() !== "TEXTAREA" &&
	(stateField.tagName.toUpperCase() !== "INPUT" ||
	 stateField.type !== "hidden" &&
	 stateField.type !== "text")) {
      return;
    }
    _stateField = stateField;
    if (_UAie) {
      if (typeof histFrame === "string") {
	histFrame = document.getElementById(histFrame);
      }
      if (!histFrame || histFrame.tagName.toUpperCase() !== "IFRAME") {
	return;
      }
      _histFrame = histFrame;
    }
  },
  navigate: function (state) {
    if (!_initialized) {
      return;
    }
    fqstate = state;
    if (_UAie) {
      _updateIFrame(fqstate);
      return;
    } else {
      location.hash = fqstate;
      if (_UAwebkit) {
	_fqstates[history.length] = fqstate;
	_storeStates();
      }
      return;
    }
  },
  getCurrentState: function () {
    if (!_initialized) {
      return "";
    }
    return _currentState;
  }
  };
})();

})();

window._$_APP_CLASS_$_ = new (function() {

var self = this;
var WT = _$_WT_CLASS_$_;

var downX = 0;
var downY = 0;

function saveDownPos(e) {
  var coords = WT.pageCoordinates(e);
  downX = coords.x;
  downY = coords.y;
};

var currentHash = null;

function onHashChange() {
  var newLocation = WT.history.getCurrentState();
  if (currentHash == newLocation) {
    return;
  } else {
    currentHash = newLocation;
    setTimeout(function() { update(null, 'hash', null, true); }, 1);
  }
};

function setHash(newLocation) {
  if (currentHash != newLocation) {
    currentHash = newLocation;
    WT.history.navigate(escape(newLocation));
  }
};

var dragState = {
  object: null,
  sourceId: null,
  mimeType: null,
  dropOffsetX: null,
  dragOffsetY: null,
  dropTarget: null,
  objectPrevStyle: null,
  xy: null
};

function initDragDrop() {
  window.onresize=function() { doJavaScript(); };

  document.body.ondragstart=function() {
    return false;
  };
}

function dragStart(obj, e) {
  // drag element attributes:
  //   dwid = dragWidgetId
  //   dsid = dragSourceId
  //   dmt = dragMimeType
  // drop element attributes:
  //   amts = acceptMimeTypes
  //   ds = dropSignal

  var ds = dragState;

  ds.object = WT.getElement(obj.getAttribute("dwid"));
  if (ds.object == null)
    return true;

  ds.sourceId = obj.getAttribute("dsid");
  ds.objectPrevStyle = {
    position: ds.object.style["position"],
    display: ds.object.style["display"],
    left: ds.object.style["left"],
    top: ds.object.style["top"],
    className: ds.object.className
  };

  ds.object.parentNode.removeChild(ds.object);
  ds.object.style["position"] = 'absolute';
  ds.object.className = '';
  document.body.appendChild(ds.object);

  WT.capture(null);
  WT.capture(ds.object);
  ds.object.onmousemove = dragDrag;
  ds.object.onmouseup = dragEnd;

  ds.offsetX = -4;
  ds.offsetY = -4;
  ds.dropTarget = null;
  ds.mimeType = obj.getAttribute("dmt");
  ds.xy = WT.pageCoordinates(e);

  WT.cancelEvent(e);

  return false;
};

function dragDrag(e) {
  if (dragState.object != null) {
    var ds = dragState;
    var xy = WT.pageCoordinates(e);

    if (ds.object.style["display"] != '' && ds.xy.x != xy.x && ds.xy.y != xy.y)
      ds.object.style["display"] = '';

    ds.object.style["left"] = (xy.x - ds.offsetX) + 'px';
    ds.object.style["top"] = (xy.y - ds.offsetY) + 'px';

    var prevDropTarget = ds.dropTarget;
    var t = WT.target(e);
    var mimeType = "{" + ds.mimeType + ":";
    var amts = null;

    ds.dropTarget = null;
    while (t) {
      amts = t.getAttribute("amts");
      if ((amts != null) && (amts.indexOf(mimeType) != -1)) {
	ds.dropTarget = t;
	break;
      }
      t = t.parentNode;
      if (!t.tagName || WT.hasTag(t, "HTML"))
	break;
    }

    if (ds.dropTarget != prevDropTarget) {
      if (ds.dropTarget) {
        var s = amts.indexOf(mimeType) + mimeType.length;
	var se = amts.indexOf("}", s);
	var style = amts.substring(s, se);
	if (style.length != 0) {
          ds.dropTarget.setAttribute("dos", ds.dropTarget.className);
	  ds.dropTarget.className = ds.dropTarget.className + " " + style;
        }
      } else {
        ds.object.styleClass = '';
      }

      if (prevDropTarget != null) {
	if (prevDropTarget.handleDragDrop)
	  prevDropTarget.handleDragDrop('end', ds.object, e, '', mimeType);
	var dos = prevDropTarget.getAttribute("dos");
        if (dos != null)
	  prevDropTarget.className = dos;
      }
    }

    if (ds.dropTarget) {
      if (ds.dropTarget.handleDragDrop)
	ds.dropTarget.handleDragDrop('drag', ds.object, e, '', mimeType);
      else
	ds.object.className = 'Wt-valid-drop';
    }

    return false;
  }

  return true;
};

function dragEnd(e) {
  WT.capture(null);

  var ds = dragState;

  if (ds.object) {
    if (ds.dropTarget) {
      var dos = ds.dropTarget.getAttribute("dos");
      if (dos != null)
	  ds.dropTarget.className = dos;
      if (ds.dropTarget.handleDragDrop)
	ds.dropTarget.handleDragDrop('drop', ds.object, e,
				     ds.sourceId, ds.mimeType);
      else
	emit(ds.dropTarget, {name: "_drop", eventObject: ds.dropTarget,
	      event: e}, ds.sourceId, ds.mimeType);
    } else {
      // could not be dropped, animate it floating back ?
    }

    ds.object.style["position"] = ds.objectPrevStyle.position;
    ds.object.style["display"] = ds.objectPrevStyle.display;
    ds.object.style["left"] = ds.objectPrevStyle.left;
    ds.object.style["top"] = ds.objectPrevStyle.top;
    ds.object.className = ds.objectPrevStyle.className;

    ds.object = null;
  }
};

function encodeTouches(s, touches, widgetCoords) {
  var i, il, result;

  result = s + "=";
  for (i = 0, il = touches.length; i < il; ++i) {
    var t = touches[i];
    result += [ t.identifier,
		t.clientX, t.clientY,
		t.pageX, t.pageY,
		t.screenX, t.screenY,
		t.pageX - widgetCoords.x, t.pageY - widgetCoords.y ].join(';');
  }

  return result;
}

var formObjects = _$_FORM_OBJECTS_$_;

function encodeEvent(event, i) {
  var se, result, e;

  e = event.event;
  se = i > 0 ? '&e' + i : '&';
  result = se + 'signal=' + event.signal;

  if (event.id) {
    result += se + 'id=' + event.id
        + se + 'name=' + encodeURIComponent(event.name)
        + se + 'an=' + event.args.length;

    for (var j = 0; j < event.args.length; ++j)
      result += se + 'a' + j + '=' + encodeURIComponent(event.args[j]);
  }

  for (var x = 0; x < formObjects.length; ++x) {
    var el = WT.getElement(formObjects[x]), v = null, j, jl;

    if (el == null)
      continue;

    if (el.type == 'select-multiple') {
      for (j = 0, jl = el.options.length; j < jl; j++)
	if (el.options[j].selected) {
	  result += se + formObjects[x] + '='
	    + encodeURIComponent(el.options[j].value);
	}
    } else if (el.type == 'checkbox' || el.type == 'radio') {
      if (el.indeterminate || el.style.opacity == '0.5')
	v = 'i';
      else if (el.checked)
	v = el.value;
    } else if (WT.hasTag(el, "VIDEO") || WT.hasTag(el, "AUDIO")) {
      v = '' + el.volume + ';' + el.currentTime + ';' + el.duration + ';' + (el.paused?'1':'0') + ';' + (el.ended?'1':'0');
    } else if (el.type != 'file') {
      if ($(el).hasClass('Wt-edit-emptyText'))
	v = '';
      else
	v = '' + el.value;

      if (WT.hasFocus(el)) {
	var range = WT.getSelectionRange(el);
	result += se + "selstart=" + range.start
	  + se + "selend=" + range.end;
      }
    }

    if (v != null)
      result += se + formObjects[x] + '=' + encodeURIComponent(v);
  }


  try {
    if (document.activeElement)
      result += se + "focus=" + document.activeElement.id;
  } catch (e) { }

  if (currentHash != null)
    result += se + '_=' + encodeURIComponent(unescape(currentHash));

  if (!e) {
    event.data = result;
    return event;
  }

  var t = WT.target(e);
  while (!t.id && t.parentNode)
    t = t.parentNode;
  if (t.id)
    result += se + 'tid=' + t.id;

  try {
    result += se + 'type=' + e.type;
  } catch (e) {
  }

  if (e.clientX || e.clientY)
    result += se + 'clientX=' + e.clientX + se + 'clientY=' + e.clientY;

  var pageCoords = WT.pageCoordinates(e);
  var posX = pageCoords.x;
  var posY = pageCoords.y;

  if (posX || posY) {
    result += se + 'documentX=' + posX + se + 'documentY=' + posY;
    result += se + 'dragdX=' + (posX - downX) + se + 'dragdY=' + (posY - downY);

    var delta = WT.wheelDelta(e);
    result += se + 'wheel=' + delta;
  }

  if (e.screenX || e.screenY)
    result += se + 'screenX=' + e.screenX + se + 'screenY=' + e.screenY;

  var widgetCoords = { x: 0, y: 0 };
  if (event.object && event.object.nodeType != 9) {
    widgetCoords = WT.widgetPageCoordinates(event.object);
    var objX = widgetCoords.x;
    var objY = widgetCoords.y;

    result += se + 'scrollX=' + event.object.scrollLeft
      + se + 'scrollY=' + event.object.scrollTop
      + se + 'width=' + event.object.clientWidth
      + se + 'height=' + event.object.clientHeight
      + se + 'widgetX=' + (posX - objX) + se + 'widgetY=' + (posY - objY);
  }

  var button = WT.button(e);
  if (!button) {
    if (WT.buttons & 1)
      button = 1;
    else if (WT.buttons & 2)
      button = 2;
    else if (WT.buttons & 4)
      button = 4;
  }
  result += se + 'button=' + button;

  if (typeof e.keyCode !== 'undefined')
    result += se + 'keyCode=' + e.keyCode;

  if (typeof e.charCode !== 'undefined')
    result += se + 'charCode=' + e.charCode;

  if (e.altKey)
    result += se + 'altKey=1';
  if (e.ctrlKey)
    result += se + 'ctrlKey=1';
  if (e.metaKey)
    result += se + 'metaKey=1';
  if (e.shiftKey)
    result += se + 'shiftKey=1';

  if (typeof e.touches !== 'undefined')
    result += encodeTouches(se + "touches", e.touches, widgetCoords);
  if (typeof e.targetTouches !== 'undefined')
    result += encodeTouches(se + "ttouches", e.targetTouches, widgetCoords);
  if (typeof e.changedTouches !== 'undefined')
    result += encodeTouches(se + "ctouches", e.changedTouches, widgetCoords);

  if (e.scale)
    result += se + "scale=" + e.scale;
  if (e.rotation)
    result += se + "rotation=" + e.rotation;

  event.data = result;
  return event;
};

// returns the button associated with the event (0 if none)
WT.button = function(e)
{
  if (e.which) {
    if (e.which == 3)
      return 4;
    else if (e.which == 2)
      return 2;
    else
      return 1;
  } else if (WT.isIE && typeof e.button != 'undefined') {
    if (e.button == 2)
      return 4;
    else if (e.button == 4)
      return 2;
    else
      return 1;
  } else if (typeof e.button != 'undefined') {
    if (e.button == 2)
      return 4;
    else if (e.button == 1)
      return 2;
    else
      return 1;
  } else {
    return 0;
  }
};

var sentEvents = [], pendingEvents = [];

function encodePendingEvents() {
  var result = '', feedback = false;

  for (var i = 0; i < pendingEvents.length; ++i) {
    feedback = feedback || pendingEvents[i].feedback;
    result += pendingEvents[i].data;
  }

  sentEvents = pendingEvents;
  pendingEvents = [];

  return { feedback: feedback, result: result };
}

var url = _$_RELATIVE_URL_$_,
  quited = false,
  norestart = false,
  loaded = false,
  responsePending = null,
  pollTimer = null,
  keepAliveTimer = null,
  commErrors = 0,
  serverPush = false,
  updateTimeout = null;

function quit() {
  quited = true;
  if (keepAliveTimer) {
    clearTimeout(keepAliveTimer);
    keepAliveTimer = null;
  }
  var tr = $('#Wt-timers');
  if (tr.size() > 0)
    WT.setHtml(tr.get(0), '', false);
};

function doKeepAlive() {
  WT.history._initTimeout();
  if (commErrors == 0)
    update(null, 'none', null, false);
  keepAliveTimer = setTimeout(doKeepAlive, _$_KEEP_ALIVE_$_000);
};

function debug(s) {
  document.body.innerHTML += s;
};

function setTitle(title) {
  if (WT.isIEMobile) return;
  document.title = title;
};

function load() {
  if (!document.activeElement) {
    function trackActiveElement(evt) {
      if (evt && evt.target) {
	document.activeElement = evt.target == document ? null : evt.target;
      }
    }

    function trackActiveElementLost(evt) {
      document.activeElement = null;
    }

    document.addEventListener("focus", trackActiveElement, true);
    document.addEventListener("blur", trackActiveElementLost, true);
  }

  WT.history._initialize();
  initDragDrop();
  if (!loaded) {
    loaded = true;
    _$_ONLOAD_$_();
    if (!quited)
      keepAliveTimer = setTimeout(doKeepAlive, _$_KEEP_ALIVE_$_000);
  }
};

var currentHideLoadingIndicator = null;

function cancelFeedback(t) {
  clearTimeout(t);
  document.body.style.cursor = 'auto';

  if (currentHideLoadingIndicator != null) {
    try {
      currentHideLoadingIndicator();
    } catch (e) {
    }
    currentHideLoadingIndicator = null;
  }
};

function waitFeedback() {
  document.body.style.cursor = 'wait';
  currentHideLoadingIndicator = hideLoadingIndicator;
  showLoadingIndicator();
};

/** @const */ var WebSocketsUnknown = 0;
/** @const */ var WebSocketsWorking = 1;
/** @const */ var WebSocketsUnavailable = 2;

var websocket = {
  state: WebSocketsUnknown,
  socket: null
};

function setServerPush(how) {
  serverPush = how;
}

function doJavaScript(js) {
  if (js) {
    js = "(function() {" + js + "})();";
    if (window.execScript)
      window.execScript(js);
    else
      window.eval(js);
  }

  self._p_.autoJavaScript();
}

function handleResponse(status, msg, timer) {
  if (quited)
    return;

  if (status == 0) {
_$_$ifnot_DEBUG_$_();
    try {
_$_$endif_$_();
      doJavaScript(msg);
_$_$ifnot_DEBUG_$_();
    } catch (e) {
      alert("Wt internal error: " + e + ", code: " +  e.code
	    + ", description: " + e.description /* + ":" + msg */);
    }
_$_$endif_$_();

    if (timer)
      cancelFeedback(timer);
  } else {
    pendingEvents = sentEvents.concat(pendingEvents);
  }

  sentEvents = [];

  if (pollTimer) {
    clearTimeout(pollTimer);
    pollTimer = null;
  }

  responsePending = null;

  if (status > 0)
    ++commErrors;
  else
    commErrors = 0;

  if (quited)
    return;

  if (serverPush || pendingEvents.length > 0) {
    if (status == 1) {
      var ms = Math.min(120000, Math.exp(commErrors) * 500);
      updateTimeout = setTimeout(function() { sendUpdate(); }, ms);
    } else
      sendUpdate();
  }
};

var randomSeed = new Date().getTime();

function doPollTimeout() {
  responsePending.abort();
  responsePending = null;
  pollTimer = null;

  if (!quited)
    sendUpdate();
}

function update(el, signalName, e, feedback) {
  WT.checkReleaseCapture(el, e);

  _$_$if_STRICTLY_SERIALIZED_EVENTS_$_();
  if (!responsePending) {
  _$_$endif_$_();

  var pendingEvent = new Object(), i = pendingEvents.length;
  pendingEvent.object = el;
  pendingEvent.signal = signalName;
  pendingEvent.event = e;
  pendingEvent.feedback = feedback;

  pendingEvents[i] = encodeEvent(pendingEvent, i);

  scheduleUpdate();

  doJavaScript();

  _$_$if_STRICTLY_SERIALIZED_EVENTS_$_();
  }
  _$_$endif_$_();
}

var updateTimeoutStart;

function scheduleUpdate() {
  _$_$if_WEB_SOCKETS_$_();
  if (websocket.state != WebSocketsUnavailable) {
    if (typeof window.WebSocket === 'undefined')
      websocket.state = WebSocketsUnavailable;
    else {
      var ws = websocket.socket;

      if ((ws == null || ws.readyState > 1)) {
	if (ws != null && websocket.state == WebSocketsUnknown)
	  websocket.state = WebSocketsUnavailable;
	else {
	  var query = url.substr(url.indexOf('?'));
	  var wsurl = "ws" + location.protocol.substr(4)
	    + "//" + location.hostname + ":"
	    + location.port + location.pathname + query;
	  websocket.socket = ws = new WebSocket(wsurl);

	  ws.onmessage = function(event) {
	    websocket.state = WebSocketsWorking;
	    handleResponse(0, event.data, null);
	  };
	}
      }

      if (ws.readyState == 1) {
	sendUpdate();
	return;
      }
    }
  }
  _$_$endif_$_();

  if (responsePending != null && pollTimer != null) {
    clearTimeout(pollTimer);
    responsePending.abort();
    responsePending = null;
  }

  if (responsePending == null) {
    if (updateTimeout == null) {
      updateTimeout = setTimeout(function() { sendUpdate(); }, WT.updateDelay);
      updateTimeoutStart = (new Date).getTime();
    } else if (commErrors) {
      clearTimeout(updateTimeout);
      sendUpdate();
    } else {
      var diff = (new Date).getTime() - updateTimeoutStart;
      if (diff > WT.updateDelay) {
	clearTimeout(updateTimeout);
	sendUpdate();
      }
    }
  }
}

var ackUpdateId = 0;

function responseReceived(updateId) {
  ackUpdateId = updateId;
  self._p_.comm.responseReceived(updateId);
}

function sendUpdate() {
  if (responsePending)
    return;

  updateTimeout = null;
  var feedback;

  if (WT.isIEMobile) feedback = false;

  if (quited) {
    if (norestart)
      return;
    if (confirm("The application was quited, do you want to restart?")) {
      document.location = document.location;
      norestart = true;
      return;
    } else {
      norestart = true;
      return;
    }
  }

  var data, tm, poll,
    query = '&rand=' + Math.round(Math.random(randomSeed) * 100000);

  if (pendingEvents.length > 0) {
    data = encodePendingEvents();
    tm = data.feedback ? setTimeout(waitFeedback, _$_INDICATOR_TIMEOUT_$_)
      : null;
    poll = false;
  } else {
    data = {result: '&signal=poll' };
    tm = null;
    poll = true;
  }

  if (websocket.socket != null && websocket.socket.readyState == 1) {
    responsePending = null;

    if (tm != null) {
      clearTimeout(tm);
      tm = null;
    }

    if (!poll) {
      websocket.socket.send(data.result + '&ackId=' + ackUpdateId);
    }
  } else {
    responsePending = self._p_.comm.sendUpdate
      (url + query,
       'request=jsupdate' + data.result + '&ackId=' + ackUpdateId,
       tm, ackUpdateId, -1);

    pollTimer
     = poll ? setTimeout(doPollTimeout, _$_SERVER_PUSH_TIMEOUT_$_) : null;
  }
}

function emit(object, config) {
  var userEvent = new Object(), ei = pendingEvents.length;
  userEvent.signal = "user";

  if (typeof object === "string")
    userEvent.id = object;
  else if (object == self)
    userEvent.id = "app";
  else
    userEvent.id = object.id;

  if (typeof config === "object") {
    userEvent.name = config.name;
    userEvent.object = config.eventObject;
    userEvent.event = config.event;
  } else {
    userEvent.name = config;
    userEvent.object = userEvent.event = null;
  }

  userEvent.args = [];
  for (var i = 2; i < arguments.length; ++i) {
    var a = arguments[i], r;
    if (a === false)
      r = 0;
    else if (a === true)
      r = 1;
    else if (a.toDateString)
      r = a.toDateString();
    else
      r = a;
    userEvent.args[i-2] = r;
  }
  userEvent.feedback = true;

  pendingEvents[ei] = encodeEvent(userEvent, ei);

  scheduleUpdate();
}

function addTimerEvent(timerid, msec, repeat) {
  var tm = function() {
    var obj = WT.getElement(timerid);
    if (obj) {
      if (repeat)
	obj.timer = setTimeout(obj.tm, msec);
      else {
	obj.timer = null;
	obj.tm = null;
      }
      if (obj.onclick)
	obj.onclick();
    }
  };

  var obj = WT.getElement(timerid);
  obj.timer = setTimeout(tm, msec);
  obj.tm = tm;
}

var jsLibsLoaded = {};

function onJsLoad(path, f) {
  // setTimeout needed for Opera
  setTimeout(function() {
    if (jsLibsLoaded[path] === true) {
      f();
    } else
      jsLibsLoaded[path] = f;
    }, 20);
};

function jsLoaded(path)
{
  if (jsLibsLoaded[path] === true)
    return;
  else {
    if (typeof jsLibsLoaded[path] !== 'undefined')
      jsLibsLoaded[path]();
    jsLibsLoaded[path] = true;
  }
};

function loadScript(uri, symbol, tries)
{
  function onerror() {
    var t = tries === undefined ? 2 : tries;
    if (t > 1) {
      loadScript(uri, symbol, t - 1);
    } else {
      alert('Fatal error: failed loading ' + uri);
      quit();
    }
  }

  var loaded = false;
  if (symbol != "") {
    try {
      loaded = !eval("typeof " + symbol + " === 'undefined'");
    } catch (e) {
      loaded = false;
    }
  }

  if (!loaded) {
    var s = document.createElement('script');
    s.setAttribute('src', uri);
    s.onload = function() { jsLoaded(uri); };
    s.onerror = onerror;
    s.onreadystatechange = function() {
      var rs = s.readyState;
      if (rs == 'loaded') {
	if (WT.isOpera) {
	  jsLoaded(uri);
	} else
	  onerror();
      } else if (rs == 'complete') {
	jsLoaded(uri);
      }
    };
    var h = document.getElementsByTagName('head')[0];
    h.appendChild(s);
  } else
    jsLoaded(uri);
};

function ImagePreloader(uris, callback) {
  this.callback = callback;
  this.work = uris.length;
  this.images = [];

  if (uris.length == 0)
    callback(this.images);
  else
    for (var i = 0; i < uris.length; i++)
      this.preload(uris[i]);
};

ImagePreloader.prototype.preload = function(uri) {
  var image = new Image;
  this.images.push(image);
  image.onload = ImagePreloader.prototype.onload;
  image.onerror = ImagePreloader.prototype.onload;
  image.onabort = ImagePreloader.prototype.onload;
  image.imagePreloader = this;

  image.src = uri;
};

ImagePreloader.prototype.onload = function() {
  var preloader = this.imagePreloader;
  if (--preloader.work == 0)
    preloader.callback(preloader.images);
};

WT.history.register(_$_INITIAL_HASH_$_, onHashChange);

// For use in FlashObject.js. In IE7, the alternative content is
// not inserted in the DOM and when it is, it cannot contain JavaScript.
// Through a hack in the style attribute, we do execute JS, but what we
// can do there is limited. Hence this helper method.
function ieAlternative(d)
{
  if (d.ieAlternativeExecuted) return '0';
  self.emit(d.parentNode, 'IeAltnernative');
  d.style.width = '';
  d.ieAlternativeExecuted = true;
  return '0';
}

window.onunload = function()
{
  if (!quited) {
    self.emit(self, "Wt-unload");
    sendUpdate();
  }
};

function setCloseMessage(m)
{
  if (m && m != '') {
    window.onbeforeunload = function(event) {
      var e = event || window.event;

      if (e)
	e.returnValue = m;

      return m;
    };
  } else
    window.onbeforeunload = null;
};

this._p_ = {
  ieAlternative : ieAlternative,
  loadScript : loadScript,
  onJsLoad : onJsLoad,
  setTitle : setTitle,
  update : update,
  quit : quit,
  setFormObjects : function(o) { formObjects = o; },
  saveDownPos : saveDownPos,
  addTimerEvent : addTimerEvent,
  load : load,
  handleResponse : handleResponse,
  setServerPush : setServerPush,

  dragStart : dragStart,
  dragDrag : dragDrag,
  dragEnd : dragEnd,
  capture : WT.capture,

  onHashChange : onHashChange,
  setHash : setHash,
  ImagePreloader : ImagePreloader,

  autoJavaScript : function() {  _$_AUTO_JAVASCRIPT_$_(); },

  response : responseReceived,
  setCloseMessage : setCloseMessage
};

this.WT = _$_WT_CLASS_$_;
this.emit = emit;

})();

window.WtSignalEmit = _$_APP_CLASS_$_.emit;

window.WtScriptLoaded = false;

window.onLoad = function() {
  if (!window.WtScriptLoaded) {
    window.isLoaded = true;
    return;
  }

  _$_WT_CLASS_$_.history.initialize("Wt-history-field", "Wt-history-iframe");
  _$_APP_CLASS_$_._p_.load();
};
