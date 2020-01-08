// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2019 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLEAFLETMAP_H_
#define WLEAFLETMAP_H_

#include <Wt/WBrush.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WPen.h>

#include <Wt/Json/Object.h>

namespace Wt {

/*! \class WLeafletMap Wt/WLeafletMap Wt/WLeafletMap
 *  \brief A widget that displays a leaflet map.
 *
 * This is a simple wrapper around the <a href="https://leafletjs.com/" target="_blank">Leaflet</a> JavaScript library.
 *
 * Leaflet itself does not provide maps. It is a JavaScript library that enables you to use any "tile server",
 * like OpenStreetMap. If you just create a %WLeafletMap (and give it a size), then you will be presented with
 * an empty map. You can then add tile layers to the map using addTileLayer().
 *
 * %WLeafletMap is not exhaustive in its support for Leaflet features. It supports a subset out of the box.
 * One of these features is markers, which come in two flavors: standard leaflet markers (WLeafletMap::LeafletMarker)
 * and widget markers (WLeafletMap::WidgetMarker). Using a widget marker, you can place arbitrary widgets on the map.
 *
 * If you need direct access to the leaflet map in your own custom JavaScript, you can use mapJsRef().
 *
 * \if cpp
 * Leaflet itself is not bundled with %Wt. Use the <tt>leafletJSURL</tt> and <tt>leafletCSSURL</tt> properties
 * to configure where the JavaScript and CSS of Leaflet should be loaded from.
 * You can add these properties to <tt>wt_config.xml</tt>:
 * \code{.xml}
 * <property name="leafletJSURL">https://unpkg.com/leaflet@1.5.1/dist/leaflet.js</property>
 * <property name="leafletCSSURL">https://unpkg.com/leaflet@1.5.1/dist/leaflet.css</property>
 * \endcode
 * \endif
 *
 * \if java
 * Leaflet itself is not bundled with %JWt. Use the <tt>leafletJSURL</tt> and <tt>leafletCSSURL</tt> properties
 * to configure where the JavaScript and CSS of Leaflet should be loaded from.
 * \endif
 */
class WT_API WLeafletMap : public WCompositeWidget
{
  class Impl;
public:
  /*! \class Coordinate
   *  \brief A geographical coordinate (latitude/longitude)
   */
  class WT_API Coordinate {
  public:
    /*! \brief Default constructor
     *
     * Constructs a coordinate with latitude and longitude set to 0.
     */
    Coordinate();

    /*! \brief Create a coordinate (latitude, longitude)
     */
    Coordinate(double latitude, double longitude);

    /*! \brief Set the latitude
     */
    void setLatitude(double latitude);

    /*! \brief Get the latitude
     */
    double latitude() const { return lat_; }

    /*! \brief Set the longitude
     */
    void setLongitude(double longitude);

    /*! \brief Get the longitude
     */
    double longitude() const { return lng_; }

    /*! \brief Equality comparison operator
     */
    bool operator==(const Coordinate &other) const { return lat_ == other.lat_ && lng_ == other.lng_; }

    /*! \brief Inequality comparison operator
     */
    bool operator!=(const Coordinate &other) const { return lat_ != other.lat_ || lng_ != other.lng_; }

  private:
    double lat_, lng_;
  };

  /*! \class Marker
   *  \brief An abstract marker
   *
   * This marker can be placed on a WLeafletMap at certain coordinates.
   */
  class WT_API Marker {
  public:
    virtual ~Marker();

    Marker(const Marker &) = delete;
    Marker& operator=(const Marker &) = delete;
    Marker(Marker &&) = delete;
    Marker& operator=(Marker &&) = delete;

    /*! \brief Move the marker
     *
     * If this marker belongs to a map, this will trigger an update of the WLeafletMap
     * to move the marker. If it doesn't belong to a map, the position is merely updated.
     */
    void move(const Coordinate &pos);

    /*! \brief Get the current position
     *
     * \sa move
     */
    Coordinate position() const { return pos_; }

  protected:
    explicit Marker(const Coordinate &pos);

    WLeafletMap *map() { return map_; }
    const WLeafletMap *map() const { return map_; }

    virtual void setMap(WLeafletMap *map);
    virtual void createMarkerJS(WStringStream &ss, WStringStream &postJS) const = 0;
    virtual void unrender();
    virtual bool needsUpdate() const;
    virtual void update(WStringStream &js);

