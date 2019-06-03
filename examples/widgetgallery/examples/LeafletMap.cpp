#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WImage.h>
#include <Wt/WLeafletMap.h>
#include <Wt/WLink.h>

#include <Wt/Json/Object.h>
#include <Wt/Json/Value.h>

#include <vector>

class LeafletMapExample : public Wt::WContainerWidget
{
public:
  LeafletMapExample()
  {
    auto layout = setLayout(Wt::cpp14::make_unique<Wt::WHBoxLayout>());

    setHeight(400);

    map_ = layout->addWidget(Wt::cpp14::make_unique<Wt::WLeafletMap>(), 1);

    Wt::Json::Object options;
#ifndef WT_TARGET_JAVA
    options["maxZoom"] = 19;
    options["attribution"] = "&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors";
#else // WT_TARGET_JAVA
    options["maxZoom"] = Wt::Json::Value(19);
    options["attribution"] = Wt::Json::Value("&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors");
#endif // WT_TARGET_JAVA
    map_->addTileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", options);

    map_->panTo(EMWEB_COORDS);

    addEmwebLogoMarker();

    Wt::WPen pen = Wt::WColor(0, 191, 255);
    pen.setCapStyle(Wt::PenCapStyle::Round);
    pen.setJoinStyle(Wt::PenJoinStyle::Round);
    pen.setWidth(3.0);
    map_->addPolyline(roadDescription(), pen);
  }

private:
  Wt::WLeafletMap *map_;

  void addEmwebLogoMarker()
  {
    auto emwebLogo = Wt::cpp14::make_unique<Wt::WImage>(
          Wt::WLink("https://www.emweb.be/css/emweb_small_filled.png"));
    emwebLogo->setInline(false);
    emwebLogo->resize(118, 32);
    auto emwebMarker = Wt::cpp14::make_unique<Wt::WLeafletMap::WidgetMarker>(EMWEB_COORDS, std::move(emwebLogo));
    map_->addMarker(std::move(emwebMarker));
  }

  std::vector<Wt::WLeafletMap::Coordinate> roadDescription() {
    std::vector<Wt::WLeafletMap::Coordinate> result;
    result.push_back(Wt::WLeafletMap::Coordinate(50.9082, 4.66056));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90901, 4.66426));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90944, 4.66514));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90968, 4.66574));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91021, 4.66541));
    result.push_back(Wt::WLeafletMap::Coordinate(50.9111, 4.66508));
    result.push_back(Wt::WLeafletMap::Coordinate(50.9119, 4.66469));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91224, 4.66463));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91227, 4.66598));
    result.push_back(Wt::WLeafletMap::Coordinate(50.9122, 4.66786));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91199, 4.66962));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91169, 4.67117));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91107, 4.67365));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91061, 4.67515));
    result.push_back(Wt::WLeafletMap::Coordinate(50.91023, 4.67596));
    result.push_back(Wt::WLeafletMap::Coordinate(50.9098, 4.67666));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90953, 4.67691));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90912, 4.67746));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90882, 4.67772));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90838, 4.67801));
    result.push_back(Wt::WLeafletMap::Coordinate(50.9083, 4.67798));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90803, 4.67814));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90742, 4.67836));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90681, 4.67845));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90209, 4.67871));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90134, 4.67893));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90066, 4.6793));
    result.push_back(Wt::WLeafletMap::Coordinate(50.90015, 4.67972));
    result.push_back(Wt::WLeafletMap::Coordinate(50.89945, 4.68059));
    result.push_back(Wt::WLeafletMap::Coordinate(50.89613, 4.68582));
    result.push_back(Wt::WLeafletMap::Coordinate(50.8952, 4.68719));
    result.push_back(Wt::WLeafletMap::Coordinate(50.89464, 4.68764));
    result.push_back(Wt::WLeafletMap::Coordinate(50.89183, 4.69032));
    result.push_back(Wt::WLeafletMap::Coordinate(50.89131, 4.69076));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88916, 4.69189));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88897, 4.69195));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88859, 4.69195));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88813, 4.69193));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88697, 4.69135));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88669, 4.6913));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88531, 4.69155));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88425, 4.69196));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88398, 4.69219));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88391, 4.69226));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88356, 4.69292));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88323, 4.69361));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88067, 4.6934));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88055, 4.69491));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88036, 4.69616));
    result.push_back(Wt::WLeafletMap::Coordinate(50.88009, 4.69755));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87973, 4.69877));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87951, 4.69856));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87933, 4.69831));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87905, 4.69811));
    result.push_back(Wt::WLeafletMap::Coordinate(50.879, 4.69793));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87856, 4.69745));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87849, 4.69746));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87843, 4.69758));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87822, 4.69758));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87814, 4.69766));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87813, 4.69788));
    result.push_back(Wt::WLeafletMap::Coordinate(50.87789, 4.69862));

    return result;
  }

  static const Wt::WLeafletMap::Coordinate EMWEB_COORDS;
};

const Wt::WLeafletMap::Coordinate LeafletMapExample::EMWEB_COORDS(50.906901, 4.655973);


SAMPLE_BEGIN(LeafletMap)

auto map = Wt::cpp14::make_unique<LeafletMapExample>();

SAMPLE_END(return std::move(map))
