#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WGoogleMap.h>
#include <Wt/WPushButton.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTemplate.h>

class GoogleMapExample : public WContainerWidget
{
public:
    GoogleMapExample()
        : WContainerWidget()
    {
        auto layout = setLayout(cpp14::make_unique<WHBoxLayout>());

	setHeight(400);

	auto map = cpp14::make_unique<WGoogleMap>(GoogleMapsVersion::v3);
	map_ = map.get();
	layout->addWidget(std::move(map), 1);

	map_->setMapTypeControl(MapTypeControl::Default);
	map_->enableScrollWheelZoom();

	auto controls =
	    cpp14::make_unique<WTemplate>(tr("graphics-GoogleMap-controls"));
	auto controls_ = controls.get();
	layout->addWidget(std::move(controls));

	auto zoomIn = cpp14::make_unique<WPushButton>("+");
	auto zoomIn_ = controls_->bindWidget("zoom-in", std::move(zoomIn));
	zoomIn_->addStyleClass("zoom");

	zoomIn_->clicked().connect([=] {
	    map_->zoomIn();
	});

	auto zoomOut = cpp14::make_unique<WPushButton>("-");
	auto zoomOut_ = controls_->bindWidget("zoom-out", std::move(zoomOut));
	zoomOut_->addStyleClass("zoom");

        zoomOut_->clicked().connect([=] {
            map_->zoomOut();
	});

	std::string cityNames[] = { "Brussels", "Lisbon", "Paris" };
	WGoogleMap::Coordinate cityCoords[] = {
	    WGoogleMap::Coordinate(50.85034,4.35171),
	    WGoogleMap::Coordinate(38.703731,-9.135475),
	    WGoogleMap::Coordinate(48.877474, 2.312579)
	};
	    
	for (unsigned i = 0; i < 3; ++i) {
	    auto city = cpp14::make_unique<WPushButton>(cityNames[i]);
	    auto city_ = controls_->bindWidget(cityNames[i], std::move(city));

	    WGoogleMap::Coordinate coord = cityCoords[i];
	    city_->clicked().connect([=] {
		map_->panTo(coord);
	    });
	}

	auto reset = cpp14::make_unique<WPushButton>("Reset");
	auto reset_ = controls_->bindWidget("emweb", std::move(reset));

        reset_->clicked().connect([=] {
            this->panToEmWeb();
        });

	auto savePosition =
	    cpp14::make_unique<WPushButton>("Save current position");
	auto savePosition_ = controls_->bindWidget("save-position", std::move(savePosition));

        savePosition_->clicked().connect([=] {
            this->savePosition();
        });

	auto returnToPosition = cpp14::make_unique<WPushButton>("Return to saved position");
	returnToPosition_ = controls_->bindWidget("return-to-saved-position", std::move(returnToPosition));
	returnToPosition_->setEnabled(false);

	returnToPosition_->clicked().connect([=] {
            map_->returnToSavedPosition();
        });

	mapTypeModel_ = std::make_shared<WStringListModel>();
	addMapTypeControl("No control", MapTypeControl::None);
	addMapTypeControl("Default", MapTypeControl::Default);
	addMapTypeControl("Menu", MapTypeControl::Menu);
	if (map_->apiVersion() == GoogleMapsVersion::v2)
	    addMapTypeControl("Hierarchical",
			      MapTypeControl::Hierarchical);
	if (map_->apiVersion() == GoogleMapsVersion::v3)
	    addMapTypeControl("Horizontal bar",
			      MapTypeControl::HorizontalBar);

	auto menuControls = cpp14::make_unique<WComboBox>();
	auto menuControls_ = controls_->bindWidget("control-menu-combo", std::move(menuControls));
	menuControls_->setModel(mapTypeModel_);
	menuControls_->setCurrentIndex(1);

        menuControls_->activated().connect([=] (int mapType) {
            this->setMapTypeControl(mapType);
        });

	auto draggingCB = cpp14::make_unique<WCheckBox>("Enable dragging");
	auto draggingCB_ = controls_->bindWidget("dragging-cb", std::move(draggingCB));
	draggingCB_->setChecked(true);
	map_->enableDragging();

        draggingCB_->checked().connect([=] {
            map_->enableDragging();
        });

        draggingCB_->unChecked().connect([=] {
            map_->disableDragging();
        });

        auto enableDoubleClickZoomCB =
            cpp14::make_unique<WCheckBox>("Enable double click zoom");
        auto enableDoubleClickZoomCB_ =
            controls_->bindWidget("double-click-zoom-cb", std::move(enableDoubleClickZoomCB));
        enableDoubleClickZoomCB_->setChecked(false);
	map_->disableDoubleClickZoom();

        enableDoubleClickZoomCB_->checked().connect([=] {
            map_->enableDoubleClickZoom();
	});

        enableDoubleClickZoomCB_->unChecked().connect([=] {
            map_->disableDoubleClickZoom();
        });

        auto enableScrollWheelZoomCB =
            cpp14::make_unique<WCheckBox>("Enable scroll wheel zoom");
        auto enableScrollWheelZoomCB_ =
            controls_->bindWidget("scroll-wheel-zoom-cb", std::move(enableScrollWheelZoomCB));
        enableScrollWheelZoomCB_->setChecked(true);
	map_->enableScrollWheelZoom();

        enableScrollWheelZoomCB_->checked().connect([=] {
            map_->enableScrollWheelZoom();
        });

        enableScrollWheelZoomCB_->unChecked().connect([=] {
            map_->disableScrollWheelZoom();
        });

        std::vector<WGoogleMap::Coordinate> road = roadDescription();

        map_->addPolyline(road, WColor(0, 191, 255));

	//Koen's favourite bar!
	map_->addMarker(WGoogleMap::Coordinate(50.885069,4.71958));

	map_->setCenter(road[road.size()-1]);

	map_->openInfoWindow(road[0],
           "<img src=\"http://www.emweb.be/css/emweb_small.jpg\" />"
           "<p><strong>Emweb office</strong></p>");

        map_->clicked().connect([=] (WGoogleMap::Coordinate c) {
            this->googleMapClicked(c);
        });

	map_->doubleClicked().connect([=] (WGoogleMap::Coordinate c) {
            this->googleMapDoubleClicked(c);
        });
    }

private:
    void panToEmWeb() {
        map_->panTo(WGoogleMap::Coordinate(50.9082, 4.66056));
    }

