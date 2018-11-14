window.onresize = function() { };

function loadScript(url, callback) {
  var h = document.getElementsByTagName('head')[0];
  var agent = navigator.userAgent.toLowerCase();
  var re = /firefox\/(\d+)\./;
  var m = re.exec(agent);

  if (m && m[1] >= 20) {
    var async = new XMLHttpRequest();
    async.open('GET', url, true);

    async.onreadystatechange = function() {
      if (async.readyState == 4) {
	var s = document.createElement('script');
	s.type = 'text/javascript';
	s.innerHTML=async.responseText;
	h.appendChild(s);
	if (callback)
	  callback();
      }
    };

    async.send(null);
  } else {
    var s = document.createElement('script');
    if (callback) {
      if (s.readyState) {
	s.onreadystatechange = function() {
	  if (s.readyState == "loaded" || s.readyState == "complete") {
	    s.onreadystatechange = null;
	    callback();
	  }
	};
      } else {
	s.onload = function() {
	  callback();
	};
      }
    }

    s.setAttribute('src', url);
    h.appendChild(s);
  }
}

_$_$if_PROGRESS_$_();
var delayedClicks = [];
function delayClick(e) {
  /* IE8 does not actually do detachEvent() in progressed() ? */
  var form = document.getElementById('Wt-form');
  if (form == null)
    return true;

  var ec = {
    bubbles: e.bubbles,
    cancelable: e.cancelable,
    detail: e.detail,
    screenX: e.screenX, screenY: e.screenY,
    clientX: e.clientX, clientY: e.clientY,
    ctrlKey: e.ctrlKey, altKey: e.altKey, shiftKey: e.shiftKey,
    metaKey: e.metaKey, button: e.button,
    targetId: (e.target || e.srcElement).id
  };

  delayedClicks.push(ec);

  if (e.stopPropagation)
    e.stopPropagation();
  if (e.preventDefault)
    e.preventDefault();
  e.cancelBubble = true;
  e.returnValue = false;
  return false;
}

function setupDelayClick() {
  var db = document.body;
  if (!db)
     setTimeout(setupDelayClick, 1);
  else {
    if (db.addEventListener)
      db.addEventListener('click', delayClick, true);
    else
      db.attachEvent('onclick', delayClick);
  }
}
_$_$endif_$_();

