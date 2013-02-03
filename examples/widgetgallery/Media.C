/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "Media.h"
#include "TopicTemplate.h"

Media::Media() 
  : TopicWidget()
{
  addText(tr("specialpurposewidgets-intro"), this);
}

void Media::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("WMediaPlayer",
                deferCreate(boost::bind(&Media::mediaPlayer, this)))
    ->setPathComponent("");
  menu->addItem("WSound",
                deferCreate(boost::bind(&Media::sound, this)));
  menu->addItem("WAudio",
                deferCreate(boost::bind(&Media::audio, this)));
  menu->addItem("WVideo",
                deferCreate(boost::bind(&Media::video, this)));
  menu->addItem("WFlashObject",
                deferCreate(boost::bind(&Media::flashObject, this)));
  menu->addItem("Resources",
                deferCreate(boost::bind(&Media::resources, this)));
  menu->addItem("PDF output",
                deferCreate(boost::bind(&Media::pdf, this)));

}


#include "examples/MediaPlayerVideo.cpp"
#include "examples/MediaPlayerAudio.cpp"

Wt::WWidget *Media::mediaPlayer()
{
  Wt::WTemplate *result = new TopicTemplate("media-MediaPlayer");

  result->bindWidget("MediaPlayerVideo", MediaPlayerVideo());
  result->bindWidget("MediaPlayerAudio", MediaPlayerAudio());

  return result;
}


#include "examples/Sound.cpp"

Wt::WWidget *Media::sound()
{
  Wt::WTemplate *result = new TopicTemplate("media-Sound");

  result->bindWidget("Sound", Sound());

  return result;
}


#include "examples/Audio.cpp"

Wt::WWidget *Media::audio()
{
  Wt::WTemplate *result = new TopicTemplate("media-Audio");

  result->bindWidget("Audio", Audio());

  return result;
}


#include "examples/Video.cpp"
#include "examples/VideoFallback.cpp"

Wt::WWidget *Media::video()
{
  Wt::WTemplate *result = new TopicTemplate("media-Video");

  result->bindWidget("Video", Video());
  result->bindWidget("VideoFallback", VideoFallback());

  return result;
}


#include "examples/Flash.cpp"

Wt::WWidget *Media::flashObject()
{
  Wt::WTemplate *result = new TopicTemplate("media-FlashObject");

  result->bindWidget("Flash", Flash());

  return result;
}


#include "examples/ResourceCustom.cpp"
#include "examples/ResourceStatic.cpp"

Wt::WWidget *Media::resources()
{
  Wt::WTemplate *result = new TopicTemplate("media-Resources");

  result->bindWidget("ResourceCustom", ResourceCustom());
  result->bindWidget("ResourceStatic", ResourceStatic());

  return result;
}

#ifdef WT_HAS_WPDFIMAGE
#include "examples/PdfImage.cpp"
#ifdef WT_TARGET_JAVA
#include "examples/JavaPdfRenderer.cpp"
#else
#include "examples/PdfRenderer.cpp"
#endif
#endif

Wt::WWidget *Media::pdf()
{
  Wt::WTemplate *result = new TopicTemplate("media-PDF");

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
                     reindent(tr("media-PdfImageWrite")), Wt::PlainText);
  return result;
}
