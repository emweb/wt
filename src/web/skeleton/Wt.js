
/**
 * @preserve Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * For terms of use, see LICENSE.
 */
_$_$if_DYNAMIC_JS_$_();
window.JavaScriptFunction = 1;
window.JavaScriptConstructor = 2;
window.JavaScriptObject = 3;
window.JavaScriptPrototype = 4;
window.WT_DECLARE_WT_MEMBER = function(i, type, name, fn)
{
  if (type == JavaScriptPrototype) {
    var proto = name.indexOf('.prototype');
    _$_WT_CLASS_$_[name.substr(0, proto)]
      .prototype[name.substr(proto + '.prototype.'.length)] = fn;
  } else if (type == JavaScriptFunction) {
    _$_WT_CLASS_$_[name] = function() { fn.apply(_$_WT_CLASS_$_, arguments); };
  } else {
    _$_WT_CLASS_$_[name] = fn;
  }
};

window.WT_DECLARE_APP_MEMBER = function(i, type, name, fn)
{
  var app = window.currentApp;
  if (type == JavaScriptPrototype) {
    var proto = name.indexOf('.prototype');
    app[name.substr(0, proto)]
      .prototype[name.substr(proto + '.prototype.'.length)] = fn;
  } else if (type == JavaScriptFunction) {
    app[name] = function() { fn.apply(app, arguments); };
  } else {
    app[name] = fn;
  }
};

_$_$endif_$_();