(function() {
  function doLoad() {

var doc = document, win = window;

try {
  doc.execCommand("BackgroundImageCache", false, true);
} catch (err) { }

function rand() {
  return Math.round(Math.random()*1000000) + _$_RANDOMSEED_$_;
}

function setUrl(url) {
  if (win.location.replace)
    win.location.replace(url);
  else
    win.location.href=url;
}

function hideForm() {
  var f = doc.getElementById('Wt-form');
  if (f != null)
    f.style.visibility='hidden';
  else
    setTimeout(hideForm, 10);
}

function getParams() {
  var queryString = window.location.search;
  if (queryString.length > 1 && queryString.charAt(0) == '?')
    queryString = queryString.substr(1);

  return queryString.split("&");
}

function getParameter(name) {
  var i, params, tokens, len;

  params = getParams();

  for (i = 0, len = params.length; i < len; i++) {
    tokens = params[i].split("=");
    if (tokens.length >= 2)
      if (tokens[0] === name)
	return unescape(tokens[1]);
  }

  return null;
}

function createUrl(name, value) {
  var i, params, tokens, len, found = false;

  params = getParams();

  for (i = 0, len = params.length; i < len; i++) {
    tokens = params[i].split("=");
    if (tokens.length >= 2)
      if (tokens[0] === name) {
	tokens[1] = escape(value);
	params[i] = tokens.join("=");
	found = true;
	break;
      }
  }

  if (!found)
    params.push(name + "=" + escape(value));

  return "?" + params.join("&") + window.location.hash;
}

if (win.opera)
  win.opera.setOverrideHistoryNavigationMode("compatible");

var pathInfo = _$_PATH_INFO_$_,
    deployPath = win.location.pathname;

if (!win.opera)
  deployPath = decodeURIComponent(deployPath);

/*
 * Java's weird session encoding could put the path not in the end, e.g.
 * /hello/internalpath;jsessionid=xyz
 */
if (pathInfo.length > 0) {
  var pathI = deployPath.lastIndexOf(pathInfo);
  if (pathI != -1)
    deployPath = deployPath.substr(0, pathI)
        + deployPath.substr(pathI + pathInfo.length);
}
var deployPathInfo = '&deployPath=' + encodeURIComponent(deployPath);

// ajax support
var ajax = (win.XMLHttpRequest || win.ActiveXObject);

var no_replace = _$_RELOAD_IS_NEWSESSION_$_;
var inOneSecond = new Date();
inOneSecond.setTime(inOneSecond.getTime() + 1000);

_$_$if_COOKIE_CHECKS_$_();
// client-side cookie support
var testcookie='jscookietest=valid';
doc.cookie=testcookie;
no_replace = no_replace || 
	  (_$_USE_COOKIES_$_ && doc.cookie.indexOf(testcookie) != -1);
doc.cookie=testcookie+';expires=Thu, 01 Jan 1970 00:00:00 GMT';

// server-side cookie support
doc.cookie='WtTestCookie=ok;path=/;expires=' + inOneSecond.toGMTString();
_$_$endif_$_();

// hash to query
var hash = win.location.hash;
if (hash.length > 0)
  hash = hash.substr(1);
var qstart = hash.indexOf('?');
if (qstart != -1)
  hash = hash.substr(0, qstart);

// workaround inconsistencies in hash character encoding
var ua = navigator.userAgent.toLowerCase();
if ((ua.indexOf("gecko") == -1) || (ua.indexOf("webkit") != -1))
  hash = unescape(hash);

// scale (VML)
var otherInfo = "";
if (screen.deviceXDPI != screen.logicalXDPI)
  otherInfo = "&scale=" + screen.deviceXDPI / screen.logicalXDPI;

_$_$if_WEBGL_DETECT_$_();
// webgl-check
var webGLInfo = "";
if (window.WebGLRenderingContext) {
    var canvas = document.createElement("canvas");
    var ctx = null;
    try {
        ctx = canvas.getContext('webgl', {antialias: true});
    } catch (e) {}
    if (ctx == null) {
        try {
            ctx = canvas.getContext('experimental-webgl');
        } catch (e) {}
    }
    if (ctx != null) {
	otherInfo += "&webGL=true";
    }
}
_$_$endif_$_();

// info about screen resolution
otherInfo += "&scrW=" + screen.width + "&scrH=" + screen.height;

// determine url
var selfUrl = _$_SELF_URL_$_ + '&sid=' + _$_SCRIPT_ID_$_;

// determine html history support
var htmlHistory = !!(window.history && window.history.pushState),
    htmlHistoryInfo = htmlHistory ? "&htmlHistory=true" : "";

// determine time zone offset
var tzOffset = (new Date()).getTimezoneOffset();
otherInfo += "&tz=" + (-tzOffset);

// determine time zone name, if available
if (typeof Intl === 'object' &&
    typeof Intl.DateTimeFormat === "function" &&
    typeof Intl.DateTimeFormat().resolvedOptions === "function" &&
    Intl.DateTimeFormat().resolvedOptions().timeZone) {
  otherInfo += "&tzS=" + encodeURIComponent(Intl.DateTimeFormat().resolvedOptions().timeZone);
}

var needSessionInUrl = !no_replace || !ajax;

if (needSessionInUrl) {
  if (getParameter("wtd") === '_$_SESSION_ID_$_')
    needSessionInUrl = false;
}

if (needSessionInUrl) {
  if (htmlHistory)
    setUrl(createUrl("wtd", '_$_SESSION_ID_$_'));
  else {
    var h;
    if (hash.length > 1 && hash.charAt(0) == '/')
      h = hash;
    else
      h = _$_INTERNAL_PATH_$_;

    if (h.length > 0)
      selfUrl += '#' + h;
    setUrl(selfUrl);
  }
} else if (ajax) {
  var canonicalUrl = _$_AJAX_CANONICAL_URL_$_,
      hashInfo = '';
  if (!htmlHistory && canonicalUrl.length > 1) {
_$_$if_HYBRID_$_();
    var pathcookie='WtInternalPath=' + escape(_$_INTERNAL_PATH_$_)
      + ';path=/;expires=' + inOneSecond.toGMTString();
    doc.cookie=pathcookie;
_$_$endif_$_();
    /* Otherwise we do not get a page reload */
    if (canonicalUrl.charAt(0) == '#')
      canonicalUrl = '../' + canonicalUrl;
    setUrl(canonicalUrl);
  } else {
    if (hash.length > 1 && hash.charAt(0) == '/') {
      hashInfo = '&_=' + encodeURIComponent(hash);
_$_$if_HYBRID_$_();
      if (hash != _$_INTERNAL_PATH_$_)
        setTimeout(hideForm, 10);
_$_$endif_$_();
    }

_$_$if_PROGRESS_$_();
    /*
      Make sure that we are not processing click events while progressing.
      Instead, delay them.
    */
    setupDelayClick();
_$_$endif_$_();

    var allInfo = hashInfo + otherInfo + htmlHistoryInfo + deployPathInfo;
_$_$ifnot_SPLIT_SCRIPT_$_();
    loadScript(selfUrl + allInfo + '&request=script&rand=' + rand(),
               null);
_$_$endif_$_();
_$_$if_SPLIT_SCRIPT_$_();
    /* Ideally, we should be able to omit the sessionid too */
    loadScript(selfUrl + allInfo + '&request=script&skeleton=true',
               function() {
                 loadScript(selfUrl + allInfo
                            + '&request=script&rand=' + rand(), null);
               });
_$_$endif_$_();
  }
}
    }

 setTimeout(doLoad, 0);

})();

