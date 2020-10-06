/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WMediaPlayer.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> app
      = std::make_unique<WApplication>(env);

  app->messageResourceBundle().use(WApplication::appRoot() + "text");

  std::string ogvVideo =
    "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";
  std::string mp4Video =
    "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
  std::string mp3Audio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.mp3";

  std::string poster = "sintel_trailer.jpg";
  
  app->root()->addWidget(std::make_unique<WText>(WString::tr("intro")));

  app->root()->addWidget(std::make_unique<WText>(WString::tr("video")));

  WMediaPlayer *player = app->root()->addWidget(
        std::make_unique<WMediaPlayer>(MediaType::Video));

  player->addSource(MediaEncoding::M4V, mp4Video);
  player->addSource(MediaEncoding::OGV, ogvVideo);
  player->addSource(MediaEncoding::PosterImage, poster);
  player->setTitle("<a href=\"http://durian.blender.org/\""
		   "target=\"_blank\">Sintel</a>, "
		   "(c) copyright Blender Foundation");

  app->root()->addWidget(std::make_unique<WText>(WString::tr("audio")));

  player = app->root()->addWidget(std::make_unique<WMediaPlayer>(MediaType::Audio));

  player->addSource(MediaEncoding::MP3, mp3Audio);
  player->setTitle("La Sera - Never Come Around");
  
  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

