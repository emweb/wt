/**
 * @preserve Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * For terms of use, see LICENSE.
 */
/* global
  _$_ACK_UPDATE_ID_$_
  _$_APP_CLASS_$_
  _$_CLOSE_CONNECTION_$_
  _$_DEPLOY_PATH_$_
  _$_IDLE_TIMEOUT_$_
  _$_INDICATOR_TIMEOUT_$_
  _$_INNER_HTML_$_
  _$_KEEP_ALIVE_$_
  _$_MAX_FORMDATA_SIZE_$_
  _$_MAX_PENDING_EVENTS_$_
  _$_QUITTED_STR_$_
  _$_SERVER_PUSH_TIMEOUT_$_
  _$_SESSION_URL_$_
  _$_WS_ID_$_
  _$_WS_PATH_$_
  _$_WT_CLASS_$_
  _$_$if_CATCH_ERROR_$_
  _$_$if_DYNAMIC_JS_$_
  _$_$ifnot_DYNAMIC_JS_$_
  _$_$if_SHOW_ERROR_$_
  _$_$if_STRICTLY_SERIALIZED_EVENTS_$_
  _$_$if_UGLY_INTERNAL_PATHS_$_
  _$_$ifnot_UGLY_INTERNAL_PATHS_$_
  _$_$if_WEB_SOCKETS_$_
  _$_$endif_$_
  delayClick
  delayedClicks
  google
  hideLoadingIndicator
  showLoadingIndicator
*/
_$_$if_DYNAMIC_JS_$_();
window.JavaScriptFunction = 1;
window.JavaScriptConstructor = 2;
window.JavaScriptObject = 3;
window.JavaScriptPrototype = 4;
window.WT_DECLARE_WT_MEMBER = function(i, type, name, fn) {
  if (type === JavaScriptPrototype) {
    const proto = name.indexOf(".prototype");
    _$_WT_CLASS_$_[name.substring(0, proto)]
      .prototype[name.substring(proto + ".prototype.".length)] = fn;
  } else if (type === JavaScriptFunction) {
    _$_WT_CLASS_$_[name] = function() {
      return fn.apply(_$_WT_CLASS_$_, arguments);
    };
  } else {
    _$_WT_CLASS_$_[name] = fn;
  }
};
window.WT_DECLARE_WT_MEMBER_BIG = window.WT_DECLARE_WT_MEMBER;

window.WT_DECLARE_APP_MEMBER = function(i, type, name, fn) {
  const app = window.currentApp;
  if (type === JavaScriptPrototype) {
    const proto = name.indexOf(".prototype");
    app[name.substring(0, proto)]
      .prototype[name.substring(proto + ".prototype.".length)] = fn;
  } else if (type === JavaScriptFunction) {
    app[name] = function() {
      return fn.apply(app, arguments);
    };
  } else {
    app[name] = fn;
  }
};

_$_$endif_$_();

_$_$ifnot_DYNAMIC_JS_$_();
window.JavaScriptConstructor = 2;
window.WT_DECLARE_WT_MEMBER_BIG = function(i, type, name, fn) {
  return fn;
};
_$_$endif_$_();

