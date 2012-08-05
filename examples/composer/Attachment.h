// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ATTACHMENT_H_
#define ATTACHMENT_H_

/**
 * @addtogroup composerexample
 */
/*@{*/

/*! \brief An email attachment.
 *
 * This widget is part of the %Wt composer example.
 */
class Attachment
{
public:
  /*! \brief The file name.
   */
  std::wstring fileName;

  /*! \brief The content description.
   */
  std::wstring contentDescription;

  /*! \brief the spooled file name.
   */
  std::string spoolFileName;

  /*! \brief Create an attachment.
   */
  Attachment(const std::wstring aFileName,
	     const std::wstring aContentDescription,
	     const std::string aSpoolFileName)
    : fileName(aFileName),
      contentDescription(aContentDescription),
      spoolFileName(aSpoolFileName)
  { }
};

/*@}*/

#endif // ATTACHMENT_H_
