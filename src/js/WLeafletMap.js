/*
 * Copyright (C) 2019 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WLeafletMap",
  function(APP, el, options_str, lat, lng, zoom) {
    if (el.wtObj) {
      // uninit existing WLeafletMap
      el.wtObj.map.remove();
    }

    el.wtObj = this;

    var self = this;
    var WT = APP.WT;

    this.map = null;
    var markers = {};

    var lastZoom = zoom;
    var lastLatLng = [lat, lng];

    this.addTileLayer = function(url_template, options_str) {
      var options = JSON.parse(options_str);
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
      var options = JSON.parse(options_str);
      L.polyline(points, options).addTo(self.map);
    };

    this.addCircle = function(center, options_str) {
      var options = JSON.parse(options_str);
      L.circle(center, options).addTo(self.map);
    };

    this.addMarker = function(marker_id, marker) {
      marker.addTo(self.map);
      markers[marker_id] = marker;
    };

    this.removeMarker = function(marker_id) {
      var marker = markers[marker_id];
      if (marker) {
        self.map.removeLayer(marker);
        delete markers[marker_id];
      }
    };

    this.moveMarker = function(marker_id, position) {
      var marker = markers[marker_id];
      if (marker) {
        marker.setLatLng(position);
      }
    };

    this.wtResize = function() {
      self.map.invalidateSize();
    };

    el.wtEncodeValue = function() {
      var center = self.map.getCenter();
      var position = [center.lat, center.lng];
      var zoom = self.map.getZoom();
      return JSON.stringify({
        position: position,
        zoom: zoom
      });
    };

    this.init = function(options_str, position, zoom) {
      var options = JSON.parse(options_str);
      options.center = position;
      options.zoom = zoom;
      self.map = L.map(el, options);

      var baseZIndex = parseInt((function(){
        var p = el.parentNode;
        while (p) {
          if (p.wtPopup) {
            return p.style.zIndex;
          }
          p = p.parentNode;
        }
        return 0;
      })(), 10);

      if (baseZIndex > 0) {
        self.map.getPane('tilePane').style.zIndex = baseZIndex + 200;
        self.map.getPane('overlayPane').style.zIndex = baseZIndex + 400;
        self.map.getPane('shadowPane').style.zIndex = baseZIndex + 500;
        self.map.getPane('markerPane').style.zIndex = baseZIndex + 600;
        self.map.getPane('tooltipPane').style.zIndex = baseZIndex + 650;
        self.map.getPane('popupPane').style.zIndex = baseZIndex + 700;
      }

      self.map.on('zoomend', function() {
        var zoom = self.map.getZoom();
        if (zoom != lastZoom) {
          APP.emit(el, 'zoomLevelChanged', zoom);
          lastZoom = zoom;
        }
      });
      self.map.on('moveend', function() {
        var center = self.map.getCenter();
        if (center.lat != lastLatLng[0] ||
            center.lng != lastLatLng[1]) {
          APP.emit(el, 'panChanged', center.lat, center.lng);
          lastLatLng = [center.lat, center.lng];
        }
      });
    };

    this.init(options_str, [lat, lng], zoom);
  });
