#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLeafletMap.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WSpinBox.h>

#include <Wt/Json/Object.h>

const Wt::WLeafletMap::Coordinate COORD_EMWEB(50.906705, 4.655800);
const Wt::WLeafletMap::Coordinate COORD_PARIS(48.856613, 2.352222);

class LeafletApplication : public Wt::WApplication {
public:
  LeafletApplication(const Wt::WEnvironment &env);
  virtual ~LeafletApplication();

private:
  Wt::WLeafletMap *map_;
  std::unique_ptr<Wt::WLeafletMap::Marker> uMarker_;
  Wt::WLeafletMap::Marker *marker_;

  void panToEmweb();
  void panToParis();
  void toggleMarker();
};

LeafletApplication::LeafletApplication(const Wt::WEnvironment &env)
  : WApplication(env),
    map_(root()->addNew<Wt::WLeafletMap>()),
    uMarker_(new Wt::WLeafletMap::LeafletMarker(Wt::WLeafletMap::Coordinate(50.906705, 4.655800))),
    marker_(uMarker_.get())
{
  map_->resize(500, 500);

  // Add OpenStreetMap tile layer
  Wt::Json::Object options;
  options["maxZoom"] = 19;
  options["attribution"] = "&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors";
  map_->addTileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", options);

  // Pan to Emweb
  map_->panTo(Wt::WLeafletMap::Coordinate(50.906705, 4.655800));

  // Add marker
  map_->addMarker(std::move(uMarker_));

  // Add a polyline from the center of Leuven to Emweb
  std::vector<Wt::WLeafletMap::Coordinate> points;
  points.push_back(Wt::WLeafletMap::Coordinate(50.9082,  4.66056));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90901, 4.66426));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90944, 4.66514));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90968, 4.66574));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91021, 4.66541));
  points.push_back(Wt::WLeafletMap::Coordinate(50.9111,  4.66508));
  points.push_back(Wt::WLeafletMap::Coordinate(50.9119,  4.66469));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91224, 4.66463));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91227, 4.66598));
  points.push_back(Wt::WLeafletMap::Coordinate(50.9122,  4.66786));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91199, 4.66962));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91169, 4.67117));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91107, 4.67365));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91061, 4.67515));
  points.push_back(Wt::WLeafletMap::Coordinate(50.91023, 4.67596));
  points.push_back(Wt::WLeafletMap::Coordinate(50.9098,  4.67666));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90953, 4.67691));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90912, 4.67746));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90882, 4.67772));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90838, 4.67801));
  points.push_back(Wt::WLeafletMap::Coordinate(50.9083,  4.67798));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90803, 4.67814));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90742, 4.67836));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90681, 4.67845));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90209, 4.67871));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90134, 4.67893));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90066, 4.6793));
  points.push_back(Wt::WLeafletMap::Coordinate(50.90015, 4.67972));
  points.push_back(Wt::WLeafletMap::Coordinate(50.89945, 4.68059));
  points.push_back(Wt::WLeafletMap::Coordinate(50.89613, 4.68582));
  points.push_back(Wt::WLeafletMap::Coordinate(50.8952,  4.68719));
  points.push_back(Wt::WLeafletMap::Coordinate(50.89464, 4.68764));
  points.push_back(Wt::WLeafletMap::Coordinate(50.89183, 4.69032));
  points.push_back(Wt::WLeafletMap::Coordinate(50.89131, 4.69076));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88916, 4.69189));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88897, 4.69195));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88859, 4.69195));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88813, 4.69193));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88697, 4.69135));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88669, 4.6913));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88531, 4.69155));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88425, 4.69196));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88398, 4.69219));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88391, 4.69226));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88356, 4.69292));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88323, 4.69361));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88067, 4.6934));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88055, 4.69491));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88036, 4.69616));
  points.push_back(Wt::WLeafletMap::Coordinate(50.88009, 4.69755));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87973, 4.69877));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87951, 4.69856));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87933, 4.69831));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87905, 4.69811));
  points.push_back(Wt::WLeafletMap::Coordinate(50.879,   4.69793));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87856, 4.69745));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87849, 4.69746));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87843, 4.69758));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87822, 4.69758));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87814, 4.69766));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87813, 4.69788));
  points.push_back(Wt::WLeafletMap::Coordinate(50.87789, 4.69862));
  Wt::WPen pen = Wt::WColor(0, 191, 255);
  pen.setCapStyle(Wt::PenCapStyle::Round);
  pen.setJoinStyle(Wt::PenJoinStyle::Round);
  pen.setWidth(3.0);
  map_->addPolyline(points, pen);

  // Add a circle around Emweb
  Wt::WBrush brush = Wt::WColor(0, 191, 255, 100);
  map_->addCircle(COORD_EMWEB, 50.0, pen, brush);

  Wt::WPushButton *panToEmwebBtn = root()->addNew<Wt::WPushButton>(Wt::utf8("pan to Emweb"));
  panToEmwebBtn->clicked().connect(this, &LeafletApplication::panToEmweb);

  Wt::WPushButton *panToParisBtn = root()->addNew<Wt::WPushButton>(Wt::utf8("pan to Paris"));
  panToParisBtn->clicked().connect(this, &LeafletApplication::panToParis);

  Wt::WPushButton *toggleMarkerBtn = root()->addNew<Wt::WPushButton>(Wt::utf8("toggle marker"));
  toggleMarkerBtn->clicked().connect(this, &LeafletApplication::toggleMarker);
}

LeafletApplication::~LeafletApplication()
{ }

void LeafletApplication::panToEmweb()
{
  map_->panTo(COORD_EMWEB);
  marker_->move(COORD_EMWEB);
}

void LeafletApplication::panToParis()
{
  map_->panTo(COORD_PARIS);
  marker_->move(COORD_PARIS);
}

void LeafletApplication::toggleMarker()
{
  if (uMarker_) {
    map_->addMarker(std::move(uMarker_));
  } else {
    uMarker_ = map_->removeMarker(marker_);
  }
}

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment &env)
{
  return Wt::cpp14::make_unique<LeafletApplication>(env);
}

int main(int argc, char *argv[])
{
  return Wt::WRun(argc, argv, &createApplication);
}
