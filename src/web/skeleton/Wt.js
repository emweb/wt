var _$_WT_CLASS_$_ = {

DOCUMENT_ELEMENT_NODE: 1,
DOCUMENT_TEXT_NODE: 3,
DOCUMENT_CDATA_SECTION_NODE: 4,
DOCUMENT_COMMENT_NODE: 8,

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

setHtml: function (el, html) {
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
	for (i = 0, il = e.childNodes.length; i < il;)
	  newNode.appendChild(myImportNode(e.childNodes[i++], deep));
      return newNode;
      break;
    case 3: // text
    case 4: // cdata
    case 5: // comment
      return document.createTextNode(e.nodeValue);
      break;
    }
  }

  if (_$_INNER_HTML_$_) {
    el.innerHTML = html;
  } else {
    var d, b;
    d = new DOMParser();
    b = d.parseFromString('<div>'+html+'<\/div>','application/xhtml+xml');
    d = b.documentElement;
    if (d.nodeType != 1) // element
      d = d.nextSibling;

    el.innerHTML = '';
    for (var i = 0, il = d.childNodes.length; i < il;)
      el.appendChild(myImportNode(d.childNodes[i++], true));
  }
},

hasTag: function(e, s) {
  return e.tagName.toUpperCase() === s;
},

copyhide: function(from, to) {
  to.style.position = from.style.position;
  to.style.left = from.style.left;
  to.style.visibility = from.style.visibility;
},

