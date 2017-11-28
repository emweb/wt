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

  ///////////////////////////////////////////////////////////////////////////////////////
  //WLeaflet
  ///////////////////////////////////////////////////////////////////////////////////////

  class WT_API WLeaflet : public WCompositeWidget
  {
  public:
    WLeaflet(tile_provider_t tile, double lat, double lon, int zoom);

    void Circle(const std::string &lat, const std::string &lon);

    void Circle(const double lat, 
      const double lon,
      const std::string &color);

    void Polygon(const std::vector<double> &lat, 
      const std::vector<double> &lon,
      const std::string &color);

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