    void savePosition() {
        returnToPosition_->setEnabled(true);
        map_->savePosition();
    }

    void addMapTypeControl(const WString &description,
                           MapTypeControl value) {
	int r = mapTypeModel_->rowCount();
	mapTypeModel_->insertRow(r);
	mapTypeModel_->setData(r, 0, description);
	mapTypeModel_->setData(r, 0, value, ItemDataRole::User);
    }

    void setMapTypeControl(int row) {
        cpp17::any mtc = mapTypeModel_->data(row, 0, ItemDataRole::User);
        map_->setMapTypeControl(cpp17::any_cast<MapTypeControl>
				(mtc));
    }
    
    std::vector<WGoogleMap::Coordinate> roadDescription() {
        std::vector<WGoogleMap::Coordinate> result
        {
          WGoogleMap::Coordinate(50.9082,  4.66056),
          WGoogleMap::Coordinate(50.90901, 4.66426),
          WGoogleMap::Coordinate(50.90944, 4.66514),
          WGoogleMap::Coordinate(50.90968, 4.66574),
          WGoogleMap::Coordinate(50.91021, 4.66541),
          WGoogleMap::Coordinate(50.9111,  4.66508),
          WGoogleMap::Coordinate(50.9119,  4.66469),
          WGoogleMap::Coordinate(50.91224, 4.66463),
          WGoogleMap::Coordinate(50.91227, 4.66598),
          WGoogleMap::Coordinate(50.9122,  4.66786),
          WGoogleMap::Coordinate(50.91199, 4.66962),
          WGoogleMap::Coordinate(50.91169, 4.67117),
          WGoogleMap::Coordinate(50.91107, 4.67365),
          WGoogleMap::Coordinate(50.91061, 4.67515),
          WGoogleMap::Coordinate(50.91023, 4.67596),
          WGoogleMap::Coordinate(50.9098,  4.67666),
          WGoogleMap::Coordinate(50.90953, 4.67691),
          WGoogleMap::Coordinate(50.90912, 4.67746),
          WGoogleMap::Coordinate(50.90882, 4.67772),
          WGoogleMap::Coordinate(50.90838, 4.67801),
          WGoogleMap::Coordinate(50.9083,  4.67798),
          WGoogleMap::Coordinate(50.90803, 4.67814),
          WGoogleMap::Coordinate(50.90742, 4.67836),
          WGoogleMap::Coordinate(50.90681, 4.67845),
          WGoogleMap::Coordinate(50.90209, 4.67871),
          WGoogleMap::Coordinate(50.90134, 4.67893),
          WGoogleMap::Coordinate(50.90066, 4.6793),
          WGoogleMap::Coordinate(50.90015, 4.67972),
          WGoogleMap::Coordinate(50.89945, 4.68059),
          WGoogleMap::Coordinate(50.89613, 4.68582),
          WGoogleMap::Coordinate(50.8952,  4.68719),
          WGoogleMap::Coordinate(50.89464, 4.68764),
          WGoogleMap::Coordinate(50.89183, 4.69032),
          WGoogleMap::Coordinate(50.89131, 4.69076),
          WGoogleMap::Coordinate(50.88916, 4.69189),
          WGoogleMap::Coordinate(50.88897, 4.69195),
          WGoogleMap::Coordinate(50.88859, 4.69195),
          WGoogleMap::Coordinate(50.88813, 4.69193),
          WGoogleMap::Coordinate(50.88697, 4.69135),
          WGoogleMap::Coordinate(50.88669, 4.6913),
          WGoogleMap::Coordinate(50.88531, 4.69155),
          WGoogleMap::Coordinate(50.88425, 4.69196),
          WGoogleMap::Coordinate(50.88398, 4.69219),
          WGoogleMap::Coordinate(50.88391, 4.69226),
          WGoogleMap::Coordinate(50.88356, 4.69292),
          WGoogleMap::Coordinate(50.88323, 4.69361),
          WGoogleMap::Coordinate(50.88067, 4.6934),
          WGoogleMap::Coordinate(50.88055, 4.69491),
          WGoogleMap::Coordinate(50.88036, 4.69616),
          WGoogleMap::Coordinate(50.88009, 4.69755),
          WGoogleMap::Coordinate(50.87973, 4.69877),
          WGoogleMap::Coordinate(50.87951, 4.69856),
          WGoogleMap::Coordinate(50.87933, 4.69831),
          WGoogleMap::Coordinate(50.87905, 4.69811),
          WGoogleMap::Coordinate(50.879,   4.69793),
          WGoogleMap::Coordinate(50.87856, 4.69745),
          WGoogleMap::Coordinate(50.87849, 4.69746),
          WGoogleMap::Coordinate(50.87843, 4.69758),
          WGoogleMap::Coordinate(50.87822, 4.69758),
          WGoogleMap::Coordinate(50.87814, 4.69766),
          WGoogleMap::Coordinate(50.87813, 4.69788),
          WGoogleMap::Coordinate(50.87789, 4.69862),
        };
	
	return result;
    }

    void googleMapDoubleClicked(WGoogleMap::Coordinate c) {
	std::cerr << "Double clicked at coordinate ("
		  << c.latitude() << "," << c.longitude() << ")";
    }

    void googleMapClicked(WGoogleMap::Coordinate c) {
	std::cerr << "Clicked at coordinate ("
		  << c.latitude() << "," << c.longitude() << ")";
    }

private:
    WGoogleMap                           *map_;
    std::shared_ptr<WAbstractItemModel>   mapTypeModel_;

    WPushButton                          *returnToPosition_;
};

SAMPLE_BEGIN(GoogleMap)

auto map = cpp14::make_unique<GoogleMapExample>();

SAMPLE_END(return std::move(map))