cancelEvent: function(e) {
  if (e.preventDefault) e.preventDefault(); else e.returnValue=false;
  if (e.stopPropagation) e.stopPropagation(); else e.cancelBubble=true;
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
  var objX = objY = 0, op, lop = null;

  while (obj) {
    objX += obj.offsetLeft;
    objY += obj.offsetTop;

    op = obj.offsetParent;
    if (op == null)
      obj = null;
    else {
      lop = op;
      do {
        obj = obj.parentNode;
	if (_$_WT_CLASS_$_.hasTag(obj, "DIV")) {
	  objX -= obj.scrollLeft;
	  objY -= obj.scrollTop;
	}
      } while (obj != op);
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
  var self = _$_WT_CLASS_$_;

  var r = c.parentNode.clientWidth
   - self.px(c, 'marginLeft')
   - self.px(c, 'marginRight')
   - self.px(c, 'borderLeftWidth')
   - self.px(c, 'borderRightWidth')
   - self.px(c.parentNode, 'paddingLeft')
   - self.px(c.parentNode, 'paddingRight');

  var m = /^\s*(\d+)\s*px\s*$/.exec(min);
  var v = m && m.length == 2 ? m[1] : "0";
  min = v ? parseInt(v) : 0;

  m = /^\s*(\d+)\s*px\s*$/.exec(max);
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
},

clone: function(o) {
  if (o == null || typeof(o) != 'object')
    return o;
  var temp = { };
  for (var i in o) {
    temp[i] = o[i];
  }

  return temp;
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
	if (cssRule) {
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
  if (typeof (window.innerWidth) == 'number')
    return {x: window.innerWidth, y: window.innerHeight };
  else
    return {x: document.documentElement.clientWidth,
	    y: document.documentElement.clientHeight };
},

fitToWindow: function(e, x, y, rightx, bottomy) {
  var ws = _$_WT_CLASS_$_.windowSize();

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

  e.style.left = x + 'px';
  e.style.top = y + 'px';  
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
}

var dragStart = function(obj, e) {
  captureElement = null;

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
  if (captureElement != null) {
    if (!e) e = window.event;
    var el = captureElement;
    captureElement = null;
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
	emit(ds.dropTarget, "_drop", ds.sourceId, ds.mimeType);
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

var encodeEvent = function(event, i) {
  var se, result, e;
  
  e = event.event;
  se = '&e' + i;
  result = se + 'signal=' + event.signal;

  if (event.id) {
    result += se + 'id=' + event.id
        + se + 'name=' + encodeURIComponent(event.name)
        + se + 'an=' + event.args.length;

    for (var j = 0; j < event.args.length; ++j)
      result += se + 'a' + j + '=' + encodeURIComponent(event.args[j]);
  }

  if (!e)
    return result;

  var t = e.target || e.srcElement;
  while (!t.id && t.parentNode)
    t = t.parentNode;
  if (t.id)
    result += se + 'tid=' + t.id;

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

  return result;
};

var pendingEvents = [];

var encodePendingEvents = function() {
  var result = "";

  if (currentHash != null)
    result += '&_=' + encodeURIComponent(unescape(currentHash));

  for (var i = 0; i < pendingEvents.length; ++i)
    result += encodeEvent(pendingEvents[i], i);

  pendingEvents = [];

  return result;
}

var formObjects = _$_FORM_OBJECTS_$_;
var url = _$_RELATIVE_URL_$_;
var quited = false;
var norestart = false;
var loaded = false;
var responsesPending = 0;
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
  if (!loaded) {
    loaded = true;
    _$_ONLOAD_$_;
    update(null, 'load', null, false);
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

var handleResponse = function(msg, timer) {
  if (quited)
    return;

  try {
    eval(msg);
    _$_APP_CLASS_$_._p_.autoJavaScript();
  } catch (e) {
    alert("Wt internal error: " + e + ", code: " +  e.code 
    + ", description: " + e.description /* + ":" + msg */);
  }

  if (timer)
    cancelFeedback(timer);

  --responsesPending;

  if (pendingEvents.length > 0)
    sendUpdate(true);
};

var randomSeed = new Date().getTime();

var updateTimeout = null;

var captureElement = null;

var update = function(self, signalName, e, feedback) {
  if (captureElement && (self == captureElement) && e.type == "mouseup")
    captureElement = null;
  _$_APP_CLASS_$_._p_.autoJavaScript();

  if (_$_STRICTLY_SERIALIZED_EVENTS_$_ && responsesPending)
    return;

  var pendingEvent = new Object();
  pendingEvent.object = self;
  pendingEvent.signal = signalName;
  pendingEvent.event = WT.clone(e);

  pendingEvents[pendingEvents.length] = pendingEvent;

  scheduleUpdate(feedback);
}

var scheduleUpdate = function(feedback) {
  if (responsesPending == 0) {
    if (updateTimeout == null)
      updateTimeout = setTimeout(function() { sendUpdate(feedback); },
				 WT.updateDelay);
  }
}

var sendUpdate = function(feedback) {
  updateTimeout = null;
  ++responsesPending;

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

  var query = '&rand=' + Math.round(Math.random(randomSeed) * 100000);

  var querydata = encodePendingEvents();

  for (var x = 0; x < formObjects.length; ++x) {
    var el = WT.getElement(formObjects[x]);
    if (el == null)
      continue;

    if (el.type == 'select-multiple') {
      for (var i = 0; i < el.options.length; i++)
	if (el.options[i].selected)
	  querydata += '&' + formObjects[x]
	    + '=' + encodeURIComponent(el.options[i].value);
    } else if ((el.type != 'file')
	       && (((el.type != 'checkbox') && (el.type != 'radio'))
		   || el.checked))
      querydata += '&' +formObjects[x]
	+ '=' + encodeURIComponent(el.value);
  }

  var tm = feedback ? setTimeout(waitFeedback, _$_INDICATOR_TIMEOUT_$_) : null;

  _$_APP_CLASS_$_._p_.sendUpdate(url + query, querydata, tm);
};

var emit = function(object, config) {
  var userEvent = new Object();
  userEvent.signal = "user";

  if (typeof(object) == "string")
    userEvent.id = object;
  else if (object == _$_APP_CLASS_$_)
    userEvent.id = "app";
  else
    userEvent.id = object.id;

  if (typeof config == "object") {
    userEvent.name = config.name;
    userEvent.object = config.eventObject;
    userEvent.event = WT.clone(config.event);
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

  pendingEvents[pendingEvents.length] = userEvent;

  if (responsesPending == 0)
    scheduleUpdate(true);
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
  if (symbol != "")
    loaded = !eval("typeof " + symbol + " == 'undefined'");

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

    "dragStart" : dragStart,
    "dragDrag" : dragDrag,
    "dragEnd" : dragEnd,
    "capture" : capture,

    "onHashChange" : onHashChange,
    "setHash" : setHash,
    "ImagePreloader" : ImagePreloader,

    "autoJavaScript" : function() { _$_AUTO_JAVASCRIPT_$_ }
  },

  "emit" : emit
};

}();

var WtSignalEmit = _$_APP_CLASS_$_.emit;
var scriptLoaded = false;

function onLoad() {
  if (!scriptLoaded) {
    isLoaded = true;
    return;
  }

  var WT = _$_WT_CLASS_$_;
  var APP = _$_APP_CLASS_$_;

  WT.history.initialize("Wt-history-field", "Wt-history-iframe");
  window.onresize=APP._p_.autoJavaScript;
  document.body.onmousemove=function(e) {
    if (!e) e = window.event;
    return APP._p_.dragDrag(e);
  }
  document.body.onmouseup=function(e) {
    if (!e) e = window.event;
    return APP._p_.dragEnd(e);
  }
  document.body.ondragstart=function() {
    return false;
  };

  APP._p_.load();
}

function loadWidgetTree() { }
