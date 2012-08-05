/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WMediaPlayer>

using namespace Wt;

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  WApplication* app = new WApplication(env);

  app->messageResourceBundle().use(WApplication::appRoot() + "text");

  std::string ogvVideo =
    "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";
  std::string mp4Video =
    "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
  std::string mp3Audio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.mp3";

  std::string poster = "sintel_trailer.jpg";
  
  new WText(WString::tr("intro"), app->root());

  new WText(WString::tr("video"), app->root());

  WMediaPlayer *player = new WMediaPlayer(WMediaPlayer::Video, app->root());

  player->addSource(WMediaPlayer::M4V, mp4Video);
  player->addSource(WMediaPlayer::OGV, ogvVideo);
  player->addSource(WMediaPlayer::PosterImage, poster);
  player->setTitle("<a href=\"http://durian.blender.org/\""
		   "target=\"_blank\">Sintel</a>, "
		   "(c) copyright Blender Foundation");

  new WText(WString::tr("audio"), app->root());

  player = new WMediaPlayer(WMediaPlayer::Audio, app->root());

  player->addSource(WMediaPlayer::MP3, mp3Audio);
  player->setTitle("La Sera - Never Come Around");
  
  return app;
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}

