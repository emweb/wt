#ifndef WLEAFLET_H_
#define WLEAFLET_H_

#include <Wt/WColor>
#include <Wt/WCompositeWidget>
#include <Wt/WJavaScript>
#include <Wt/WSignal>
#include <vector>
#include <string>

namespace Wt
{
  enum class tile_provider_t
  {
    CARTODB, RRZE
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //icon_size_t
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  class WT_API icon_size_t
  {
  public:
    icon_size_t(int w_, int h_) :
      w(w_),
      h(h_)
    {
    }
    int w;
    int h;
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //marker_icon_t
  /////////////////////////////////////////////////////////////////////////////////////////////////////


  class WT_API marker_icon_t
  {
  public:
    marker_icon_t(const std::string &icon_url_,
      const std::string &shadow_url_,
      icon_size_t icon_size_,
      icon_size_t icon_anchor_,
      icon_size_t popup_anchor_,
      icon_size_t shadow_size_) :
      icon_url(icon_url_),
      shadow_url(shadow_url_),
      icon_size(icon_size_),
      icon_anchor(icon_anchor_),
      popup_anchor(popup_anchor_),
      shadow_size(shadow_size_)
    {
    }
    std::string icon_url;
    std::string shadow_url;
    icon_size_t icon_size;
    icon_size_t icon_anchor;
    icon_size_t popup_anchor;
    icon_size_t shadow_size;
  };

  ///////////////////////////////////////////////////////////////////////////////////////
  //WLeaflet
  ///////////////////////////////////////////////////////////////////////////////////////

  class WT_API WLeaflet : public WCompositeWidget
  {
  public:
    WLeaflet(tile_provider_t tile, double lat, double lon, int zoom);

    void Circle(const std::string &lat, const std::string &lon);
    void Circle(const double lat, const double lon, const std::string &color);
    void Circle(const std::string &lat, const std::string &lon, const std::string &color);
    void Polygon(const std::vector<double> &lat, const std::vector<double> &lon, const std::string &color);
    void Marker(const std::string &lat, const std::string &lon, const std::string &text);
    void Marker(const std::string &lat, const std::string &lon, const std::string &text, marker_icon_t icon);

  protected:
    tile_provider_t m_tile;
    double m_lat;
    double m_lon;
    int m_zoom;

  protected:
    virtual void render(WFlags<RenderFlag> flags);
    std::vector<std::string> m_additions;
  };

} //namespace Wt

#endif // WLEAFLET_H_
