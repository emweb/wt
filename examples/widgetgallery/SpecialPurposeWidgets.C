/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SpecialPurposeWidgets.h"
#include "EventDisplayer.h"
#include "DeferredWidget.h"

#include "Wt/WAudio"
#include "Wt/WBreak"
#include "Wt/WCheckBox"
#include "Wt/WComboBox"
#include "Wt/WFlashObject"
#include "Wt/WHBoxLayout"
#include "Wt/WImage"
#include "Wt/WMediaPlayer"
#include "Wt/WPushButton"
#include "Wt/WSound"
#include "Wt/WStandardItemModel"
#include "Wt/WTable"
#include "Wt/WText"
#include "Wt/WTemplate"
#include "Wt/WVideo"

using namespace Wt;

namespace {
  std::string ogvVideo =
    "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";
  std::string mp4Video =
    "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
  std::string mp3Audio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.mp3";
  std::string oggAudio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.ogg";

  std::string poster = "pics/sintel_trailer.jpg";
}

SpecialPurposeWidgets::SpecialPurposeWidgets(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  addText(tr("specialpurposewidgets-intro"), this);
}

void SpecialPurposeWidgets::populateSubMenu(WMenu *menu)
{
  menu->addItem("WGoogleMap",
		deferCreate
		(boost::bind(&SpecialPurposeWidgets::wGoogleMap, this)));
  menu->addItem("WMediaPlayer",
		deferCreate(boost::bind
			    (&SpecialPurposeWidgets::wMediaPlayer, this)));
  menu->addItem("WSound",
		deferCreate(boost::bind(&SpecialPurposeWidgets::wSound, this)));
  menu->addItem("WVideo",
		deferCreate(boost::bind(&SpecialPurposeWidgets::wVideo, this)));
  menu->addItem("WAudio",
		deferCreate(boost::bind(&SpecialPurposeWidgets::wAudio, this)));
  menu->addItem("WFlashObject",
		deferCreate(boost::bind(&SpecialPurposeWidgets::wFlashObject,
					this)));
}

