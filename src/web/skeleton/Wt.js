var _$_WT_CLASS_$_ = {

// Array Remove - By John Resig (MIT Licensed)
arrayRemove: function(a, from, to) {
  var rest = a.slice((to || from) + 1 || a.length);
  a.length = from < 0 ? a.length + from : from;
  return a.push.apply(a, rest);
},

isIE: navigator.userAgent.toLowerCase().indexOf("msie") != -1
  && navigator.userAgent.toLowerCase().indexOf("opera") == -1,

isGecko: navigator.userAgent.toLowerCase().indexOf("gecko") != -1,

isIEMobile: navigator.userAgent.toLowerCase().indexOf("msie 4") != -1
  || navigator.userAgent.toLowerCase().indexOf("msie 5") != -1,

updateDelay: this.isIE ? 10 : 51,

setHtml: function (el, html, add) {
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
  }

  if (_$_WT_CLASS_$_.isIE || (_$_INNER_HTML_$_ && !add)) {
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
},

hasTag: function(e, s) {
  return e.tagName.toUpperCase() === s;
},

insertAt: function(p, c, i) {
  if (p.childNodes.length == 0)
    p.appendChild(c);
  else
    p.insertBefore(c, p.childNodes[i]);
},

unstub: function(from, to, methodDisplay) {
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
},

unwrap: function(e) {
  var WT = _$_WT_CLASS_$_;
  e = WT.getElement(e);
  if (e.parentNode.className.indexOf('Wt-wrap') == 0) {
    wrapped = e;
    e = e.parentNode;
    wrapped.style.margin = e.style.margin;
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
},

CancelPropagate: 0x1,
CancelDefaultAction: 0x2,
CancelAll: 0x3,

cancelEvent: function(e, cancelType) {
  var WT = _$_WT_CLASS_$_;

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

    if (document.activeElement && document.activeElement.blur)
      if (WT.hasTag(document.activeElement, "TEXTAREA"))
	document.activeElement.blur();
  }
},

getElement: function(id) {
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
},

// Get coordinates of element relative to page origin.
widgetPageCoordinates: function(obj) {
  var objX = objY = 0, op, lop = null, WT = _$_WT_CLASS_$_;

  // bug in safari, according to W3C, offsetParent for an area element should
  // be the map element, but safari returns null.
  if (WT.hasTag(obj, "AREA"))
    obj = obj.parentNode.nextSibling; // img after map

  while (obj) {
    objX += obj.offsetLeft;
    objY += obj.offsetTop;

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
},

// Get coordinates of (mouse) event relative to a element.
widgetCoordinates: function(obj, e) {
  var p = _$_WT_CLASS_$_.pageCoordinates(e);
  var w = _$_WT_CLASS_$_.widgetPageCoordinates(obj);
  return { x: p.x - w.x, y: p.y - w.y };
},

// Get coordinates of (mouse) event relative to page origin.
pageCoordinates: function(e) {
  if (!e) e = window.event;
  var posX = posY = 0;
  if (e.pageX || e.pageY) {
    posX = e.pageX; posY = e.pageY;
  } else if (e.clientX || e.clientY) {
    posX = e.clientX + document.body.scrollLeft
      + document.documentElement.scrollLeft;
    posY = e.clientY + document.body.scrollTop
      + document.documentElement.scrollTop;
  }

  return { x: posX, y: posY };
},

isKeyPress: function(e) {
  if (!e) e = window.event;
  if (e.altKey || e.ctrlKey || e.metaKey)
    return false;

  var charCode = (typeof e.charCode !== 'undefined') ? e.charCode : 0;

  if (charCode > 0 || _$_WT_CLASS_$_.isIE)
    return true;
  else
    return (e.keyCode == 13 || e.keyCode == 27 || e.keyCode == 32
	   || (e.keyCode > 46 && e.keyCode < 112));
},

// Get an element metric in pixels
px: function(c, s) {
  var v = null;
  if (document.defaultView && document.defaultView.getComputedStyle) {
    v = document.defaultView.getComputedStyle(c, null)[s];
  } else if (c.currentStyle) {
    v = c.currentStyle[s];
  } else {
    v = c.style[s];
  }
  if (v == 'auto' || v == null)
    return 0;
  var m = /^\s*(\d+)\s*px\s*$/.exec(v);
  var v = m && m.length == 2 ? m[1] : "0";
  return v ? parseInt(v) : 0;
},

// Return if an element (or one of its ancestors) is hidden
isHidden: function(w) {
  if (w.style.display == 'none')
    return true;
  else { 
    w = w.parentNode;
    if (w != null && w.tagName.toLowerCase() != "body")
      return _$_WT_CLASS_$_.isHidden(w);
    else
      return false;
  }
},

// Get a widget style in pixels, when set directly
pxself: function(c, s) {
  var v = c.style[s];
  if (v == 'auto' || v == null)
    return 0;
  var m = /^\s*(-?\d+)\s*px\s*$/.exec(v);
  var v = m && m.length == 2 ? m[1] : "0";
  return v ? parseInt(v) : 0;
},

pctself: function(c, s) {
  var v = c.style[s];
  if (v == 'auto' || v == null)
    return 0;
  var m = /^\s*(\d+)\s*\%\s*$/.exec(v);
  var v = m && m.length == 2 ? m[1] : "0";
  return v ? parseFloat(v) : 0;
},

IEwidth: function(c, min, max) {
  if (c.parentNode) {
    var WT = _$_WT_CLASS_$_;

    var r = c.parentNode.clientWidth
    - WT.px(c, 'marginLeft')
    - WT.px(c, 'marginRight')
    - WT.px(c, 'borderLeftWidth')
    - WT.px(c, 'borderRightWidth')
    - WT.px(c.parentNode, 'paddingLeft')
    - WT.px(c.parentNode, 'paddingRight');

    var m = /^\s*(\d+)\.?\d*\s*px\s*$/.exec(min);
    var v = m && m.length == 2 ? m[1] : "0";
    min = v ? parseInt(v) : 0;

    m = /^\s*(\d+)\.?\d*\s*px\s*$/.exec(max);
    v = m && m.length == 2 ? m[1] : "100000";
    max = v ? parseInt(v) : 100000;

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
},

hide: function(o) { this.getElement(o).style.display = 'none'; },

inline: function(o) { this.getElement(o).style.display = 'inline'; },

block: function(o) { this.getElement(o).style.display = 'block'; },

show: function(o) { this.getElement(o).style.display = ''; },

getElementsByClassName: function(className, parentElement) {
  if (document.getElementsByClassName) {
    return document.getElementsByClassName(className, parentElement);
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
},

addCss: function(selector, style) {
  var s = document.styleSheets[0];
  s.insertRule(selector + ' { ' + style + ' }', s.cssRules.length);
},

addCssText: function(cssText) {
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
},

// from: http://www.hunlock.com/blogs/Totally_Pwn_CSS_with_Javascript
getCssRule: function(selector, deleteFlag) {
  selector=selector.toLowerCase();

  if (document.styleSheets) {
    for (var i=0; i<document.styleSheets.length; i++) {
      var styleSheet=document.styleSheets[i];
      var ii=0;
      var cssRule=false;
      do {
	if (styleSheet.cssRules)
	  cssRule = styleSheet.cssRules[ii];
	else
	  cssRule = styleSheet.rules[ii];
	if (cssRule && cssRule.selectorText) {
	  if (cssRule.selectorText.toLowerCase()==selector) {
	    if (deleteFlag=='delete') {
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
},

removeCssRule: function(selector) {
  return _$_WT_CLASS_$_.getCssRule(selector, 'delete');
},

addStyleSheet: function(uri) {
  if (document.createStyleSheet) {
    setTimeout(function() { document.createStyleSheet(uri); }, 15);
  } else {
    var s = document.createElement('link');
    s.setAttribute('type', 'text/css');
    s.setAttribute('href', uri);
    s.setAttribute('type','text/css');
    s.setAttribute('rel','stylesheet');
    var h = document.getElementsByTagName('head')[0];
    h.appendChild(s);
  }
},

windowSize: function() {
  var x, y;

  if (typeof (window.innerWidth) == 'number') {
    x = window.innerWidth;
    y = window.innerHeight;
  } else {
    x = document.documentElement.clientWidth;
    y = document.documentElement.clientHeight;
  }

  return { x: x, y: y};
},

fitToWindow: function(e, x, y, rightx, bottomy) {
  var WT = _$_WT_CLASS_$_;
  var ws = WT.windowSize();

  var wx = document.body.scrollLeft + document.documentElement.scrollLeft;
  var wy = document.body.scrollTop + document.documentElement.scrollTop;

  if (x + e.offsetWidth > wx + ws.x)
    x = rightx - e.offsetWidth;
  if (y + e.offsetHeight > wy + ws.y) {
    if (bottomy > wy + ws.y)
      bottomy = wy + ws.y;
    y = bottomy - e.offsetHeight;
  }

  if (x < wx)
    x = wx + ws.x - e.offsetWidth - 3;
  if (y < wy)
    y = wy + ws.y - e.offsetHeight - 3;

  var ow = WT.widgetPageCoordinates(e.offsetParent);

  e.style.left = (x - ow.x) + 'px';
  e.style.top = (y - ow.y) + 'px';
},

positionXY: function(id, x, y) {
  var WT = _$_WT_CLASS_$_;

  var w = WT.getElement(id);
  WT.fitToWindow(w, x, y, x, y);
},

positionAtWidget: function(id, atId) {
  var WT = _$_WT_CLASS_$_;
  var w = WT.getElement(id);
  var atw = WT.getElement(atId);
  var xy = WT.widgetPageCoordinates(atw);

  w.style.display='block';
  WT.fitToWindow(w, xy.x + atw.offsetWidth, xy.y,
		 xy.x, xy.y + atw.offsetHeight);
},

history: (function() {
/*
Original copyright: heavily simplified for Wt
Copyright (c) 2008, Yahoo! Inc. All rights reserved.
Code licensed under the BSD License:
http://developer.yahoo.net/yui/license.txt
version: 2.5.2
*/
  var _UAwebkit = false;
  var _UAie = false;
  var _UAopera = false;
  var _onLoadFn = null;
  var _histFrame = null;
  var _stateField = null;
  var _initialized = false;
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
  function _initialize() {
    var parts, counter, hash;
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
      counter = history.length;
      hash = _getHash();
      setInterval(function () {
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
    } else if (typeof window.opera !== "undefined") {
      _UAopera = true;
    } else if (typeof document.all !== "undefined") {
      _UAie = true;
    } else if (vendor.indexOf("Apple Computer, Inc.") > -1) {
      _UAwebkit = true;
    }
    if (typeof stateField === "string") {
      stateField = document.getElementById(stateField);
    }
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
      return _updateIFrame(fqstate);
    } else {
      location.hash = fqstate;
      if (_UAwebkit) {
	_fqstates[history.length] = fqstate;
	_storeStates();
      }
      return true;
    }
  },
  getCurrentState: function () {
    if (!_initialized) {
      return "";
    }
    return _currentState;
  }
  };
})()
};

var _$_APP_CLASS_$_ = function() {

var WT = _$_WT_CLASS_$_;

var downX = 0;
var downY = 0;

var saveDownPos = function(e) {
  var coords = WT.pageCoordinates(e);
  downX = coords.x;
  downY = coords.y;
};

var currentHash = null;

var onHashChange = function() {
  var newLocation = WT.history.getCurrentState();
  if (currentHash == newLocation) {
    return;
  } else {
    currentHash = newLocation;
    setTimeout("_$_APP_CLASS_$_._p_.update(null, 'hash', null, true);", 1);
  }
};

var setHash = function(newLocation) {
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
  objectPrevStyle: null
};

var capture = function(obj) {
  captureElement = obj;
  if (document.body.setCapture)
    if (obj != null)
      document.body.setCapture();
    else
      document.body.releaseCapture();
}

var initDragDrop = function() {
  var APP = _$_APP_CLASS_$_;

  window.onresize=function() { APP._p_.autoJavaScript(); }

  var mouseMove = function(e) {
    if (!e) e = window.event;
    return dragDrag(e);
  }

  var mouseUp = function(e) {
    if (!e) e = window.event;
    return dragEnd(e);
  }

  var db = document.body;
  if (db.addEventListener) {
    db.addEventListener('mousemove', mouseMove, true);
    db.addEventListener('mouseup', mouseUp, true);

    if (WT.isGecko) {
      window.addEventListener('mouseout', function(e) {
	  if (!e.relatedTarget && WT.hasTag(e.target, "HTML"))
	    mouseUp(e);
	}, true);
    }
  } else {
    db.attachEvent('onmousemove', mouseMove);
    db.attachEvent('onmouseup', mouseUp);
  }

  document.body.ondragstart=function() {
    return false;
  };
}

var dragStart = function(obj, e) {
  capture(null);

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
    return;

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

  ds.offsetX = -4;
  ds.offsetY = -4;
  ds.dropTarget = null;
  ds.mimeType = obj.getAttribute("dmt");

  WT.cancelEvent(e);
  return false;
};

var dragDrag = function(e) {
  if (captureElement != null) {
    if (!e) e = window.event;
    if (captureElement.onmousemove)
      captureElement.onmousemove(e);
    return false;
  }

  if (dragState.object != null) {
    var ds = dragState;
    var xy = WT.pageCoordinates(e);

    ds.object.style["display"] = '';
    ds.object.style["left"] = (xy.x - ds.offsetX) + 'px';
    ds.object.style["top"] = (xy.y - ds.offsetY) + 'px';

    var prevDropTarget = ds.dropTarget;
    var t = e.target || e.srcElement;
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
      if (WT.hasTag(t, "HTML"))
	break;
    }

    if (ds.dropTarget != prevDropTarget) {
      if (ds.dropTarget) {
        var s = amts.indexOf(mimeType) + mimeType.length;
	var e = amts.indexOf("}", s);
	var style = amts.substring(s, e);
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
	ds.object.className = 'valid-drop';
    }

    WT.cancelEvent(e);
    return false;
  }

  return true;
};

var dragEnd = function(e) {
  if (!e) e = window.event;
  if (captureElement != null) {
    var el = captureElement;
    capture(null);
    if (el.onmouseup)
      el.onmouseup(e);
    return false;
  }

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
	emit(ds.dropTarget, {name: "_drop", eventObject: ds.dropTarget, event: e}, ds.sourceId, ds.mimeType);
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

var formObjects = _$_FORM_OBJECTS_$_;

var encodeEvent = function(event, i) {
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
    var el = WT.getElement(formObjects[x]), v = null;
    if (el == null)
      continue;

    if (el.type == 'select-multiple') {
      for (var i = 0; i < el.options.length; i++)
	if (el.options[i].selected) {
	  result += se + formObjects[x] + '='
	    + encodeURIComponent(el.options[i].value);
	}
    } else if (WT.hasTag(el, "SPAN")) {
      for (var i = 0; i < el.childNodes.length; i++) {
	if (el.childNodes[i].type == 'checkbox') {
	  var cb = el.childNodes[i];

	  if (cb.style.display == 'none')
	    v = 'indeterminate';
	  else
	    if (cb.checked)
	      v = cb.value;

	  break;
	}
      }
    } else if (el.type == 'checkbox' || el.type == 'radio') {
      if (el.indeterminate)
	v = 'indeterminate';
      else
	if (el.checked)
	  v = el.value;
    } else if (el.type != 'file')
      v = '' + el.value;

    if (v != null)
      result += se + formObjects[x] + '=' + encodeURIComponent(v);
  }

  if (currentHash != null)
    result += se + '_=' + encodeURIComponent(unescape(currentHash));

  if (!e) {
    event.data = result;
    return event;
  }

  var t = e.target || e.srcElement;
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
  }

  if (e.screenX || e.screenY)
    result += se + 'screenX=' + e.screenX + se + 'screenY=' + e.screenY;

  if (event.object && event.object.nodeType != 9) {
    var widgetCoords = WT.widgetPageCoordinates(event.object);
    var objX = widgetCoords.x;
    var objY = widgetCoords.y;

    result += se + 'scrollX=' + event.object.scrollLeft
      + se + 'scrollY=' + event.object.scrollTop
      + se + 'width=' + event.object.clientWidth
      + se + 'height=' + event.object.clientHeight
      + se + 'widgetX=' + (posX - objX) + se + 'widgetY=' + (posY - objY);
  }

  if (e.which)
    result += se + 'right=' + (e.which==3);
  else
    if (e.button)
      result += se + 'right=' + (e.button==2);

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

  event.data = result;
  return event;
};

var pendingEvents = [];

var encodePendingEvents = function() {
  var result = '';

  feedback = false;

  for (var i = 0; i < pendingEvents.length; ++i) {
    feedback = feedback || pendingEvents[i].feedback;
    result += pendingEvents[i].data;
  }

  pendingEvents = [];

  return {feedback: feedback, result: result};
}

var url = _$_RELATIVE_URL_$_;
var quited = false;
var norestart = false;
var loaded = false;
var responsePending = null;
var pollTimer = null;
var keepAliveTimer = null;

var doKeepAlive = function() {
  update(null, 'none', null, false);
  keepAliveTimer = setTimeout(doKeepAlive, _$_KEEP_ALIVE_$_000);
};

var debug = function(s) {
  document.body.innerHTML += s;
};

var setTitle = function(title) {
  if (WT.isIEMobile) return;
  document.title = title;
};

var load = function() {
  WT.history._initialize();
  initDragDrop();
  if (!loaded) {
    loaded = true;
    _$_ONLOAD_$_;
    keepAliveTimer = setTimeout(doKeepAlive, _$_KEEP_ALIVE_$_000);
  }
};

var currentHideLoadingIndicator = null;

var cancelFeedback = function(t) {
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

var waitFeedback = function() {
  document.body.style.cursor = 'wait';
  currentHideLoadingIndicator = hideLoadingIndicator;
  showLoadingIndicator();
};

var serverPush = false;

var setServerPush = function(how) {
  serverPush = how;
}

var handleResponse = function(msg, timer) {
  if (quited)
    return;

  _$_$ifnot_DEBUG_$_; try { _$_$endif_$_;
  eval(msg);
  _$_APP_CLASS_$_._p_.autoJavaScript();
  _$_$ifnot_DEBUG_$_; } catch (e) {
    alert("Wt internal error: " + e + ", code: " +  e.code
	  + ", description: " + e.description /* + ":" + msg */);
  } _$_$endif_$_;

  if (timer)
    cancelFeedback(timer);

  if (pollTimer) {
    clearTimeout(pollTimer);
    pollTimer = null;
  }

  responsePending = null;

  if (serverPush || pendingEvents.length > 0)
    sendUpdate();
};

var randomSeed = new Date().getTime();

var updateTimeout = null, captureElement = null;

var doPollTimeout = function() {
  responsePending.abort();
  responsePending = null;
  pollTimer = null;

  sendUpdate();
}

var update = function(self, signalName, e, feedback) {
  if (captureElement && (self == captureElement) && e.type == "mouseup")
    capture(null);

  _$_APP_CLASS_$_._p_.autoJavaScript();

  _$_$if_STRICTLY_SERIALIZED_EVENTS_$_;
  if (responsePending)
    return;
  _$_$endif_$_;

  var pendingEvent = new Object(), i = pendingEvents.length;
  pendingEvent.object = self;
  pendingEvent.signal = signalName;
  pendingEvent.event = e;
  pendingEvent.feedback = feedback;

  pendingEvents[i] = encodeEvent(pendingEvent, i);

  scheduleUpdate();
}

var scheduleUpdate = function() {
  if (responsePending != null && pollTimer != null) {
    clearTimeout(pollTimer);
    responsePending.abort();
    responsePending = null;
  }

  if (responsePending == null) {
    if (updateTimeout == null)
      updateTimeout = setTimeout(function() { sendUpdate(); }, WT.updateDelay);
  }
}

var ackUpdateId = 0;

var responseReceived = function(updateId) {
  ackUpdateId = updateId;

   _$_APP_CLASS_$_._p_.commResponseReceived(updateId);
}

var sendUpdate = function() {
  updateTimeout = null;

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
    data = {result: 'signal=poll' };
    tm = null;
    poll = true;
  }

  responsePending = _$_APP_CLASS_$_._p_.sendUpdate
    (url + query, 'request=jsupdate&' + data.result + '&ackId=' + ackUpdateId,
     tm, ackUpdateId);

  pollTimer
    = poll ? setTimeout(doPollTimeout, _$_SERVER_PUSH_TIMEOUT_$_) : null;
};

var emit = function(object, config) {
  var userEvent = new Object(), ei = pendingEvents.length;
  userEvent.signal = "user";

  if (typeof object == "string")
    userEvent.id = object;
  else if (object == _$_APP_CLASS_$_)
    userEvent.id = "app";
  else
    userEvent.id = object.id;

  if (typeof config == "object") {
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
};

var addTimerEvent = function(timerid, msec, repeat) {
  var tm = function() {
    var obj=_$_WT_CLASS_$_.getElement(timerid);
    if (obj) {
      if (repeat)
	obj.timer = setTimeout(obj.tm, msec);
      else {
	obj.timer = null;
	obj.tm = null;
      }
      obj.onclick();
    }
  };

  var obj = _$_WT_CLASS_$_.getElement(timerid);
  obj.timer = setTimeout(tm, msec);
  obj.tm = tm;
};

var jsLibsLoaded = {};

var onJsLoad = function(path, f)
{
  // setTimeout needed for Opera
  setTimeout(function() {
    if (jsLibsLoaded[path] === true) {
      f();
    } else
      jsLibsLoaded[path] = f;
    }, 20);
};

var jsLoaded = function(path)
{
  if (jsLibsLoaded[path] === true)
    return;
  else {
    if (typeof jsLibsLoaded[path] !== 'undefined')
      jsLibsLoaded[path]();
    jsLibsLoaded[path] = true;
  }
};

var loadScript = function(uri, symbol)
{
  var loaded = false;
  if (symbol != "") {
    try {
      loaded = !eval("typeof " + symbol + " == 'undefined'");
    } catch (e) {
      loaded = false;
    }
  }

  if (!loaded) {
    var s = document.createElement('script');
    s.setAttribute('src', uri);
    s.onload = function() { jsLoaded(uri);};
    s.onreadystatechange = function() {
      if (s.readyState == 'complete' || s.readyState == 'loaded') {
	jsLoaded(uri);
      }
    };
    var h = document.getElementsByTagName('head')[0];
    h.appendChild(s);
  } else
    jsLoaded(uri);
};

var ImagePreloader = function(uris, callback)
{
  this.callback = callback;
  this.work = uris.length;
  this.images = [];

  if (uris.length == 0)
    callback(this.images);
  else
    for (var i = 0; i < uris.length; i++)
      this.preload(uris[i]);
};

ImagePreloader.prototype.preload = function(uri)
{
  var image = new Image;
  this.images.push(image);
  image.onload = ImagePreloader.prototype.onload;
  image.onerror = ImagePreloader.prototype.onload;
  image.onabort = ImagePreloader.prototype.onload;
  image.imagePreloader = this;

  image.src = uri;
};

ImagePreloader.prototype.onload = function()
{
  var preloader = this.imagePreloader;
  if (--preloader.work == 0)
    preloader.callback(preloader.images);
};

WT.history.register('_$_INITIAL_HASH_$_', onHashChange);

// Public static methods
return {
  _p_: {
    "loadScript" : loadScript,
    "onJsLoad" : onJsLoad,
    "setTitle" : setTitle,
    "update" : update,
    "quit" : function() { quited = true; clearTimeout(keepAliveTimer); },
    "setFormObjects" : function(o) { formObjects = o; },
    "saveDownPos" : saveDownPos,
    "addTimerEvent" : addTimerEvent,
    "load" : load,
    "handleResponse" : handleResponse,
    "setServerPush" : setServerPush,

    "dragStart" : dragStart,
    "dragDrag" : dragDrag,
    "dragEnd" : dragEnd,
    "capture" : capture,

    "onHashChange" : onHashChange,
    "setHash" : setHash,
    "ImagePreloader" : ImagePreloader,

    "autoJavaScript" : function() { _$_AUTO_JAVASCRIPT_$_ },

    "response" : responseReceived
  },

  "emit" : emit
};

}();

var WtSignalEmit = _$_APP_CLASS_$_.emit;
window.WtScriptLoaded = false;

function onLoad() {
  if (!window.WtScriptLoaded) {
    window.isLoaded = true;
    return;
  }

  _$_WT_CLASS_$_.history.initialize("Wt-history-field", "Wt-history-iframe");
  _$_APP_CLASS_$_._p_.load();
}
