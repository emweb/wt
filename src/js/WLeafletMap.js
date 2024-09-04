/* global L:readonly */

/*
 * Copyright (C) 2019 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WLeafletMap", function(APP, el, options_str, lat, lng, zoom) {
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

  this.addOverlayItem = function(overlayItem_id, overlayItem) {
    overlayItem.item_id = overlayItem_id;
    this.map.addLayer(overlayItem);
    mapItems[overlayItem_id] = overlayItem;
  };

  this.addMapItem = function(mapItem_id, mapItem) {
    mapItem.addTo(self.map);
    mapItems[mapItem_id] = mapItem;
  };

  this.removeMapItem = function(mapItem_id) {
    const mapItem = mapItems[mapItem_id];
    if (mapItem) {
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

  this.setOverlayItemContent = function(overlayItem_id, content) {
    const overlayItem = mapItems[overlayItem_id];
    if (overlayItem) {
      overlayItem.setContent(content);
    }
  };

  this.toggleOverlayItem = function(overlayItem_id, doOpen) {
    const overlayItem = mapItems[overlayItem_id];
    if (overlayItem) {
      if (doOpen && !overlayItem.isOpen()) {
        overlayItem.openOn(this.map);
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
      APP.emit(el, "overlayItemToggled", e.popup.item_id, false);
    });
    self.map.on("popupopen", function(e) {
      APP.emit(el, "overlayItemToggled", e.popup.item_id, true);
    });
  };

  this.init(options_str, [lat, lng], zoom);
});