class GoogleMapExample : public WContainerWidget {
public:
  GoogleMapExample(WContainerWidget* parent, ControlsWidget *controlsWidget) 
    : WContainerWidget(parent),
      controlsWidget_(controlsWidget) {
    WHBoxLayout* layout = new WHBoxLayout();
    setLayout(layout);

    setHeight(400);

    map_ = new WGoogleMap(WGoogleMap::Version3);
    layout->addWidget(map_, 1);

    map_->setMapTypeControl(WGoogleMap::DefaultControl);
    map_->enableScrollWheelZoom();

    WTemplate *controls = 
      new WTemplate(tr("specialpurposewidgets-WGoogleMap-controls"));
    layout->addWidget(controls);

    WPushButton* zoomIn = new WPushButton("+");
    zoomIn->addStyleClass("zoom");
    controls->bindWidget("zoom-in", zoomIn);
    zoomIn->clicked().connect(map_, &WGoogleMap::zoomIn);
    WPushButton* zoomOut = new WPushButton("-");
    zoomOut->addStyleClass("zoom");
    controls->bindWidget("zoom-out", zoomOut);
    zoomOut->clicked().connect(map_, &WGoogleMap::zoomOut);

    WPushButton* brussels = new WPushButton("Brussels");
    controls->bindWidget("brussels", brussels);
    brussels->clicked().connect(this, &GoogleMapExample::panToBrussels);

    WPushButton* lisbon = new WPushButton("Lisbon");
    controls->bindWidget("lisbon", lisbon);
    lisbon->clicked().connect(this, &GoogleMapExample::panToLisbon);

    WPushButton* paris = new WPushButton("Paris");
    controls->bindWidget("paris", paris);
    paris->clicked().connect(this, &GoogleMapExample::panToParis);

    WPushButton* savePosition = new WPushButton("Save current position");
    controls->bindWidget("save-position", savePosition);
    savePosition->clicked().connect(this, &GoogleMapExample::savePosition); 

    returnToPosition_ = new WPushButton("Return to saved position");
    controls->bindWidget("return-to-saved-position", returnToPosition_);
    returnToPosition_->setEnabled(false);
    returnToPosition_->clicked().
      connect(map_, &WGoogleMap::returnToSavedPosition);
    
    mapTypeModel_ = new WStandardItemModel();
    addMapTypeControl("No control", WGoogleMap::NoControl);
    addMapTypeControl("Default", WGoogleMap::DefaultControl);
    addMapTypeControl("Menu", WGoogleMap::MenuControl);
    if (map_->apiVersion() == WGoogleMap::Version2)
      addMapTypeControl("Hierarchical", WGoogleMap::HierarchicalControl);
    if (map_->apiVersion() == WGoogleMap::Version3)
      addMapTypeControl("Horizontal bar", WGoogleMap::HorizontalBarControl);

    WComboBox* menuControls = new WComboBox();
    menuControls->setModel(mapTypeModel_);
    controls->bindWidget("control-menu-combo", menuControls);
    menuControls->activated().
      connect(this, &GoogleMapExample::setMapTypeControl);
    menuControls->setCurrentIndex(1);

    WCheckBox *draggingCB = new WCheckBox("Enable dragging ");
    controls->bindWidget("dragging-cb", draggingCB);
    draggingCB->setChecked(true);
    map_->enableDragging();
    draggingCB->checked().
      connect(map_, &WGoogleMap::enableDragging);
    draggingCB->unChecked().
      connect(map_, &WGoogleMap::disableDragging);

    WCheckBox *enableDoubleClickZoomCB = 
      new WCheckBox("Enable double click zoom ");
    controls->bindWidget("double-click-zoom-cb", enableDoubleClickZoomCB);
    enableDoubleClickZoomCB->setChecked(false);
    map_->disableDoubleClickZoom();
    enableDoubleClickZoomCB->checked().
      connect(map_, &WGoogleMap::enableDoubleClickZoom);
    enableDoubleClickZoomCB->unChecked().
      connect(map_, &WGoogleMap::disableDoubleClickZoom);
    
    WCheckBox *enableScrollWheelZoomCB = 
      new WCheckBox("Enable scroll wheel zoom ");
    controls->bindWidget("scroll-wheel-zoom-cb", enableScrollWheelZoomCB);
    enableScrollWheelZoomCB->setChecked(true);
    map_->enableScrollWheelZoom();
    enableScrollWheelZoomCB->checked().
      connect(map_, &WGoogleMap::enableScrollWheelZoom);
    enableScrollWheelZoomCB->unChecked().
      connect(map_, &WGoogleMap::disableScrollWheelZoom);

    std::vector<WGoogleMap::Coordinate> road;
    roadDescription(road);
    map_->addPolyline(road, WColor(0, 191, 255));
    
    //Koen's favourite bar!
    map_->addMarker(WGoogleMap::Coordinate(50.885069,4.71958));
    
    map_->setCenter(road[road.size()-1]);
    
    map_->openInfoWindow
      (road[0], 
       "<img src=\"http://www.emweb.be/css/emweb_small.jpg\" />"
       "<br/>"
       "<b>Emweb office</b>");
    
    map_->clicked()
      .connect(this, &GoogleMapExample::googleMapClicked);
    map_->doubleClicked()
      .connect(this, &GoogleMapExample::googleMapDoubleClicked);
  }

private:
  void panToBrussels() {
    map_->panTo(WGoogleMap::Coordinate(50.85034,4.35171));
  }

  void panToLisbon() {
    map_->panTo(WGoogleMap::Coordinate(38.703731,-9.135475));
  }

  void panToParis() {
    map_->panTo(WGoogleMap::Coordinate(48.877474, 2.312579));
  }

  void savePosition() {
    returnToPosition_->setEnabled(true);
    map_->savePosition();
  }

  void addMapTypeControl(const WString &description, 
			 WGoogleMap::MapTypeControl value) {
    int r = mapTypeModel_->rowCount();
    mapTypeModel_->insertRow(r);
    mapTypeModel_->setData(r, 0, description);
    mapTypeModel_->setData(r, 0, value, UserRole);
  }

  void setMapTypeControl(int row) {
    boost::any mtc = mapTypeModel_->data(row, 0, UserRole);
    map_->setMapTypeControl(boost::any_cast<WGoogleMap::MapTypeControl>(mtc));
  }

