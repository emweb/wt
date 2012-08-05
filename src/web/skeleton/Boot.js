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

var pathInfo = _$_PATH_INFO_$_, deployPath = win.location.pathname;

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

// workaround inconsistencies in hash character encoding
var ua = navigator.userAgent.toLowerCase();
if ((ua.indexOf("gecko") == -1) || (ua.indexOf("webkit") != -1))
  hash = unescape(hash);

// scale (VML)
var scaleInfo = "";
if (screen.deviceXDPI != screen.logicalXDPI)
  scaleInfo = "&scale=" + screen.deviceXDPI / screen.logicalXDPI;

// determine url
var selfUrl = _$_SELF_URL_$_ + '&sid=' + _$_SCRIPT_ID_$_;

// determine html history support
var htmlHistory = !!(window.history && window.history.pushState),
    htmlHistoryInfo = htmlHistory ? "&htmlHistory=true" : "";

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

    var allInfo = hashInfo + scaleInfo + htmlHistoryInfo + deployPathInfo;
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

_$_$if_DEFER_SCRIPT_$_();
 setTimeout(doLoad, 0);
_$_$endif_$_();
_$_$ifnot_DEFER_SCRIPT_$_();
 doLoad();
_$_$endif_$_();

})();

