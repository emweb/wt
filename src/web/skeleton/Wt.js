
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
    _$_WT_CLASS_$_[name] = function() {
	return fn.apply(_$_WT_CLASS_$_, arguments);
    };
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
    app[name] = function() { return fn.apply(app, arguments); };
  } else {
    app[name] = fn;
  }
};

_$_$endif_$_();

if (!window._$_WT_CLASS_$_)
  window._$_WT_CLASS_$_ = new (function()
{
var WT = this;
/** @const */ var UNDEFINED = 'undefined';
/** @const */ var UNKNOWN = 'unknown'; // seen on IE for reasons unknown

this.condCall = function(o, f, a) {
  if (o[f])
    o[f](a);
};

// buttons currently down
this.buttons = 0;

// button last released (for reporting in IE's click event)
var lastButtonUp = 0, mouseDragging = 0;

// returns the button associated with the event (0 if none)
this.button = function(e)
{
  try {
    var t = e.type;

    if (WT.isIE && (t == "click" || t == "dblclick"))
      return lastButtonUp; // IE does not provide button information then

    if (t != "mouseup" && t != "mousedown" && t != "click" && t != "dblclick")
      return 0;
  } catch (e) {
    return 0;
  }

  if (!WT.isGecko && 
      typeof e.which !== UNDEFINED && 
      typeof e.which !== UNKNOWN) {
    if (e.which == 3)
      return 4;
    else if (e.which == 2)
      return 2;
    else if (e.which == 1)
      return 1;
    else
      return 0;
  } else if (WT.isIE && 
	     typeof e.which !== UNDEFINED &&
	     typeof e.which !== UNKNOWN) {
    if (e.button == 2)
      return 4;
    else if (e.button == 4)
      return 2;
    else if (e.button == 1)
      return 1;
    else
      return 0;
  } else if (typeof e.which !== UNDEFINED &&
	     typeof e.which !== UNKNOWN) {
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
  lastButtonUp = WT.button(e);
  WT.buttons &= ~lastButtonUp;

  /*
   * mouse click will follow immediately and should still see the old
   * value of mouseDragging
   */
  setTimeout(function() {
    mouseDragging = 0;
  }, 5);
};

/*
 * Used to prevent a mouse click if we're actually dragging
 */
this.dragged = function(e) {
  return mouseDragging > 2;
};

this.drag = function(e) {
  ++mouseDragging;
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
this.isIE8 = ie === 8;
this.isIElt9 = ie < 9;
this.isIEMobile = agent.indexOf("msie 4")!=-1 || agent.indexOf("msie 5")!=-1;
this.isOpera = typeof window.opera !== UNDEFINED;
this.isAndroid = (agent.indexOf("safari") != -1)
		  && (agent.indexOf("android") != -1);
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
  var crossDomain = 
    (url.indexOf("://") != -1 || url.indexOf("//") == 0) &&
    host(url) != window.location.host;

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

	  if (!sessionUrl)
	    return;

	  if (good) {
	    handled = true;
	    handler(0, request.responseText, userData);
	  } else {
	    handler(1, null, userData); 
	  }

	  if (request) {
	    request.onreadystatechange = new Function;
	    try {
	      request.onload = request.onreadystatechange;
	    } catch (e) {
	      /*
	       * See comment below.
	       */
	    }
	    request = null;
	  }

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
	  if (!sessionUrl)
	    return;

	  request.onreadystatechange = new Function;
	  request = null;
	  handled = true;
	  handler(2, null, userData);
	}

	this.abort = function() {
	  if (request != null) {
	    request.onreadystatechange = new Function;
	    handled = true;
	    request.abort();
	    request = null;
	  }
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
	if (!sessionUrl)
	  return null;
	return new Request(data, userData, id, timeout);
      };

      this.cancel = function() {
	sessionUrl = null;
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
	if (!sessionUrl)
	  return null;
        request = new Request(data, userData, id, timeout);
        return request;
      };

      this.cancel = function() {
	sessionUrl = null;
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
    else {
      WT.saveReparented(el);
      el.innerHTML = html;
    }
  } else {
    var d, b;
    d = new DOMParser();
    b = d.parseFromString('<div>'+html+'<\/div>','application/xhtml+xml');
    d = b.documentElement;
    if (d.nodeType != 1) // element
      d = d.nextSibling;

    if (!add) {
      WT.saveReparented(el);
      el.innerHTML = '';
    }

    for (var i = 0, il = d.childNodes.length; i < il;)
      el.appendChild(myImportNode(d.childNodes[i++], true));
  }
};

this.hasTag = function(e, s) {
  return e.nodeType == 1 && e.tagName && e.tagName.toUpperCase() === s;
};

this.insertAt = function(p, c, pos) {
  if (!p.childNodes.length)
    p.appendChild(c);
  else {
    var i, j, il;
    for (i = 0, j = 0, il = p.childNodes.length; i < il; ++i) {
      if ($(p.childNodes[i]).hasClass("wt-reparented"))
         continue;
      if (j === pos) {
          p.insertBefore(c, p.childNodes[i]);
          return;
      }
      ++j;
    }
    p.appendChild(c);
  }
};

this.remove = function(id)
{
  // TODO(Roel): what about removal?
  var e = WT.getElement(id);
  if (e) {
    WT.saveReparented(e);
    e.parentNode.removeChild(e);
  }
};

this.replaceWith = function(w1Id, $w2)
{
  var $w1 = $("#" + w1Id);
  $w1.replaceWith($w2);

  /* Reapply client-side validation, bootstrap applys validation classes
     also outside the element into its ancestors */
  if ($w2.get(0).wtValidate && WT.validate) {
    setTimeout(function() { 
      WT.validate($w2.get(0));
    }, 0);
  }
}

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

  to.style.boxSizing = from.style.boxSizing;
  var attrName = WT.styleAttribute('box-sizing');
  var vendorPrefix = WT.vendorPrefix(attrName);
  if (vendorPrefix)
    to.style[attrName] = from.style[attrName];
};

this.saveReparented = function(el) {
  $(el).find('.wt-reparented').each(function() {
      var domRoot = $('.Wt-domRoot').get(0);
      domRoot.appendChild(this.parentNode.removeChild(this));
    });
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
    } else if (WT.hasTag(e, 'A') && e.href.indexOf('&signal=') != -1) {
        e.href = 'javascript:void(0)';
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
      } else {
	while (href.substr(0, 3) == "../")
	  href = href.substr(3);
	if (href.charAt(0) != '/')
	  href = '/' + href;
	internalPath = href.substr(basePath.length);
	if (internalPath.substr(0, 2) == "_=" &&
	    basePath.charAt(basePath.length - 1) == '?')
	  internalPath = '?' + internalPath;  /* eaten one too much */
      }

      if (internalPath.length == 0 || internalPath.charAt(0) != '/')
	internalPath = '/' + internalPath;
      if (internalPath.substr(0, 4) == "/?_=")
	internalPath = internalPath.substr(4);
      this.setAttribute('href', href); // computes this.href
      this.setAttribute('href', this.href);
      this.onclick = function(event) {
	WT.navigateInternalPath(event, internalPath);
      };
      $(this).removeClass("Wt-ip");
    });
};