  private:
    Coordinate pos_;
    WLeafletMap *map_;
    bool moved_;

    friend class WLeafletMap;

  };

  /*! \class WidgetMarker
   *  \brief A marker rendered with a widget
   *
   * This can be used to place arbitrary widgets on the map.
   *
   * The widgets will stay the same size regardless of the zoom level of the map.
   */
  class WT_API WidgetMarker : public Marker {
  public:
    /*! \brief Create a new WidgetMarker at the given position with the given widget.
     */
    explicit WidgetMarker(const Coordinate &pos, std::unique_ptr<WWidget> widget);

    virtual ~WidgetMarker();

    WidgetMarker(const WidgetMarker &) = delete;
    WidgetMarker& operator=(const WidgetMarker &) = delete;
    WidgetMarker(WidgetMarker &&) = delete;
    WidgetMarker& operator=(WidgetMarker &&) = delete;

    /*! \brief Get the widget
     */
    WWidget *widget();

    /*! \brief Get the widget (const)
     */
    const WWidget *widget() const;

    /*! \brief Set the anchor point of the marker.
     *
     * This determines the "tip" of the marker (relative to its
     * top left corner). The marker will be aligned so that this
     * point is at the marker's geographical location.
     *
     * If x is negative, the anchor point is in the
     * horizontal center of the widget. If y is negative,
     * the anchor point is in the vertical center of the widget.
     *
     * By default the anchor point is in the middle (horizontal and vertical center).
     */
    void setAnchorPoint(double x, double y);

  protected:
    virtual void setMap(WLeafletMap *map) override;
    virtual void createMarkerJS(WStringStream &ss, WStringStream &postJS) const override;
    virtual void unrender() override;
    virtual bool needsUpdate() const override;
    virtual void update(WStringStream &js) override;

  private:
    std::unique_ptr<WContainerWidget> container_;
    double anchorX_, anchorY_;
    bool anchorPointChanged_;

    void createContainer();
    void updateAnchorJS(WStringStream &js) const;
  };

  /*! \class LeafletMarker
   *  \brief A standard leaflet marker
   *
   * See https://leafletjs.com/reference.html#marker
   */
  class WT_API LeafletMarker : public Marker {
  public:
    /*! \brief Create a new marker at the given position
     */
    explicit LeafletMarker(const Coordinate &pos);

    virtual ~LeafletMarker();

    LeafletMarker(const LeafletMarker &) = delete;
    LeafletMarker& operator=(const LeafletMarker &) = delete;
    LeafletMarker(LeafletMarker &&) = delete;
    LeafletMarker& operator=(LeafletMarker &&) = delete;

  protected:
    virtual void createMarkerJS(WStringStream &ss, WStringStream &postJS) const override;
  };

  /*! \brief Create a new WLeafletMap
   */
  WLeafletMap();

  /*! \brief Create a new WLeafletMap with the given options
   *
   * \sa setOptions()
   */
  explicit WLeafletMap(const Json::Object &options);

  virtual ~WLeafletMap();

  WLeafletMap(const WLeafletMap &) = delete;
  WLeafletMap& operator=(const WLeafletMap &) = delete;
  WLeafletMap(WLeafletMap &&) = delete;
  WLeafletMap& operator=(WLeafletMap &&) = delete;

  /*! \brief Change the options of the %WLeafletMap
   *
   * \note This fully rerenders the map, because it creates a new Leaflet map,
   * so any custom JavaScript modifying the map with e.g. doJavaScript() is undone,
   * much like reloading the page when <tt>reload-is-new-session</tt> is set to false.
   *
   * See https://leafletjs.com/reference.html#map for a list of options.
   */
  void setOptions(const Json::Object &options);

  /*! \brief Add a new tile layer
   *
   * See https://leafletjs.com/reference.html#tilelayer
   *
   * \if cpp
   * Example for OpenStreetMap:
   * \code
   * Wt::Json::Object options;
   * options["maxZoom"] = 19;
   * options["attribution"] = "&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors";
   * marker->addTileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", options);
   * \endcode
   * \endif
   */
  void addTileLayer(const std::string &urlTemplate,
                    const Json::Object &options);

  /*! \brief Add the given marker
   */
  void addMarker(std::unique_ptr<Marker> marker);

#ifndef WT_TARGET_JAVA
  template<typename M>
  M *addMarker(std::unique_ptr<M> marker)
  {
    M *result = marker.get();
    addMarker(std::unique_ptr<Marker>(std::move(marker)));
    return result;
  }
#endif // WT_TARGET_JAVA

