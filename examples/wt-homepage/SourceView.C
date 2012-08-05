#include "SourceView.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <stdlib.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <Wt/WApplication>
#include <Wt/WText>
#include <Wt/WImage>

using namespace Wt;
namespace fs = boost::filesystem;

SourceView::SourceView(int fileNameRole, int contentRole, int filePathRole)
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
    std::string fp = index.data(filePathRole_).empty() ? std::string()
      : boost::any_cast<std::string>(index.data(filePathRole_));

    if (!index.data(contentRole_).empty()
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
#ifndef WIN32
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

WWidget * SourceView::renderView() 
{
  if (!index_.isValid()) {
    // no content
    WText *result = new WText();
    result->setInline(false);
    return result;
  }

  /*
   * read the contents, from string or file name
   */
  boost::any contentsData = index_.data(contentRole_);
  std::string content;
  if (!contentsData.empty())
   content = boost::any_cast<const std::string&>(contentsData);
  boost::any fileNameData = index_.data(fileNameRole_);
  std::string fileName = 
    boost::any_cast<const std::string&>(fileNameData);
  boost::any filePathData = index_.data(filePathRole_);
  std::string filePath;
  if (!filePathData.empty())
    filePath = boost::any_cast<const std::string&>(filePathData);

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

    if (!filePathData.empty())
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

    if (filePathData.empty())
      unlink(inputFileName.c_str());
  } 

  if (content == "")
    // do not load binary files, we would need to perform proper UTF-8
    // transcoding to display them
    if (!boost::iends_with(fileName, ".jar")
	&& !boost::iends_with(fileName, ".war")
	&& !boost::iends_with(fileName, ".class"))
      content = readFileToString(fileName);

  delete imageResource_;
  imageResource_ = 0;

  WWidget *result = 0;

  if (!imageExtension(fileName).empty()) {
    WImage *image = new WImage();
    imageResource_ = new WMemoryResource(this);
    imageResource_->setMimeType("mime/" + imageExtension(fileName));
    imageResource_->setData((const unsigned char*)content.data(),
			    (int)content.length());
    image->setImageLink(imageResource_);
    result = image;
  } else if (lang != "") {
    WText *text = new WText();
    text->setTextFormat(XHTMLUnsafeText);
    text->setText(WString::fromUTF8(content));
    result = text;
  } else {
    WText *text = new WText();
    text->setTextFormat(PlainText);
    text->setText(WString::fromUTF8(content));
    result = text;
  }

  result->setInline(false);
  WApplication::instance()
    ->doJavaScript(result->jsRef() + ".parentNode.scrollTop = 0;");
  return result;
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