this.resolveRelativeAnchors = function() {
  if (window.$)
    $('.Wt-rr').each(function() {
      if (this.href)
	this.setAttribute('href', this.href);
      if (this.src)
	this.setAttribute('src', this.src);

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

this.$ = this.getElement;

this.filter = function(edit, event, tokens) {
  var c = String.fromCharCode((typeof event.charCode !== UNDEFINED) ?
                              event.charCode : event.keyCode);
  if (!new RegExp(tokens).test(c))
    WT.cancelEvent(event);
};

// Get coordinates of element relative to an ancestor object (or page origin).
// It computes the location of the left-top corner of the margin-box.
this.widgetPageCoordinates = function(obj, reference) {
  var objX = 0, objY = 0, op;

  if (!obj.parentNode)
    return { x: 0, y: 0 };

  // bug in safari, according to W3C, offsetParent for an area element should
  // be the map element, but safari returns null.
  if (WT.hasTag(obj, "AREA"))
    obj = obj.parentNode.nextSibling; // img after map

  var rtl = $(document.body).hasClass('Wt-rtl');

  while (obj && obj !== reference) {
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

  var target = e.target || e.srcElement;

  // if this is an iframe, offset against the frame's position
  if (target && (target.ownerDocument != document))
    for (var i=0; i < window.frames.length; i++) {
      if (target.ownerDocument == window.frames[i].document) {
        try{
          var rect = window.frames[i].frameElement.getBoundingClientRect();
          posX = rect.left;
          posY = rect.top;
        }catch (e) {
        }
      }
    }
  
  if (e.touches && e.touches[0]) {
    return WT.pageCoordinates(e.touches[0]);
  } else if (!WT.isIE && e.changedTouches && e.changedTouches[0]) {
    posX += e.changedTouches[0].pageX;
    posY += e.changedTouches[0].pageY;
  } else if (typeof e.pageX === 'number') {
    posX += e.pageX; posY = e.pageY;
  } else if (typeof e.clientX === 'number') {
    posX += e.clientX + document.body.scrollLeft
      + document.documentElement.scrollLeft;
    posY += e.clientY + document.body.scrollTop
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

/**
 * @preserve Includes normalizeWheel from Fixed Data Tables for React by Facebook (BSD Licensed)
 */
this.normalizeWheel = function(event) {
  var PIXEL_STEP = 10;
  var LINE_HEIGHT = 40;
  var PAGE_HEIGHT = 800;

  var sX = 0,
      sY = 0,
      // spinX, spinY
  pX = 0,
      pY = 0; // pixelX, pixelY

  // Legacy
  if ('detail' in event) {
    sY = event.detail;
  }
  if ('wheelDelta' in event) {
    sY = -event.wheelDelta / 120;
  }
  if ('wheelDeltaY' in event) {
    sY = -event.wheelDeltaY / 120;
  }
  if ('wheelDeltaX' in event) {
    sX = -event.wheelDeltaX / 120;
  }

  // side scrolling on FF with DOMMouseScroll
  if ('axis' in event && event.axis === event.HORIZONTAL_AXIS) {
    sX = sY;
    sY = 0;
  }

  pX = sX * PIXEL_STEP;
  pY = sY * PIXEL_STEP;

  if ('deltaY' in event) {
    pY = event.deltaY;
  }
  if ('deltaX' in event) {
    pX = event.deltaX;
  }

  if ((pX || pY) && event.deltaMode) {
    if (event.deltaMode == 1) {
      // delta in LINE units
      pX *= LINE_HEIGHT;
      pY *= LINE_HEIGHT;
    } else {
      // delta in PAGE units
      pX *= PAGE_HEIGHT;
      pY *= PAGE_HEIGHT;
    }
  }

  // Fall-back if spin cannot be determined
  if (pX && !sX) {
    sX = pX < 1 ? -1 : 1;
  }
  if (pY && !sY) {
    sY = pY < 1 ? -1 : 1;
  }

  return { spinX: sX,
    spinY: sY,
    pixelX: pX,
    pixelY: pY };
}

this.wheelDelta = function(e) {
  var delta = 0;
  if (e.deltaY) { /* WheelEvent */
    delta = e.deltaY > 0 ? -1 : 1;
  } else if (e.wheelDelta) { /* IE/Opera. */
    delta = e.wheelDelta > 0 ? 1 : -1;
    /* if (window.opera)
       delta = -delta; */
  } else if (e.detail) {
    delta = e.detail < 0 ? 1 : -1;
  }
  return delta;
};

this.scrollIntoView = function(id) {
  setTimeout(function() {
      var hashI = id.indexOf('#');
      if (hashI != -1)
	id = id.substr(hashI + 1);

      var obj = document.getElementById(id);
      if (obj) {
	/* Locate a suitable ancestor to scroll */
	var p;
	for (p = obj.parentNode; p != document.body; p = p.parentNode) {
	  if (p.scrollHeight > p.clientHeight &&
	      WT.css(p, 'overflow-y') == 'auto') {
	    var xy = WT.widgetPageCoordinates(obj, p);
	    p.scrollTop += xy.y;
	    return;
	  }
	}

	obj.scrollIntoView(true);
      }
    }, 100);
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

  if (typeof elem.selectionStart !== UNDEFINED) {
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

  var charCode = (typeof e.charCode !== UNDEFINED) ? e.charCode : 0;

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

this.isDblClick = function(o, e) {
  if (o.wtClickTimeout &&
      Math.abs(o.wtE1.clientX - e.clientX) < 3 &&
      Math.abs(o.wtE1.clientY - e.clientY) < 3) {
      clearTimeout(o.wtClickTimeout);
      o.wtClickTimeout = null; o.wtE1 = null;
      return true;
  } else
      return false;
};

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

this.parsePct = function(v, defaultValue) {
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
  return WT.parsePct(c.style[s], 0);
};

// Convert from css property to element attribute (possibly a vendor name)
this.styleAttribute = function(cssProp) {
    function toCamelCase(str) {
	var n=str.search(/-./);
	while (n != -1) {
	    var letter = (str.charAt(n+1)).toUpperCase();
	    str = str.replace(/-./, letter);
	    n=str.search(/-./);
	}
	return str;
    }

    var prefixes = ['', '-moz-', '-webkit-', '-o-', '-ms-'];
    var elem = document.createElement('div'), i, il;

    for (i = 0, il = prefixes.length; i < il; ++i) {
	var attr = toCamelCase(prefixes[i] + cssProp);
	if (attr in elem.style)
	    return attr;
    }
    return toCamelCase(cssProp);
};

this.vendorPrefix = function(attr) {
    var prefixes = ['Moz', 'Webkit', 'O', 'Ms'];
    for (i = 0, il = prefixes.length; i < il; ++i) {
	if (attr.search(prefixes[i]) != -1)
	    return prefixes[i];
    }
    return '';
};

this.boxSizing = function(w) {
  return (WT.css(w, WT.styleAttribute('box-sizing'))) === 'border-box';
};

// Return if an element (or one of its ancestors) is hidden
this.isHidden = function(w) {
  if (w.style.display == 'none' || $(w).hasClass('out'))
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

    min = WT.parsePct(min, 0);
    max = WT.parsePct(max, 100000);

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
  try {
    return WT.firedTarget || event.target || event.srcElement;
  } catch (err) {
    return null;
  }
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

function attachMouseHandlers(el) {
  if (el.addEventListener) {
    el.addEventListener('mousemove', mouseMove, true);
    el.addEventListener('mouseup', mouseUp, true);

    if (WT.isGecko) {
      window.addEventListener('mouseout', function(e) {
				if (!e.relatedTarget
				    && WT.hasTag(e.target, "HTML"))
				  mouseUp(e);
			      }, true);
    }
  } else {
    el.attachEvent('onmousemove', mouseMove);
    el.attachEvent('onmouseup', mouseUp);
  }
}

function initCapture() {
  if (captureInitialized)
    return;

  captureInitialized = true;

  var db = document.body;
  attachMouseHandlers(db);
}

this.capture = function(obj) {
  initCapture();

  if (captureElement && obj)
    return;

  // attach to possible iframes
  for (var i=0; i < window.frames.length; i++)
    try{
      if (! window.frames[i].document.body.hasMouseHandlers) {
        attachMouseHandlers(window.frames[i].document.body);
        window.frames[i].document.body.hasMouseHandlers = true;
      }
    }catch (e) {
    }

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
	     firefox security error 1000 when access a stylesheet.cssRules
	     hosted from another domain
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
    s.setAttribute('href', uri);
    s.setAttribute('type','text/css');
    s.setAttribute('rel','stylesheet');
    if (media != '' && media != 'all')
      s.setAttribute('media', media);
    var ll = document.getElementsByTagName('link');
    if (ll.length > 0) {
      var l = ll[ll.length - 1];
      l.parentNode.insertBefore(s, l.nextSibling);
    } else {
      document.body.appendChild(s);
    }
  }
};

this.removeStyleSheet = function(uri) {
  if ($('link[rel=stylesheet][href~="' + uri + '"]'))
    $('link[rel=stylesheet][href~="' + uri + '"]').remove();
  var sheets = document.styleSheets;
  for (var i=0; i<sheets.length; ++i) {
    var sheet = sheets[i];
    j = 0;
    if (sheet) {
      var rule = null;
      do {
        try {
          if (sheet.cssRules)
            rule = sheet.cssRules[j]; // firefox
          else
            rule = sheet.rules[j];    // IE

          if (rule && rule.cssText ===
              "@import url(\"" + uri +  "\");") {
            if (sheet.cssRules)
              sheet.deleteRule(j);// firfox
            else
              sheet.removeRule(j);//IE
            break; // only remove 1 rule !!!!
          }
        } catch (err) {
          /*
        * firefox security error 1000 when access a stylesheet.cssRules
        * hosted from another domain
        */
        }
        ++j;
      } while(rule)
    }
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
  var hsides = [ 'left', 'right' ],
      vsides = [ 'top', 'bottom' ];

  e.style[hsides[0]] = e.style[hsides[1]] = 'auto';
  e.style[vsides[0]] = e.style[vsides[1]] = 'auto';

  var reserveWidth = e.offsetWidth,
      reserveHeight = e.offsetHeight,
      hside, vside,
      windowSize = WT.windowSize(),
      windowX = document.body.scrollLeft + document.documentElement.scrollLeft,
      windowY = document.body.scrollTop + document.documentElement.scrollTop;

    /*
     * Should really distinguish between static versus dynamic: for a
     * widget that can grow dynamically (e.g. a suggestion popup) we
     * should prepare ourselves and consider maximum size here
     */
  if (!$(e).hasClass('Wt-tooltip')) {
    reserveWidth = WT.px(e, 'maxWidth') || reserveWidth;
    reserveHeight = WT.px(e, 'maxHeight') || reserveHeight;
  }

  var op = e.offsetParent;
  if (!op)
    return;

  var offsetParent = WT.widgetPageCoordinates(op);

  if (reserveWidth > windowSize.x) {
    // wider than window
    x = windowX;
    hside = 0;
  } else if (x + reserveWidth > windowX + windowSize.x) {
    // too far right, chose other side
    var scrollX = op.scrollLeft;
    if (op == document.body)
      scrollX = (op.clientWidth - windowSize.x);
    rightx = rightx - offsetParent.x + scrollX;
    x = op.clientWidth - (rightx + WT.px(e, 'marginRight'));
    hside = 1;
  } else {
    var scrollX = op.scrollLeft;
    if (op == document.body)
      scrollX = 0;
    x = x - offsetParent.x + scrollX;
    x = x - WT.px(e, 'marginLeft');
    hside = 0;
  }

  if (reserveHeight > windowSize.y) {
    // taller than window
    y = windowY;
    vside = 0;
  } else if (y + reserveHeight > windowY + windowSize.y) {
    // too far below, chose other side
    if (bottomy > windowY + windowSize.y)
      bottomy = windowY + windowSize.y;
    var scrollY = op.scrollTop;
    if (op == document.body)
      scrollY = (op.clientHeight - windowSize.y);
    bottomy = bottomy - offsetParent.y + scrollY;
    y = op.clientHeight - 
	  (bottomy + WT.px(e, 'marginBottom') + WT.px(e, 'borderBottomWidth'));
    vside = 1;
  } else {
    var scrollY = op.scrollTop;
    if (op == document.body)
      scrollY = 0;
    y = y - offsetParent.y + scrollY;
    y = y - WT.px(e, 'marginTop') + WT.px(e, 'borderTopWidth');
    vside = 0;
  }

  /*
  if (x < wx)
    x = wx + ws.x - e.offsetWidth - 3;
  if (y < wy)
    y = wy + ws.y - e.offsetHeight - 3;
  */

  e.style[hsides[hside]] = x + 'px';
  e.style[vsides[vside]] = y + 'px';
};

this.positionXY = function(id, x, y) {
  var w = WT.getElement(id);

  if (!WT.isHidden(w)) {
    w.style.display = 'block';
    WT.fitToWindow(w, x, y, x, y);
  }
};

this.Horizontal = 0x1;
this.Vertical = 0x2;

this.positionAtWidget = function(id, atId, orientation, delta) {
  var w = WT.getElement(id),
    atw = WT.getElement(atId);

  if (!delta)
    delta = 0;

  if (!atw || !w)
    return;

  var xy = WT.widgetPageCoordinates(atw),
    x, y, rightx, bottomy;

  w.style.position = 'absolute';
  if (WT.css(w, 'display') == 'none')
    w.style.display = 'block';

  if (orientation === WT.Horizontal) {
    x = xy.x + atw.offsetWidth;
    y = xy.y + delta;
    rightx = xy.x;
    bottomy = xy.y + atw.offsetHeight - delta;
  } else {
    x = xy.x;
    y = xy.y + atw.offsetHeight;
    rightx = xy.x + atw.offsetWidth;
    bottomy = xy.y;
  }

  /*
   * Reparent the widget in a suitable parent:
   *  an ancestor of w which isn't overflowing
   */
  if (!w.wtNoReparent && !$(w).hasClass("wt-no-reparent")) {
    var p, pp = atw, domRoot = $('.Wt-domRoot').get(0);
    w.parentNode.removeChild(w);
  
    for (p = pp.parentNode; p != domRoot; p = p.parentNode) {
      if (p.wtResize) {
	p = pp;
	break;
      }

      // e.g. a layout widget has clientHeight=0 since it's relative
      // with only absolutely positioned children. We are a bit more liberal
      // here to catch other simular situations, and 100px seems like space
      // needed anyway?
      if (WT.css(p, 'display') != 'inline' &&
	  p.clientHeight > 100 &&
	  (p.scrollHeight > p.clientHeight ||
	   p.scrollWidth > p.clientWidth)) {
	break;
      }

      pp = p;
    }

    var posP = WT.css(p, 'position');
    if (posP != 'absolute' && posP != 'relative')
      p.style.position = 'relative';
    
    p.appendChild(w);
    $(w).addClass('wt-reparented');
  }

  WT.fitToWindow(w, x, y, rightx, bottomy);

  w.style.visibility='';
};

this.hasFocus = function(el) {
  try {
    return el === document.activeElement;
  } catch(e) {
    return false;
  }
};

this.progressed = function(domRoot) {
  var doc = document, db = doc.body;
  var form = this.getElement('Wt-form');

  domRoot.style.display = form.style.display;
  form.parentNode.replaceChild(domRoot, form);

  if (db.removeEventListener)
    db.removeEventListener('click', delayClick, true);
  else
    db.detachEvent('click', delayClick);

  setTimeout(function() {
      var i, il;
      for (i = 0, il = delayedClicks.length; i < il; ++i) {
	if (doc.createEvent) {
	  var e = delayedClicks[i];
	  var ec = doc.createEvent('MouseEvents');
	  ec.initMouseEvent('click', e.bubbles, e.cancelable, window,
			    e.detail, e.screenX, e.screenY,
			    e.clientX, e.clientY, e.ctrlKey, e.altKey,
			    e.shiftKey, e.metaKey, e.button, null);
	  var el = WT.getElement(e.targetId);
	  if (el)
	    el.dispatchEvent(ec);
	} else {
	  var e = delayedClicks[i];
	  var ec = doc.createEventObject();
	  for (i in e)
	    ec[i] = e[i];
	  var el = WT.getElement(e.targetId);
	  if (el)
	    el.fireEvent('onclick', ec);
	}
      }
    }, 0);
};

var html5History = !!(window.history && window.history.pushState);

/*
 * A less aggressive URL encoding than encodeURIComponent which does
 * for example not encode '/'
 */
function gentleURIEncode(s) {
  return s.replace(/%/g, '%25')
    .replace(/\+/g, '%2b')
    .replace(/ /g, '%20')
    //.replace(/#/g, '%23')
    .replace(/&/g, '%26');
}

if (html5History) {
  this.history = (function()
{
  var currentState = null, baseUrl = null, ugly = false, cb = null,
      stateMap = { }, w = window;

  function saveState(state) {
    stateMap[w.location.pathname + w.location.search] = state;
  }

  return {
    _initialize: function() { },

    _initTimeout: function() { },

    register: function (initialState, onStateChange) {
      currentState = initialState;
      cb = onStateChange;
      saveState(initialState);

      function onPopState(event) {
	var newState = event.state;

	if (newState == null)
	  newState = stateMap[w.location.pathname + w.location.search];

	if (newState == null) {
	  var endw = w.location.pathname.lastIndexOf(currentState);
	  if (endw != -1 &&
	      endw == w.location.pathname.length - currentState.length) {
	    saveState(currentState);
	    return;
	  } else {
	    newState = w.location.pathname.substr(baseUrl.length);
	  }
	}

	if (newState != currentState) {
	  currentState = newState;
	  onStateChange(currentState != "" ? currentState : "/");
	}
      }

      w.addEventListener("popstate", onPopState, false);
    },

    initialize: function (stateField, histFrame, deployUrl) {
      WT.resolveRelativeAnchors();

      baseUrl = deployUrl;
      if (baseUrl.length >= 1 && baseUrl[baseUrl.length - 1] == '/') {
_$_$if_UGLY_INTERNAL_PATHS_$_();
	ugly = true;
_$_$endif_$_();
_$_$ifnot_UGLY_INTERNAL_PATHS_$_();
	baseUrl = baseUrl.substr(0, baseUrl.length - 1);
_$_$endif_$_();
      }
    },

    navigate: function (state, generateEvent) {
      WT.resolveRelativeAnchors();

      currentState = state;

      var ip = gentleURIEncode(state), url = baseUrl;

      if (ip.length != 0)
	url += (ugly ? "?_=" : "") + ip;

      if (!ugly) {
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

	if (q.length > 1) {
	  if (q.length > 2 && q[0] == '?' && q[1] == '&')
	    q = q.substr(1);
	  if (url.indexOf('?') == -1)
	    url += '?' + q.substr(1);
	  else
	    url += '&' + q.substr(1);
	}
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
} else if (WT.isIE8) {
  this.history = (function()
{
  var currentState = null, cb = null, w = window;

  return {
    _initialize: function() { },

    _initTimeout: function() { },

    register: function (initialState, onStateChange) {
      currentState = initialState;
      cb = onStateChange;

      function onHashChange() {
	if (currentState != w.location.hash) {
	  currentState = w.location.hash.substring(1);
	  cb(currentState);
	}
      }

      w.onhashchange = onHashChange;
    },

    initialize: function (stateField, histFrame, deployUrl) {
    },

    navigate: function (state, generateEvent) {
      currentState = state;

      w.location.hash = state;

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
	  if (location.hash != newHash && 
	      location.hash.substring(1) != newHash)
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
      if (fqstate.length > 0) {
	location.hash = fqstate;
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

if (window._$_APP_CLASS_$_ && window._$_APP_CLASS_$_._p_) {
  try {
    window._$_APP_CLASS_$_._p_.quit(null);
  } catch (e) {
  }
}

window._$_APP_CLASS_$_ = new (function() {

var self = this;
var WT = _$_WT_CLASS_$_;
/** @const */ var UNDEFINED = 'undefined';
/** @const */ var UNKNOWN = 'unknown'; // seen on IE for reasons unknown

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

  if (newLocation != null &&
      newLocation.length > 0 &&
      newLocation.substr(0, 1) != '/')
    return;

  if (currentHash == newLocation)
    return;

  currentHash = newLocation;

  setTimeout(function() { update(null, 'hash', null, true); }, 1);
};

function setHash(newLocation, generateEvent) {
  if (currentHash == newLocation || (!currentHash && newLocation == '/'))
    return;

  if (!generateEvent)
    currentHash = newLocation;

  WT.history.navigate(newLocation, generateEvent);
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
  document.body.ondragstart=function() {
    return false;
  };
}

function dragStart(obj, e) {
  if (e.ctrlKey || WT.button(e) > 1) //Ignore drags with rigth click.
    return true;
  var t = WT.target(e);
  if (t) {
    /*
     * Ignore drags that start on a scrollbar (#1231)
     */
    if (WT.css(t, 'display') !== 'inline' &&
	(t.offsetWidth > t.clientWidth ||
	 t.offsetHeight > t.clientHeight)) {
      var wc = WT.widgetPageCoordinates(t);
      var pc = WT.pageCoordinates(e);
      var x = pc.x - wc.x;
      var y = pc.y - wc.y;
      if (x > t.clientWidth || y > t.clientHeight)
	return true;
    }
  }

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
    position: ds.object.style.position,
    display: ds.object.style.display,
    left: ds.object.style.left,
    top: ds.object.style.top,
    className: ds.object.className,
    parent: ds.object.parentNode,
    zIndex: ds.object.zIndex
  };

  ds.object.parentNode.removeChild(ds.object);
  ds.object.style.position = 'absolute';
  ds.object.className = ds.objectPrevStyle.className + '';
  ds.object.style.zIndex = '1000';
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
  e = e || window.event;
  if (dragState.object !== null) {
    var ds = dragState;
    var xy = WT.pageCoordinates(e);

    if (ds.object.style.display !== '' &&
	ds.xy.x !== xy.x &&
	ds.xy.y !== xy.y)
      ds.object.style.display = '';

    ds.object.style.left = (xy.x - ds.offsetX) + 'px';
    ds.object.style.top = (xy.y - ds.offsetY) + 'px';

    var prevDropTarget = ds.dropTarget;
    var t = WT.target(e);
    if (t == ds.object) {
      if (document.elementFromPoint) {
	ds.object.style['display']='none';
	t = document.elementFromPoint(e.clientX, e.clientY);
	ds.object.style['display']='';
      }
    }

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
	ds.object.className = ds.objectPrevStyle.className + ' Wt-valid-drop';
    } else
      ds.object.className = ds.objectPrevStyle.className + '';

    return false;
  }

  return true;
};

function dragEnd(e) {
  e = e || window.event;
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

    document.body.removeChild(ds.object);
    ds.objectPrevStyle.parent.appendChild(ds.object);

    ds.object.style.zIndex = ds.objectPrevStyle.zIndex;
    ds.object.style.position = ds.objectPrevStyle.position;
    ds.object.style.display = ds.objectPrevStyle.display;
    ds.object.style.left = ds.objectPrevStyle.left;
    ds.object.style.top = ds.objectPrevStyle.top;
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
		Math.round(t.clientX), Math.round(t.clientY),
		Math.round(t.pageX), Math.round(t.pageY),
		Math.round(t.screenX), Math.round(t.screenY),
		Math.round(t.pageX - widgetCoords.x),
		Math.round(t.pageY - widgetCoords.y) ].join(';');
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
    if (typeof e.type === 'string')
      result += se + 'type=' + e.type;
  } catch (e) {
  }

  if (typeof e.clientX !== UNDEFINED && 
      typeof e.clientX !== UNKNOWN)
    result += se + 'clientX=' + Math.round(e.clientX) + se
	+ 'clientY=' + Math.round(e.clientY);

  var pageCoords = WT.pageCoordinates(e);
  var posX = pageCoords.x;
  var posY = pageCoords.y;

  if (posX || posY) {
    result += se + 'documentX=' + Math.round(posX) + se
	  + 'documentY=' + Math.round(posY);
    result += se + 'dragdX=' + Math.round(posX - downX) + se
	  + 'dragdY=' + Math.round(posY - downY);

    var delta = WT.wheelDelta(e);
    result += se + 'wheel=' + Math.round(delta);
  }

  if (typeof e.screenX !== UNDEFINED &&
      typeof e.screenX !== UNKNOWN)
    result += se + 'screenX=' + Math.round(e.screenX) + se
	+ 'screenY=' + Math.round(e.screenY);

  var widgetCoords = { x: 0, y: 0 };

  if (event.object && event.object.nodeType != 9) {
    widgetCoords = WT.widgetPageCoordinates(event.object);
    var objX = widgetCoords.x;
    var objY = widgetCoords.y;

    if (typeof event.object.scrollLeft !== UNDEFINED &&
	typeof event.object.scrollLeft !== UNKNOWN) {
      result += se + 'scrollX=' + Math.round(event.object.scrollLeft)
	+ se + 'scrollY=' + Math.round(event.object.scrollTop)
	+ se + 'width=' + Math.round(event.object.clientWidth)
	+ se + 'height=' + Math.round(event.object.clientHeight);
    }

    result += se + 'widgetX=' + Math.round(posX - objX) + se
	  + 'widgetY=' + Math.round(posY - objY);
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

  if (typeof e.keyCode !== UNDEFINED && 
      typeof e.keyCode !== UNKNOWN)
    result += se + 'keyCode=' + e.keyCode;

  if (typeof e.type === 'string') {
    var charCode = 0;
    if (typeof e.charCode !== UNDEFINED) {
      if (e.type === 'keypress')
	charCode = e.charCode;
    } else {
      if (e.type === 'keypress')
	charCode = e.keyCode;
    }
    result += se + 'charCode=' + charCode;
  }

  if (typeof e.altKey !== UNDEFINED && 
      typeof e.altKey !== UNKNOWN &&
      e.altKey)
    result += se + 'altKey=1';
  if (typeof e.ctrlKey !== UNDEFINED &&
      typeof e.ctrlKey !== UNKNOWN &&
      e.ctrlKey)
    result += se + 'ctrlKey=1';
  if (typeof e.metaKey !== UNDEFINED &&
      typeof e.metaKey !== UNKNOWN &&
      e.metaKey)
    result += se + 'metaKey=1';
  if (typeof e.shiftKey !== UNDEFINED && typeof e.shiftKey !== UNKNOWN &&
      e.shiftKey)
    result += se + 'shiftKey=1';

  if (typeof e.touches !== UNDEFINED)
    result += encodeTouches(se + "touches", e.touches, widgetCoords);
  if (typeof e.targetTouches !== UNDEFINED)
    result += encodeTouches(se + "ttouches", e.targetTouches, widgetCoords);
  if (typeof e.changedTouches !== UNDEFINED)
    result += encodeTouches(se + "ctouches", e.changedTouches, widgetCoords);

  if (typeof e.scale !== UNDEFINED &&
      typeof e.scale !== UNKNOWN &&
      e.scale)
    result += se + "scale=" + e.scale;
  if (typeof e.rotation !== UNDEFINED &&
      typeof e.rotation !== UNKNOWN &&
      e.rotation)
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
  hasQuit = false,
  quitStr = _$_QUITTED_STR_$_,
  loaded = false,
  responsePending = null,
  pollTimer = null,
  keepAliveTimer = null,
  commErrors = 0,
  serverPush = false,
  updateTimeout = null;

function quit(hasQuitMessage) {
  hasQuit = true;
  quitStr = hasQuitMessage;
  if (keepAliveTimer) {
    clearInterval(keepAliveTimer);
    keepAliveTimer = null;
  }
  if (pollTimer) {
    clearInterval(pollTimer);
    pollTimer = null;
  }
  comm.cancel();
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

  // this could be cancelled leading to havoc?
  $(document).mousedown(WT.mouseDown).mouseup(WT.mouseUp);

  WT.history._initialize();
  initDragDrop();
  loaded = true;

  if (fullapp)
    window._$_APP_CLASS_$_LoadWidgetTree();

  if (!hasQuit) {
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

/** @const */ var WebSocketUnknown = 0;
/** @const */ var WebSocketConnecting = 1;
/** @const */ var WebSocketAckConnect = 2;
/** @const */ var WebSocketWorking = 3;
/** @const */ var WebSocketUnavailable = 4;

var websocket = {
  state: WebSocketUnknown,
  socket: null,
  keepAlive: null,
  reconnectTries: 0
};

var connectionMonitor = null;

function setServerPush(how) {
  serverPush = how;
}

function doAutoJavaScript() {
    self._p_.autoJavaScript();
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

function webSocketAckConnect() {
  websocket.socket.send('&signal=none&connected=' + ackUpdateId);
  websocket.state = WebSocketWorking;
}

function handleResponse(status, msg, timer) {
  if (connectionMonitor)
    connectionMonitor.onStatusChange('connectionStatus', status == 0 ? 1 : 0);

  if (hasQuit)
    return;

  if (waitingForJavaScript) {
    setTimeout(function() { handleResponse(status, msg, timer); }, 50);
    return;
  }

  if (status == 0) {
    WT.resolveRelativeAnchors();
_$_$if_CATCH_ERROR_$_();
    try {
_$_$endif_$_();
      doJavaScript(msg);
_$_$if_CATCH_ERROR_$_();
    } catch (e) {
      var stack = e.stack || e.stacktrace;
      var description = e.description || e.message;
      var err = { "exception_code": e.code,
		  "exception_description": description,
		  "exception_js": msg };
      err.stack = stack;
      sendError(err,
		"Wt internal error; code: " +  e.code
		+ ", description: " + description);
      throw e;
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

  if (hasQuit)
    return;

  if (websocket.state == WebSocketAckConnect)
    webSocketAckConnect();

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

  if (!hasQuit)
    sendUpdate();
}

function setConnectionMonitor(aMonitor) {
  connectionMonitor = aMonitor;
  connectionMonitor.status = {};
  connectionMonitor.status.connectionStatus = 0;
  connectionMonitor.status.websocket = false;
  connectionMonitor.onStatusChange = function(type, newS) {
    var old = connectionMonitor.status[type];
    if (old == newS)
	return;
    connectionMonitor.status[type] = newS;
    connectionMonitor.onChange(type, old, newS);
  };
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

function schedulePing() {
  if (websocket.keepAlive)
    clearInterval(websocket.keepAlive);

  websocket.keepAlive = setInterval
    (function() {
      var ws = websocket.socket;
      if (ws.readyState == 1)
	ws.send('&signal=ping');
      else {
	clearInterval(websocket.keepAlive);
	websocket.keepAlive = null;
      }
    }, _$_SERVER_PUSH_TIMEOUT_$_);
}

function scheduleUpdate() {
  if (hasQuit) {
    if (!quitStr)
      return;
    if (confirm(quitStr)) {
      document.location = document.location;
      quitStr = null;
      return;
    } else {
      quitStr = null;
      return;
    }
  }

_$_$if_WEB_SOCKETS_$_();
  if (websocket.state != WebSocketUnavailable) {
    if (typeof window.WebSocket === UNDEFINED
        && typeof window.MozWebSocket === UNDEFINED)
      websocket.state = WebSocketUnavailable;
    else {
      var ws = websocket.socket;

      if ((ws == null || ws.readyState > 1)) {
	if (ws != null && websocket.state == WebSocketUnknown)
	  websocket.state = WebSocketUnavailable;
	else {
	  function reconnect() {
	    if (!hasQuit) {
	      ++websocket.reconnectTries;
	      var ms = Math.min(120000, Math.exp(websocket.reconnectTries)
				* 500);
	      setTimeout(function() { scheduleUpdate(); }, ms);
	    }
	  }

	  var protocolEnd = sessionUrl.indexOf("://"), wsurl;
	  if (protocolEnd != -1) {
	    wsurl = "ws" + sessionUrl.substr(4);
	  } else {
	    var query = sessionUrl.substr(sessionUrl.indexOf('?'));
	    wsurl = "ws" + location.protocol.substr(4)
	      + "//" + location.host + deployUrl + query;
	  }

	  wsurl += "&request=ws";

	  if (typeof window.WebSocket !== UNDEFINED)
	    websocket.socket = ws = new WebSocket(wsurl);
	  else
	    websocket.socket = ws = new MozWebSocket(wsurl);

	  websocket.state = WebSocketConnecting;

	  if (websocket.keepAlive)
	    clearInterval(websocket.keepAlive);
	  websocket.keepAlive = null;

	  ws.onmessage = function(event) {
	    var js = null;

	    if (websocket.state == WebSocketConnecting) {
	      if (event.data == "connect") {
		if (responsePending != null && pollTimer != null) {
		  clearTimeout(pollTimer);
		  responsePending.abort();
		  responsePending = null;
		}

		if (responsePending)
		  websocket.state = WebSocketAckConnect;
		else
		  webSocketAckConnect();
	      } else {
		console.log("WebSocket: was expecting a connect?");
		console.log(event.data);
		return;
	      }
	    } else {
	      if (connectionMonitor) {
		connectionMonitor.onStatusChange('websocket', true);
		connectionMonitor.onStatusChange('connectionStatus', 1);
	      }
              websocket.state = WebSocketWorking;
	      js = event.data;
	    }

	    websocket.reconnectTries = 0;
	    if (js != null)
              handleResponse(0, js, null);
	  };

	  ws.onerror = function(event) {
	    /*
	     * Sometimes, we can connect but cannot send data
	     */
	    if (connectionMonitor)
	      connectionMonitor.onStatusChange('websocket', false);
	    if (websocket.reconnectTries == 3 &&
		websocket.state == WebSocketUnknown)
	      websocket.state = WebSocketUnavailable;
	    reconnect();
	  };

	  ws.onclose = function(event) {
	    /*
	     * Sometimes, we can connect but cannot send data
	     */
	    if (connectionMonitor)
	      connectionMonitor.onStatusChange('websocket', false);
	    if (websocket.reconnectTries == 3 &&
		websocket.state == WebSocketUnknown)
	      websocket.state = WebSocketUnavailable;
	    reconnect();
	  };

	  ws.onopen = function(event) {
	    /*
	     * WebSockets are suppossedly reliable, but there is nothing
	     * in the protocol that makes them so...
	     *
	     * WebSockets are supposedly using a ping/pong protocol to
	     * motivate proxies to keep connections open, but we've never
	     * seen a browser pinging us ?
	     *
	     * So, we ping pong ourselves.
	     */
	    if (connectionMonitor) {
	      connectionMonitor.onStatusChange('websocket', true);
	      connectionMonitor.onStatusChange('connectionStatus', 1);
	    }

	    /*
	      ws.send('&signal=ping'); // to get our first onmessage
	     */
	    schedulePing();
	  };
	}
      }

      if ((ws.readyState == 1) && (ws.state == WebSocketWorking)) {
	schedulePing();
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
function setPage(id) {
  pageId = id;
}

function sendError(err, errMsg) {
  responsePending = comm.sendUpdate
    ('request=jserror&err=' + encodeURIComponent(JSON.stringify(err)),
     false, ackUpdateId, -1);
_$_$if_SHOW_ERROR_$_();
  alert(errMsg);
_$_$endif_$_();
}

function sendUpdate() {
  if (self != window._$_APP_CLASS_$_) {
    quit(null);
    return;
  }

  if (responsePending)
    return;

  updateTimeout = null;
  var feedback;

  if (WT.isIEMobile) feedback = false;

  if (hasQuit)
    return;

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
    data.result += '&Wt-params=' + encodeURIComponent(params);

  if ((websocket.socket != null) &&
      (websocket.socket.readyState == 1) &&
      (websocket.state == WebSocketWorking)) {
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

function propagateSize(element, width, height) {
  /*
   * Propagate the size, even if it's the elements unconstrained size.
   */
  if (width == -1)
    width = element.offsetWidth;
  if (height == -1)
    height = element.offsetHeight;

  if ((typeof element.wtWidth === UNDEFINED)
      || (element.wtWidth != width)
      || (typeof element.wtHeight === UNDEFINED)
      || (element.wtHeight != height)) {
    element.wtWidth = width;
    element.wtHeight = height;

    if (width >= 0 && height >= 0)
      emit(element, 'resized', Math.round(width), Math.round(height));
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
    else if (a && a.toDateString)
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
  if (obj.timer)
    clearTimeout(obj.timer);
  obj.timer = setTimeout(tm, msec);
  obj.tm = tm;
}

var jsLibsLoaded = {};
var waitingForJavaScript = false;

function onJsLoad(path, f) {
  // setTimeout needed for Opera
  setTimeout(function() {
    if (jsLibsLoaded[path] === true) {
      waitingForJavaScript = false;
      f();
    } else
      jsLibsLoaded[path] = f;
    }, 20);

  waitingForJavaScript = true;
};

function jsLoaded(path)
{
  if (jsLibsLoaded[path] === true)
    return;
  else {
    if (typeof jsLibsLoaded[path] !== UNDEFINED) {
      waitingForJavaScript = false;
      jsLibsLoaded[path]();
    }
    jsLibsLoaded[path] = true;
  }
};

function loadScript(uri, symbol, tries)
{
  var loaded = false, error = false;

  function onerror() {
    if (!loaded && !error) {
      error = true;

      var t = tries === undefined ? (WT.isIE ? 1 : 2) : tries;
      if (t > 1) {
	loadScript(uri, symbol, t - 1);
      } else {
	var err = {
	  "error-description" : 'Fatal error: failed loading ' + uri
	};
	sendError(err, err["error-description"]);
	quit(null);
      }     
    }
  }

  function onload() {
    if (!loaded && !error) {
      loaded = true;
      jsLoaded(uri);
    }
  }

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
    s.onload = onload;
    s.onerror = onerror;
    s.onreadystatechange = function() {
      var rs = s.readyState;
      if (rs == 'loaded') { // may still be 404 in IE<=8; too bad!
	if (WT.isOpera || WT.isIE) {
	  onload();
	} else
	  onerror();
      } else if (rs == 'complete') {
	onload();
      }
    };
    var h = document.getElementsByTagName('head')[0];
    h.appendChild(s);
  } else {
    jsLoaded(uri);
  }
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

/////////////////////////////////////////////////////////////////////
// TG: A binary Buffer preloader

// Constructor, preloads the given uris and stores them in arrayBuffers[]
function ArrayBufferPreloader(uris, callback) {
  // init members
  // callback, when everything is loaded
  this.callback = callback;
  // number of open requests
  this.work = uris.length;
  // resulting buffers
  this.arrayBuffers = [];
  
  // if urls are missing, call callback without buffers
  if (uris.length == 0)
    callback(this.arrayBuffers);
  else 
  {
    // if uris are given, load them asynchronously
    for (var i = 0; i < uris.length; i++)
      this.preload(uris[i], i);
  }
};

// preload function: downloads buffer at the given URI
ArrayBufferPreloader.prototype.preload = function(uri, index) {
  var xhr = new XMLHttpRequest();
  // open the resource, send asynchronously (without waiting for answer)
  xhr.open("GET", uri, true);
  xhr.responseType = "arraybuffer"; 

  // give xhr write access to array
  xhr.arrayBuffers = this.arrayBuffers;
  xhr.preloader = this;
  xhr.index = index; // needed to maintain the mapping
  xhr.uri = uri;

  // behaviour when it was loaded-> redirect to ArrayBufferPreloader
  xhr.onload = function(e) {

    console.log("XHR load buffer " + this.index + " from uri " + this.uri);

    //this.arrayBuffers[this.index] = new Uint8Array(this.response);
    this.arrayBuffers[this.index] = this.response;
    this.preloader.afterLoad();
  };

  xhr.onerror = ArrayBufferPreloader.prototype.afterload;
  xhr.onabort = ArrayBufferPreloader.prototype.afterload;

  // actually start the query
  xhr.send();
};

ArrayBufferPreloader.prototype.afterLoad = function() {
  if (--this.work == 0)
    // last request finished -> call callback
    this.callback(this.arrayBuffers);
};
/////////////////////////////////////////////////////////////////////


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
  self.emit(d.parentNode, 'IeAlternative');
  d.style.width = '';
  d.ieAlternativeExecuted = true;
  return '0';
}

window.onunload = function()
{
  if (!hasQuit) {
    self.emit(self, "Wt-unload");
    scheduleUpdate();
    sendUpdate();
  }
};

function setLocale(m)
{	
  if (WT.isIEMobile || m == '') return;
  document.documentElement.lang = m
}

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

var firstCall = true;
var globalEventsFunctions = null;
var keyEvents = [ 'keydown', 'keyup', 'keypress' ];

function updateGlobal(id) {
  firstCall = false;
  var domId;
  if (id == null) {
    domId = $('.Wt-domRoot').get(0).id
  } else {
    domId = id;
  }

  for (var i = 0; i < keyEvents.length ; ++i) {
    var elemEvents = globalEventsFunctions ? globalEventsFunctions[domId] : null;
    var eventFunc = null;

    if (elemEvents) 
      eventFunc = elemEvents[keyEvents[i]];

    var bindEvent = function(evtfunc) {
      return function(event) {
	var g=event||window.event;
	var t=g.target||g.srcElement;
	if ((!t| WT.hasTag(t,'DIV')
	  || WT.hasTag(t,'BODY')
	    || WT.hasTag(t,'HTML'))) {
	      var func = evtfunc;
	      if(func)
		func(event);
	    }
      };
    };

    if (eventFunc)
      document['on' + keyEvents[i] ] = bindEvent(eventFunc);
    else 
      document['on' + keyEvents[i] ] = null;

  }

  // cleanup functions of widgets that do no longer exist
  if (globalEventsFunctions) {
    for (var i in globalEventsFunctions) {
      if (!document.getElementById(i)) {
	delete globalEventsFunctions[i];
      }
    }
  }
}

function bindGlobal(event, id, f) {
  var init = false;
  if (!globalEventsFunctions) {
    globalEventsFunctions = {};
    init = true;
  }

  // Saves the event functions
  if (!globalEventsFunctions[id])
    globalEventsFunctions[id] = {};

  globalEventsFunctions[id][event]=f;
  if(init)
    setTimeout(function() {
      if(firstCall)
	updateGlobal(null);
    }, 0);
}

this._p_ = {
  ieAlternative : ieAlternative,
  loadScript : loadScript,
  onJsLoad : onJsLoad,
  setTitle : setTitle,
  setLocale : setLocale,
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
  ArrayBufferPreloader : ArrayBufferPreloader,
  
  doAutoJavaScript : doAutoJavaScript,
  autoJavaScript : function() { },

  response : responseReceived,
  setPage : setPage,
  setCloseMessage : setCloseMessage,
  setConnectionMonitor : setConnectionMonitor,
  updateGlobal: updateGlobal,
  bindGlobal: bindGlobal,

  propagateSize : propagateSize
};

this.WT = _$_WT_CLASS_$_;
this.emit = emit;

})();

window._$_APP_CLASS_$_SignalEmit = _$_APP_CLASS_$_.emit;

window._$_APP_CLASS_$_OnLoad = function() {
  _$_APP_CLASS_$_._p_.load();
};