if (!window._$_WT_CLASS_$_)
  window._$_WT_CLASS_$_ = new (function()
{
var WT = this;

this.condCall = function(o, f, a) {
  if (o[f])
    o[f](a);
};

// buttons currently down
this.buttons = 0;

// returns the button associated with the event (0 if none)
this.button = function(e)
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


this.mouseDown = function(e) {
  WT.buttons |= WT.button(e);
};

this.mouseUp = function(e) {
  WT.buttons &= ~WT.button(e);
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
};

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

var agent = navigator.userAgent.toLowerCase();

this.isIE = ie !== undefined;
this.isIE6 = ie === 6;
this.isIElt9 = ie < 9;
this.isIEMobile = agent.indexOf("msie 4")!=-1 || agent.indexOf("msie 5")!=-1;
this.isOpera = typeof window.opera !== 'undefined';
this.isAndroid = (agent.indexOf("safari") != -1)
		  && (agent.indexOf("android") != -1);
this.isMobileWebKit = (agent.indexOf("applewebkit") != -1)
		       && (agent.indexOf("mobile") != -1);
this.isWebKit = (agent.indexOf("applewebkit") != -1);
this.isGecko = agent.indexOf("gecko") != -1 && !this.isWebKit;

this.updateDelay = this.isIE ? 10 : 51;

if (this.isAndroid) {
  console.error('init console.error');
  console.info('init console.info');
  console.log('init console.log');
  console.warn('init console.warn');
}

var traceStart = new Date();
this.trace = function(v, start) {
  if (start)
    traceStart = new Date();
  var now = new Date();

  var diff = (now.getMinutes() - traceStart.getMinutes()) * 60000
    + (now.getSeconds() - traceStart.getSeconds()) * 1000
    + (now.getMilliseconds() - traceStart.getMilliseconds());

  if (window.console)
    console.log("[" + diff + "]: " + v);
};

function host(url) {
  var parts = url.split('/');
  return parts[2];
}

this.initAjaxComm = function(url, handler) {
  var crossDomain = url.indexOf("://") != -1
                    && host(url) != window.location.host;

  function createRequest(method, url) {
    var request = null;
    var supportsRequestHeader = true;
    if (window.XMLHttpRequest) {
      request = new XMLHttpRequest();
      if (crossDomain) {
	if ("withCredentials" in request) {
	  if (url) {
	    request.open(method, url, true);
	    request.withCredentials = "true";
	  }
	} else if (typeof XDomainRequest != "undefined") {
	  request = new XDomainRequest();
	  if (url) {
	    supportsRequestHeader = false;
	    try {
	      request.open(method, url + '&contentType=x-www-form-urlencoded');
	    } catch (err) {
	      request = null;
	    }
	  }
	} else
	  request = null;
      } else
	if (url)
	  request.open(method, url, true);
    } else if (!crossDomain && window.ActiveXObject) {
      try {
	request = new ActiveXObject("Msxml2.XMLHTTP");
      } catch (err) {
	try {
	  request = new ActiveXObject("Microsoft.XMLHTTP");
	} catch (err2) {
	}
      }
      if (url && request)
	request.open(method, url, true);
    }

    if (request && url && supportsRequestHeader)
      request.setRequestHeader("Content-type",
			       "application/x-www-form-urlencoded");

    return request;
  }

  var req = createRequest('POST', url);

  if (req != null) {
    return new (function() {
      var sessionUrl = url;

      function Request(data, userData, id, timeout) {
	var request = createRequest('POST', sessionUrl);
	var timer = null;
	var handled = false;

	function handleResponse(good) {
	  if (handled)
	    return;

	  clearTimeout(timer);

	  if (good)
	    handler(0, request.responseText, userData);
	  else
	    handler(1, null, userData);

	  request.onreadystatechange = new Function;
	  try {
	    request.onload = request.onreadystatechange;
	  } catch (e) {
	    /*
	     * See comment below.
	     */
	  }
	  request = null;

	  handled = true;
	}

	function recvCallback() {
	  if (request.readyState == 4) {
	    var good = request.status == 200
	      && request.getResponseHeader("Content-Type")
	      && request.getResponseHeader("Content-Type")
		.indexOf("text/javascript") == 0;

	    handleResponse(good);
	  }
	}

	function handleTimeout() {
	  request.onreadystatechange = new Function;
	  request = null;
	  handled = true;
	  handler(2, null, userData);
	}

	this.abort = function() {
	  request.onreadystatechange = new Function;
	  handled = true;
	  request.abort();
	  request = null;
	};

	if (_$_CLOSE_CONNECTION_$_)
	  request.setRequestHeader("Connection","close");

	if (timeout > 0)
	  timer = setTimeout(handleTimeout, timeout);
	  request.onreadystatechange = recvCallback;
	  try {
	    request.onload = function() {
	      handleResponse(true);
	    };
	    request.onerror = function() {
	      handleResponse(false);
	    };
	  } catch (e) {
	    /*
	     * On IE, when "Enable Native XMLHTTP Support is unchecked",
	     * setting these members will result in an exception.
	     */
	  }
	request.send(data);
      }

      this.responseReceived = function(updateId) { };

      this.sendUpdate = function(data, userData, id, timeout) {
	return new Request(data, userData, id, timeout);
      };

      this.setUrl = function(url) {
	sessionUrl = url;
      };
    })();
  } else {
    return new (function() {
      var sessionUrl = url;
      var request = null;

      function Request(data, userData, id, timeout) {
	var self = this;

	this.userData = userData;

	var s = this.script = document.createElement('script');
	s.id = "script" + id;
	s.setAttribute('src', sessionUrl + '&' + data);

	function onerror() {
	  handler(1, null, userData);
	  s.parentNode.removeChild(s);
	}

	s.onerror = onerror;
/*
	s.onreadystatechange = function() {
	  var rs = s.readyState;
	  if (rs == 'loaded' && !WT.isOpera)
	    onerror();
	};
*/

	var h = document.getElementsByTagName('head')[0];
	h.appendChild(s);

	this.abort = function() {
	  s.parentNode.removeChild(s);
	};
      }

      this.responseReceived = function(updateId) {
        if (request != null) {
          var req = request;
	  request.script.parentNode.removeChild(request.script);
          request = null;
	  handler(0, "", req.userData);
	}
      };

      this.sendUpdate = function(data, userData, id, timeout) {
        request = new Request(data, userData, id, timeout);
        return request;
      };

      this.setUrl = function(url) {
	sessionUrl = url;
      };

    })();
  }
};

this.setHtml = function (el, html, add) {
  function myImportNode(e, deep) {
    var newNode, i, il;
    switch (e.nodeType) {
    case 1: // element
      if (e.namespaceURI === null)
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
  return e.nodeType == 1 && e.tagName && e.tagName.toUpperCase() === s;
};

this.insertAt = function(p, c, i) {
  if (!p.childNodes.length)
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

  while (p && !WT.hasTag(p, "BODY")) {
    if (p == w1)
      return true;
    p = p.parentNode;
  }

  return false;
};

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

this.changeTag = function(e, type)
{
  var n = document.createElement(type);

   /* For some reason fails on 'a' */
  if (type == 'img' && n.mergeAttributes) {
    n.mergeAttributes(e, false);
    n.src = e.src;
  } else {
    if (e.attributes && e.attributes.length > 0) {
      var i, il;
      for (i = 0, il = e.attributes.length; i < il; i++) {
	var nn = e.attributes[i].nodeName;
	if (nn != 'type' && nn != 'name')
	  n.setAttribute(nn, e.getAttribute(nn));
      }
    }
  }

  while (e.firstChild)
    n.appendChild(e.removeChild(e.firstChild));

  e.parentNode.replaceChild(n, e);
};

this.unwrap = function(e) {
  e = WT.getElement(e);
  if (!e.parentNode.className.indexOf('Wt-wrap')) {
    var wrapped = e;
    e = e.parentNode;
    if (e.className.length >= 8)
      wrapped.className = e.className.substring(8);
    var style = e.getAttribute('style');
    if (style) {
      if (WT.isIE)
	wrapped.style.setAttribute('cssText', style);
      else
	wrapped.setAttribute('style', style);
    }
    e.parentNode.replaceChild(wrapped, e);
  } else {
    if (e.getAttribute('type') == 'submit') {
      e.setAttribute('type', 'button');
      e.removeAttribute('name');
    } if (WT.hasTag(e, 'INPUT') && e.getAttribute('type') == 'image') {
      WT.changeTag(e, 'img');
    }
  }
};

this.navigateInternalPath = function(event, path) {
  var e = event || window.event;
  if (!e.ctrlKey && !e.metaKey && WT.button(e) <= 1) {
    WT.history.navigate(path, true);
    WT.cancelEvent(e, WT.CancelDefaultAction);
  };
};

this.ajaxInternalPaths = function(basePath) {
  $('.Wt-ip').each(function() {
      var href = this.getAttribute('href'), wtd = href.lastIndexOf('?wtd');
      if (wtd === -1)
	wtd = href.lastIndexOf('&wtd');
      if (wtd !== -1)
	href = href.substr(0, wtd);

      var internalPath;

      /*
       * On IE < 8, an absolute URL is read from href. In that case we
       * also turn the basePath into an absolute URL.
       */
      if (href.indexOf("://") != -1) {
	var el= document.createElement('div');
	el.innerHTML= '<a href="' + basePath +'">x</a>';
	var absBase = el.firstChild.href;
	internalPath = href.substr(absBase.length - 1);
      } else
	internalPath = href.substr(basePath.length);

      if (internalPath.substr(0, 3) == "?_=")
	  internalPath = internalPath.substr(3);
      this.setAttribute('href', href); // computes this.href
      this.setAttribute('href', this.href);
      this.onclick = function(event) {
	WT.navigateInternalPath(event, internalPath);
      };
      $(this).removeClass("Wt-ip");
    });
};

this.resolveRelativeAnchors = function() {
  $('.Wt-rr').each(function() {
      this.setAttribute('href', this.href);
      $(this).removeClass("Wt-rr");
    });
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

this.validate = function(edit) {
  var v = edit.wtValidate.validate(edit.value);
  if (v.valid) {
    edit.removeAttribute('title');
    $(edit).removeClass('Wt-invalid');
  } else {
    edit.setAttribute('title', v.message);
    $(edit).addClass('Wt-invalid');
  }
};

this.filter = function(edit, event, tokens) {
  var c = String.fromCharCode((typeof event.charCode !== 'undefined') ?
                              event.charCode : event.keyCode);
  if (!new RegExp(tokens).test(c))
    WT.cancelEvent(event);
};

// Get coordinates of element relative to page origin.
this.widgetPageCoordinates = function(obj) {
  var objX = 0, objY = 0, op;

  // bug in safari, according to W3C, offsetParent for an area element should
  // be the map element, but safari returns null.
  if (WT.hasTag(obj, "AREA"))
    obj = obj.parentNode.nextSibling; // img after map

  var rtl = $(document.body).hasClass('Wt-rtl');

  while (obj) {
    objX += obj.offsetLeft;
    objY += obj.offsetTop;

    var f = WT.css(obj, 'position');
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
	  if (rtl && !WT.isGecko) {
	    if (obj.scrollWidth > obj.parentNode.scrollWidth) {
	      /*
	       * This seems to be a bug in every browser out there,
	       * except for Gecko ?
	       */
	      var sl = obj.scrollLeft
		+ obj.parentNode.scrollWidth - obj.scrollWidth;
	      objX -= sl;
	    }
	  } else
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
      var start = -1;
      var end = -1;

      var val = $(elem).val();
      if (val) {
        var range = document.selection.createRange().duplicate();

        range.moveEnd("character", val.length);
        start = (range.text == "" ? val.length : val.lastIndexOf(range.text));

        range = document.selection.createRange().duplicate();
        range.moveStart("character", -val.length);
        end = range.text.length;
      }

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

var repeatT = null, repeatI = null;

this.eventRepeat = function(fun, startDelay, repeatInterval) {
  WT.stopRepeat();

  startDelay = startDelay || 500;
  repeatInterval = repeatInterval || 50;

  fun();

  repeatT = setTimeout(function() {
    repeatT = null;
    fun();
    repeatI = setInterval(fun, repeatInterval);
  }, startDelay);
};

this.stopRepeat = function() {
  if (repeatT) {
    clearTimeout(repeatT);
    repeatT = null;
  }

  if (repeatI) {
    clearInterval(repeatI);
    repeatI = null;
  }
};

var cacheC = null, cacheS = null;

this.css = function(c, s) {
  if (c.style[s])
    return c.style[s];
  else {
    if (c !== cacheC) {
      cacheC = c;

      if (window.getComputedStyle)
	cacheS = window.getComputedStyle(c, null);
      else if (c.currentStyle)
	cacheS = c.currentStyle;
      else
	cacheS = null;
    }

    return cacheS ? cacheS[s] : null;
  }
};

function parseCss(value, regex, defaultvalue) {
  if (value == 'auto' || value == null)
    return defaultvalue;
  var m = regex.exec(value),
      v = m && m.length == 2 ? m[1] : null;
  return v ? parseFloat(v) : defaultvalue;
}

this.parsePx = function(v) {
  return parseCss(v, /^\s*(-?\d+(?:\.\d+)?)\s*px\s*$/i, 0);
};

function parsePct(v, defaultValue) {
  return parseCss(v, /^\s*(-?\d+(?:\.\d+)?)\s*\%\s*$/i, defaultValue);
}

// Get an element metric in pixels
this.px = function(c, s) {
  return WT.parsePx(WT.css(c, s));
};

// Get a widget style in pixels, when set directly
this.pxself = function(c, s) {
  return WT.parsePx(c.style[s]);
};

this.pctself = function(c, s) {
  return parsePct(c.style[s], 0);
};

this.cssPrefix = function(prop) {
  var prefixes = ['Moz', 'Webkit'],
    elem = document.createElement('div'),
    i, il;

  for (i = 0, il = prefixes.length; i < il; ++i) {
    if ((prefixes[i] + prop) in elem.style)
      return prefixes[i];
  }
}

this.boxSizing = function(w) {
  return (w.style['boxSizing']
	  || w.style['MozBoxSizing']
	  || w.style['WebkitBoxSizing']) === 'border-box';
};

// Return if an element (or one of its ancestors) is hidden
this.isHidden = function(w) {
  if (w.style.display == 'none')
    return true;
  else {
    w = w.parentNode;
    if (w && !WT.hasTag(w, "BODY"))
      return WT.isHidden(w);
    else
      return false;
  }
};

this.innerWidth = function(el) {
  var result = el.offsetWidth;
  if (!WT.boxSizing(el)) {
    result -= WT.px(el, 'paddingLeft') + WT.px(el, 'paddingRight')
	  + WT.px(el, 'borderLeftWidth') + WT.px(el, 'borderRightWidth');
  }
  return result;
};

this.innerHeight = function(el) {
  var result = el.offsetHeight;
  if (!WT.boxSizing(el)) {
    result -= WT.px(el, 'paddingTop') + WT.px(el, 'paddingBottom')
	  + WT.px(el, 'borderTopWidth') + WT.px(el, 'borderBottomWidth');
  }
  return result;
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
  if (!document.body.addEventListener)
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
  if (e && captureElement && (obj == captureElement) && e.type == "mouseup")
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

/* Firefox, IE9 etc... */
var inlineStyleSheet = null;

function getInlineStyleSheet() {
  if (!inlineStyleSheet) {
    var i, il, ds = document.styleSheets;
    for (i = 0, il = ds.length; i < il; ++i) {
      var s = ds[i];
      if (WT.hasTag(ds[i].ownerNode, 'STYLE')) {
	inlineStyleSheet = s;
	break;
      }
    }

    if (!inlineStyleSheet) {
      var s = document.createElement('style');
      document.getElementsByTagName('head')[0].appendChild(s);

      inlineStyleSheet = s.sheet;
    }
  }

  return inlineStyleSheet;
}

this.addCss = function(selector, style) {
  var s = getInlineStyleSheet();

  // strange error with IE9 when in iframe
  var pos = s.cssRules ? s.cssRules.length : 0;
  s.insertRule(selector + ' { ' + style + ' }', pos);
};

/* IE<9 & Konqueror */
this.addCssText = function(cssText) {
  var s = document.getElementById('Wt-inline-css');

  if (!s) {
    s = document.createElement('style');
    s.id = 'Wt-inline-css';
    document.getElementsByTagName('head')[0].appendChild(s);
  }

  if (!s.styleSheet) { // Konqueror
    var t = document.createTextNode(cssText);
    s.appendChild(t);
  } else {
    var ss = s.previousSibling;
    if (!ss
	|| !WT.hasTag(ss, "STYLE")
	|| ss.styleSheet.cssText.length > 32*1024) {
      ss = document.createElement('style');
      s.parentNode.insertBefore(ss, s);
      ss.styleSheet.cssText = cssText;
    } else {
      ss.styleSheet.cssText += cssText;
    }
  }
};

// from: http://www.hunlock.com/blogs/Totally_Pwn_CSS_with_Javascript
this.getCssRule = function(selector, deleteFlag) {
  selector = selector.toLowerCase();

  if (document.styleSheets) {
    for (var i=0; i<document.styleSheets.length; i++) {
      var styleSheet = document.styleSheets[i];
      var ii = 0;
      var cssRule;
      do {
	cssRule = null;
	try {
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
	} catch (err) {
	  /*
	     firefox security error 1000 when access a stylesheet.cssRules hosted
	     from another domain
	   */
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

  if (!e.offsetParent)
    return;

  var ow = WT.widgetPageCoordinates(e.offsetParent);

  var hsides = [ 'left', 'right' ],
      vsides = [ 'top', 'bottom' ],
      ew = WT.px(e, 'maxWidth') || e.offsetWidth,
      eh = WT.px(e, 'maxHeight') || e.offsetHeight,
      hside, vside;

  if (ew > ws.x) { // wider than window
    x = wx;
    hside = 0;
  } else if (x + ew > wx + ws.x) { // too far right, chose other side
    rightx -= ow.x;
    x = e.offsetParent.offsetWidth - (rightx + WT.px(e, 'marginRight'));
    hside = 1;
  } else {
    x -= ow.x;
    x = x - WT.px(e, 'marginLeft');
    hside = 0;
  }

  if (ew > ws.y) { // taller than window
    y = wy;
    vside = 0;
  } else if (y + eh > wy + ws.y) { // too far below, chose other side
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

this.positionAtWidget = function(id, atId, orientation, parentInRoot,
				 autoShow) {
  var w = WT.getElement(id),
    atw = WT.getElement(atId);

  if (!atw || !w)
    return;

  var xy = WT.widgetPageCoordinates(atw),
    x, y, rightx, bottomy;

  if (parentInRoot) {
    w.parentNode.removeChild(w);
    $('.Wt-domRoot').get(0).appendChild(w);
  }

  w.style.position = 'absolute';
  if (autoShow)
    w.style.display = 'block';

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

var html5History = !WT.isMobileWebKit
    && !!(window.history && window.history.pushState);

/*
 * A less aggressive URL encoding than encodeURIComponent which does
 * for example not encode '/'
 */
function gentleURIEncode(s) {
  return s.replace(/%/g, '%25')
    .replace(/\+/g, '%2b')
    .replace(/ /g, '%20')
    .replace(/#/g, '%23')
    .replace(/&/g, '%26');
}

if (html5History) {
  this.history = (function()
{
  var currentState = null, baseUrl = null, cb = null;

  return {
    _initialize: function() { },

    _initTimeout: function() { },

    register: function (initialState, onStateChange) {
      currentState = initialState;
      cb = onStateChange;
      var expectNullState = WT.isWebKit;

      function onPopState(event) {
	var newState = event.state;

	/*
	 * A null state event is given onload, but we may already have
	 * pushed another state... this is simply silly.
	 *
	 * see http://html5.org/tools/web-apps-tracker?from=5345&to=5346
	 */

	if (!newState)
	  if (expectNullState) {
	    expectNullState = false;
	    return;
	  } else
	    newState = initialState;

	if (newState != currentState) {
	  currentState = newState;
	  onStateChange(currentState != "" ? currentState : "/");
	}
      }

      function onHashChange() {
	var p = window.location.hash;
	var newState = null;
	if (p == '')
	  newState = p;
	else if (p.substr(0, 2) == '#/')
	  newState = p.substr(2);
	if (newState !== currentState) {
	  currentState = newState;
	  onStateChange(currentState);
	}
      }

      window.addEventListener("popstate", onPopState, false);
      window.addEventListener("hashchange", onHashChange, false);
    },

    initialize: function (stateField, histFrame, deployUrl) {
      WT.resolveRelativeAnchors();

      baseUrl = deployUrl;
      if (baseUrl.length >= 1 && baseUrl[baseUrl.length - 1] == '/') {
_$_$if_UGLY_INTERNAL_PATHS_$_();
	baseUrl += "?_=";
_$_$endif_$_();
_$_$ifnot_UGLY_INTERNAL_PATHS_$_();
	baseUrl = baseUrl.substr(0, baseUrl.length - 1);
_$_$endif_$_();
      }
    },

    navigate: function (state, generateEvent) {
      WT.resolveRelativeAnchors();

      currentState = state;

      var url = baseUrl + gentleURIEncode(state);

      if (baseUrl.length < 3 || baseUrl.substr(baseUrl.length - 3) != "?_=") {
	url += window.location.search;
      } else {
	function stripHashParameter(q) {
	  if (q.length > 1)
	    q = q.substr(1);

	  var qp = q.split("&"), i, il;
	  q = "";

	  for (i=0, il = qp.length; i<il; ++i)
	    if (qp[i].split("=")[0] != '_')
	      q += (q.length ? '&' : '?') + qp[i];

	  return q;
	}

	var q = stripHashParameter(window.location.search);
	if (q.length > 1)
	  url += '&' + q.substr(1);
      }

      try {
	window.history.pushState(state ? state : "", document.title, url);
      } catch (error) {
	/*
	 * In case we are wrong about our baseUrl or base href
	 * In any case, this shouldn't be fatal.
	 */
	console.log(error.toString());
      }

      WT.scrollIntoView(state);

      if (generateEvent)
	cb(state);
    },

    getCurrentState: function () {
      return currentState;
    }
  };
})();

} else {
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
  var _onStateChange = [];
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
  function onStateChange() {
    var i, il;
    for (i = 0, il = _onStateChange.length; i < il; ++i) {
      _onStateChange[i](unescape(_currentState));
    }
  }
  function _handleFQStateChange(fqstate) {
    var currentState;
    if (!fqstate) {
      _currentState = _initialState;
      onStateChange();
      return;
    }
    currentState = fqstate;
    if (!currentState || _currentState !== currentState) {
      _currentState = currentState || _initialState;
      onStateChange();
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
    } else
      _initialState = _currentState = "";

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
  _initialize: function() {
    if (_stateField != null)
      _initialize();
  },
  _initTimeout: function() {
      _initTimeout();
  },
  register: function (initialState, onStateChange) {
    if (!_initialized) {
      _initialState = escape(initialState);
      _currentState = _initialState;
    }
    _onStateChange.push(onStateChange);
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
  navigate: function (state, generateEvent) {
    state = gentleURIEncode(state);

    if (!_initialized) {
      return;
    }
    var fqstate = state;
    if (_UAie) {
      _updateIFrame(fqstate);
    } else {
      location.hash = fqstate;
      if (_UAwebkit) {
	_fqstates[history.length] = fqstate;
	_storeStates();
      }
    }
    if (generateEvent)
      onStateChange();
  },
  getCurrentState: function () {
    if (!_initialized) {
      return "";
    }
    return unescape(_currentState);
  }
  };
})();

}

})();

if (window._$_APP_CLASS_$_) {
  try {
    window._$_APP_CLASS_$_._p_.quit();
  } catch (e) {
  }
}

window._$_APP_CLASS_$_ = new (function() {

var self = this;
var WT = _$_WT_CLASS_$_;

var downX = 0;
var downY = 0;

var deployUrl = _$_DEPLOY_PATH_$_;

function saveDownPos(e) {
  var coords = WT.pageCoordinates(e);
  downX = coords.x;
  downY = coords.y;
};

var currentHash = null;

function onHashChange() {
  var newLocation = _$_WT_CLASS_$_.history.getCurrentState();
  if (currentHash == newLocation) {
    return;
  } else {
    currentHash = newLocation;
    setTimeout(function() { update(null, 'hash', null, true); }, 1);
  }
};

function setHash(newLocation) {
  if (currentHash == newLocation || !currentHash && newLocation == '/')
    return;

  currentHash = newLocation;

  WT.history.navigate(newLocation, false);
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
  window.onresize = function() { doJavaScript(); };

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
  ds.object.style["z-index"] = '1000';
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

  WT.cancelEvent(e, WT.CancelPropagate);

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
    if (i != 0)
      result += ';';
    result += [ t.identifier,
		t.clientX, t.clientY,
		t.pageX, t.pageY,
		t.screenX, t.screenY,
		t.pageX - widgetCoords.x, t.pageY - widgetCoords.y ].join(';');
  }

  return result;
}

var formObjects = [];

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

    if (el.wtEncodeValue)
      v = el.wtEncodeValue(el);
    else if (el.type == 'select-multiple') {
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
    } else if (el.type != 'file') {
      if ($(el).hasClass('Wt-edit-emptyText'))
	v = '';
      else {
	/* For WTextEdit */
	if (el.ed)
	  el.ed.save();
	v = '' + el.value;
      }

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
    result += se + '_=' + encodeURIComponent(currentHash);

  if (!e) {
    event.data = result;
    return event;
  }

  var t = WT.target(e);
  while (t && !t.id && t.parentNode)
    t = t.parentNode;
  if (t && t.id)
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

var sessionUrl,
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
    clearInterval(keepAliveTimer);
    keepAliveTimer = null;
  }
  var tr = $('#Wt-timers');
  if (tr.size() > 0)
    WT.setHtml(tr.get(0), '', false);
}

function doKeepAlive() {
  WT.history._initTimeout();
  if (commErrors == 0)
    update(null, 'none', null, false);
}

function debug(s) {
  document.body.innerHTML += s;
}

function setTitle(title) {
  if (WT.isIEMobile) return;
  document.title = title;
}

function load(fullapp) {
  if (loaded)
    return;

  if (fullapp) {
    if (!window._$_APP_CLASS_$_LoadWidgetTree)
      return; // That's too soon baby.

    WT.history.initialize("Wt-history-field", "Wt-history-iframe", deployUrl);
  }

  if (!("activeElement" in document)) {
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
  loaded = true;

  if (fullapp)
    window._$_APP_CLASS_$_LoadWidgetTree();

  if (!quited) {
    if (!keepAliveTimer) {
      keepAliveTimer = setInterval(doKeepAlive, _$_KEEP_ALIVE_$_000);
    }
  }
}

var currentHideLoadingIndicator = null;

function cancelFeedback(timer) {
  clearTimeout(timer);
  document.body.style.cursor = 'auto';

  if (currentHideLoadingIndicator != null) {
    try {
      currentHideLoadingIndicator();
    } catch (e) {
    }
    currentHideLoadingIndicator = null;
  }
}

function waitFeedback() {
  document.body.style.cursor = 'wait';
  currentHideLoadingIndicator = hideLoadingIndicator;
  showLoadingIndicator();
}

/** @const */ var WebSocketsUnknown = 0;
/** @const */ var WebSocketsWorking = 1;
/** @const */ var WebSocketsUnavailable = 2;

var websocket = {
  state: WebSocketsUnknown,
  socket: null,
  keepAlive: null,
  reconnectTries: 0
};

function setServerPush(how) {
  serverPush = how;
}

var autoJavaScriptScheduled = false;
function doAutoJavaScript() {
  if (autoJavaScriptScheduled)
    return;

  autoJavaScriptScheduled = true;

  setTimeout(function() { autoJavaScriptScheduled = false;
			  self._p_.autoJavaScript(); }, 1);
}

function doJavaScript(js) {
  if (js) {
    js = "(function() {" + js + "})();";
    if (window.execScript)
      window.execScript(js);
    else
      window.eval(js);
  }

  if (self == window._$_APP_CLASS_$_)
    doAutoJavaScript();
}

function handleResponse(status, msg, timer) {
  if (quited)
    return;

  if (status == 0) {
    WT.resolveRelativeAnchors();
_$_$if_CATCH_ERROR_$_();
    try {
_$_$endif_$_();
      doJavaScript(msg);
_$_$if_CATCH_ERROR_$_();
    } catch (e) {
      var stack = null;

_$_$if_SHOW_STACK_$_();
      stack = e.stack || e.stacktrace;
_$_$endif_$_();
      alert("Wt internal error: " + e + ", code: " +  e.code
	    + ", description: " + e.description
	    + (stack ? (", stack:\n" + stack) : ""));
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

function setSessionUrl(url) {
  if (url.indexOf("://") != -1 || url[0] == '/')
    sessionUrl = url;
  else
    sessionUrl = deployUrl + url;

  if (comm)
    comm.setUrl(url);
}

setSessionUrl(_$_SESSION_URL_$_);

var comm = WT.initAjaxComm(sessionUrl, handleResponse);

function doPollTimeout() {
  responsePending.abort();
  responsePending = null;
  pollTimer = null;

  if (!quited)
    sendUpdate();
}

var updating = false;

function update(el, signalName, e, feedback) {
  /*
   * Konqueror may recurisvely call update() because
   * /reading/ offsetLeft or offsetTop triggers an onscroll event ??
   */
  if (updating)
    return;

  updating = true;

  WT.checkReleaseCapture(el, e);

_$_$if_STRICTLY_SERIALIZED_EVENTS_$_();
  if (!responsePending) {
_$_$endif_$_();

  var pendingEvent = new Object(), i = pendingEvents.length;
  pendingEvent.object = el;
  pendingEvent.signal = signalName;
  pendingEvent.event = window.fakeEvent || e;
  pendingEvent.feedback = feedback;

  pendingEvents[i] = encodeEvent(pendingEvent, i);

  scheduleUpdate();

  doJavaScript();

_$_$if_STRICTLY_SERIALIZED_EVENTS_$_();
  }
_$_$endif_$_();

  updating = false;
}

var updateTimeoutStart;

function scheduleUpdate() {
  if (quited)
    return;

_$_$if_WEB_SOCKETS_$_();
  if (websocket.state != WebSocketsUnavailable) {
    if (typeof window.WebSocket === 'undefined'
        && typeof window.MozWebSocket === 'undefined')
      websocket.state = WebSocketsUnavailable;
    else {
      var ws = websocket.socket;

      if ((ws == null || ws.readyState > 1)) {
	if (ws != null && websocket.state == WebSocketsUnknown)
	  websocket.state = WebSocketsUnavailable;
	else {
	  function reconnect() {
	    ++websocket.reconnectTries;
	    var ms = Math.min(120000, Math.exp(websocket.reconnectTries) * 500);
	    setTimeout(function() { scheduleUpdate(); }, ms);
	  }

	  var protocolEnd = sessionUrl.indexOf("://"), wsurl;
	  if (protocolEnd != -1) {
	    wsurl = "ws" + sessionUrl.substr(4);
	  } else {
	    var query = sessionUrl.substr(sessionUrl.indexOf('?'));

	    wsurl = "ws" + location.protocol.substr(4)
	      + "//" + location.hostname + ":"
	     + location.port + deployUrl + query;
	  }

	  if (typeof window.WebSocket !== 'undefined')
	    websocket.socket = ws = new WebSocket(wsurl);
	  else
	    websocket.socket = ws = new MozWebSocket(wsurl);

	  if (websocket.keepAlive)
	    clearInterval(websocket.keepAlive);
	  websocket.keepAlive = null;

	  ws.onmessage = function(event) {
	    websocket.reconnectTries = 0;
	    websocket.state = WebSocketsWorking;
	    handleResponse(0, event.data, null);
	  };

	  ws.onerror = function(event) {
	    reconnect();
	  };

	  ws.onclose = function(event) {
	    reconnect();
	  };

	  ws.onopen = function(event) {
	    websocket.state = WebSocketsWorking;

	    /*
	     * WebSockets are suppossedly reliable, but there is nothing
	     * in the protocol that makes them so ?
	     */
	    websocket.keepAlive = setInterval
	    (function() {
	       if (ws.readyState == 1)
		 ws.send('&signal=none');
	       else {
		 clearInterval(websocket.keepAlive);
		 websocket.keepAlive = null;
	       }
	     }, 3 * _$_SERVER_PUSH_TIMEOUT_$_);
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

var ackUpdateId = _$_ACK_UPDATE_ID_$_, ackPuzzle = null;
function responseReceived(updateId, puzzle) {
  ackPuzzle = puzzle;
  ackUpdateId = updateId;
  comm.responseReceived(updateId);
}

var pageId = 0;
function setPage(id)
{
  pageId = id;
}

function sendUpdate() {
  if (self != window._$_APP_CLASS_$_) {
    quit();
    return;
  }

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

  var data, tm, poll;

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

  data.result += '&ackId=' + ackUpdateId + '&pageId=' + pageId;

  if (ackPuzzle) {
    var solution = '';
    var d = $('#' + ackPuzzle).get(0);
    if (d) {
      d = d.parentNode;

      for (; !WT.hasTag(d, 'BODY'); d = d.parentNode) {
	if (d.id) {
	  if (solution != '') solution += ',';
	  solution += d.id;
	}
      }
    }

    data.result += '&ackPuzzle=' + encodeURIComponent(solution);
  }

  var params = "_$_PARAMS_$_";
  if (params.length > 0)
    data.result += '&' + params;

  if (websocket.socket != null && websocket.socket.readyState == 1) {
    responsePending = null;

    if (tm != null) {
      clearTimeout(tm);
      tm = null;
    }

    if (!poll) {
      websocket.socket.send(data.result);
    }
  } else {
    responsePending = comm.sendUpdate
      ('request=jsupdate' + data.result, tm, ackUpdateId, -1);

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
    var t = tries === undefined ? (WT.isIE ? 1 : 2) : tries;
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
	if (WT.isOpera || WT.isIE) {
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

function enableInternalPaths(initialHash) {
  currentHash = initialHash;
  WT.history.register(initialHash, onHashChange);
}

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
    scheduleUpdate();
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
  setSessionUrl : setSessionUrl,
  setFormObjects : function(o) { formObjects = o; },
  saveDownPos : saveDownPos,
  addTimerEvent : addTimerEvent,
  load : load,
  setServerPush : setServerPush,

  dragStart : dragStart,
  dragDrag : dragDrag,
  dragEnd : dragEnd,
  capture : WT.capture,

  enableInternalPaths : enableInternalPaths,
  onHashChange : onHashChange,
  setHash : setHash,
  ImagePreloader : ImagePreloader,

  doAutoJavaScript : doAutoJavaScript,
  autoJavaScript : function() { },

  response : responseReceived,
  setPage : setPage,
  setCloseMessage : setCloseMessage
};

this.WT = _$_WT_CLASS_$_;
this.emit = emit;

})();

window._$_APP_CLASS_$_SignalEmit = _$_APP_CLASS_$_.emit;

window._$_APP_CLASS_$_OnLoad = function() {
  _$_APP_CLASS_$_._p_.load();
};
