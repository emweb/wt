function loadScript(url, callback) {
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
  var h = document.getElementsByTagName('head')[0];
  h.appendChild(s);
}

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

if (win.opera)
  win.opera.setOverrideHistoryNavigationMode("compatible");

// ajax support
var ajax = (win.XMLHttpRequest || win.ActiveXObject);

// client-side cookie support
var testcookie='jscookietest=valid';
doc.cookie=testcookie;
var no_replace = _$_RELOAD_IS_NEWSESSION_$_
  || (_$_USE_COOKIES_$_ && doc.cookie.indexOf(testcookie) != -1);
doc.cookie=testcookie+';expires=Thu, 01 Jan 1970 00:00:00 GMT';

// server-side cookie support
var inOneSecond = new Date();
inOneSecond.setTime(inOneSecond.getTime() + 1000);
doc.cookie='WtTestCookie=ok;path=/;expires=' + inOneSecond.toGMTString();

// hash to query
var hash = win.location.hash;
if (hash.length > 0)
  hash = hash.substr(1);
var qstart = hash.indexOf('?');
if (qstart != -1)
  hash = hash.substr(0, qstart);

var ua = navigator.userAgent.toLowerCase();
if ((ua.indexOf("gecko") == -1) || (ua.indexOf("webkit") != -1))
  hash = unescape(hash);

// scale (VML)
var scaleInfo = "";
if (screen.deviceXDPI != screen.logicalXDPI)
  scaleInfo = "&scale=" + screen.deviceXDPI / screen.logicalXDPI;

// determine url
var selfUrl=_$_SELF_URL_$_;

var needSessionInUrl = !no_replace || !ajax;

if (needSessionInUrl) {
  function getSessionFromUrl() {
    var url, idx, i, queryString, params, tokens;
    url = top.location.href;
    idx = url.indexOf('?');
    queryString = idx >= 0 ? url.substr(idx + 1) : url;
    idx = queryString.lastIndexOf("#");
    queryString = idx >= 0 ? queryString.substr(0, idx) : queryString;
    params = queryString.split("&");

    for (i = 0, len = params.length; i < len; i++) {
      tokens = params[i].split("=");
      if (tokens.length >= 2)
        if (tokens[0] === "wtd")
          return unescape(tokens[1]);
    }

    return null;
  }

  if (getSessionFromUrl() === '_$_SESSION_ID_$_')
    needSessionInUrl = false;
}

if (needSessionInUrl) {
  if (hash.length > 0)
    selfUrl += '#' + hash;
  setUrl(selfUrl);
} else if (ajax) {
  var htmlHistory = !!(win.history && win.history.pushState);
  var canonicalUrl = _$_AJAX_CANONICAL_URL_$_;
  if (!htmlHistory && canonicalUrl.length > 1) {
_$_$if_HYBRID_$_();
    var pathcookie='WtInternalPath=' + escape(_$_INTERNAL_PATH_$_)
      + ';path=/;expires=' + inOneSecond.toGMTString();
    doc.cookie=pathcookie;
_$_$endif_$_();
    setUrl(canonicalUrl);
  } else {
    if (hash.length > 1 && hash.charAt(0) == '/') {
      selfUrl += '&_=' + encodeURIComponent(hash);
_$_$if_HYBRID_$_();
      if (hash != _$_INTERNAL_PATH_$_)
        setTimeout(hideForm, 10);
_$_$endif_$_();
    }

    loadScript(selfUrl + scaleInfo + '&request=script&rand=' + rand(), null);
  }
}
    }

_$_$if_DEFER_SCRIPT_$_();
 setTimeout(doLoad, 0);
_$_$endif_$_();
_$_$ifnot_DEFER_SCRIPT_$_();
 doLoad();
_$_$endif_$_();

})();

