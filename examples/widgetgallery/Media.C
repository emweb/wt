/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "Media.h"
#include "TopicTemplate.h"

#include <Wt/WMenu.h>

Media::Media() 
  : TopicWidget()
{
  addText(tr("specialpurposewidgets-intro"), this);
}

void Media::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("WMediaPlayer",
                deferCreate([this]{ return mediaPlayer(); }))
    ->setPathComponent("");
  menu->addItem("WSound",
                deferCreate([this]{ return sound(); }));
  menu->addItem("WAudio",
                deferCreate([this]{ return audio(); }));
  menu->addItem("WVideo",
                deferCreate([this]{ return video(); }));
  menu->addItem("WFlashObject",
                deferCreate([this]{ return flashObject(); }));
  menu->addItem("Resources",
                deferCreate([this]{ return resources(); }));
  menu->addItem("PDF output",
                deferCreate([this]{ return pdf(); }));

}


#include "examples/MediaPlayerVideo.cpp"
#include "examples/MediaPlayerAudio.cpp"

std::unique_ptr<Wt::WWidget> Media::mediaPlayer()
{
  auto result = std::make_unique<TopicTemplate>("media-MediaPlayer");

  result->bindWidget("MediaPlayerVideo", MediaPlayerVideo());
  result->bindWidget("MediaPlayerAudio", MediaPlayerAudio());

  return std::move(result);
}


#include "examples/Sound.cpp"

std::unique_ptr<Wt::WWidget> Media::sound()
{
  auto result = std::make_unique<TopicTemplate>("media-Sound");

  result->bindWidget("Sound", Sound());

  return std::move(result);
}


#include "examples/Audio.cpp"

std::unique_ptr<Wt::WWidget> Media::audio()
{
  auto result = std::make_unique<TopicTemplate>("media-Audio");

  result->bindWidget("Audio", Audio());

  return std::move(result);
}


#include "examples/Video.cpp"
#include "examples/VideoFallback.cpp"

std::unique_ptr<Wt::WWidget> Media::video()
{
  auto result = std::make_unique<TopicTemplate>("media-Video");

  result->bindWidget("Video", Video());
  result->bindWidget("VideoFallback", VideoFallback());

  return std::move(result);
}


#include "examples/Flash.cpp"

std::unique_ptr<Wt::WWidget> Media::flashObject()
{
  auto result = std::make_unique<TopicTemplate>("media-FlashObject");

  result->bindWidget("Flash", Flash());

  return std::move(result);
}


#include "examples/ResourceCustom.cpp"
#include "examples/ResourceStatic.cpp"

std::unique_ptr<Wt::WWidget> Media::resources()
{
  auto result = std::make_unique<TopicTemplate>("media-Resources");

  result->bindWidget("ResourceCustom", ResourceCustom());
  result->bindWidget("ResourceStatic", ResourceStatic());

  return std::move(result);
}

#ifdef WT_HAS_WPDFIMAGE
#include "examples/PdfImage.cpp"
#ifdef WT_TARGET_JAVA
#include "examples/JavaPdfRenderer.cpp"
#else
#include "examples/PdfRenderer.cpp"
#endif
#endif

std::unique_ptr<Wt::WWidget> Media::pdf()
{
  auto result = std::make_unique<TopicTemplate>("media-PDF");

#ifdef WT_HAS_WPDFIMAGE
  result->bindWidget("PdfImage", PdfImage());
  result->bindWidget("PdfRenderer", PdfRenderer());
#else
  result->bindString("PdfImage", "This example requires Wt built with PDF"
             " support.");
  result->bindString("PdfImage", "This example requires Wt built with PDF"
             " support.");
#endif

  // Show the source code only for write to file example.
  result->bindString("PdfImageWrite",
		     reindent(tr("media-PdfImageWrite")), Wt::TextFormat::Plain);
  return std::move(result);
}
