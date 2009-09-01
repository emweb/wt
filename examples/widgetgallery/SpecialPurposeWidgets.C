/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SpecialPurposeWidgets.h"
#include "EventDisplayer.h"
#include "DeferredWidget.h"

#include "Wt/WText"
#include "Wt/WTable"
#include "Wt/WPushButton"
#include "Wt/WCheckBox"
#include "Wt/WSound"
#include "Wt/WBreak"

using namespace Wt;

SpecialPurposeWidgets::SpecialPurposeWidgets(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  new WText(tr("specialpurposewidgets-intro"), this);
}

void SpecialPurposeWidgets::populateSubMenu(WMenu *menu)
{
  menu->addItem("WGoogleMap",
		deferCreate(boost::bind(&SpecialPurposeWidgets::wGoogleMap,
					this)));
  menu->addItem("WSound",
		deferCreate(boost::bind(&SpecialPurposeWidgets::wSound, this)));
}

WWidget *SpecialPurposeWidgets::wGoogleMap()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WGoogleMap", result);
  new WText(tr("specialpurposewidgets-WGoogleMap"), result);

  WTable* layout = new WTable(result);
  WGoogleMap* map = new WGoogleMap(layout->elementAt(0,0));
  map->resize(700, 500);

  map->setMapTypeControl(WGoogleMap::DefaultControl);
  map->enableScrollWheelZoom();

  layout->elementAt(0,1)->setPadding(3);

  WContainerWidget* zoomContainer = 
    new WContainerWidget(layout->elementAt(0,1));
  new WText("Zoom: ", zoomContainer);
  WPushButton* zoomIn = new WPushButton("+", zoomContainer);
  zoomIn->clicked().connect(SLOT(map, WGoogleMap::zoomIn));
  WPushButton* zoomOut = new WPushButton("-", zoomContainer);
  zoomOut->clicked().connect(SLOT(map, WGoogleMap::zoomOut));

  std::vector<WGoogleMap::Coordinate> road;
  roadDescription(road);
  map->addPolyline(road, WColor(0, 191, 255));

  map->setCenter(road[road.size()-1]);

  map->openInfoWindow(road[0], 
  		      "<img src=\"http://emweb.be/img/emweb_small.jpg\" />"
  		      "<br/>"
  		      "<b>Emweb office</b>");

  map->clicked().connect(SLOT(this, SpecialPurposeWidgets::googleMapClicked));
  map->doubleClicked()
    .connect(SLOT(this, SpecialPurposeWidgets::googleMapDoubleClicked));

  return result;
}

void SpecialPurposeWidgets::
roadDescription(std::vector<WGoogleMap::Coordinate>& roadDescription) 
{ 
  roadDescription.push_back(WGoogleMap::Coordinate(50.85342000000001, 4.7281));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85377, 4.72573));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85393, 4.72496));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85393, 4.72496));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85372, 4.72482));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85304, 4.72421));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8519, 4.72297));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85154, 4.72251));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85154, 4.72251));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85153, 4.72205));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85153, 4.72205));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85752, 4.7186));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85847, 4.71798));
  roadDescription.push_back(WGoogleMap::Coordinate(50.859, 4.71753));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8593, 4.71709));
  roadDescription.push_back(WGoogleMap::Coordinate(50.85986999999999, 4.71589));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8606, 4.7147));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8611, 4.71327));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86125999999999, 4.71293));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86184000000001, 4.71217));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86219, 4.71202));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86346, 4.71178));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86406, 4.71146));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86478, 4.71126));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86623000000001, 4.71111));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86659999999999, 4.71101));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8668, 4.71072));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86709, 4.71018));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86739, 4.70941));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86751, 4.70921));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86869, 4.70843));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8691, 4.70798));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8691, 4.70798));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86936, 4.70763));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86936, 4.70763));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86874, 4.70469));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86858, 4.70365));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86845999999999, 4.70269));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86839, 4.70152));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86843, 4.70043));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86851000000001, 4.69987));
  roadDescription.push_back(WGoogleMap::Coordinate(50.86881999999999, 4.69869));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8689, 4.69827));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87006, 4.6941));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87006, 4.6941));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87045999999999, 4.69348));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87172, 4.69233));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87229000000001, 4.69167));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87229000000001, 4.69167));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8725, 4.69123));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8725, 4.69123));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87408, 4.69142));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87423, 4.69125));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87464, 4.69116));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87579999999999, 4.69061));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87595, 4.69061));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87733, 4.69073));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87742, 4.69078));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87784, 4.69131));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87784, 4.69131));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87759, 4.69267));
  roadDescription.push_back(WGoogleMap::Coordinate(50.8775, 4.6935));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87751, 4.69395));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87768, 4.69545));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87769, 4.69666));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87759, 4.69742));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87734, 4.69823));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87734, 4.69823));
  roadDescription.push_back(WGoogleMap::Coordinate(50.87790999999999, 4.69861));
}

void SpecialPurposeWidgets
::googleMapDoubleClicked(WGoogleMap::Coordinate c)
{
  std::ostringstream strm;
  strm << "Double clicked at coordinate (" 
       << c.latitude() 
       << "," 
       << c.longitude()
       << ")";

  ed_->setStatus(strm.str());
}

void SpecialPurposeWidgets
::googleMapClicked(WGoogleMap::Coordinate c)
{
  std::ostringstream strm;
  strm << "Clicked at coordinate (" 
       << c.latitude() 
       << "," 
       << c.longitude()
       << ")";

  ed_->setStatus(strm.str());
}

WWidget *SpecialPurposeWidgets::wSound()
{
  WContainerWidget *result = new WContainerWidget(); 
  topic("WSound", result);
  new WText(tr("specialpurposewidgets-WSound"), result);

  new WText("The beep will be repeated 3 times.", result);
  new WBreak(result);
  WSound *sound = new WSound("sounds/beep.mp3", result);
  sound->setLoops(3);
  WPushButton *playButton = new WPushButton("Beep!", result);
  playButton->setMargin(5);
  WPushButton *stopButton = new WPushButton("Make it stop!!!", result);
  stopButton->setMargin(5);
  playButton->clicked().connect(SLOT(sound, WSound::play));
  stopButton->clicked().connect(SLOT(sound, WSound::stop));

  ed_->mapConnect(playButton->clicked(), "Beeping started!");
  ed_->mapConnect(stopButton->clicked(), "Beeping stopped!");

  return result;
}


