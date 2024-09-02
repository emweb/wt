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
  struct ItemEntry;

  struct OrderedAction {
    enum class Type {
      None,
      Add,
      MoveFront,
      MoveBack
    };

    OrderedAction(Type type = Type::None);

    Type type;
    int sequenceNumber;
    ItemEntry* itemEntry;
  };

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

  /*! \class AbstractMapItem
   *  \brief An abstract map item
   *
   * This is the base class for all items that can be added to a
   * WLeafletMap. As this is an abstract class, it should not be added
   * directly to a map as is. Instead, you should use one of the
   * subclasses.
   */
  class WT_API AbstractMapItem: public WObject {
  public:
    virtual ~AbstractMapItem();

    AbstractMapItem(const AbstractMapItem&) = delete;
    AbstractMapItem& operator=(const AbstractMapItem&) = delete;
    AbstractMapItem(AbstractMapItem&&) = delete;
    AbstractMapItem& operator=(AbstractMapItem&&) = delete;

    /*! \brief Move the map item
     *
     * If this map item belongs to a map, this will trigger an update
     * of the WLeafletMap to move the map item. If it doesn't belong to
     * a map, the position is merely updated.
     */
    void move(const Coordinate& pos);

    /*! \brief Get the current position
     *
     * \sa move
     */
    Coordinate position() const { return pos_; }

    Signal<>& clicked() { return clicked_; }
    Signal<>& doubleClicked() { return dblclicked_; }
    Signal<>& mouseWentDown() { return mousedown_; }
    Signal<>& mouseWentUp() { return mouseup_; }
    Signal<>& mouseWentOver() { return mouseover_; }
    Signal<>& mouseWentOut() { return mouseout_; }


  protected:
    /*! \brief Constructor
     *
     * Since this is an abstract class, it should not be used directly.
     */
    explicit AbstractMapItem(const Coordinate& pos);

    /*! \brief Writes the JS code to create this item to the stream.
     *
     * This method should write in \p ss the JS code that creates the
     * item. The \p postJS stream can be used to write JS code that
     * should be executed after the item has been created.
     */
    virtual void createItemJS(WStringStream& ss, WStringStream& postJS, long long id) = 0;

    /*! \brief Unrender the item.
     *
     * This is called when the map needs to be recreated. You can
     * override this function in case you need to do some cleanup
     * before the map is recreated.
     *
     * By default, this does nothing.
     *
     * \sa WLeafletMap::setOptions()
     */
    virtual void unrender();

    /*! \brief Return whether this item needs to be updated.
     *
     * This is called when the map is rendered. If this returns true,
     * update() will be used to update the item.
     */
    virtual bool needsUpdate() const;

    //! Returns the map this item belongs to.
    WLeafletMap* map() { return map_; }

    //!  Returns the map this item belongs to.
    const WLeafletMap* map() const { return map_; }

    /*! \brief Writes the JS to update this item to the stream.
     *
     * This is called when the map is rendered if needsUpdate()
     * returns true.
     */
    virtual void update(WStringStream& js);

    /*! \brief Set the map this item belongs to.
     *
     * This is called to set the map the item belongs to. You can
     * override this function if you need to do something when the item
     * is added to a map.
     *
     * You should not call this function directly.
     *
     * \sa map()
     */
    virtual void setMap(WLeafletMap* map);

  private:
    static const int BIT_MOVED = 0;

    std::bitset<1> flags_;

    WLeafletMap* map_;
    Coordinate pos_;
    OrderedAction orderedAction_;

    Signal<> clicked_;
    Signal<> dblclicked_;
    Signal<> mousedown_;
    Signal<> mouseup_;
    Signal<> mouseover_;
    Signal<> mouseout_;

    /// This method should write the JS code needed to update the item.
    /// The JS code written can use o.wtObj to refer to the WLeafletMap
    /// JS object.
    virtual void applyChangeJS(WStringStream& ss, long long id);

    virtual bool changed() const { return flags_.any(); }

    /// The name of the JS function that adds the item to the map.
    virtual std::string addFunctionJs() const { return "addMapItem"; };

    OrderedAction& getOrderedAction() { return orderedAction_ ; }
    void resetOrderedAction();

    friend class WLeafletMap;
  };

  /*! \class AbstractOverlayItem
   *  \brief An abstract map item with text
   *
   * This is the base class for all AbstractMapItems that can be added
   * to other AbstractMapItems. This is an abstract class, so it should
   * not be used directly.
   */
  class WT_API AbstractOverlayItem : public AbstractMapItem {
  public:
    virtual ~AbstractOverlayItem();

    AbstractOverlayItem(const AbstractOverlayItem& ) = delete;
    AbstractOverlayItem& operator=(const AbstractOverlayItem& ) = delete;
    AbstractOverlayItem(AbstractOverlayItem&& ) = delete;
    AbstractOverlayItem& operator=(AbstractOverlayItem&& ) = delete;

    /*! \brief Set the options of the AbstractOverlayItem
     *
     * Set the options that will be passed to the WLeafletMap
     * AbstractOverlayItem at construction.
     *
     * \if cpp
     * Usage example:
     * \code
     * Wt::Json::Object options;
     * options["autoClose"] = true;
     * options["closeOnClick"] = true;
     * popup->setOptions(options);
     * \endcode
     * \endif
     *
     * See https://leafletjs.com/reference.html for the list of options.
     *
     * \note Modifying the options after the AbstractOverlayItem has
     *       been loaded by the user will not work as the
     *       AbstractOverlayItem was already constructed. Some option
     *       like 'content' can be changed after load using the
     *       appropriate function.
     *
     * \sa setContent
     */
    void setOptions(const Json::Object& options);

    //! Set the content.
    void setContent(std::unique_ptr<WWidget> content);

    //! Set the content.
    void setContent(const WString& content);

    /*! \brief Get the content.
     *
     * \sa setContent
     */
    const WWidget* content() const { return content_.get(); }

    /*! \brief Opens the AbstractOverlayItem
     *
     * \sa close, toggle
     */
    void open();

    /*! \brief Closes the AbstractOverlayItem
     *
     * \sa open, toggle
     */
    void close();

    /*! \brief Opens or closes the AbstractOverlayItem
     *
     * \sa open, close
     */
    void toggle();

    /*! \brief Returns whether the AbstractOverlayItem is open
     *
     * \sa close, toggle
     */
    bool isOpen() const { return open_; }

    /*! \brief Signal emited after the AbstractOverlayItem was opened
     *
     * \sa closed
     */
    Signal<>& opened() { return opened_; }

    /*! \brief Signal emited after the AbstractOverlayItem was closed
     *
     * \sa opened
     */
    Signal<>& closed() { return closed_; }

    /*! \brief Brings this AbstractOverlayItem to the front.
     *
     * This brings this AbstractOverlayItem to the front of other
     * AbstractOverlayItem of the same type.
     *
     * \warning This function only works after the AbstractOverlayItem
     *          is added to the map.
     *
     * \sa bringToBack
     */
    void bringToFront();

    /*! \brief Brings this AbstractOverlayItem to the back.
     *
     * This brings this AbstractOverlayItem to the back of other
     * AbstractOverlayItem of the same type.
     *
     * \warning This function only works after the AbstractOverlayItem
     *          is added to the map.
     *
     * \sa bringToFront
     */
    void bringToBack();

  protected:
    /*! \brief Constructor
     *
     * Creates a new AbstractOverlayItem that has the given coordinates.
     *
     * Since this is an abstract class, this should not be used
     * directly.
     */
    explicit AbstractOverlayItem(const Coordinate& pos);

    /*! \brief Constructor
     *
     * Creates a new AbstractOverlayItem with the given content and that
     * has the given coordinates.
     *
     * Since this is an abstract class, this should not be used
     * directly.
     */
    AbstractOverlayItem(const Coordinate& pos, std::unique_ptr<WWidget> content);

    void setMap(WLeafletMap* map) override;

  private:
    static const int BIT_CONTENT_CHANGED = 0;
    static const int BIT_OPEN_CHANGED = 1;

    std::bitset<2> flags_;
    std::unique_ptr<WWidget> content_;
    bool open_;
    Json::Object options_;

    Signal<> opened_;
    Signal<> closed_;

    bool changed() const override { return flags_.any() || AbstractMapItem::changed(); }
    void applyChangeJS(WStringStream& ss, long long id) override;
    void init();

    std::string addFunctionJs() const override { return "addOverlayItem"; }

    friend class WLeafletMap;
  };

  /*! \class Popup
   *  \brief A popup that can be added to the WLeafletMap.
   *
   * Popups are interactive windows that can be opened on the map,
   * typically linked to a map location or a Marker.
   *
   * \note Multiple popups can be added to a map (using coordinates),
   *       but only one popup at the time can be linked to each Marker.
   *
   * \sa WLeafletMap::addPopup(), Marker::addPopup()
   */
  class WT_API Popup : public AbstractOverlayItem {
  public:
    //! Create a popup with the given coordinates.
    explicit Popup(const Coordinate& pos = Coordinate(0,0));

    //! Create a popup with the given content.
    explicit Popup(std::unique_ptr<WWidget> content);

    /*! \brief Create a popup with the given content.
     *
     * This is a shortcut for creating a popup with a WText widget
     * as content.
     */
    explicit Popup(const WString& content);

    //! Create a popup with the given content and coordinates.
    Popup(const Coordinate& pos, std::unique_ptr<WWidget> content);

    /*! \brief Create a popup with the given content and coordinates.
     *
     * This is a shortcut for creating a popup with a WText widget
     * as content.
     */
    Popup(const Coordinate& pos, const WString& content);

    virtual ~Popup();

    Popup(const Popup&) = delete;
    Popup& operator=(const Popup&) = delete;
    Popup(Popup&&) = delete;
    Popup& operator=(Popup&&) = delete;

  protected:
    void createItemJS(WStringStream& ss, WStringStream& postJS, long long id) override;

  private:
    std::string addFunctionJs() const override { return "addPopup"; }

    friend class WLeafletMap;
  };

  /*! \class Tooltip
   *  \brief A Tooltip that can be added to the WLeafletMap
   *
   * Tooltips are interactive windows that can be opened on the map,
   * typically linked to a map location or a Marker.
   *
   * \note Multiple tooltips can be added to a map (using coordinates),
   *       but only one tooltip at the time can be linked to each
   *       Marker.
   *
   * \sa WLeafletMap::addTooltip(), Marker::addTooltip()
   */
  class WT_API Tooltip : public AbstractOverlayItem {
  public:
    //! Create a tooltip with the given coordinates.
    explicit Tooltip(const Coordinate& pos = Coordinate(0,0));

    //! Create a tooltip with the given content.
    explicit Tooltip(std::unique_ptr<WWidget> content);

    /*! \brief Create a tooltip with the given content.
     *
     * This is a shortcut for creating a tooltip with a WText widget
     * as content.
     */
    explicit Tooltip(const WString& content);

    //! Create a tooltip with the given content and coordinates.
    Tooltip(const Coordinate& pos, std::unique_ptr<WWidget> content);

    /*! \brief Create a tooltip with the given content and coordinates.
     *
     * This is a shortcut for creating a tooltip with a WText widget
     * as content.
     */
    Tooltip(const Coordinate& pos, const WString& content);

    virtual ~Tooltip();

    Tooltip(const Tooltip&) = delete;
    Tooltip& operator=(const Tooltip&) = delete;
    Tooltip(Tooltip&&) = delete;
    Tooltip& operator=(Tooltip&&) = delete;

  protected:
    void createItemJS(WStringStream& ss, WStringStream& postJS, long long id) override;

  private:
    std::string addFunctionJs() const override { return "addTooltip"; }

    friend class WLeafletMap;
  };

  /*! \class Marker
   *  \brief An abstract marker
   *
   * This marker can be placed on a WLeafletMap at certain coordinates.
   */
  class WT_API Marker : public AbstractMapItem {
  public:
    virtual ~Marker();

    Marker(const Marker &) = delete;
    Marker& operator=(const Marker &) = delete;
    Marker(Marker &&) = delete;
    Marker& operator=(Marker &&) = delete;

    /*! \brief Add the popup to the Marker
     *
     * Add the popup to the Marker. This will remove any popup
     * previously added to this Marker.
     *
     * A popup added to a Marker will have it's coordinate set to the
     * coordinate of the Marker and will be closed by default. If the
     * Marker's interactive option is true, the popup will switch
     * between closed and open when the Marker is clicked.
     *
     * \sa removePopup
     */
    void addPopup(std::unique_ptr<Popup> popup);

#ifndef WT_TARGET_JAVA
    template<typename P>
    P* addPopup(std::unique_ptr<P> popup)
    {
      P* result = popup.get();
      addPopup(std::unique_ptr<Popup>(std::move(popup)));
      return result;
    }
#endif // WT_TARGET_JAVA

    /*! \brief Removes the popup from the Marker
     *
     * \sa addPopup
     */
    std::unique_ptr<Popup> removePopup();

    /*! \brief Return the popup added to the Marker
     *
     * \sa addPopup
     */
    Popup* popup() { return popup_; }

    /*! \brief Add the tooltip to the Marker
     *
     * Add the tooltip to the Marker. This will remove any tooltip
     * previously added to this Marker.
     *
     * A tooltip added to a Marker will have it's coordinate set to the
     * coordinate of the Marker. If the Marker's option interactive is
     * true, the tooltip will switch between closed and open when the
     * mouse hover over the Marker.
     *
     * \sa removeTooltip
     */
    void addTooltip(std::unique_ptr<Tooltip> tooltip);

#ifndef WT_TARGET_JAVA
    template<typename T>
    T* addTooltip(std::unique_ptr<T> tooltip)
    {
      T* result = tooltip.get();
      addTooltip(std::unique_ptr<Tooltip>(std::move(tooltip)));
      return result;
    }
#endif // WT_TARGET_JAVA

    /*! \brief Removes the tooltip from the Marker
     *
     * \sa addTooltip
     */
    std::unique_ptr<Tooltip> removeTooltip();

    /*! \brief Return the tooltip added to the Marker
     *
     * \sa addTooltip
     */
    Tooltip *tooltip() { return tooltip_; }

  protected:
    explicit Marker(const Coordinate &pos);

    void setMap(WLeafletMap* map) override;

  private:
    Popup* popup_;
    std::unique_ptr<Popup> popupBuffer_;
    Tooltip *tooltip_;
    std::unique_ptr<Tooltip> tooltipBuffer_;

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
    virtual void createItemJS(WStringStream& ss, WStringStream& postJS, long long id) override;
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

    /*! \brief Set the options of the marker.
     *
     * Set the options that will be passed to the WLeafletMap marker.
     *
     * See https://leafletjs.com/reference.html#marker for the list of options.
     */
    void setOptions(const Json::Object &options);

  protected:
    virtual void createItemJS(WStringStream& ss, WStringStream& postJS, long long id) override;

  private:
    Json::Object options_;
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

  /*! \brief Add the given popup
   */
  void addPopup(std::unique_ptr<Popup> popup);

#ifndef WT_TARGET_JAVA
  template<typename P>
  P* addPopup(std::unique_ptr<P> popup)
  {
    P* result = popup.get();
    addPopup(std::unique_ptr<Popup>(std::move(popup)));
    return result;
  }
#endif // WT_TARGET_JAVA

  /*! \brief Remove the given popup
   */
  std::unique_ptr<Popup> removePopup(Popup* popup);

  /*! \brief Add the given tooltip
   */
  void addTooltip(std::unique_ptr<Tooltip> tooltip);

#ifndef WT_TARGET_JAVA
  template<typename T>
  T* addTooltip(std::unique_ptr<T> tooltip)
  {
    T* result = tooltip.get();
    addTooltip(std::unique_ptr<Tooltip>(std::move(tooltip)));
    return result;
  }
#endif // WT_TARGET_JAVA

  /*! \brief Remove the given tooltip
   */
  std::unique_ptr<Tooltip> removeTooltip(Tooltip *tooltip);

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
  JSignal<long long, bool> overlayItemToggled_;
  Coordinate position_;
  int zoomLevel_;
  long long nextMarkerId_;
  int nextActionSequenceNumber_;

  JSignal<long long> itemClicked_;
  JSignal<long long> itemDblclicked_;
  JSignal<long long> itemMousedown_;
  JSignal<long long> itemMouseup_;
  JSignal<long long> itemMouseover_;
  JSignal<long long> itemMouseout_;

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

  struct WT_API ItemEntry {
      static const int BIT_ADDED = 0;
      static const int BIT_REMOVED = 1;

      ItemEntry();

      std::unique_ptr<AbstractMapItem> uMapItem;
      AbstractMapItem* mapItem;
      ItemEntry *parent;
      long long id;
      std::bitset<2> flags;
  };

  std::vector<std::unique_ptr<ItemEntry> > mapItems_; // goes on the item pane, z-index 600, 650 or 700

  void setup(); // called from constructors to reduce code duplication
  void defineJavaScript();
  void addTileLayerJS(WStringStream &ss, const TileLayer &layer) const;
  void panToJS(WStringStream &ss, const Coordinate &position) const;
  void zoomJS(WStringStream &ss, int level) const;
  AbstractMapItem* getItem(long long id) const;
  void addItem(std::unique_ptr<AbstractMapItem> mapItem, Marker *parent = nullptr);
  std::unique_ptr<AbstractMapItem> removeItem(AbstractMapItem *mapItem, Marker *parent = nullptr);
  std::unique_ptr<Popup> removePopup(Popup *popup, Marker *parent);
  std::unique_ptr<Tooltip> removeTooltip(Tooltip *tooltip, Marker *parent);
  void addItemJS(WStringStream &ss, ItemEntry &entry) const;
  void removeItemJS(WStringStream &ss, long long id) const;
  void updateItemJS(WStringStream &ss, ItemEntry &entry) const;
  void updateItemJS(WStringStream &ss, ItemEntry &entry, const std::string &fname) const;

  void handlePanChanged(double latitude, double longitude);
  void handleZoomLevelChanged(int zoomLevel);
  void handleOverlayItemToggled(long long id, bool open);
  int getNextActionSequenceNumber();

  void handleItemClicked(long long id);
  void handleItemDblClicked(long long id);
  void handleItemMousedown(long long id);
  void handleItemMouseup(long long id);
  void handleItemMouseover(long long id);
  void handleItemMouseout(long long id);

  static void addPathOptions(Json::Object &options, const WPen &stroke, const WBrush &fill);

};

}

#endif // WLEAFLETMAP_H_