  void roadDescription(std::vector<WGoogleMap::Coordinate>& roadDescription) { 
    roadDescription.push_back(WGoogleMap::Coordinate(50.85342, 4.7281));
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
    roadDescription.push_back(WGoogleMap::Coordinate(50.8598699, 4.71589));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8606, 4.7147));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8611, 4.71327));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8612599, 4.71293));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86184, 4.71217));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86219, 4.71202));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86346, 4.71178));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86406, 4.71146));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86478, 4.71126));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86623, 4.71111));
    roadDescription.push_back(WGoogleMap::Coordinate(50.866599, 4.71101));
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
    roadDescription.push_back(WGoogleMap::Coordinate(50.8684599, 4.70269));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86839, 4.70152));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86843, 4.70043));
    roadDescription.push_back(WGoogleMap::Coordinate(50.86851, 4.69987));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8688199, 4.69869));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8689, 4.69827));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87006, 4.6941));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87006, 4.6941));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8704599, 4.69348));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87172, 4.69233));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87229, 4.69167));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87229, 4.69167));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8725, 4.69123));
    roadDescription.push_back(WGoogleMap::Coordinate(50.8725, 4.69123));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87408, 4.69142));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87423, 4.69125));
    roadDescription.push_back(WGoogleMap::Coordinate(50.87464, 4.69116));
    roadDescription.push_back(WGoogleMap::Coordinate(50.875799, 4.69061));
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
    roadDescription.push_back(WGoogleMap::Coordinate(50.877909, 4.69861));
  }

  void googleMapDoubleClicked(WGoogleMap::Coordinate c) {
    std::ostringstream strm;
    strm << "Double clicked at coordinate (" 
	 << c.latitude() 
	 << "," 
	 << c.longitude()
	 << ")";
    
    controlsWidget_->eventDisplayer()->setStatus(strm.str());
  }

  void googleMapClicked(WGoogleMap::Coordinate c) {
    std::ostringstream strm;
    strm << "Clicked at coordinate (" 
	 << c.latitude() 
	 << "," 
	 << c.longitude()
	 << ")";
    
    controlsWidget_->eventDisplayer()->setStatus(strm.str());
  }

private:
  WGoogleMap *map_;
  WAbstractItemModel *mapTypeModel_;

  WPushButton *returnToPosition_;

  ControlsWidget *controlsWidget_; 
};

WWidget *SpecialPurposeWidgets::wGoogleMap()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WGoogleMap", result);
  addText(tr("specialpurposewidgets-WGoogleMap"), result);
  
  new GoogleMapExample(result, this);

  return result;
}

WWidget *SpecialPurposeWidgets::wMediaPlayer()
{
  WContainerWidget *result = new WContainerWidget();
  result->setStyleClass("wmediaplayer");
  topic("WMediaPlayer", result);

  addText(tr("specialpurposewidgets-WMediaPlayer"), result);

  addText(tr("specialpurposewidgets-WMediaPlayer-video"), result);

  WMediaPlayer *player = new WMediaPlayer(WMediaPlayer::Video, result);

  player->addSource(WMediaPlayer::M4V, WLink(mp4Video));
  player->addSource(WMediaPlayer::OGV, WLink(ogvVideo));
  player->addSource(WMediaPlayer::PosterImage, WLink(poster));
  player->setTitle("<a href=\"http://durian.blender.org/\""
		   "target=\"_blank\">Sintel</a>, "
		   "(c) copyright Blender Foundation");

  ed_->showEvent(player->playbackStarted(), "Video playing");
  ed_->showEvent(player->playbackPaused(), "Video paused");
  ed_->showEvent(player->ended(), "Video ended");
  ed_->showEvent(player->volumeChanged(), "Volume changed");

  addText(tr("specialpurposewidgets-WMediaPlayer-audio"), result);

  player = new WMediaPlayer(WMediaPlayer::Audio, result);

  player->addSource(WMediaPlayer::MP3, WLink(mp3Audio));
  player->addSource(WMediaPlayer::OGA, WLink(oggAudio));
  player->setTitle("La Sera - Never Come Around");

  ed_->showEvent(player->playbackStarted(), "Song playing");
  ed_->showEvent(player->playbackPaused(), "Song paused");
  ed_->showEvent(player->ended(), "Song ended");
  ed_->showEvent(player->volumeChanged(), "Volume changed");

  return result;
}

