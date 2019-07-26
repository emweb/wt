// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Richard Ulrich.
 */
#ifndef WGOOGLEMAP_H_
#define WGOOGLEMAP_H_

#include <Wt/WColor.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WSignal.h>

#include <vector>
#include <string>

namespace Wt
{

/*! \brief Google Maps API version
 */
enum class GoogleMapsVersion {
  v2, //!< API Version 2.x
  v3  //!< API Version 3.x
};

/*! \brief Google Maps map type control
 */
enum class MapTypeControl {
  None,         //!< Show no maptype control
  Default,      //!< Show the default maptype control 
                // (the behaviour depends on the Google Maps 
                // API version)
  Menu,         //!< Show the dropdown menu maptype control
  Hierarchical, //!< Show the hierarchical maptype control 
                // (only availble in Google Maps API v2)
  HorizontalBar //!< Show the horizontal bar maptype control
                // (only availble in Google Maps API v3)
};

/*! \class WGoogleMap Wt/WGoogleMap.h Wt/WGoogleMap.h
 *  \brief A widget that displays a google map.
 *
 * This widget uses the online Google Maps server to display a map. It
 * exposes a part of the google maps API.
 *
 * This widget supports both version 2 and version 3 of the Google Maps API.
 * The version 2 API is used by default, to enable the version 3 API, use the 
 * constructor's version argument.
 *
 * To use the map on a public server you will need to obtain a
 * key. The widget will look for this key as the configuration
 * property <tt>"google_api_key"</tt>. If this configuration property
 * has not been set, it will use a key that is suitable for localhost.
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 *
 * Contributed by: Richard Ulrich.
 */
class WT_API WGoogleMap : public WCompositeWidget
{
public:
  /*! \brief Typedef for enum Wt::GoogleMapsVersion */
  typedef GoogleMapsVersion Version;

  /*! \class Coordinate
   *  \brief A geographical coordinate (latitude/longitude)
   */
  class WT_API Coordinate
  {
  public:
    /*! \brief Default constructor
     */
    Coordinate();

    /*! \brief Creates with given latitude and longitude.
     */
    Coordinate(double latitude, double longitude);

    #ifndef WT_TARGET_JAVA
    explicit Coordinate(const std::pair<double, double>& lat_long);
    #endif

    /*! \brief Sets the latitude.
     */
    void setLatitude(double latitude);

    /*! \brief Sets the longitude.
     */
    void setLongitude(double longitude);

    /*! \brief Returns the latitude.
     */
    double latitude() const { return lat_; }

    /*! \brief Returns the longitude.
     */
    double longitude() const { return lon_; }

    /*! \brief Calculates the distance between two points in km (approximate).
     *
     * The calculation uses a sphere. Results can be out by 0.3%.
     */
    double distanceTo(const Coordinate &rhs) const;

    #ifndef WT_TARGET_JAVA
    /*! \brief Conversion operator to pair of double.
     */
    std::pair<double, double> operator ()() const;
    #endif

  private:
    double lat_, lon_;
  };

  /*! \brief Creates a map widget with a version.
   */
  WGoogleMap(GoogleMapsVersion version = GoogleMapsVersion::v3);

  /*! \brief Destructor.
   */
  virtual ~WGoogleMap();

  /*! \brief Adds a marker overlay to the map.
   */
  void addMarker(const Coordinate &pos);

  /*! \brief Adds a polyline overlay to the map.
   *
   *  Specify a value between 0.0 and 1.0 for the opacity or set the alpha 
   *  value in the color.
   */
  void addPolyline(const std::vector<Coordinate>& points,
		   const WColor& color = WColor(StandardColor::Red),
		   int width = 2, double opacity = 1.0);

  /*! \brief Adds a circle to the map.
   *
   * The stroke and fill opacity can be configured respectively in
   * the strokeColor and fillColor.
   * This feature is only supported by the Google Maps API version 3.
   */
  void addCircle(const Coordinate& center, double radius, 
		 const WColor& strokeColor, int strokeWidth, 
		 const WColor& fillColor = WColor());

  /*! \brief Adds a icon marker overlay to the map.
   */
  void addIconMarker(const Coordinate &pos, const std::string& iconURL);

  /*! \brief Removes all overlays from the map.
   */
  void clearOverlays();

  /*! \brief Opens a text bubble with html text at a specific location.
   */
  void openInfoWindow(const Coordinate&pos, const Wt::WString& myHtml);

