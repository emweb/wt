#include "SourceView.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <stdlib.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WImage.h>

using namespace Wt;
namespace fs = boost::filesystem;

SourceView::SourceView(ItemDataRole fileNameRole,
                       ItemDataRole contentRole,
                       ItemDataRole filePathRole)
    : fileNameRole_(fileNameRole),
      contentRole_(contentRole),
      filePathRole_(filePathRole),
      imageResource_(0)
{}

SourceView::~SourceView() 
{ }

bool SourceView::setIndex(const WModelIndex& index) 
{
  if (index != index_ && index.isValid()) {
    std::string fp = !cpp17::any_has_value(index.data(filePathRole_)) ? std::string()
      : asString(index.data(filePathRole_)).toUTF8();

    if (cpp17::any_has_value(index.data(contentRole_))
	|| (!fp.empty() && !fs::is_directory(fp))) {
      index_ = index;
      update();

      return true;
    }
  }

  return false;
}

std::string tempFileName() 
{
#ifndef WT_WIN32
  char spool[20];
  strcpy(spool, "/tmp/wtXXXXXX");

  int i = mkstemp(spool);
  close(i);
#else
  char spool[2 * L_tmpnam];
  tmpnam(spool);
#endif
  return std::string(spool);
}

std::string getLanguageFromFileExtension(const std::string &fileName)
{
  if (boost::iends_with(fileName, ".h")
      || boost::iends_with(fileName, ".C")
      || boost::iends_with(fileName, ".cpp"))
    return "cpp";
  else if (boost::iends_with(fileName, ".xml"))
    return "xml";
  else if (boost::iends_with(fileName, ".html"))
    return "html";
  else if (boost::iends_with(fileName, ".java")) 
    return "java";
  else if (boost::iends_with(fileName, ".js")) 
    return "javascript";
  else if (boost::iends_with(fileName, ".css")) 
    return "css";
  else
    return std::string();
} 

std::string readFileToString(const std::string& fileName) 
{
  std::size_t outputFileSize = (std::size_t)fs::file_size(fileName);
  std::fstream file (fileName.c_str(), std::ios::in | std::ios::binary);
  char* memblock = new char [outputFileSize];
  file.read(memblock, (std::streamsize)outputFileSize);
  file.close();
  std::string data = std::string(memblock, outputFileSize);
  delete [] memblock;
  return data;
}

std::unique_ptr<WWidget> SourceView::renderView()
{
  if (!index_.isValid()) {
    // no content
    auto result = std::make_unique<WText>();
    result->setInline(false);
    return std::move(result);
  }

  /*
   * read the contents, from string or file name
   */
  cpp17::any contentsData = index_.data(contentRole_);
  std::string content;
  if (cpp17::any_has_value(contentsData))
   content = asString(contentsData).toUTF8();
  cpp17::any fileNameData = index_.data(fileNameRole_);
  std::string fileName = 
    asString(fileNameData).toUTF8();
  cpp17::any filePathData = index_.data(filePathRole_);
  std::string filePath;
  if (cpp17::any_has_value(filePathData))
    filePath = asString(filePathData).toUTF8();

  /*
   * determine source language, for source highlight
   */
  std::string lang = getLanguageFromFileExtension(fileName);
  if (content != "" && content.substr(0, 100).find("-*- C++ -*-")
      != std::string::npos)
    lang = "cpp";

  std::string outputFileName;

  if (lang != "") {
    std::string inputFileName;

    if (cpp17::any_has_value(filePathData))
      inputFileName = filePath;
    else {
      inputFileName = tempFileName();
      std::ofstream out(inputFileName.c_str(), 
			std::ios::out | std::ios::binary);
      out.write(content.c_str(), (std::streamsize)content.length());
      out.close();
    }
    
    outputFileName = tempFileName();

    std::string sourceHighlightCommand = "source-highlight ";
    sourceHighlightCommand += "--src-lang=" + lang + " ";
    sourceHighlightCommand += "--out-format=xhtml ";
    sourceHighlightCommand += "--input=" + inputFileName + " ";
    sourceHighlightCommand += "--output=" + outputFileName + " ";

    std::cerr << sourceHighlightCommand << std::endl;
    bool sourceHighlightOk = system(sourceHighlightCommand.c_str()) == 0;

    if (sourceHighlightOk)
      content = readFileToString(outputFileName);
    else {
      content = readFileToString(inputFileName);
      lang = "";
    }
    unlink(outputFileName.c_str());

    if (!cpp17::any_has_value(filePathData))
      unlink(inputFileName.c_str());
  } 

  if (content == "")
    // do not load binary files, we would need to perform proper UTF-8
    // transcoding to display them
    if (!boost::iends_with(fileName, ".jar")
	&& !boost::iends_with(fileName, ".war")
	&& !boost::iends_with(fileName, ".class"))
      content = readFileToString(fileName);

  std::unique_ptr<WWidget> result;

  if (!imageExtension(fileName).empty()) {
    std::unique_ptr<WImage> image(std::make_unique<WImage>());
    imageResource_ = std::make_shared<WMemoryResource>();
    imageResource_->setMimeType("mime/" + imageExtension(fileName));
    imageResource_->setData((const unsigned char*)content.data(),
			    (int)content.length());
    image->setImageLink(WLink(imageResource_));
    result = std::move(image);
  } else if (lang != "") {
    auto text = std::make_unique<WText>();
    text->setTextFormat(TextFormat::UnsafeXHTML);
    text->setText(content);
    result = std::move(text);
  } else {
    auto text = std::make_unique<WText>();
    text->setTextFormat(TextFormat::Plain);
    text->setText(content);
    result = std::move(text);
  }

  result->setInline(false);
  WApplication::instance()
    ->doJavaScript(result->jsRef() + ".parentNode.scrollTop = 0;");
  return std::move(result);
}

std::string SourceView::imageExtension(const std::string& fileName)
{
  static const char *imageExtensions[] = {
    ".png", ".gif", ".jpg", "jpeg", ".ico", 0
  };

  fs::path p(fileName);
  std::string extension = fs::extension(p);

  for (const char **s = imageExtensions; *s != 0; ++s)
    if (*s == extension)
      return extension.substr(1);

  return std::string();
}
