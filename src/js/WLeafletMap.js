/* global L:readonly */

/*
 * Copyright (C) 2019 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WLeafletMap",
  function(APP, el, options_str, lat, lng, zoom, double_click_timeout) {
    const TOOLTIP = 0;
    const POPUP = 1;

    if (el.wtObj) {
      // uninit existing WLeafletMap
      el.wtObj.map.remove();
    }

    el.wtObj = this;

    const self = this;

    this.map = null;
    const mapItems = {};

    let lastZoom = zoom;
    let lastLatLng = [lat, lng];

    this.addTileLayer = function(url_template, options_str) {
      const options = JSON.parse(options_str);
      L.tileLayer(url_template, options).addTo(self.map);
    };

    this.zoom = function(zoom) {
      lastZoom = zoom;
      self.map.setZoom(zoom);
    };

    this.panTo = function(lat, lng) {
      lastLatLng = [lat, lng];
      self.map.panTo([lat, lng]);
    };

    this.addPolyline = function(points, options_str) {
      const options = JSON.parse(options_str);
      L.polyline(points, options).addTo(self.map);
    };

    this.addCircle = function(center, options_str) {
      const options = JSON.parse(options_str);
      L.circle(center, options).addTo(self.map);
    };

    function onMapItemClick(mapItem_id, mapItem) {
      if (mapItem.doubleClickTimeout) {
        clearTimeout(mapItem.doubleClickTimeout);
        mapItem.doubleClickTimeout = null;
        APP.emit(el, "itemDblClicked", mapItem_id);
      } else {
        mapItem.doubleClickTimeout = setTimeout(function() {
          mapItem.doubleClickTimeout = null;
          APP.emit(el, "itemClicked", mapItem_id);
        }, double_click_timeout);
      }
    }

    function addListeners(mapItem_id, mapItem) {
      mapItem.doubleClickTimeout = null;
      mapItem.on("click", function() {
        onMapItemClick(mapItem_id, mapItem); // handles both click and double-click
      });
      mapItem.on("mousedown", function() {
        APP.emit(el, "itemMousedown", mapItem_id);
      });
      mapItem.on("mouseup", function() {
        APP.emit(el, "itemMouseup", mapItem_id);
      });
      mapItem.on("mouseover", function() {
        APP.emit(el, "itemMouseover", mapItem_id);
      });
      mapItem.on("mouseout", function() {
        APP.emit(el, "itemMouseout", mapItem_id);
      });
    }

    this.addPopup = function(popup_id, parent_id, popup) {
      popup.item_type = POPUP;
      self.addOverlayItem(popup_id, parent_id, popup);
    };

    this.addTooltip = function(tooltip_id, parent_id, tooltip) {
      tooltip.item_type = TOOLTIP;
      self.addOverlayItem(tooltip_id, parent_id, tooltip);
    };

    this.addOverlayItem = function(overlayItem_id, parent_id, overlayItem) {
      addListeners(overlayItem_id, overlayItem);
      overlayItem.contentHolder = document.createElement("div");
      overlayItem.contentHolder.style.cssText = "visibility: hidden;";
      el.appendChild(overlayItem.contentHolder);
      overlayItem.item_id = overlayItem_id;
      overlayItem.parent_id = parent_id;
      mapItems[overlayItem_id] = overlayItem;
      self.toggleOverlayItem(overlayItem_id, true);
    };

    this.addMapItem = function(mapItem_id, parent_id, mapItem) {
      addListeners(mapItem_id, mapItem);
      mapItem.addTo(self.map);
      mapItems[mapItem_id] = mapItem;
    };

    this.removeMapItem = function(mapItem_id) {
      const mapItem = mapItems[mapItem_id];
      if (mapItem) {
        const parent_id = mapItem.parent_id;
        if ((parent_id || parent_id === 0) && parent_id !== -1) {
          const parent = mapItems[parent_id];
          if (parent) {
            if (parent.getPopup() && parent.getPopup().item_id === mapItem_id) {
              parent.unbindPopup();
            } else if (parent.getTooltip() && parent.getTooltip().item_id === mapItem_id) {
              parent.unbindTooltip();
            }
          }
        }

        if (mapItem.contentHolder) {
          mapItem.contentHolder.remove();
        }

        self.map.removeLayer(mapItem);
        delete mapItems[mapItem_id];
      }
    };

    this.moveMapItem = function(marker_id, position) {
      const marker = mapItems[marker_id];
      if (marker) {
        marker.setLatLng(position);
      }
    };

    this.setOverlayItemContent = function(overlayItem_id, content, content_id) {
      const overlayItem = mapItems[overlayItem_id];
      if (overlayItem) {
        overlayItem.contentId = content_id;
        overlayItem.contentHolder.innerHTML = content;

        if (overlayItem.isOpen()) {
          const c = overlayItem.contentHolder.firstChild;
          overlayItem.setContent(overlayItem.contentHolder.removeChild(c));
        }

        self.delayedJS();
      }
    };

    this.moveOverlayItemToFront = function(overlayItem_id) {
      const overlayItem = mapItems[overlayItem_id];
      if (overlayItem) {
        overlayItem.bringToFront();
      }
    };

    this.moveOverlayItemToBack = function(overlayItem_id) {
      const overlayItem = mapItems[overlayItem_id];
      if (overlayItem) {
        overlayItem.bringToBack();
      }
    };

    this.toggleOverlayItem = function(overlayItem_id, doOpen) {
      const overlayItem = mapItems[overlayItem_id];
      if (overlayItem) {
        if (doOpen && !overlayItem.isOpen()) {
          const parent_id = overlayItem.parent_id;
          if (parent_id === -1) {
            overlayItem.openOn(this.map);
          } else {
            const parent = mapItems[parent_id];
            if (parent) {
              if (overlayItem.item_type === POPUP) {
                parent.bindPopup(overlayItem).openPopup();
              } else if (overlayItem.item_type === TOOLTIP) {
                parent.bindTooltip(overlayItem).openTooltip();
              }
            }
          }
        } else if (!doOpen && overlayItem.isOpen()) {
          overlayItem.close();
        }
      }
    };

    this.wtResize = function() {
      self.map.invalidateSize();
    };

    el.wtEncodeValue = function() {
      const center = self.map.getCenter();
      const position = [center.lat, center.lng];
      const zoom = self.map.getZoom();
      return JSON.stringify({
        position: position,
        zoom: zoom,
      });
    };

    this.init = function(options_str, position, zoom) {
      const options = JSON.parse(options_str);
      options.center = position;
      options.zoom = zoom;
      self.map = L.map(el, options);

      const baseZIndex = parseInt(
        (function() {
          let p = el.parentNode;
          while (p) {
            if (p.wtPopup) {
              return p.style.zIndex;
            }
            p = p.parentNode;
          }
          return 0;
        })(),
        10
      );

      if (baseZIndex > 0) {
        self.map.getPane("tilePane").style.zIndex = baseZIndex + 200;
        self.map.getPane("overlayPane").style.zIndex = baseZIndex + 400;
        self.map.getPane("shadowPane").style.zIndex = baseZIndex + 500;
        self.map.getPane("markerPane").style.zIndex = baseZIndex + 600;
        self.map.getPane("tooltipPane").style.zIndex = baseZIndex + 650;
        self.map.getPane("popupPane").style.zIndex = baseZIndex + 700;
      }

      self.map.on("zoomend", function() {
        const zoom = self.map.getZoom();
        if (zoom !== lastZoom) {
          APP.emit(el, "zoomLevelChanged", zoom);
          lastZoom = zoom;
        }
      });
      self.map.on("moveend", function() {
        const center = self.map.getCenter();
        if (
          center.lat !== lastLatLng[0] ||
          center.lng !== lastLatLng[1]
        ) {
          APP.emit(el, "panChanged", center.lat, center.lng);
          lastLatLng = [center.lat, center.lng];
        }
      });
      self.map.on("popupclose", function(e) {
        const c = e.popup.getContent();
        c.remove();
        e.popup.contentHolder.appendChild(c);

        APP.emit(el, "overlayItemToggled", e.popup.item_id, false);
      });
      self.map.on("popupopen", function(e) {
        const c = e.popup.contentHolder.firstChild;
        if (c) {
          e.popup.setContent(e.popup.contentHolder.removeChild(c));
        }

        APP.emit(el, "overlayItemToggled", e.popup.item_id, true);
      });
      self.map.on("tooltipclose", function(e) {
        const c = e.tooltip.getContent();
        c.remove();
        e.tooltip.contentHolder.appendChild(c);

        APP.emit(el, "overlayItemToggled", e.tooltip.item_id, false);
      });
      self.map.on("tooltipopen", function(e) {
        const c = e.tooltip.contentHolder.firstChild;
        if (c) {
          e.tooltip.setContent(e.tooltip.contentHolder.removeChild(c));
        }

        APP.emit(el, "overlayItemToggled", e.tooltip.item_id, true);
      });
    };

    this.init(options_str, [lat, lng], zoom);
  }
);