  /*! \brief Remove the given marker
   */
  std::unique_ptr<Marker> removeMarker(Marker *marker);

#ifndef WT_TARGET_JAVA
  template<typename M>
  std::unique_ptr<M> removeMarker(M *marker)
  {
    auto result = removeMarker(static_cast<Marker*>(marker));
    return std::unique_ptr<M>(static_cast<M*>(result.release()));
  }
#endif // WT_TARGET_JAVA

  /*! \brief Add a polyline
   *
   * This will draw a polyline on the map going through
   * the given list of coordinates, with the given pen.
   *
   * See https://leafletjs.com/reference.html#polyline
   */
  void addPolyline(const std::vector<Coordinate> &points,
                   const WPen &pen);

  /*! \brief Add a circle
   *
   * This will draw a circle on the map centered at \p center,
   * with the given \p radius (in meters), drawn with the given
   * \p stroke and \p fill.
   */
  void addCircle(const Coordinate &center,
                 double radius,
                 const WPen &stroke,
                 const WBrush &fill);

  /*! \brief Set the current zoom level.
   */
  void setZoomLevel(int level);

  /*! \brief Get the current zoom level.
   */
  int zoomLevel() const { return zoomLevel_; }

  /*! \brief Pan to the given coordinate.
   */
  void panTo(const Coordinate &center);

  /*! \brief Get the current position.
   */
  Coordinate position() const { return position_; }

  /*! \brief Signal emitted when the user has changed the zoom level of the map
   */
  JSignal<int> &zoomLevelChanged() { return zoomLevelChanged_; }

  /*! \brief Signal emitted when the user has panned the map
   */
  JSignal<double, double> &panChanged() { return panChanged_; }

  /*! \brief Returns a JavaScript expression to the Leaflet map object.
   *
   * You may want to use this in conjunction with JSlot or
   * doJavaScript() in custom JavaScript code, e.g. to access
   * features not built-in to %WLeafletMap.
   */
  std::string mapJsRef() const;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  static const int BIT_ZOOM_CHANGED = 0;
  static const int BIT_PAN_CHANGED = 1;
  static const int BIT_OPTIONS_CHANGED = 2;

  static const std::string WIDGETMARKER_CONTAINER_RULENAME;
  static const std::string WIDGETMARKER_CONTAINER_CHILDREN_RULENAME;

  Impl *impl_;
  Json::Object options_;
  std::bitset<3> flags_;
  JSignal<int> zoomLevelChanged_;
  JSignal<double, double> panChanged_;
  Coordinate position_;
  int zoomLevel_;
  long long nextMarkerId_;

  struct WT_API TileLayer {
    std::string urlTemplate;
    Json::Object options;
  };

  struct Overlay;
  struct Polyline;
  struct Circle;

  std::vector<TileLayer> tileLayers_; // goes on the tilePane, z-index 200
  std::size_t renderedTileLayersSize_;
  std::vector<std::unique_ptr<Overlay> > overlays_; // goes on the overlayPane, z-index 400
  std::size_t renderedOverlaysSize_;

  struct WT_API MarkerEntry {
      static const int BIT_ADDED = 0;
      static const int BIT_REMOVED = 1;

      MarkerEntry();

      std::unique_ptr<Marker> uMarker;
      Marker *marker;
      long long id;
      std::bitset<2> flags;
  };

  std::vector<MarkerEntry> markers_; // goes on the markerPane, z-index 600

  void setup(); // called from constructors to reduce code duplication
  void defineJavaScript();
  void addTileLayerJS(WStringStream &ss, const TileLayer &layer) const;
  void panToJS(WStringStream &ss, const Coordinate &position) const;
  void zoomJS(WStringStream &ss, int level) const;
  void addMarkerJS(WStringStream &ss, long long id, const Marker *marker) const;
  void removeMarkerJS(WStringStream &ss, long long id) const;
  void moveMarkerJS(WStringStream &ss, long long id, const Coordinate &position) const;

  void handlePanChanged(double latitude, double longitude);
  void handleZoomLevelChanged(int zoomLevel);

  static void addPathOptions(Json::Object &options, const WPen &stroke, const WBrush &fill);

};

}

#endif // WLEAFLETMAP_H_