if (!window._$_WT_CLASS_$_) {
  window._$_WT_CLASS_$_ = new (function() {
    const WT = this;
    const UNDEFINED = "undefined";

    // Alternative for jQuery $(document).ready
    this.ready = function(f) {
      if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", f);
      } else {
        f();
      }
    };

    // Function to test if object is empty
    this.isEmptyObject = function(obj) {
      for (const elem in obj) {
        if (Object.prototype.hasOwnProperty.call(obj, elem)) {
          return false;
        }
      }
      return true;
    };

    this.condCall = function(o, f, a) {
      if (o[f]) {
        o[f](a);
      }
    };

    // buttons currently down
    this.buttons = 0;

    // button last released (for reporting in IE's click event)
    let lastButtonUp = 0, mouseDragging = 0;

    // returns the button associated with the event (0 if none)
    this.button = function(e) {
      try {
        const t = e.type;

        if (t !== "mouseup" && t !== "mousedown" && t !== "click" && t !== "dblclick") {
          return 0;
        }
      } catch (e) {
        return 0;
      }

      if (e.button === 0) {
        return 1;
      } else if (e.button === 1) {
        return 2;
      } else if (e.button === 2) {
        return 4;
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
    this.dragged = function(_e) {
      return mouseDragging > 2;
    };

    this.drag = function(_e) {
      ++mouseDragging;
    };

    /**
     * @preserve Includes Array Remove - By John Resig (MIT Licensed)
     */
    this.arrayRemove = function(a, from, to) {
      const rest = a.slice((to || from) + 1 || a.length);
      a.length = from < 0 ? a.length + from : from;
      return a.push.apply(a, rest);
    };

    this.addAll = function(a1, a2) {
      for (let i = 0, il = a2.length; i < il; ++i) {
        a1.push(a2[i]);
      }
    };

    const agent = navigator.userAgent.toLowerCase();

    // It's never IE, because we don't support IE
    this.isIE = false;
    this.isIE6 = false;
    this.isIE8 = false;
    this.isIElt9 = false;
    this.isIEMobile = false;
    // It's never old Opera, because we don't support old Opera
    this.isOpera = false;
    this.isAndroid = (agent.indexOf("safari") !== -1) &&
      (agent.indexOf("android") !== -1);
    this.isWebKit = agent.indexOf("applewebkit") !== -1;
    this.isGecko = agent.indexOf("gecko") !== -1 && !this.isWebKit;
    this.isIOS = agent.indexOf("iphone") !== -1 || agent.indexOf("ipad") !== -1 || agent.indexOf("ipod") !== -1;

    this.updateDelay = 51;

    if (this.isAndroid) {
      console.error("init console.error");
      console.info("init console.info");
      console.log("init console.log");
      console.warn("init console.warn");
    }

    let traceStart = new Date();
    this.trace = function(v, start) {
      if (start) {
        traceStart = new Date();
      }
      const now = new Date();

      const diff = (now.getMinutes() - traceStart.getMinutes()) * 60000 +
        (now.getSeconds() - traceStart.getSeconds()) * 1000 +
        (now.getMilliseconds() - traceStart.getMilliseconds());

      if (window.console) {
        console.log("[" + diff + "]: " + v);
      }
    };

    function host(url) {
      const parts = url.split("/");
      return parts[2];
    }

    this.initAjaxComm = function(url, handler) {
      const crossDomain = (url.indexOf("://") !== -1 || url.indexOf("//") === 0) &&
        host(url) !== window.location.host;

      function createRequest(method, url) {
        let request = null;
        const supportsRequestHeader = true;
        request = new XMLHttpRequest();
        if (crossDomain) {
          if ("withCredentials" in request) {
            if (url) {
              request.open(method, url, true);
              request.withCredentials = "true";
            }
          } else {
            request = null;
          }
        } else if (url) {
          request.open(method, url, true);
        }

        if (request && url && supportsRequestHeader) {
          request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        }

        return request;
      }

      const req = createRequest("POST", url);

      if (req !== null) {
        return new (function() {
          let sessionUrl = url;

          function Request(data, userData, id, timeout, isPollRequest) {
            let request = createRequest("POST", sessionUrl);
            let timer = null;
            let handled = false;

            function handleResponse(status) {
              if (handled) {
                return;
              }

              clearTimeout(timer);

              if (!sessionUrl) {
                return;
              }

              handled = true;

              const rq = request;
              if (request) {
                request.onreadystatechange = new Function();
                try {
                  request.onload = request.onreadystatechange;
                } catch (e) {
                  /*
                   * See comment below.
                   */
                }
                request = null;
              }

              if (status === WT.ResponseStatus.OK) {
                handler(status, rq.responseText, userData);
              } else {
                handler(status, null, userData);
              }
            }

            function recvCallback() {
              if (
                request.status === 200 &&
                request.getResponseHeader("Content-Type") &&
                request.getResponseHeader("Content-Type")
                    .indexOf("text/javascript") === 0
              ) {
                handleResponse(WT.ResponseStatus.OK);
              } else if (isPollRequest && request.status === 504) {
                handleResponse(WT.ResponseStatus.Timeout);
              } else {
                handleResponse(WT.ResponseStatus.Error);
              }
            }

            function handleTimeout() {
              if (handled) {
                return;
              }

              if (!sessionUrl) {
                return;
              }

              request.onreadystatechange = new Function();
              request = null;
              handled = true;
              handler(WT.ResponseStatus.Timeout, null, userData);
            }

            this.abort = function() {
              if (request !== null) {
                request.onreadystatechange = new Function();
                handled = true;
                request.abort();
                request = null;
              }
            };

            if (_$_CLOSE_CONNECTION_$_) {
              request.setRequestHeader("Connection", "close");
            }

            if (timeout > 0) {
              timer = setTimeout(handleTimeout, timeout);
            }

            request.onreadystatechange = function() {
              if (request.readyState === 4) {
                recvCallback();
              }
            };
            try {
              request.onload = recvCallback;
              request.onerror = function() {
                handleResponse(WT.ResponseStatus.Error);
              };
            } catch (e) {
              /*
               * On IE, when "Enable Native XMLHTTP Support is unchecked",
               * setting these members will result in an exception.
               */
            }
            request.send(data);
          }

          this.responseReceived = function(_updateId) {};

          this.sendUpdate = function(data, userData, id, timeout, isPoll) {
            if (!sessionUrl) {
              return null;
            }
            return new Request(data, userData, id, timeout, isPoll);
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
          let sessionUrl = url;
          let request = null;

          function Request(data, userData, id, _timeout) {
            this.userData = userData;

            const s = this.script = document.createElement("script");
            s.id = "script" + id;
            s.setAttribute("src", sessionUrl + "&" + data);

            function onerror() {
              handler(WT.ResponseStatus.Error, null, userData);
              s.parentNode.removeChild(s);
            }

            s.onerror = onerror;

            const h = document.getElementsByTagName("head")[0];
            h.appendChild(s);

            this.abort = function() {
              s.parentNode.removeChild(s);
            };
          }

          this.responseReceived = function(_updateId) {
            if (request !== null) {
              const req = request;
              request.script.parentNode.removeChild(request.script);
              request = null;
              handler(WT.ResponseStatus.OK, "", req.userData);
            }
          };

          this.sendUpdate = function(data, userData, id, timeout, _isPoll) {
            if (!sessionUrl) {
              return null;
            }
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

    this.setHtml = function(el, html, add) {
      function myImportNode(e, deep) {
        let newNode, i, il;
        switch (e.nodeType) {
          case 1: // element
            if (e.namespaceURI === null) {
              newNode = document.createElement(e.nodeName);
            } else {
              newNode = document.createElementNS(e.namespaceURI, e.nodeName);
            }
            if (e.attributes && e.attributes.length > 0) {
              for (i = 0, il = e.attributes.length; i < il;) {
                newNode.setAttribute(e.attributes[i].nodeName, e.getAttribute(e.attributes[i++].nodeName));
              }
            }
            if (deep && e.childNodes.length > 0) {
              for (i = 0, il = e.childNodes.length; i < il;) {
                const c = myImportNode(e.childNodes[i++], deep);
                if (c) {
                  newNode.appendChild(c);
                }
              }
            }
            return newNode;
          case 3: // text
          case 4: // cdata
          case 5: // comment
            return document.createTextNode(e.nodeValue);
        }

        return null;
      }

      if (_$_INNER_HTML_$_ && !add) {
        if (add) {
          el.innerHTML += html;
        } else {
          WT.saveReparented(el);
          el.innerHTML = html;
        }
      } else {
        let d = new DOMParser();
        const b = d.parseFromString("<div>" + html + "</div>", "application/xhtml+xml");
        d = b.documentElement;
        if (d.nodeType !== 1) { // element
          d = d.nextSibling;
        }

        if (!add) {
          WT.saveReparented(el);
          el.innerHTML = "";
        }

        for (let i = 0, il = d.childNodes.length; i < il;) {
          el.appendChild(myImportNode(d.childNodes[i++], true)); // TODO(Roel): eww
        }
      }
    };

    this.hasTag = function(e, s) {
      return e.nodeType === 1 && e.tagName && e.tagName.toUpperCase() === s;
    };

    this.insertAt = function(p, c, pos) {
      if (!p.childNodes.length) {
        p.appendChild(c);
      } else {
        for (let i = 0, j = 0, il = p.childNodes.length; i < il; ++i) {
          if (p.childNodes[i].classList.contains("wt-reparented")) {
            continue;
          }
          if (j === pos) {
            p.insertBefore(c, p.childNodes[i]);
            return;
          }
          ++j;
        }
        p.appendChild(c);
      }
    };

    this.remove = function(id) {
      const e = WT.getElement(id);
      if (e) {
        WT.saveReparented(e);
        e.parentNode.removeChild(e);
      }
    };

    this.replaceWith = function(w1Id, w2) {
      WT.$(w1Id).replaceWith(w2);

      /* Reapply client-side validation, bootstrap applys validation classes
         also outside the element into its ancestors */
      if (w2.wtValidate && WT.validate) {
        setTimeout(function() {
          WT.validate(w2);
        }, 0);
      }
    };

    this.contains = function(w1, w2) {
      let p = w2.parentNode;

      while (p && !WT.hasTag(p, "BODY")) {
        if (p === w1) {
          return true;
        }
        p = p.parentNode;
      }

      return false;
    };

    this.unstub = function(from, to, methodDisplay) {
      if (methodDisplay === 1) {
        if (from.style.display !== "none") {
          to.style.display = from.style.display;
        }
      } else {
        to.style.position = from.style.position;
        to.style.left = from.style.left;
        to.style.visibility = from.style.visibility;
      }

      if (from.style.height) {
        to.style.height = from.style.height;
      }
      if (from.style.width) {
        to.style.width = from.style.width;
      }

      to.style.boxSizing = from.style.boxSizing;
      const attrName = WT.styleAttribute("box-sizing");
      const vendorPrefix = WT.vendorPrefix(attrName);
      if (vendorPrefix) {
        to.style[attrName] = from.style[attrName];
      }
    };

    this.saveReparented = function(el) {
      el.querySelectorAll(".wt-reparented").forEach(function(elem) {
        const domRoot = document.querySelector(".Wt-domRoot");
        domRoot.appendChild(elem.parentNode.removeChild(elem));
      });
    };

    this.changeTag = function(e, type) {
      const n = document.createElement(type);

      /* For some reason fails on 'a' */
      if (type === "img" && n.mergeAttributes) {
        n.mergeAttributes(e, false);
        n.src = e.src;
      } else {
        if (e.attributes && e.attributes.length > 0) {
          for (let i = 0, il = e.attributes.length; i < il; i++) {
            const nn = e.attributes[i].nodeName;
            if (nn !== "type" && nn !== "name") {
              n.setAttribute(nn, e.getAttribute(nn));
            }
          }
        }
      }

      while (e.firstChild) {
        n.appendChild(e.removeChild(e.firstChild));
      }

      e.parentNode.replaceChild(n, e);
    };

    this.unwrap = function(e) {
      e = WT.getElement(e);
      if (!e.parentNode.className.indexOf("Wt-wrap")) {
        const wrapped = e;
        e = e.parentNode;
        if (e.className.length >= 8) {
          wrapped.className = e.className.substring(8);
        }
        const style = e.getAttribute("style");
        if (style) {
          wrapped.setAttribute("style", style);
        }
        e.parentNode.replaceChild(wrapped, e);
      } else {
        if (e.getAttribute("type") === "submit") {
          e.setAttribute("type", "button");
          e.removeAttribute("name");
        } else if (WT.hasTag(e, "A") && e.href.indexOf("&signal=") !== -1) {
          e.href = "javascript:void(0)";
        }
        if (WT.hasTag(e, "INPUT") && e.getAttribute("type") === "image") {
          WT.changeTag(e, "img");
        }
      }
    };

    this.navigateInternalPath = function(event, path) {
      const e = event || window.event;
      if (!e.ctrlKey && !e.metaKey && WT.button(e) <= 1) {
        WT.history.navigate(path, true);
        WT.cancelEvent(e, WT.CancelDefaultAction);
      }
    };

    this.ajaxInternalPaths = function(basePath) {
      document.querySelectorAll(".Wt-ip").forEach(function(elem) {
        let href = elem.getAttribute("href"), wtd = href.lastIndexOf("?wtd");
        if (wtd === -1) {
          wtd = href.lastIndexOf("&wtd");
        }
        if (wtd !== -1) {
          href = href.substring(0, wtd);
        }

        let internalPath;

        /*
         * On IE < 8, an absolute URL is read from href. In that case we
         * also turn the basePath into an absolute URL.
         */
        if (href.indexOf("://") !== -1) {
          const el = document.createElement("div");
          el.innerHTML = '<a href="' + basePath + '">x</a>';
          const absBase = el.firstChild.href;
          internalPath = href.substring(absBase.length - 1);
        } else {
          while (href.startsWith("../")) {
            href = href.substring(3);
          }
          if (href.charAt(0) !== "/") {
            href = "/" + href;
          }
          internalPath = href.substring(basePath.length);
          if (
            internalPath.startsWith("_=") &&
            basePath.charAt(basePath.length - 1) === "?"
          ) {
            internalPath = "?" + internalPath; /* eaten one too much */
          }
        }

        if (internalPath.length === 0 || internalPath.charAt(0) !== "/") {
          internalPath = "/" + internalPath;
        }
        if (internalPath.startsWith("/?_=")) {
          internalPath = internalPath.substring(4);
        }
        elem.setAttribute("href", href); // computes this.href
        elem.setAttribute("href", elem.href);
        elem.onclick = function(event) {
          WT.navigateInternalPath(event, internalPath);
        };
        elem.classList.remove("Wt-ip");
      });
    };

    this.resolveRelativeAnchors = function() {
      document.querySelectorAll(".Wt-rr").forEach(function(elem) {
        if (elem.href) {
          elem.setAttribute("href", elem.href);
        }
        if (elem.src) {
          elem.setAttribute("src", elem.src);
        }

        elem.classList.remove("Wt-rr");
      });
    };

    let delegating = false;

    this.CancelPropagate = 0x1;
    this.CancelDefaultAction = 0x2;
    this.CancelAll = 0x3;

    this.cancelEvent = function(e, cancelType) {
      if (delegating) {
        return;
      }

      const ct = typeof cancelType === UNDEFINED ? WT.CancelAll : cancelType;

      if (ct & WT.CancelDefaultAction) {
        if (e.preventDefault) {
          e.preventDefault();
        } else {
          e.returnValue = false;
        }
      }

      if (ct & WT.CancelPropagate) {
        if (e.stopPropagation) {
          e.stopPropagation();
        } else {
          e.cancelBubble = true;
        }
      }
    };

    this.getElement = function(id) {
      let el = document.getElementById(id);
      if (!el) {
        for (let i = 0; i < window.frames.length; ++i) {
          try {
            el = window.frames[i].document.getElementById(id);
            if (el) {
              return el;
            }
          } catch (e) {
            // Empty catch
          }
        }
      }
      return el;
    };

    this.$ = this.getElement;

    this.filter = function(edit, event, tokens) {
      const c = String.fromCharCode(
        (typeof event.charCode !== UNDEFINED) ?
          event.charCode :
          event.keyCode
      );
      if (!new RegExp(tokens).test(c)) {
        WT.cancelEvent(event);
      }
    };

    // Get coordinates of element relative to an ancestor object (or page origin).
    // It computes the location of the left-top corner of the margin-box.
    this.widgetPageCoordinates = function(obj, reference) {
      if (!obj.getBoundingClientRect) {
        return { x: 0, y: 0 };
      }

      const pageXOffset = typeof window.pageXOffset !== UNDEFINED ?
        window.pageXOffset :
        document.documentElement.scrollLeft;
      const pageYOffset = typeof window.pageYOffset !== UNDEFINED ?
        window.pageYOffset :
        document.documentElement.scrollTop;

      const rect = obj.getBoundingClientRect();
      let refLeft = -pageXOffset, refTop = -pageYOffset;
      if (reference) {
        const refRect = reference.getBoundingClientRect();
        refLeft = refRect.left;
        refTop = refRect.top;
      }
      return { x: rect.left - refLeft, y: rect.top - refTop };
    };

    // Get coordinates of (mouse) event relative to a element.
    this.widgetCoordinates = function(obj, e) {
      const p = WT.pageCoordinates(e);
      const w = WT.widgetPageCoordinates(obj);
      return { x: p.x - w.x, y: p.y - w.y };
    };

    // Get coordinates of (mouse) event relative to page origin.
    this.pageCoordinates = function(e) {
      if (!e) {
        e = window.event;
      }

      let posX = 0, posY = 0;

      const target = e.target || e.srcElement;

      // if this is an iframe, offset against the frame's position
      if (target && (target.ownerDocument !== document)) {
        for (let i = 0; i < window.frames.length; i++) {
          if (target.ownerDocument === window.frames[i].document) {
            try {
              const rect = window.frames[i].frameElement.getBoundingClientRect();
              posX = rect.left;
              posY = rect.top;
            } catch (e) {
              // Empty catch
            }
          }
        }
      }

      if (e.touches && e.touches[0]) {
        return WT.pageCoordinates(e.touches[0]);
      } else if (e.changedTouches && e.changedTouches[0]) {
        posX += e.changedTouches[0].pageX;
        posY += e.changedTouches[0].pageY;
      } else if (typeof e.pageX === "number") {
        posX += e.pageX;
        posY = e.pageY;
      } else if (typeof e.clientX === "number") {
        posX += e.clientX + document.body.scrollLeft +
          document.documentElement.scrollLeft;
        posY += e.clientY + document.body.scrollTop +
          document.documentElement.scrollTop;
      }

      return { x: posX, y: posY };
    };

    this.windowCoordinates = function(e) {
      const p = WT.pageCoordinates(e);
      const cx = p.x - document.body.scrollLeft - document.documentElement.scrollLeft;
      const cy = p.y - document.body.scrollTop - document.documentElement.scrollTop;

      return { x: cx, y: cy };
    };

    /**
     * @preserve Includes normalizeWheel from Fixed Data Tables for React by Facebook (BSD Licensed)
     */
    this.normalizeWheel = function(event) {
      const PIXEL_STEP = 10;
      const LINE_HEIGHT = 40;
      const PAGE_HEIGHT = 800;

      let sX = 0,
        sY = 0,
        // spinX, spinY
        pX = 0,
        pY = 0; // pixelX, pixelY

      // Legacy
      if ("detail" in event) {
        sY = event.detail;
      }
      if ("wheelDelta" in event) {
        sY = -event.wheelDelta / 120;
      }
      if ("wheelDeltaY" in event) {
        sY = -event.wheelDeltaY / 120;
      }
      if ("wheelDeltaX" in event) {
        sX = -event.wheelDeltaX / 120;
      }

      // side scrolling on FF with DOMMouseScroll
      if ("axis" in event && event.axis === event.HORIZONTAL_AXIS) {
        sX = sY;
        sY = 0;
      }

      pX = sX * PIXEL_STEP;
      pY = sY * PIXEL_STEP;

      if ("deltaY" in event) {
        pY = event.deltaY;
      }
      if ("deltaX" in event) {
        pX = event.deltaX;
      }

      if ((pX || pY) && event.deltaMode) {
        if (event.deltaMode === 1) {
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

      return { spinX: sX, spinY: sY, pixelX: pX, pixelY: pY };
    };

    this.wheelDelta = function(e) {
      let delta = 0;
      if (e.deltaY) {
        /* WheelEvent */
        delta = e.deltaY > 0 ? -1 : 1;
      } else if (e.wheelDelta) {
        /* IE/Opera. */
        delta = e.wheelDelta > 0 ? 1 : -1;
        /* if (window.opera)
          delta = -delta; */
      } else if (e.detail) {
        delta = e.detail < 0 ? 1 : -1;
      }
      return delta;
    };

    this.scrollHistory = function() {
      // after any hash change event (forward/backward, or user clicks
      // on an achor with internal path), the server calls this function
      // to update the scroll position of the main window
      try {
        if (window.history.state) {
          if (typeof window.history.state.pageXOffset !== UNDEFINED) {
            // scroll to a historic position where we have been before
            // console.log("scrollHistory: " + JSON.stringify(window.history.state));
            window.scrollTo(window.history.state.pageXOffset, window.history.state.pageYOffset);
          } else {
            // we went to a new hash (following an anchor, we assume some equivalence
            // with 'new page') that hasn't been scrolled yet.
            // Scroll to the top, which may be overriden by scrollIntoView (if the hash
            // exists somewhere as an object ID)
            // console.log("scrollHistory: new page scroll strategy");
            window.scrollTo(0, 0);
            WT.scrollIntoView(window.history.state.state);
          }
        }
      } catch (error) {
        console.log(error);
      }
    };

    this.scrollIntoView = function(id) {
      const hashI = id.indexOf("#");
      if (hashI !== -1) {
        id = id.substring(hashI + 1);
      }

      const obj = document.getElementById(id);
      if (obj) {
        /* Locate a suitable ancestor to scroll */
        for (let p = obj.parentNode; p !== document.body; p = p.parentNode) {
          if (
            p.scrollHeight > p.clientHeight &&
            WT.css(p, "overflow-y") === "auto"
          ) {
            const xy = WT.widgetPageCoordinates(obj, p);
            p.scrollTop += xy.y;
            return;
          }
        }
        obj.scrollIntoView(true);
      }
    };

    function isHighSurrogate(chr) {
      return 0xD800 <= chr && chr <= 0xDBFF;
    }

    function isLowSurrogate(chr) {
      return 0xDC00 <= chr && chr <= 0xDFFF;
    }

    function toUnicodeSelection(selection, text) {
      let start = selection.start;
      let end = selection.end;
      if (text) {
        for (let i = 0; i < text.length; ++i) {
          if (i >= selection.start && i >= selection.end) {
            return { start: start, end: end };
          }
          if (
            isHighSurrogate(text.charCodeAt(i)) &&
            (i + 1) < text.length &&
            isLowSurrogate(text.charCodeAt(i + 1))
          ) {
            if (i < selection.start) {
              --start;
            }
            if (i < selection.end) {
              --end;
            }
          }
        }
      }
      return { start: start, end: end };
    }

    this.getUnicodeSelectionRange = function(elem) {
      return toUnicodeSelection(WT.getSelectionRange(elem), elem.value);
    };

    this.getSelectionRange = function(elem) {
      if (document.selection) { // IE
        if (WT.hasTag(elem, "TEXTAREA")) {
          const sel = document.selection.createRange();
          const sel2 = sel.duplicate();
          sel2.moveToElementText(elem);

          let pos = 0;
          if (sel.text.length > 1) {
            pos = pos - sel.text.length;
            if (pos < 0) {
              pos = 0;
            }
          }

          let caretPos = -1 + pos;
          sel2.moveStart("character", pos);

          while (sel2.inRange(sel)) {
            sel2.moveStart("character");
            caretPos++;
          }

          const selStr = sel.text.replace(/\r/g, "");

          return { start: caretPos, end: selStr.length + caretPos };
        } else {
          let start = -1;
          let end = -1;

          const val = elem.value;
          if (val) {
            let range = document.selection.createRange().duplicate();

            range.moveEnd("character", val.length);
            start = range.text === "" ? val.length : val.lastIndexOf(range.text);

            range = document.selection.createRange().duplicate();
            range.moveStart("character", -val.length);
            end = range.text.length;
          }

          return { start: start, end: end };
        }
      } else if (elem.selectionStart || elem.selectionStart === 0) {
        return { start: elem.selectionStart, end: elem.selectionEnd };
      } else {
        return { start: -1, end: -1 };
      }
    };

    this.setUnicodeSelectionRange = function(elem, start, end) {
      return WT.setSelectionRange(elem, start, end, true);
    };

    this.setSelectionRange = function(elem, start, end, unicode) {
      /**
       * @preserve Includes jQuery Caret Range plugin
       * Copyright (c) 2009 Matt Zabriskie
       * Released under the MIT and GPL licenses.
       */
      const val = elem.value;

      if (typeof start !== "number") {
        start = -1;
      }
      if (typeof end !== "number") {
        end = -1;
      }
      if (start < 0) {
        start = 0;
      }
      if (end > val.length) {
        end = val.length;
      }
      if (end < start) {
        end = start;
      }
      if (start > end) {
        start = end;
      }

      elem.focus();

      if (unicode) {
        for (let i = 0; i < val.length; ++i) {
          if (i >= start && i >= end) {
            break;
          }
          if (
            isHighSurrogate(val.charCodeAt(i)) &&
            (i + 1) < val.length &&
            isLowSurrogate(val.charCodeAt(i + 1))
          ) {
            if (i < start) {
              ++start;
            }
            if (i < end) {
              ++end;
            }
          }
        }
      }

      if (typeof elem.selectionStart !== UNDEFINED) {
        elem.selectionStart = start;
        elem.selectionEnd = end;
      } else if (document.selection) {
        const range = elem.createTextRange();
        range.collapse(true);
        range.moveStart("character", start);
        range.moveEnd("character", end - start);
        range.select();
      }
    };

    this.isKeyPress = function(e) {
      if (!e) {
        e = window.event;
      }

      if (e.ctrlKey || e.metaKey) {
        return false;
      }

      const charCode = (typeof e.charCode !== UNDEFINED) ? e.charCode : 0;

      if (charCode > 0) {
        return true;
      } else {
        return (e.keyCode === 13 || e.keyCode === 27 || e.keyCode === 32 ||
          (e.keyCode > 46 && e.keyCode < 112));
      }
    };

    let repeatT = null, repeatI = null;

    this.isDblClick = function(o, e) {
      if (
        o.wtClickTimeout &&
        Math.abs(o.wtE1.clientX - e.clientX) < 3 &&
        Math.abs(o.wtE1.clientY - e.clientY) < 3
      ) {
        clearTimeout(o.wtClickTimeout);
        o.wtClickTimeout = null;
        o.wtE1 = null;
        return true;
      } else {
        return false;
      }
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

    let cacheC = null, cacheS = null;

    this.css = function(c, s) {
      if (c.style[s]) {
        return c.style[s];
      } else {
        if (c !== cacheC) {
          cacheC = c;

          if (window.getComputedStyle) {
            cacheS = window.getComputedStyle(c, null);
          } else if (c.currentStyle) {
            cacheS = c.currentStyle;
          } else {
            cacheS = null;
          }
        }

        return cacheS ? cacheS[s] : null;
      }
    };

    function parseCss(value, regex, defaultvalue) {
      if (value === "auto" || value === null) {
        return defaultvalue;
      }
      const m = regex.exec(value),
        v = m && m.length === 2 ? m[1] : null;
      return v ? parseFloat(v) : defaultvalue;
    }

    this.parsePx = function(v) {
      return parseCss(v, /^\s*(-?\d+(?:\.\d+)?)\s*px\s*$/i, 0);
    };

    this.parsePct = function(v, defaultValue) {
      return parseCss(v, /^\s*(-?\d+(?:\.\d+)?)\s*%\s*$/i, defaultValue);
    };

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
        let n = str.search(/-./);
        while (n !== -1) {
          const letter = (str.charAt(n + 1)).toUpperCase();
          str = str.replace(/-./, letter);
          n = str.search(/-./);
        }
        return str;
      }

      const prefixes = ["", "-moz-", "-webkit-", "-o-", "-ms-"];
      const elem = document.createElement("div");

      for (let i = 0, il = prefixes.length; i < il; ++i) {
        const attr = toCamelCase(prefixes[i] + cssProp);
        if (attr in elem.style) {
          return attr;
        }
      }
      return toCamelCase(cssProp);
    };

    this.vendorPrefix = function(attr) {
      const prefixes = ["Moz", "Webkit", "O", "Ms"];
      for (let i = 0, il = prefixes.length; i < il; ++i) {
        if (attr.search(prefixes[i]) !== -1) {
          return prefixes[i];
        }
      }
      return "";
    };

    this.boxSizing = function(w) {
      return (WT.css(w, WT.styleAttribute("box-sizing"))) === "border-box";
    };

    // Return if an element (or one of its ancestors) is hidden
    this.isHidden = function(w) {
      if (w.style.display === "none" || w.classList.contains("out")) {
        return true;
      } else {
        w = w.parentNode;
        if (w && !WT.hasTag(w, "BODY")) {
          return WT.isHidden(w);
        } else {
          return false;
        }
      }
    };

    this.innerWidth = function(el) {
      let result = el.offsetWidth;
      if (!WT.boxSizing(el)) {
        result -= WT.px(el, "paddingLeft") + WT.px(el, "paddingRight") +
          WT.px(el, "borderLeftWidth") + WT.px(el, "borderRightWidth");
      }
      return result;
    };

    this.innerHeight = function(el) {
      let result = el.offsetHeight;
      if (!WT.boxSizing(el)) {
        result -= WT.px(el, "paddingTop") + WT.px(el, "paddingBottom") +
          WT.px(el, "borderTopWidth") + WT.px(el, "borderBottomWidth");
      }
      return result;
    };

    this.IEwidth = function(c, min, max) {
      if (c.parentNode) {
        const r = c.parentNode.clientWidth -
          WT.px(c, "marginLeft") -
          WT.px(c, "marginRight") -
          WT.px(c, "borderLeftWidth") -
          WT.px(c, "borderRightWidth") -
          WT.px(c.parentNode, "paddingLeft") -
          WT.px(c.parentNode, "paddingRight");

        min = WT.parsePct(min, 0);
        max = WT.parsePct(max, 100000);

        if (r < min) {
          return min - 1;
        } else if (r > max) {
          return max + 1;
        } else if (c.style["styleFloat"] !== "") {
          return min - 1;
        } else {
          return "auto";
        }
      } else {
        return "auto";
      }
    };

    this.hide = function(o) {
      WT.getElement(o).style.display = "none";
    };
    this.inline = function(o) {
      WT.getElement(o).style.display = "inline";
    };
    this.block = function(o) {
      WT.getElement(o).style.display = "block";
    };
    this.show = function(o, s) {
      WT.getElement(o).style.display = s;
    };

    let captureElement = null;
    this.firedTarget = null;

    this.target = function(event) {
      try {
        return WT.firedTarget || event.target || event.srcElement;
      } catch (err) {
        return null;
      }
    };

    function delegateCapture(e) {
      if (captureElement === null) {
        return null;
      }

      if (!e) {
        e = window.event;
      }

      if (e) {
        const t = WT.target(e);
        let p = t;

        while (p && p !== captureElement) {
          p = p.parentNode;
        }

        /*
         * We don't need to capture the event when the event falls inside the
         * capture element. In this way, more specific widgets inside may still
         * handle (and cancel) the event if they want.
         *
         * On IE this means that we need to delegate the event to the event
         * target; on other browsers we can just rely on event bubbling.
         */
        if (p === captureElement) {
          return null;
        } else {
          return captureElement;
        }
      } else {
        return captureElement;
      }
    }

    function mouseMove(e) {
      const d = delegateCapture(e);

      if (d && !delegating) {
        if (!e) {
          e = window.event;
        }
        delegating = true;
        WT.condCall(d, "onmousemove", e);
        delegating = false;
        return false;
      } else {
        return true;
      }
    }

    function mouseUp(e) {
      const d = delegateCapture(e);
      WT.capture(null);

      if (d) {
        if (!e) {
          e = window.event;
        }

        WT.condCall(d, "onmouseup", e);

        WT.cancelEvent(e, WT.CancelPropagate);

        return false;
      } else {
        return true;
      }
    }

    function touchMove(e) {
      const d = delegateCapture(e);

      if (d && !delegating) {
        if (!e) {
          e = window.event;
        }
        delegating = true;
        WT.condCall(d, "ontouchmove", e);
        delegating = false;
        return false;
      } else {
        return true;
      }
    }

    function touchEnd(e) {
      const d = delegateCapture(e);
      WT.capture(null);

      if (d) {
        if (!e) {
          e = window.event;
        }

        WT.condCall(d, "ontouchend", e);

        WT.cancelEvent(e, WT.CancelPropagate);

        return false;
      } else {
        return true;
      }
    }
    let captureInitialized = false;

    function attachMouseHandlers(el) {
      el.addEventListener("mousemove", mouseMove, true);
      el.addEventListener("mouseup", mouseUp, true);

      if (WT.isGecko) {
        window.addEventListener("mouseout", function(e) {
          if (
            !e.relatedTarget &&
            WT.hasTag(e.target, "HTML")
          ) {
            mouseUp(e);
          }
        }, true);
      }
    }

    function attachTouchHandlers(el) {
      el.addEventListener("touchmove", touchMove, true);
      el.addEventListener("touchend", touchEnd, true);
    }

    function initCapture() {
      if (captureInitialized) {
        return;
      }

      captureInitialized = true;

      const db = document.body;
      attachMouseHandlers(db);
      attachTouchHandlers(db);
    }

    this.capture = function(obj) {
      initCapture();

      if (captureElement && obj) {
        return;
      }

      // attach to possible iframes
      for (let i = 0; i < window.frames.length; i++) {
        try {
          if (!window.frames[i].document.body.hasMouseHandlers) {
            attachMouseHandlers(window.frames[i].document.body);
            window.frames[i].document.body.hasMouseHandlers = true;
          }
        } catch (e) {
          // Empty catch
        }
      }

      captureElement = obj;

      const db = document.body;
      if (!document.body.addEventListener) {
        if (obj !== null) {
          db.setCapture();
        } else {
          db.releaseCapture();
        }
      }

      if (obj !== null) {
        db.classList.add("unselectable");
        db.setAttribute("unselectable", "on");
        db.onselectstart = "return false;";
      } else {
        db.classList.remove("unselectable");
        db.setAttribute("unselectable", "off");
        db.onselectstart = "";
      }
    };

    this.checkReleaseCapture = function(obj, e) {
      if (e && captureElement && (obj === captureElement) && (e.type === "mouseup" || e.type === "touchend")) {
        this.capture(null);
      }
    };

    this.getElementsByClassName = function(className, parentElement) {
      if (document.getElementsByClassName) {
        return parentElement.getElementsByClassName(className);
      } else {
        const cc = parentElement.getElementsByTagName("*");
        const els = [];
        let c;
        for (let i = 0, length = cc.length; i < length; i++) {
          c = cc[i];
          if (c.className.indexOf(className) !== -1) {
            els.push(c);
          }
        }
        return els;
      }
    };

    /* Firefox, IE9 etc... */
    let inlineStyleSheet = null;

    function getInlineStyleSheet() {
      if (!inlineStyleSheet) {
        const ds = document.styleSheets;
        for (let i = 0, il = ds.length; i < il; ++i) {
          const s = ds[i];
          if (WT.hasTag(ds[i].ownerNode, "STYLE")) {
            inlineStyleSheet = s;
            break;
          }
        }

        if (!inlineStyleSheet) {
          const s = document.createElement("style");
          document.getElementsByTagName("head")[0].appendChild(s);

          inlineStyleSheet = s.sheet;
        }
      }

      return inlineStyleSheet;
    }

    this.addCss = function(selector, style) {
      const s = getInlineStyleSheet();

      // strange error with IE9 when in iframe
      const pos = s.cssRules ? s.cssRules.length : 0;
      s.insertRule(selector + " { " + style + " }", pos);
    };

    /* IE<9 & Konqueror */
    this.addCssText = function(cssText) {
      let s = document.getElementById("Wt-inline-css");

      if (!s) {
        s = document.createElement("style");
        s.id = "Wt-inline-css";
        document.getElementsByTagName("head")[0].appendChild(s);
      }

      if (!s.styleSheet) { // Konqueror
        const t = document.createTextNode(cssText);
        s.appendChild(t);
      } else {
        let ss = s.previousSibling;
        if (
          !ss ||
          !WT.hasTag(ss, "STYLE") ||
          ss.styleSheet.cssText.length > 32 * 1024
        ) {
          ss = document.createElement("style");
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
        for (let i = 0; i < document.styleSheets.length; i++) {
          const styleSheet = document.styleSheets[i];
          let ii = 0;
          let cssRule;
          do {
            cssRule = null;
            try {
              if (styleSheet.cssRules) {
                cssRule = styleSheet.cssRules[ii];
              } else if (styleSheet.rules) {
                cssRule = styleSheet.rules[ii];
              }
              if (cssRule && cssRule.selectorText) {
                if (cssRule.selectorText.toLowerCase() === selector) {
                  if (deleteFlag === "delete") {
                    if (styleSheet.cssRules) {
                      styleSheet.deleteRule(ii);
                    } else {
                      styleSheet.removeRule(ii);
                    }
                    return true;
                  } else {
                    return cssRule;
                  }
                }
              }
            } catch (err) {
              /*
               * firefox security error 1000 when access a stylesheet.cssRules
               * hosted from another domain
               */
            }

            ++ii;
          } while (cssRule);
        }
      }

      return false;
    };

    this.removeCssRule = function(selector) {
      return WT.getCssRule(selector, "delete");
    };

    this.addStyleSheet = function(uri, media) {
      if (document.createStyleSheet) {
        setTimeout(function() {
          document.createStyleSheet(uri);
        }, 15);
      } else {
        const s = document.createElement("link");
        s.setAttribute("href", uri);
        s.setAttribute("type", "text/css");
        s.setAttribute("rel", "stylesheet");
        if (media !== "" && media !== "all") {
          s.setAttribute("media", media);
        }
        const ll = document.getElementsByTagName("link");
        if (ll.length > 0) {
          const l = ll[ll.length - 1];
          l.parentNode.insertBefore(s, l.nextSibling);
        } else {
          document.body.appendChild(s);
        }
      }
    };

    this.removeStyleSheet = function(uri) {
      document.querySelectorAll('link[rel=stylesheet][href~="' + uri + '"]').forEach(function(link) {
        link.remove();
      });
      const sheets = document.styleSheets;
      for (let i = 0; i < sheets.length; ++i) {
        const sheet = sheets[i];
        let j = 0;
        if (sheet) {
          let rule = null;
          do {
            try {
              rule = sheet.cssRules[j]; // firefox

              if (
                rule && rule.cssText ===
                  '@import url("' + uri + '");'
              ) {
                sheet.deleteRule(j); // firfox
                break; // only remove 1 rule !!!!
              }
            } catch (err) {
              /*
               * firefox security error 1000 when access a stylesheet.cssRules
               * hosted from another domain
               */
            }
            ++j;
          } while (rule);
        }
      }
    };

    this.windowSize = function() {
      let x, y;

      if (typeof (window.innerWidth) === "number") {
        x = window.innerWidth;
        y = window.innerHeight;
      } else {
        x = document.documentElement.clientWidth;
        y = document.documentElement.clientHeight;
      }

      return { x: x, y: y };
    };

    /*
     * position right to (x) or left from (rightx) and
     * bottom of (y) or top from (bottomy)
     */
    this.fitToWindow = function(e, x, y, rightx, bottomy) {
      const hsides = ["left", "right"],
        vsides = ["top", "bottom"];

      e.style[hsides[0]] = e.style[hsides[1]] = "auto";
      e.style[vsides[0]] = e.style[vsides[1]] = "auto";

      let reserveWidth = e.offsetWidth,
        reserveHeight = e.offsetHeight,
        hside,
        vside;
      const windowSize = WT.windowSize(),
        windowX = document.body.scrollLeft + document.documentElement.scrollLeft,
        windowY = document.body.scrollTop + document.documentElement.scrollTop;

      /*
       * Should really distinguish between static versus dynamic: for a
       * widget that can grow dynamically (e.g. a suggestion popup) we
       * should prepare ourselves and consider maximum size here
       */
      if (!e.classList.contains("Wt-tooltip")) {
        reserveWidth = WT.px(e, "maxWidth") || reserveWidth;
        reserveHeight = WT.px(e, "maxHeight") || reserveHeight;
      }

      const op = e.offsetParent;
      if (!op) {
        return;
      }

      const offsetParent = WT.widgetPageCoordinates(op);

      if (reserveWidth > windowSize.x) {
        // wider than window
        x = windowX;
        hside = 0;
      } else if (x + reserveWidth > windowX + windowSize.x) {
        // too far right, chose other side
        let scrollX = op.scrollLeft;
        if (op === document.body) {
          scrollX = op.clientWidth - windowSize.x;
        }
        rightx = rightx - offsetParent.x + scrollX;
        x = op.clientWidth - (rightx + WT.px(e, "marginRight"));
        hside = 1;
      } else {
        let scrollX = op.scrollLeft;
        if (op === document.body) {
          scrollX = 0;
        }
        x = x - offsetParent.x + scrollX;
        x = x - WT.px(e, "marginLeft");
        hside = 0;
      }

      if (reserveHeight > windowSize.y) {
        // taller than window
        y = windowY;
        vside = 0;
      } else if (y + reserveHeight > windowY + windowSize.y) {
        // too far below, chose other side
        if (bottomy > windowY + windowSize.y) {
          bottomy = windowY + windowSize.y;
        }
        let scrollY = op.scrollTop;
        if (op === document.body) {
          scrollY = op.clientHeight - windowSize.y;
        }
        bottomy = bottomy - offsetParent.y + scrollY;
        y = op.clientHeight -
          (bottomy + WT.px(e, "marginBottom") + WT.px(e, "borderBottomWidth"));
        vside = 1;
      } else {
        let scrollY = op.scrollTop;
        if (op === document.body) {
          scrollY = 0;
        }
        y = y - offsetParent.y + scrollY;
        y = y - WT.px(e, "marginTop") + WT.px(e, "borderTopWidth");
        vside = 0;
      }

      /*
      if (x < wx)
        x = wx + ws.x - e.offsetWidth - 3;
      if (y < wy)
        y = wy + ws.y - e.offsetHeight - 3;
      */

      e.style[hsides[hside]] = x + "px";
      e.style[vsides[vside]] = y + "px";
    };

    this.positionXY = function(id, x, y) {
      const w = WT.getElement(id);

      if (!WT.isHidden(w)) {
        w.style.display = "block";
        WT.fitToWindow(w, x, y, x, y);
      }
    };

    this.Horizontal = 0x1;
    this.Vertical = 0x2;

    this.positionAtWidget = function(id, atId, orientation, delta) {
      const w = WT.getElement(id),
        atw = WT.getElement(atId);

      if (!delta) {
        delta = 0;
      }

      if (!atw || !w) {
        return;
      }

      const xy = WT.widgetPageCoordinates(atw);
      let x,
        y,
        rightx,
        bottomy;

      w.style.position = "absolute";
      if (WT.css(w, "display") === "none") {
        w.style.display = "block";
      }

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
      let p, pp = atw;
      w.parentNode.removeChild(w);

      for (p = pp.parentNode; !p.classList.contains("Wt-domRoot"); p = p.parentNode) {
        if (p.wtReparentBarrier) {
          break;
        }

        // e.g. a layout widget has clientHeight=0 since it's relative
        // with only absolutely positioned children. We are a bit more liberal
        // here to catch other simular situations, and 100px seems like space
        // needed anyway?
        //
        // We need to check whether overflowX or overflowY is not visible, because
        // of an issue on Firefox where clientWidth !== scrollWidth and
        // clientHeight !== scrollHeight when using the border-collapse CSS property.
        if (
          WT.css(p, "display") !== "inline" &&
          p.clientHeight > 100 &&
          (getComputedStyle(p).overflowY === "scroll" ||
            getComputedStyle(p).overflowX === "scroll" ||
            (p.scrollHeight > p.clientHeight && getComputedStyle(p).overflowY === "auto") ||
            (p.scrollWidth > p.clientWidth && getComputedStyle(p).overflowX === "auto"))
        ) {
          break;
        }

        pp = p;
      }

      const posP = WT.css(p, "position");
      if (posP !== "absolute" && posP !== "relative") {
        p.style.position = "relative";
      }

      p.appendChild(w);
      w.classList.add("wt-reparented");

      WT.fitToWindow(w, x, y, rightx, bottomy);

      w.style.visibility = "";
    };

    this.hasFocus = function(el) {
      try {
        return el === document.activeElement;
      } catch (e) {
        return false;
      }
    };

    this.progressed = function(domRoot) {
      const doc = document, db = doc.body;
      const form = this.getElement("Wt-form");

      domRoot.style.display = form.style.display;
      form.parentNode.replaceChild(domRoot, form);

      if (db.removeEventListener) {
        db.removeEventListener("click", delayClick, true);
      } else {
        db.detachEvent("click", delayClick);
      }

      setTimeout(function() {
        for (let i = 0, il = delayedClicks.length; i < il; ++i) {
          if (doc.createEvent) {
            const e = delayedClicks[i];
            const ec = doc.createEvent("MouseEvents");
            ec.initMouseEvent(
              "click",
              e.bubbles,
              e.cancelable,
              window,
              e.detail,
              e.screenX,
              e.screenY,
              e.clientX,
              e.clientY,
              e.ctrlKey,
              e.altKey,
              e.shiftKey,
              e.metaKey,
              e.button,
              null
            );
            const el = WT.getElement(e.targetId);
            if (el) {
              el.dispatchEvent(ec);
            }
          } else {
            const e = delayedClicks[i];
            const ec = doc.createEventObject();
            for (const i of Object.keys(e)) {
              ec[i] = e[i];
            }
            const el = WT.getElement(e.targetId);
            if (el) {
              el.fireEvent("onclick", ec);
            }
          }
        }
      }, 0);
    };

    /*
     * A less aggressive URL encoding than encodeURIComponent which does
     * for example not encode '/'
     */
    function gentleURIEncode(s) {
      return s.replace(/%/g, "%25")
        .replace(/\+/g, "%2b")
        .replace(/ /g, "%20")
        // .replace(/#/g, '%23')
        .replace(/&/g, "%26");
    }

    // we need to update the scroll position at the scroll event,
    // because we don't have the chance to update the html5history
    // state anymore at the moment that onPopState() is called.
    // For navigation, when pushState() is called, the scroll
    // history can be updated before the pushState() call.

    // delayCallback: delay the calling of the callback by delay
    //
    // as long as events keep coming in, we delay it further, so
    // the callback will only run "delay" ms after the last invocation.
    function delayCallback(callback, delay) {
      let timer = null;
      let args = null;

      function dispatch() {
        callback.apply(null, args);
        timer = null;
        args = null;
      }

      function proxy() {
        args = arguments;

        if (timer) {
          clearTimeout(timer);
          timer = null;
        }
        timer = setTimeout(dispatch, delay);
      }

      return proxy;
    }

    function updateScrollHistory() {
      // console.log("updateScrollHistory");
      try {
        let newState = window.history.state;
        if (window.history.state === null) {
          // freshly initiated session, no state present yet
          newState = {};
          newState.state = "";
          newState.title = window.document.title;
        }
        newState.pageXOffset = window.pageXOffset;
        newState.pageYOffset = window.pageYOffset;
        window.history.replaceState(newState, newState.title);
      } catch (error) {
        // shouldn't happen
        console.log(error.toString());
      }
    }
    window.addEventListener("scroll", delayCallback(updateScrollHistory, 100));

    // the 'auto' scrollRestoration gives too much flicker, since it
    // updates the scroll state before the page is updated
    // Browsers not supporting manual scrollRestoration, the flicker
    // should not be worse than what it was.
    window.history.scrollRestoration = "manual";

    this.history = (function() {
      let currentState = null, baseUrl = null, ugly = false, cb = null;
      const stateMap = {}, w = window;

      function saveState(state) {
        stateMap[w.location.pathname + w.location.search] = state;
      }

      function stripParameter(q, name) {
        if (q.length > 1) {
          q = q.substring(1);
        }

        const qp = q.split("&");
        q = "";

        for (let i = 0, il = qp.length; i < il; ++i) {
          if (qp[i].split("=")[0] !== name) {
            q += (q.length ? "&" : "?") + qp[i];
          }
        }

        return q;
      }

      return {
        _initialize: function() {},

        _initTimeout: function() {},

        register: function(initialState, onStateChange) {
          currentState = initialState;
          cb = onStateChange;
          saveState(initialState);

          function onPopState(event) {
            let newState = null;
            if (event.state && event.state.state) {
              newState = event.state.state;
            }

            if (newState === null) {
              newState = stateMap[w.location.pathname + w.location.search];
            }

            if (newState === null) {
              const endw = w.location.pathname.lastIndexOf(currentState);
              if (
                endw !== -1 &&
                endw === w.location.pathname.length - currentState.length
              ) {
                saveState(currentState);
                return;
              } else {
                newState = w.location.pathname.substring(baseUrl.length);
              }
            }

            if (newState !== currentState) {
              currentState = newState;
              onStateChange(currentState !== "" ? currentState : "/");
            }
            // console.log("onPopState: " + JSON.stringify(window.history.state));
          }

          w.addEventListener("popstate", onPopState, false);
        },

        initialize: function(stateField, histFrame, deployUrl) {
          WT.resolveRelativeAnchors();

          baseUrl = deployUrl;
          if (baseUrl.length >= 1 && baseUrl[baseUrl.length - 1] === "/") {
            _$_$if_UGLY_INTERNAL_PATHS_$_();
            ugly = true;
            _$_$endif_$_();
            _$_$ifnot_UGLY_INTERNAL_PATHS_$_();
            baseUrl = baseUrl.substring(0, baseUrl.length - 1);
            _$_$endif_$_();
          }
        },

        removeSessionId: function() {
          let pathname = w.location.pathname;
          const idx = pathname.indexOf(";jsessionid=");
          if (idx !== -1) {
            pathname = pathname.substring(0, idx);
          }

          const url = pathname + stripParameter(w.location.search, "wtd");
          w.history.replaceState(null, null, url);
        },

        navigate: function(state, generateEvent) {
          // console.log("navigate: " + state);
          WT.resolveRelativeAnchors();

          currentState = state;

          const ip = gentleURIEncode(state);
          let url = baseUrl;

          if (ip.length !== 0) {
            url += (ugly ? "?_=" : "") + ip;
          }

          if (!ugly) {
            url += window.location.search;
          } else {
            let q = stripParameter(window.location.search, "_");

            if (q.length > 1) {
              if (q.length > 2 && q[0] === "?" && q[1] === "&") {
                q = q.substring(1);
              }
              if (url.indexOf("?") === -1) {
                url += "?" + q.substring(1);
              } else {
                url += "&" + q.substring(1);
              }
            }
          }

          try {
            const historyState = {};
            historyState.state = state ? state : "";
            // By not setting historyState.page[XY]Offset, we indicate that
            // this state change was made by navigation rather than by
            // the back/forward button
            // keep title for call to replaceState when page offset is updated
            historyState.title = document.title;
            // update scroll position of stack top with the position at the time of leaving the page
            updateScrollHistory();
            // console.log("pushState before: " + JSON.stringify(window.history.state));
            window.history.pushState(historyState, document.title, url);
            // console.log("pushState after: " + JSON.stringify(window.history.state));
          } catch (error) {
            /*
               * In case we are wrong about our baseUrl or base href
               * In any case, this shouldn't be fatal.
               */
            console.log(error.toString());
          }

          // We used to call scrollIntoView here. We modified this to have
          // scrollIntoView called after the server round-trip, so that the
          // new content is certainly visible before we scroll. This avoids
          // flicker. If the rendering result was pre-learned client-side,
          // the page will scroll to the right position only after a server
          // round-trip, which is not ideal.

          if (generateEvent) {
            cb(state);
          }
        },

        getCurrentState: function() {
          return currentState;
        },
      };
    })();

    this.maxZIndex = function() {
      let maxz = 0;
      document.querySelectorAll(".Wt-dialog, .modal, .modal-dialog").forEach(function(elem) {
        maxz = Math.max(maxz, WT.css(elem, "z-index"));
      });

      return maxz;
    };

    this.ResponseStatus = {
      OK: 0,
      Error: 1,
      Timeout: 2,
    };
  })();
}

if (window._$_APP_CLASS_$_ && window._$_APP_CLASS_$_._p_) {
  try {
    window._$_APP_CLASS_$_._p_.quit(null);
  } catch (e) {
    // Empty catch
  }
}

window._$_APP_CLASS_$_ = new (function() {
  const self = this;
  const WT = _$_WT_CLASS_$_;
  const UNDEFINED = "undefined";

  let downX = 0;
  let downY = 0;

  const deployUrl = _$_DEPLOY_PATH_$_;

  function saveDownPos(e) {
    const coords = WT.pageCoordinates(e);
    downX = coords.x;
    downY = coords.y;
  }

  let currentHash = null;

  function onHashChange() {
    const newLocation = _$_WT_CLASS_$_.history.getCurrentState();

    if (
      newLocation !== null &&
      newLocation.length > 0 &&
      !newLocation.startsWith("/")
    ) {
      return;
    }

    if (currentHash === newLocation) {
      return;
    }

    currentHash = newLocation;

    setTimeout(function() {
      update(null, "hash", null, true);
    }, 1);
  }

  function setHash(newLocation, generateEvent) {
    if (currentHash === newLocation || (!currentHash && newLocation === "/")) {
      return;
    }

    if (!generateEvent) {
      currentHash = newLocation;
    }

    WT.history.navigate(newLocation, generateEvent);
  }

  const dragState = {
    object: null,
    sourceId: null,
    mimeType: null,
    dropOffsetX: null,
    dragOffsetY: null,
    dropTarget: null,
    objectPrevStyle: null,
    xy: null,
  };

  let touchTimer;
  const touchduration = 1000;

  function touchStart(obj, e) {
    touchTimer = setTimeout(function() {
      dragStart(obj, e);
    }, touchduration);
  }

  function touchEnded() {
    if (touchTimer) {
      clearTimeout(touchTimer);
    }
  }

  function dragStart(obj, e) {
    if (e.touches) {
      if ("vibrate" in navigator) {
        navigator.vibrate = navigator.vibrate || navigator.webkitVibrate || navigator.mozVibrate || navigator.msVibrate;
        if (navigator.vibrate) {
          navigator.vibrate(100);
        }
      }
    }
    if ((e.ctrlKey || WT.button(e) > 1) && !e.touches) { // Ignore drags with rigth click.
      return true;
    }
    const t = WT.target(e);
    if (t) {
      /*
       * Ignore drags that start on a scrollbar (#1231)
       */
      if (
        WT.css(t, "display") !== "inline" &&
        (t.offsetWidth > t.clientWidth ||
          t.offsetHeight > t.clientHeight)
      ) {
        const wc = WT.widgetPageCoordinates(t);
        const pc = WT.pageCoordinates(e);
        const x = pc.x - wc.x;
        const y = pc.y - wc.y;
        if (x > t.clientWidth || y > t.clientHeight) {
          return true;
        }
      }
    }

    // drag element attributes:
    //   dwid = dragWidgetId
    //   dsid = dragSourceId
    //   dmt = dragMimeType
    // drop element attributes:
    //   amts = acceptMimeTypes
    //   ds = dropSignal

    const ds = dragState;

    ds.object = WT.getElement(obj.getAttribute("dwid"));
    if (ds.object === null) {
      return true;
    }

    ds.sourceId = obj.getAttribute("dsid");
    ds.objectPrevStyle = {
      position: ds.object.style.position,
      display: ds.object.style.display,
      left: ds.object.style.left,
      top: ds.object.style.top,
      className: ds.object.className,
      parent: ds.object.parentNode,
      zIndex: ds.object.zIndex,
    };

    ds.object.parentNode.removeChild(ds.object);
    ds.object.style.position = "absolute";
    ds.object.className = ds.objectPrevStyle.className + "";
    ds.object.style.zIndex = "200000";
    document.body.appendChild(ds.object);

    WT.capture(null);
    WT.capture(ds.object);

    ds.object.onmousemove = dragDrag;
    ds.object.onmouseup = dragEnd;
    // New mousedown (other button): abort drag
    document.addEventListener("mousedown", dragAbort);
    // Release mouse outside of page (fires after ds.object.onmouseup)
    window.addEventListener("mouseup", dragAbort);
    // Another touch: abort drag
    document.addEventListener("touchstart", dragAbort);
    ds.object.ontouchmove = dragDrag;
    ds.object.ontouchend = dragEnd;

    ds.offsetX = -4;
    ds.offsetY = -4;
    ds.dropTarget = null;
    ds.mimeType = obj.getAttribute("dmt");
    ds.xy = WT.pageCoordinates(e);

    WT.cancelEvent(e, WT.CancelPropagate);

    return false;
  }

  function dragDrag(e) {
    e = e || window.event;
    if (dragState.object !== null) {
      const ds = dragState;
      const xy = WT.pageCoordinates(e);

      if (
        ds.object.style.display !== "" &&
        ds.xy.x !== xy.x &&
        ds.xy.y !== xy.y
      ) {
        ds.object.style.display = "";
      }

      ds.object.style.left = (xy.x - ds.offsetX) + "px";
      ds.object.style.top = (xy.y - ds.offsetY) + "px";

      const prevDropTarget = ds.dropTarget;
      let t;
      if (e.changedTouches) {
        ds.object.style["display"] = "none";
        t = document.elementFromPoint(e.changedTouches[0].clientX, e.changedTouches[0].clientY);
        ds.object.style["display"] = "";
      } else {
        t = WT.target(e);
        if (t === ds.object) {
          if (document.elementFromPoint) {
            ds.object.style["display"] = "none";
            t = document.elementFromPoint(e.clientX, e.clientY);
            ds.object.style["display"] = "";
          }
        }
      }

      const mimeType = "{" + ds.mimeType + ":";
      let amts = null;

      ds.dropTarget = null;

      while (t) {
        amts = t.getAttribute("amts");
        if ((amts !== null) && (amts.indexOf(mimeType) !== -1)) {
          ds.dropTarget = t;
          break;
        }
        t = t.parentNode;
        if (!t.tagName || WT.hasTag(t, "HTML")) {
          break;
        }
      }

      if (ds.dropTarget !== prevDropTarget) {
        if (ds.dropTarget) {
          const s = amts.indexOf(mimeType) + mimeType.length;
          const se = amts.indexOf("}", s);
          const style = amts.substring(s, se);
          if (style.length !== 0) {
            ds.dropTarget.setAttribute("dos", ds.dropTarget.className);
            ds.dropTarget.className = ds.dropTarget.className + " " + style;
          }
        } else {
          ds.object.styleClass = "";
        }

        if (prevDropTarget !== null) {
          if (prevDropTarget.handleDragDrop) {
            prevDropTarget.handleDragDrop("end", ds.object, e, "", mimeType);
          }
          const dos = prevDropTarget.getAttribute("dos");
          if (dos !== null) {
            prevDropTarget.className = dos;
          }
        }
      }

      if (ds.dropTarget) {
        if (ds.dropTarget.handleDragDrop) {
          ds.dropTarget.handleDragDrop("drag", ds.object, e, "", mimeType);
        } else {
          ds.object.className = ds.objectPrevStyle.className + " Wt-valid-drop";
        }
      } else {
        ds.object.className = ds.objectPrevStyle.className + "";
      }
      return false;
    }
    return true;
  }

  function dragAbort() {
    WT.capture(null);

    const ds = dragState;

    if (ds.object) {
      document.body.removeChild(ds.object);
      ds.objectPrevStyle.parent.appendChild(ds.object);

      ds.object.style.zIndex = ds.objectPrevStyle.zIndex;
      ds.object.style.position = ds.objectPrevStyle.position;
      ds.object.style.display = ds.objectPrevStyle.display;
      ds.object.style.left = ds.objectPrevStyle.left;
      ds.object.style.top = ds.objectPrevStyle.top;
      ds.object.className = ds.objectPrevStyle.className;

      ds.object = null;
      if (touchTimer) {
        clearTimeout(touchTimer);
      }
    }

    if (document.removeEventListener) {
      document.removeEventListener("mousedown", dragAbort);
      window.removeEventListener("mouseup", dragAbort);
      document.removeEventListener("touchstart", dragAbort);
    } else {
      document.detachEvent("onmousedown", dragAbort);
      window.detachEvent("onmouseup", dragAbort);
    }
  }

  function dragEnd(e) {
    e = e || window.event;
    WT.capture(null);

    const ds = dragState;

    if (ds.object) {
      if (ds.dropTarget) {
        const dos = ds.dropTarget.getAttribute("dos");
        if (dos !== null) {
          ds.dropTarget.className = dos;
        }
        if (ds.dropTarget.handleDragDrop) {
          ds.dropTarget.handleDragDrop("drop", ds.object, e, ds.sourceId, ds.mimeType);
        } else {
          if (e.touches) {
            emit(ds.dropTarget, { name: "_drop2", eventObject: ds.dropTarget, event: e }, ds.sourceId, ds.mimeType);
          } else {
            emit(ds.dropTarget, { name: "_drop", eventObject: ds.dropTarget, event: e }, ds.sourceId, ds.mimeType);
          }
        }
      } else {
        // could not be dropped, animate it floating back ?
      }

      dragAbort();
    }
  }

  function encodeTouches(touches, widgetCoords) {
    let result = "";

    for (let i = 0, il = touches.length; i < il; ++i) {
      const t = touches[i];
      if (i !== 0) {
        result += ";";
      }
      result += [
        t.identifier,
        Math.round(t.clientX),
        Math.round(t.clientY),
        Math.round(t.pageX),
        Math.round(t.pageY),
        Math.round(t.screenX),
        Math.round(t.screenY),
        Math.round(t.pageX - widgetCoords.x),
        Math.round(t.pageY - widgetCoords.y),
      ].join(";");
    }

    return result;
  }

  let formObjects = [];

  function encodeEvent(event) {
    const e = event.event;
    const result = ["signal=" + event.signal];

    if (event.id) {
      result.push("id=" + event.id, "name=" + encodeURIComponent(event.name), "an=" + event.args.length);

      for (let j = 0; j < event.args.length; ++j) {
        result.push("a" + j + "=" + encodeURIComponent(event.args[j]));
      }
    }

    for (let x = 0; x < formObjects.length; ++x) {
      const el = WT.getElement(formObjects[x]);
      let v = null;

      if (el === null) {
        continue;
      }

      if (el.wtEncodeValue) {
        v = el.wtEncodeValue(el);
      } else if (el.type === "select-multiple") {
        for (let j = 0, jl = el.options.length; j < jl; j++) {
          if (el.options[j].selected) {
            result.push(
              formObjects[x] + "=" +
                encodeURIComponent(el.options[j].value)
            );
          }
        }
      } else if (el.type === "checkbox" || el.type === "radio") {
        if (el.indeterminate || el.style.opacity === "0.5") {
          v = "i";
        } else if (el.checked) {
          v = el.value;
        }
      } else if (el.type !== "file") {
        if (el.classList.contains("Wt-edit-emptyText")) {
          v = "";
        } else {
          /* For WTextEdit */
          if (el.ed) {
            el.ed.save();
          }
          v = "" + el.value;
        }

        if (WT.hasFocus(el)) {
          const range = WT.getUnicodeSelectionRange(el);
          result.push("selstart=" + range.start, "selend=" + range.end);
        }
      }

      if (v !== null) {
        let component;
        try {
          component = encodeURIComponent(v);
          result.push(formObjects[x] + "=" + component);
        } catch (e) {
          // encoding failed, omit this form field
          // This can happen on Windows when typing a character
          // with a high and low surrogate pair (like an emoji).
          // On Chrome and Firefox this is split out into two pairs
          // of keydown/keyup events instead of one.
          console.error("Form object " + formObjects[x] + " failed to encode, discarded", e);
        }
      }
    }

    try {
      if (document.activeElement) {
        result.push("focus=" + document.activeElement.id);
      }
    } catch (e) {
      // Empty catch
    }

    if (currentHash !== null) {
      result.push("_=" + encodeURIComponent(currentHash));
    }

    if (!e) {
      event.data = result;
      return event;
    }

    let t = WT.target(e);
    while (t && !t.id && t.parentNode) {
      t = t.parentNode;
    }
    if (t && t.id) {
      result.push("tid=" + t.id);
    }

    try {
      if (typeof e.type === "string") {
        result.push("type=" + e.type);
      }
    } catch (e) {
      // Empty catch
    }

    if (typeof e.clientX !== UNDEFINED) {
      result.push("clientX=" + Math.round(e.clientX), "clientY=" + Math.round(e.clientY));
    }

    const pageCoords = WT.pageCoordinates(e);
    const posX = pageCoords.x;
    const posY = pageCoords.y;

    if (posX || posY) {
      result.push(
        "documentX=" + Math.round(posX),
        "documentY=" + Math.round(posY),
        "dragdX=" + Math.round(posX - downX),
        "dragdY=" + Math.round(posY - downY)
      );

      const delta = WT.wheelDelta(e);
      result.push("wheel=" + Math.round(delta));
    }

    if (typeof e.screenX !== UNDEFINED) {
      result.push("screenX=" + Math.round(e.screenX), "screenY=" + Math.round(e.screenY));
    }

    let widgetCoords = { x: 0, y: 0 };

    if (event.object && event.object.nodeType !== 9) {
      widgetCoords = WT.widgetPageCoordinates(event.object);
      const objX = widgetCoords.x;
      const objY = widgetCoords.y;

      if (typeof event.object.scrollLeft !== UNDEFINED) {
        result.push(
          "scrollX=" + Math.round(event.object.scrollLeft),
          "scrollY=" + Math.round(event.object.scrollTop),
          "width=" + Math.round(event.object.clientWidth),
          "height=" + Math.round(event.object.clientHeight)
        );
      }

      result.push("widgetX=" + Math.round(posX - objX), "widgetY=" + Math.round(posY - objY));
    }

    let button = WT.button(e);
    if (!button) {
      if (WT.buttons & 1) {
        button = 1;
      } else if (WT.buttons & 2) {
        button = 2;
      } else if (WT.buttons & 4) {
        button = 4;
      }
    }
    result.push("button=" + button);

    if (typeof e.keyCode !== UNDEFINED) {
      result.push("keyCode=" + e.keyCode);
    }

    if (typeof e.type === "string") {
      let charCode = 0;
      if (typeof e.charCode !== UNDEFINED) {
        if (e.type === "keypress") {
          charCode = e.charCode;
        }
      } else {
        if (e.type === "keypress") {
          charCode = e.keyCode;
        }
      }
      result.push("charCode=" + charCode);
    }

    if (typeof e.altKey !== UNDEFINED && e.altKey) {
      result.push("altKey=1");
    }
    if (typeof e.ctrlKey !== UNDEFINED && e.ctrlKey) {
      result.push("ctrlKey=1");
    }
    if (typeof e.metaKey !== UNDEFINED && e.metaKey) {
      result.push("metaKey=1");
    }
    if (typeof e.shiftKey !== UNDEFINED && e.shiftKey) {
      result.push("shiftKey=1");
    }

    if (typeof e.touches !== UNDEFINED) {
      result.push("touches=" + encodeTouches(e.touches, widgetCoords));
    }
    if (typeof e.targetTouches !== UNDEFINED) {
      result.push("ttouches=" + encodeTouches(e.targetTouches, widgetCoords));
    }
    if (typeof e.changedTouches !== UNDEFINED) {
      result.push("ctouches=" + encodeTouches(e.changedTouches, widgetCoords));
    }

    if (typeof e.scale !== UNDEFINED && e.scale) {
      result.push("scale=" + e.scale);
    }
    if (typeof e.rotation !== UNDEFINED && e.rotation) {
      result.push("rotation=" + e.rotation);
    }

    event.data = result;
    return event;
  }

  let sentEvents = [], pendingEvents = [];

  function encodePendingEvents(maxLength) {
    let se, result = "", feedback = false;

    let i = 0;
    for (; i < pendingEvents.length; ++i) {
      se = i > 0 ? "&e" + i : "&";
      let eventData = se + pendingEvents[i].data.join(se);
      if (pendingEvents[i].evAckId < ackUpdateId) {
        eventData += se + "evAckId=" + pendingEvents[i].evAckId;
      }

      if ((result.length + eventData.length) < maxLength) {
        feedback = feedback || pendingEvents[i].feedback;
        result += eventData;
      } else {
        console.warn("splitting up pending events: max-formdata-size reached (" + _$_MAX_FORMDATA_SIZE_$_ + " bytes)");
        break;
      }
    }

    if (i === 0) {
      const errMsg = "single event exceeds max-formdata-size, cannot proceed";
      sendError(errMsg, "Wt internal error; description: " + errMsg);
      throw new Error(errMsg);
    }

    // With HTTP: sentEvents should be empty before this concat
    // With WebSockets: sentEvents possibly not empty
    sentEvents = sentEvents.concat(pendingEvents.slice(0, i));
    pendingEvents = pendingEvents.slice(i);

    return { feedback: feedback, result: result };
  }

  let comm = null,
    sessionUrl,
    hasQuit = false,
    quitStr = _$_QUITTED_STR_$_,
    loaded = false,
    responsePending = null,
    pollTimer = null,
    keepAliveTimer = null;
  const idleTimeout = _$_IDLE_TIMEOUT_$_; /* idle timeout in seconds, null if disabled */
  let idleTimeoutTimer = null,
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
    if (idleTimeoutTimer) {
      clearTimeout(idleTimeoutTimer);
      idleTimeoutTimer = null;
    }
    if (pollTimer) {
      clearTimeout(pollTimer);
      pollTimer = null;
    }
    comm.cancel();
    const tr = WT.$("Wt-timers");
    if (tr) {
      WT.setHtml(tr, "", false);
    }
  }

  function doKeepAlive() {
    WT.history._initTimeout();
    if (commErrors === 0) {
      update(null, "keepAlive", null, false);
    }
  }

  function setTitle(title) {
    document.title = title;
  }

  function doIdleTimeout() {
    self.emit(self, "Wt-idleTimeout");
    idleTimeoutTimer = setTimeout(doIdleTimeout, idleTimeout * 1000);
  }

  function delayIdleTimeout() {
    if (idleTimeoutTimer !== null) {
      clearTimeout(idleTimeoutTimer);
      idleTimeoutTimer = setTimeout(doIdleTimeout, idleTimeout * 1000);
    }
  }

  function initIdleTimeout() {
    if (idleTimeout === null) {
      return;
    }

    idleTimeoutTimer = setTimeout(doIdleTimeout, idleTimeout * 1000);

    if (document.addEventListener) {
      document.addEventListener("mousedown", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("mouseup", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("wheel", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("keydown", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("keyup", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("touchstart", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("touchend", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("pointerdown", delayIdleTimeout, { capture: true, passive: true });
      document.addEventListener("pointerup", delayIdleTimeout, { capture: true, passive: true });
    }
  }

  function load(fullapp) {
    if (loaded) {
      return;
    }

    if (fullapp) {
      if (!window._$_APP_CLASS_$_LoadWidgetTree) {
        return; // That's too soon baby.
      }

      WT.history.initialize("Wt-history-field", "Wt-history-iframe", deployUrl);
    }

    if (!("activeElement" in document)) {
      function trackActiveElement(evt) {
        if (evt && evt.target) {
          document.activeElement = evt.target === document ? null : evt.target;
        }
      }

      function trackActiveElementLost(_evt) {
        document.activeElement = null;
      }

      document.addEventListener("focus", trackActiveElement, true);
      document.addEventListener("blur", trackActiveElementLost, true);
    }

    // this could be cancelled leading to havoc?
    document.addEventListener("mousedown", WT.mouseDown);
    document.addEventListener("mouseup", WT.mouseUp);

    WT.history._initialize();
    initIdleTimeout();
    loaded = true;

    if (fullapp) {
      window._$_APP_CLASS_$_LoadWidgetTree();
    }

    if (!hasQuit) {
      if (!keepAliveTimer) {
        keepAliveTimer = setInterval(doKeepAlive, _$_KEEP_ALIVE_$_ * 1000);
      }
    }
  }

  let currentHideLoadingIndicator = null;

  function cancelFeedback(timer) {
    clearTimeout(timer);
    document.body.style.cursor = "auto";

    if (currentHideLoadingIndicator !== null) {
      try {
        currentHideLoadingIndicator();
      } catch (e) {
        // Empty catch
      }
      currentHideLoadingIndicator = null;
    }
  }

  function waitFeedback() {
    document.body.style.cursor = "wait";
    currentHideLoadingIndicator = hideLoadingIndicator;
    showLoadingIndicator();
  }

  let nextWsRqId = 0;
  let pendingWsRequests = {};

  function wsWaitFeedback() {
    const now = Date.now();
    let maxRqId = -1;
    for (const wsReq of Object.keys(pendingWsRequests)) {
      if (now - pendingWsRequests[wsReq].time >= _$_INDICATOR_TIMEOUT_$_) {
        if (currentHideLoadingIndicator === null) {
          waitFeedback();
        }
        return;
      }
      const wsReqI = parseInt(wsReq, 10);
      if (wsReqI > maxRqId) {
        maxRqId = wsReqI;
      }
    }
    nextWsRqId = maxRqId + 1;
    // We're not waiting on any WebSocket requests for longer than
    // INDICATOR_TIMEOUT, so hide the loading indicator and reset
    // the cursor.
    document.body.style.cursor = "auto";
    if (currentHideLoadingIndicator !== null) {
      try {
        currentHideLoadingIndicator();
      } catch (e) {
        // Empty catch block
      }
      currentHideLoadingIndicator = null;
    }
  }

  const WebSocketUnknown = 0;
  const WebSocketConnecting = 1;
  const WebSocketAckConnect = 2;
  const WebSocketWorking = 3;
  const WebSocketUnavailable = 4;

  const websocket = {
    state: WebSocketUnknown,
    socket: null,
    keepAlive: null,
    reconnectTries: 0,
  };

  let connectionMonitor = null;

  function setServerPush(how) {
    serverPush = how;
  }

  function doAutoJavaScript() {
    self._p_.autoJavaScript();
  }

  function doJavaScript(js) {
    if (js) {
      js = "(function() {" + js + "})();";
      if (window.execScript) {
        window.execScript(js);
      } else {
        window.eval(js);
      }
    }

    if (self === window._$_APP_CLASS_$_) {
      doAutoJavaScript();
    }
  }

  function webSocketAckConnect() {
    nextWsRqId = 0;
    pendingWsRequests = {};
    websocket.socket.send("&signal=none&connected=" + ackUpdateId);
    websocket.state = WebSocketWorking;
  }

  function handleResponse(status, msg, timer) {
    if (connectionMonitor) {
      connectionMonitor.onStatusChange("connectionStatus", status === WT.ResponseStatus.OK ? 1 : 0);
    }

    if (hasQuit) {
      return;
    }

    if (waitingForJavaScript) {
      setTimeout(function() {
        handleResponse(status, msg, timer);
      }, 50);
      return;
    }

    if (pollTimer) {
      clearTimeout(pollTimer);
      pollTimer = null;
    }

    if (status === WT.ResponseStatus.OK) {
      WT.resolveRelativeAnchors();
      _$_$if_CATCH_ERROR_$_();
      try {
        _$_$endif_$_();
        doJavaScript(msg);
        _$_$if_CATCH_ERROR_$_();
      } catch (e) {
        const stack = e.stack || e.stacktrace;
        const description = e.description || e.message;
        const err = { "exception_code": e.code, "exception_description": description, "exception_js": msg };
        err.stack = stack;
        sendError(
          err,
          "Wt internal error; code: " + e.code +
            ", description: " + description
        );
        throw e;
      }
      _$_$endif_$_();

      if (timer) {
        cancelFeedback(timer);
      }
    } else {
      pendingEvents = sentEvents.concat(pendingEvents);
    }

    sentEvents = [];

    responsePending = null;

    if (status === WT.ResponseStatus.OK) {
      commErrors = 0;
    } else if (status === WT.ResponseStatus.Error) {
      ++commErrors;
    }

    if (hasQuit) {
      return;
    }

    if (websocket.state === WebSocketAckConnect) {
      webSocketAckConnect();
    }

    if ((serverPush && !waitingForJavaScript) || pendingEvents.length > 0) {
      if (status === WT.ResponseStatus.Error) {
        const ms = Math.min(120000, Math.exp(commErrors) * 500);
        updateTimeout = setTimeout(function() {
          sendUpdate();
        }, ms);
      } else if (updateTimeout === null) {
        sendUpdate();
      }
    }
  }

  function setSessionUrl(url) {
    if (url.indexOf("://") !== -1 || url[0] === "/") {
      sessionUrl = url;
    } else {
      sessionUrl = deployUrl + url;
    }

    if (comm) {
      comm.setUrl(url);
    }
  }

  setSessionUrl(_$_SESSION_URL_$_);

  comm = WT.initAjaxComm(sessionUrl, handleResponse);

  function doPollTimeout() {
    responsePending.abort();
    responsePending = null;
    pollTimer = null;

    if (!hasQuit) {
      sendUpdate();
    }
  }

  function setConnectionMonitor(aMonitor) {
    connectionMonitor = aMonitor;
    connectionMonitor.status = {};
    connectionMonitor.status.connectionStatus = 0;
    connectionMonitor.status.websocket = false;
    connectionMonitor.onStatusChange = function(type, newS) {
      const old = connectionMonitor.status[type];
      if (old === newS) {
        return;
      }
      connectionMonitor.status[type] = newS;
      connectionMonitor.onChange(type, old, newS);
    };
  }

  let updating = false;

  function update(el, signalName, e, feedback) {
    checkEventOverflow();

    /*
     * Konqueror may recurisvely call update() because
     * /reading/ offsetLeft or offsetTop triggers an onscroll event ??
     */
    if (updating) {
      return;
    }

    updating = true;

    WT.checkReleaseCapture(el, e);

    _$_$if_STRICTLY_SERIALIZED_EVENTS_$_();
    if (!responsePending) {
      _$_$endif_$_();

      const pendingEvent = {}, i = pendingEvents.length;
      pendingEvent.object = el;
      pendingEvent.signal = signalName;
      pendingEvent.event = window.fakeEvent || e;
      pendingEvent.feedback = feedback;
      pendingEvent.evAckId = ackUpdateId;

      pendingEvents[i] = encodeEvent(pendingEvent);

      scheduleUpdate();

      doJavaScript();

      _$_$if_STRICTLY_SERIALIZED_EVENTS_$_();
    }
    _$_$endif_$_();

    updating = false;
  }

  let updateTimeoutStart;

  function schedulePing() {
    if (websocket.keepAlive) {
      clearInterval(websocket.keepAlive);
    }

    websocket.keepAlive = setInterval(function() {
      const ws = websocket.socket;
      if (ws.readyState === 1) {
        ws.send("&signal=ping");
      } else {
        clearInterval(websocket.keepAlive);
        websocket.keepAlive = null;
      }
    }, _$_SERVER_PUSH_TIMEOUT_$_);
  }

  function scheduleUpdate() {
    if (hasQuit) {
      if (!quitStr) {
        return;
      }
      if (confirm(quitStr)) {
        /* eslint-disable-next-line no-self-assign */
        document.location = document.location; //  TODO(Roel): what???
        quitStr = null;
        return;
      } else {
        quitStr = null;
        return;
      }
    }

    _$_$if_WEB_SOCKETS_$_();
    if (websocket.state !== WebSocketUnavailable) {
      if (typeof window.WebSocket === UNDEFINED) {
        websocket.state = WebSocketUnavailable;
      } else {
        let ws = websocket.socket;

        if ((ws === null || ws.readyState > 1)) {
          if (ws !== null && websocket.state === WebSocketUnknown) {
            websocket.state = WebSocketUnavailable;
          } else {
            function reconnect() {
              if (!hasQuit) {
                ++websocket.reconnectTries;
                const ms = Math.min(
                  120000,
                  Math.exp(websocket.reconnectTries) *
                    500
                );
                setTimeout(function() {
                  scheduleUpdate();
                }, ms);
              }
            }

            const protocolEnd = sessionUrl.indexOf("://");
            let wsurl;
            if (protocolEnd !== -1) {
              wsurl = "ws" + sessionUrl.substring(4);
            } else {
              const query = sessionUrl.substring(sessionUrl.indexOf("?"));
              wsurl = "ws" + location.protocol.substring(4) +
                "//" + location.host + _$_WS_PATH_$_ + query;
            }

            wsurl += "&request=ws";

            const wsid = _$_WS_ID_$_;
            if (wsid.length > 0) {
              wsurl += "&wsid=" + wsid;
            }

            websocket.socket = ws = new WebSocket(wsurl);

            websocket.state = WebSocketConnecting;

            if (websocket.keepAlive) {
              clearInterval(websocket.keepAlive);
            }
            websocket.keepAlive = null;

            ws.onmessage = function(event) {
              let js = null;

              if (websocket.state === WebSocketConnecting) {
                if (event.data === "connect") {
                  if (responsePending !== null && pollTimer !== null) {
                    clearTimeout(pollTimer);
                    pollTimer = null;
                    responsePending.abort();
                    responsePending = null;
                  }

                  if (
                    responsePending ||
                    !WT.isEmptyObject(pendingWsRequests)
                  ) {
                    websocket.state = WebSocketAckConnect;
                  } else {
                    webSocketAckConnect();
                  }
                } else {
                  console.log("WebSocket: was expecting a connect?");
                  console.log(event.data);
                  return;
                }
              } else {
                if (connectionMonitor) {
                  connectionMonitor.onStatusChange("websocket", true);
                  connectionMonitor.onStatusChange("connectionStatus", 1);
                }
                websocket.state = WebSocketWorking;
                js = event.data;
              }

              websocket.reconnectTries = 0;
              if (js !== null) {
                handleResponse(WT.ResponseStatus.OK, js, null);
              }
            };

            ws.onerror = function(_event) {
              /*
               * Sometimes, we can connect but cannot send data
               */
              if (connectionMonitor) {
                connectionMonitor.onStatusChange("websocket", false);
              }
              if (
                websocket.reconnectTries === 3 &&
                websocket.state === WebSocketUnknown
              ) {
                websocket.state = WebSocketUnavailable;
              }
              reconnect();
            };

            ws.onclose = function(_event) {
              /*
               * Sometimes, we can connect but cannot send data
               */
              if (connectionMonitor) {
                connectionMonitor.onStatusChange("websocket", false);
              }
              if (
                websocket.reconnectTries === 3 &&
                websocket.state === WebSocketUnknown
              ) {
                websocket.state = WebSocketUnavailable;
              }
              reconnect();
            };

            ws.onopen = function(_event) {
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
                connectionMonitor.onStatusChange("websocket", true);
                connectionMonitor.onStatusChange("connectionStatus", 1);
              }

              /*
              ws.send('&signal=ping'); // to get our first onmessage
              */
              schedulePing();
            };
          }
        }

        if ((ws.readyState === 1) && (ws.state === WebSocketWorking)) {
          schedulePing();
          sendUpdate();
          return;
        }
      }
    }
    _$_$endif_$_();

    if (responsePending !== null && pollTimer !== null) {
      clearTimeout(pollTimer);
      pollTimer = null;
      responsePending.abort();
      responsePending = null;
    }

    if (responsePending === null) {
      if (updateTimeout === null) {
        updateTimeout = setTimeout(function() {
          sendUpdate();
        }, WT.updateDelay);
        updateTimeoutStart = (new Date()).getTime();
      } else if (commErrors) {
        clearTimeout(updateTimeout);
        updateTimeout = null;
        sendUpdate();
      } else {
        const diff = (new Date()).getTime() - updateTimeoutStart;
        if (diff > WT.updateDelay) {
          clearTimeout(updateTimeout);
          updateTimeout = null;
          sendUpdate();
        }
      }
    }
  }

  let ackUpdateId = _$_ACK_UPDATE_ID_$_, ackPuzzle = null;
  function responseReceived(updateId, puzzle) {
    ackPuzzle = puzzle;
    ackUpdateId = updateId;
    comm.responseReceived(updateId);
  }

  function wsRqsDone() {
    for (let i = 0; i < arguments.length; ++i) {
      const wsRqId = arguments[i];
      if (wsRqId in pendingWsRequests) {
        clearTimeout(pendingWsRequests[wsRqId].tm);
        delete pendingWsRequests[wsRqId];
      }
    }
    wsWaitFeedback();
  }

  let pageId = 0;
  function setPage(id) {
    pageId = id;
  }

  function sendError(err, errMsg) {
    responsePending = comm.sendUpdate(
      "request=jserror&err=" + encodeURIComponent(JSON.stringify(err)),
      false,
      ackUpdateId,
      -1
    );
    _$_$if_SHOW_ERROR_$_();
    alert(errMsg);
    _$_$endif_$_();
  }

  function sendUpdate() {
    if (self !== window._$_APP_CLASS_$_) {
      quit(null);
      return;
    }

    if (responsePending) {
      return;
    }

    updateTimeout = null;

    if (hasQuit) {
      return;
    }

    let data = "", tm, poll;

    const useWebSockets = websocket.socket !== null &&
      websocket.socket.readyState === 1 &&
      websocket.state === WebSocketWorking;

    if (!useWebSockets) {
      data += "&ackId=" + ackUpdateId;
    }

    data += "&pageId=" + pageId;

    if (ackPuzzle) {
      let solution = "";
      let d = WT.$(ackPuzzle);
      if (d) {
        d = d.parentNode;

        for (; !WT.hasTag(d, "BODY"); d = d.parentNode) {
          if (d.id) {
            if (solution !== "") {
              solution += ",";
            }
            solution += d.id;
          }
        }
      }

      data += "&ackPuzzle=" + encodeURIComponent(solution);
    }

    function getParams() {
      // Prevent minifier from optimizing away the length check.
      return "_$_PARAMS_$_";
    }
    const params = getParams();
    if (params.length > 0) {
      data += "&Wt-params=" + encodeURIComponent(params);
    }

    if (pendingEvents.length > 0) {
      let maxEventsSize = _$_MAX_FORMDATA_SIZE_$_ - data.length;
      if (useWebSockets) {
        maxEventsSize -= ("&wsRqId=" + nextWsRqId).length;
      }

      const eventsData = encodePendingEvents(maxEventsSize);
      tm = eventsData.feedback ?
        setTimeout(useWebSockets ? wsWaitFeedback : waitFeedback, _$_INDICATOR_TIMEOUT_$_) :
        null;
      data += eventsData.result;
      poll = false;
    } else {
      data += "&signal=poll";
      tm = null;
      poll = true;
    }

    if (useWebSockets) {
      responsePending = null;

      if (!poll) {
        if (tm) {
          const wsRqId = nextWsRqId;
          pendingWsRequests[wsRqId] = { time: Date.now(), tm: tm };
          ++nextWsRqId;
          data += "&wsRqId=" + wsRqId;
        }

        websocket.socket.send(data);
      }
    } else {
      if (responsePending) {
        try {
          throw new Error("responsePending is true before comm.sendUpdate");
        } catch (e) {
          const stack = e.stack || e.stacktrace;
          const description = e.description || e.message;
          const err = { "exception_description": description };
          err.stack = stack;
          sendError(err, "Wt internal error; description: " + description);
          throw e;
        }
      }

      pollTimer = poll ? setTimeout(doPollTimeout, _$_SERVER_PUSH_TIMEOUT_$_) : null;

      responsePending = 1;
      responsePending = comm.sendUpdate("request=jsupdate" + data, tm, ackUpdateId, -1, poll);
    }
  }

  function propagateSize(element, width, height) {
    /*
     * Propagate the size, even if it's the elements unconstrained size.
     */
    if (width === -1) {
      width = element.offsetWidth;
    }
    if (height === -1) {
      height = element.offsetHeight;
    }

    if (
      (typeof element.wtWidth === UNDEFINED) ||
      (element.wtWidth !== width) ||
      (typeof element.wtHeight === UNDEFINED) ||
      (element.wtHeight !== height)
    ) {
      element.wtWidth = width;
      element.wtHeight = height;

      if (width >= 0 && height >= 0) {
        emit(element, "resized", Math.round(width), Math.round(height));
      }
    }
  }

  function emit(object, config) {
    checkEventOverflow();

    const userEvent = {}, ei = pendingEvents.length;
    userEvent.signal = "user";

    if (typeof object === "string") {
      userEvent.id = object;
    } else if (object === self) {
      userEvent.id = "app";
    } else {
      userEvent.id = object.id;
    }

    if (typeof config === "object") {
      userEvent.name = config.name;
      userEvent.object = config.eventObject;
      userEvent.event = config.event;
    } else {
      userEvent.name = config;
      userEvent.object = userEvent.event = null;
    }

    userEvent.args = [];
    for (let i = 2; i < arguments.length; ++i) {
      const a = arguments[i];
      let r;
      if (a === false) {
        r = 0;
      } else if (a === true) {
        r = 1;
      } else if (a && a.toDateString) {
        r = a.toDateString();
      } else {
        r = a;
      }
      userEvent.args[i - 2] = r;
    }
    userEvent.feedback = true;
    userEvent.evAckId = ackUpdateId;

    pendingEvents[ei] = encodeEvent(userEvent);

    scheduleUpdate();
  }

  function checkEventOverflow() {
    if (
      _$_MAX_PENDING_EVENTS_$_ > 0 &&
      pendingEvents.length >= _$_MAX_PENDING_EVENTS_$_
    ) {
      const errMsg = "too many pending events";
      sendError(errMsg, "Wt internal error; description: " + errMsg);

      pendingEvents = [];
      throw new Error(errMsg);
    }
  }

  function addTimerEvent(timerid, msec, repeat) {
    const tm = function() {
      const obj = WT.getElement(timerid);
      if (obj) {
        if (repeat !== -1) {
          obj.timer = setTimeout(obj.tm, repeat);
        } else {
          obj.timer = null;
          obj.tm = null;
        }
        if (obj.onclick) {
          obj.onclick();
        }
      }
    };

    const obj = WT.getElement(timerid);
    if (obj.timer) {
      clearTimeout(obj.timer);
    }
    obj.timer = setTimeout(tm, msec);
    obj.tm = tm;
  }

  const jsLibsLoaded = {};
  let waitingForJavaScript = false;

  function onJsLoad(path, f) {
    // setTimeout needed for Opera
    setTimeout(function() {
      if (jsLibsLoaded[path] === true) {
        waitingForJavaScript = false;
        f();
        if (!waitingForJavaScript && serverPush) {
          sendUpdate();
        }
      } else {
        jsLibsLoaded[path] = f;
      }
    }, 20);

    waitingForJavaScript = true;
  }

  function jsLoaded(path) {
    if (jsLibsLoaded[path] === true) {
      return;
    } else {
      if (typeof jsLibsLoaded[path] !== UNDEFINED) {
        waitingForJavaScript = false;
        jsLibsLoaded[path]();
        if (!waitingForJavaScript && serverPush) {
          sendUpdate();
        }
      }
      jsLibsLoaded[path] = true;
    }
  }

  function loadScript(uri, symbol, tries) {
    let loaded = false, error = false;

    function onerror() {
      if (!loaded && !error) {
        error = true;

        const t = typeof tries === UNDEFINED ? 2 : tries;
        if (t > 1) {
          loadScript(uri, symbol, t - 1);
        } else {
          const err = {
            "error-description": "Fatal error: failed loading " + uri,
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

    if (symbol !== "") {
      try {
        loaded = !eval("typeof " + symbol + " === 'undefined'");
      } catch (e) {
        loaded = false;
      }
    }

    if (!loaded) {
      const s = document.createElement("script");
      s.setAttribute("src", uri);
      s.onload = onload;
      s.onerror = onerror;
      s.onreadystatechange = function() {
        const rs = s.readyState;
        if (rs === "loaded") { // may still be 404 in IE<=8; too bad!
          onerror();
        } else if (rs === "complete") {
          onload();
        }
      };
      const h = document.getElementsByTagName("head")[0];
      h.appendChild(s);
    } else {
      jsLoaded(uri);
    }
  }

  function ImagePreloader(uris, callback) {
    this.callback = callback;
    this.work = uris.length;
    this.images = [];

    if (uris.length === 0) {
      this.callback(this.images);
    } else {
      for (const uri of uris) {
        this.preload(uri);
      }
    }
  }

  ImagePreloader.prototype.preload = function(uri) {
    const image = new Image();
    this.images.push(image);
    image.onload = ImagePreloader.prototype.onload;
    image.onerror = ImagePreloader.prototype.onload;
    image.onabort = ImagePreloader.prototype.onload;
    image.imagePreloader = this;

    image.src = uri;
  };

  ImagePreloader.prototype.onload = function() {
    // Called from the image: this = the image
    const preloader = this.imagePreloader;
    if (--preloader.work === 0) {
      preloader.callback(preloader.images);
    }
  };

  ImagePreloader.prototype.cancel = function() {
    const images = this.images;
    for (const image of images) {
      image.onload = function() {};
      image.onerror = function() {};
      image.onabort = function() {};
    }
    this.callback = function() {};
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
    if (uris.length === 0) {
      callback(this.arrayBuffers);
    } else {
      // if uris are given, load them asynchronously
      for (let i = 0; i < uris.length; i++) {
        this.preload(uris[i], i);
      }
    }
  }

  // preload function: downloads buffer at the given URI
  ArrayBufferPreloader.prototype.preload = function(uri, index) {
    const xhr = new XMLHttpRequest();
    // open the resource, send asynchronously (without waiting for answer)
    xhr.open("GET", uri, true);
    xhr.responseType = "arraybuffer";

    // give xhr write access to array
    xhr.arrayBuffers = this.arrayBuffers;
    xhr.preloader = this;
    xhr.index = index; // needed to maintain the mapping
    xhr.uri = uri;

    // behaviour when it was loaded-> redirect to ArrayBufferPreloader
    xhr.onload = function(_e) {
      console.log("XHR load buffer " + this.index + " from uri " + this.uri);

      // this.arrayBuffers[this.index] = new Uint8Array(this.response);
      this.arrayBuffers[this.index] = this.response;
      this.preloader.afterLoad();
    };

    xhr.onerror = ArrayBufferPreloader.prototype.afterload;
    xhr.onabort = ArrayBufferPreloader.prototype.afterload;

    // actually start the query
    xhr.send();
  };

  ArrayBufferPreloader.prototype.afterLoad = function() {
    if (--this.work === 0) {
      // last request finished -> call callback
      this.callback(this.arrayBuffers);
    }
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
  function ieAlternative(d) {
    if (d.ieAlternativeExecuted) {
      return "0";
    }
    self.emit(d.parentNode, "IeAlternative");
    d.style.width = "";
    d.ieAlternativeExecuted = true;
    return "0";
  }

  window.onunload = function() {
    if (!hasQuit) {
      self.emit(self, "Wt-unload");
      scheduleUpdate();
      sendUpdate();
    }
  };

  function setLocale(m) {
    if (m === "") {
      return;
    }
    document.documentElement.lang = m;
  }

  function setCloseMessage(m) {
    if (m && m !== "") {
      window.onbeforeunload = function(event) {
        const e = event || window.event;

        if (e) {
          e.returnValue = m;
        }

        return m;
      };
    } else {
      window.onbeforeunload = null;
    }
  }

  let firstCall = true;
  let globalEventsFunctions = null;
  const keyEvents = ["keydown", "keyup", "keypress"];

  function updateGlobal(id) {
    firstCall = false;
    let domId;
    if (id === null) {
      domId = document.querySelector(".Wt-domRoot").id;
    } else {
      domId = id;
    }

    for (let i = 0; i < keyEvents.length; ++i) {
      const elemEvents = globalEventsFunctions ? globalEventsFunctions[domId] : null;
      let eventFunc = null;

      if (elemEvents) {
        eventFunc = elemEvents[keyEvents[i]];
      }

      const bindEvent = function(evtfunc) {
        return function(event) {
          const g = event || window.event;
          const t = g.target || g.srcElement;
          if (
            (!t | WT.hasTag(t, "DIV") ||
              WT.hasTag(t, "BODY") ||
              WT.hasTag(t, "HTML"))
          ) {
            const func = evtfunc;
            if (func) {
              func(event);
            }
          }
        };
      };

      if (eventFunc) {
        document["on" + keyEvents[i]] = bindEvent(eventFunc);
      } else {
        document["on" + keyEvents[i]] = null;
      }
    }

    // cleanup functions of widgets that do no longer exist
    if (globalEventsFunctions) {
      for (const i of Object.keys(globalEventsFunctions)) {
        if (!document.getElementById(i)) {
          delete globalEventsFunctions[i];
        }
      }
    }
  }

  function bindGlobal(event, id, f) {
    let init = false;
    if (!globalEventsFunctions) {
      globalEventsFunctions = {};
      init = true;
    }

    // Saves the event functions
    if (!globalEventsFunctions[id]) {
      globalEventsFunctions[id] = {};
    }

    globalEventsFunctions[id][event] = f;
    if (init) {
      setTimeout(function() {
        if (firstCall) {
          updateGlobal(null);
        }
      }, 0);
    }
  }

  function refreshCookie() {
    comm.sendUpdate("request=jsupdate&signal=keepAlive&ackId=" + ackUpdateId, false, ackUpdateId, -1);
  }

  let googleMapsLoaded = false;
  let googleMapsLoadedCallbacks = [];

  function loadGoogleMaps(version, key, callback) {
    if (googleMapsLoaded) {
      callback();
    } else {
      googleMapsLoadedCallbacks.push(callback);
      if (googleMapsLoadedCallbacks.length === 1) {
        google.load("maps", version, {
          other_params: "key=" + key,
          callback: function() {
            googleMapsLoaded = true;
            for (const cb of googleMapsLoadedCallbacks) {
              cb();
            }
            googleMapsLoadedCallbacks = [];
          },
        });
      }
    }
  }

  this._p_ = {
    ieAlternative,
    loadScript,
    onJsLoad,
    setTitle,
    setLocale,
    update,
    quit,
    setSessionUrl,
    setFormObjects: function(o) {
      formObjects = o;
    },
    saveDownPos,
    addTimerEvent,
    load,
    setServerPush,

    touchStart,
    touchEnded,
    dragStart,
    dragDrag,
    dragEnd,
    capture: WT.capture,

    enableInternalPaths,
    onHashChange,
    setHash,
    ImagePreloader,
    ArrayBufferPreloader,

    doAutoJavaScript,
    autoJavaScript: function() {},

    response: responseReceived,
    wsRqsDone,
    setPage,
    setCloseMessage,
    setConnectionMonitor,
    updateGlobal,
    bindGlobal,
    refreshCookie,

    propagateSize,

    loadGoogleMaps,
  };

  this.WT = _$_WT_CLASS_$_;
  this.emit = emit;
})();

window._$_APP_CLASS_$_SignalEmit = _$_APP_CLASS_$_.emit;

window._$_APP_CLASS_$_OnLoad = function() {
  _$_APP_CLASS_$_._p_.load();
};