  /*! \brief Sets the map view to the given center.
   */
  void setCenter(const Coordinate& center);

  /*! \brief Sets the map view to the given center and zoom level.
   */
  void setCenter(const Coordinate& center, int zoom);

  /*! \brief Changes the center point of the map to the given point.
   *
   * If the point is already visible in the current map view, change
   * the center in a smooth animation.
   */
  void panTo(const Coordinate& center);

  #ifndef WT_TARGET_JAVA
  /*! \brief Zooms the map to a region defined by a bounding box.
   */
  void zoomWindow(const std::pair<Coordinate, Coordinate>& bbox);
  #endif

  /*! \brief Zooms the map to a region defined by a bounding box.
   */
  void zoomWindow(const Coordinate& topLeft, const Coordinate& bottomRight);

  /*! \brief Sets the zoom level to the given new value.
   */
  void setZoom(int level);

  /*! \brief Increments zoom level by one.
   */
  void zoomIn();

  /*! \brief Decrements zoom level by one.
   */
  void zoomOut();

  /*! \brief Stores the current map position and zoom level.
   *
   * You can later restore this position using
   * returnToSavedPosition().
   */
  void savePosition();

  /*! \brief Restores the map view that was saved by savePosition().
   */
  void returnToSavedPosition();

  /*! \brief Enables the dragging of the map (enabled by default).
   */
  void enableDragging();

  /*! \brief Disables the dragging of the map.
   */
  void disableDragging();

  /*! \brief Enables double click to zoom in and out (enabled by default).
   */
  void enableDoubleClickZoom();

  /*! \brief Disables double click to zoom in and out.
   */
  void disableDoubleClickZoom();

  /*! \brief Enables the GoogleBar, an integrated search control, on the map.
   *
   * When enabled, this control takes the place of the default Powered
   * By Google logo.
   *
   * This control is initially disabled.
   *
   * \note This functionality is no longer available in the Google Maps API v3.
   */
  void enableGoogleBar();

  /*! \brief Disables the GoogleBar integrated search control.
   *
   * When disabled, the default Powered by Google logo occupies the
   * position formerly containing this control. Note that this control
   * is already disabled by default.
   *
   * \note This functionality is no longer available in the Google Maps API v3.
   */
  void disableGoogleBar();

  /*! \brief Enables zooming using a mouse's scroll wheel.
   *
   * Scroll wheel zoom is disabled by default.
   */
  void enableScrollWheelZoom();

  /*! \brief Disables zooming using a mouse's scroll wheel.
   *
   * Scroll wheel zoom is disabled by default.
   */
  void disableScrollWheelZoom();

  /*! \brief Sets the map type control.
   *
   * The control allows selecting and switching between supported map types
   * via buttons.
   */
  void setMapTypeControl(MapTypeControl type);

  /*! \brief The click event.
   *
   * This event is fired when the user clicks on the map with the mouse.
   */
  JSignal<Coordinate>& clicked() { return clicked_; }

  /*! \brief The double click event.
   *
   * This event is fired when a double click is done on the map.
   */
  JSignal<Coordinate>& doubleClicked() { return doubleClicked_; }

  /*! \brief This event is fired when the user moves the mouse inside the map.
   */
  JSignal<Coordinate>& mouseMoved();

  /*! \brief Return the used Google Maps API version.
   */
  GoogleMapsVersion apiVersion() { return apiVersion_; }

private:
  JSignal<Coordinate> clicked_;
  JSignal<Coordinate> doubleClicked_;
  JSignal<Coordinate> *mouseMoved_;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;

  /*! \brief Execute a piece of JavaScript that manipulates the map.
   *
   * This is like doJavaScript() but delays the javascript until the map
   * has been loaded.
   */
  virtual void doGmJavaScript(const std::string& jscode);

private:
  std::vector<std::string> additions_;

  void streamJSListener(const JSignal<Coordinate> &signal, 
			std::string signalName,
			WStringStream &strm);

  void setMapOption(const std::string &option, 
		    const std::string &state);

private:
  GoogleMapsVersion apiVersion_;
};

#ifndef WT_TARGET_JAVA
extern WT_API std::istream& operator>> (std::istream& i,
				        WGoogleMap::Coordinate& coordinate);
#endif

} //  namespace Wt

#endif // WGOOGLEMAP_H_INCLUDED