WWidget *SpecialPurposeWidgets::wSound()
{
  WContainerWidget *result = new WContainerWidget(); 
  topic("WSound", result);
  addText(tr("specialpurposewidgets-WSound"), result);

  addText("The beep will be repeated 3 times.", result);
  new WBreak(result);
  WSound *const sound = new WSound("sounds/beep.mp3", result);
  sound->setLoops(3);
  WPushButton *playButton = new WPushButton("Beep!", result);
  playButton->setMargin(5);
  WPushButton *stopButton = new WPushButton("Make it stop!!!", result);
  stopButton->setMargin(5);
  playButton->clicked().connect(sound, &WSound::play);
  stopButton->clicked().connect(sound, &WSound::stop);

  ed_->showSignal(playButton->clicked(), "Beeping started!");
  ed_->showSignal(stopButton->clicked(), "Beeping stopped!");

  return result;
}

WWidget *SpecialPurposeWidgets::wAudio()
{
  WContainerWidget *result = new WContainerWidget(); 
  topic("WAudio", result);

  addText(tr("specialpurposewidgets-WAudio"), result);

  WAudio *a1 = new WAudio(result);
  a1->addSource(WLink(mp3Audio));
  a1->addSource(WLink(oggAudio));
  a1->setOptions(WAudio::Controls);

  ed_->showEvent(a1->playbackStarted(), "Audio playing");
  ed_->showEvent(a1->playbackPaused(), "Audio paused");
  ed_->showEvent(a1->ended(), "Audio ended");
  ed_->showEvent(a1->volumeChanged(), "Audio volume changed");

  return result;
}

WWidget *SpecialPurposeWidgets::wVideo()
{
  WContainerWidget *result = new WContainerWidget(); 
  topic("WVideo", result);
  addText(tr("specialpurposewidgets-WVideo"), result);

  new WBreak(result);

  addText(tr("specialpurposewidgets-WVideo-1"), result);
  WVideo *v1 = new WVideo(result);
  v1->addSource(WLink(mp4Video));
  v1->addSource(WLink(ogvVideo));
  v1->setPoster(poster);
  v1->setAlternativeContent(new WImage(poster));
  v1->resize(640, 360);

  ed_->showEvent(v1->playbackStarted(), "Video 1 playing");
  ed_->showEvent(v1->playbackPaused(), "Video 1 paused");
  ed_->showEvent(v1->ended(), "Video 1 ended");
  ed_->showEvent(v1->volumeChanged(), "Video 1 volume changed");

  addText(tr("specialpurposewidgets-WVideo-2"), result);
  WFlashObject *flash2 =
    new WFlashObject("http://www.webtoolkit.eu/videos/player_flv_maxi.swf");
  flash2->setFlashVariable("startimage", "pics/sintel_trailer.jpg");
  flash2->setFlashParameter("allowFullScreen", "true");
  flash2->setFlashVariable("flv", mp4Video);
  flash2->setFlashVariable("showvolume", "1");
  flash2->setFlashVariable("showfullscreen", "1");
  flash2->setAlternativeContent(new WImage(poster));
  flash2->resize(640, 360);
  WVideo *v2 = new WVideo(result);
  v2->addSource(WLink(mp4Video));
  v2->addSource(WLink(ogvVideo));
  v2->setAlternativeContent(flash2);
  v2->setPoster(poster);
  v2->resize(640, 360);
  ed_->showEvent(v2->playbackStarted(), "Video 2 playing");
  ed_->showEvent(v2->playbackPaused(), "Video 2 paused");
  ed_->showEvent(v2->ended(), "Video 2 ended");
  ed_->showEvent(v2->volumeChanged(), "Video 2 volume changed");

  addText(tr("specialpurposewidgets-WVideo-3"), result);
  WFlashObject *flash3 =
    new WFlashObject("http://www.youtube.com/v/HOfdboHvshg", result);
  flash3->setFlashParameter("allowFullScreen", "true");
  flash3->resize(640, 360);

  return result;
}

WWidget *SpecialPurposeWidgets::wFlashObject()
{
  WContainerWidget *result = new WContainerWidget(); 
  topic("WFlashObject", result);
  addText(tr("specialpurposewidgets-WFlashObject"), result);
  WFlashObject *f =
    new WFlashObject("http://www.youtube.com/v/HOfdboHvshg", result);
  f->resize(640, 385);
  return result;
}

