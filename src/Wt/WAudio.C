// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAudio.h"
#include "DomElement.h"

using namespace Wt;

WAudio::WAudio()
{ }

DomElement *WAudio::createMediaDomElement()
{
  return DomElement::createNew(DomElementType::AUDIO);
}

std::string WAudio::jsAudioRef() const
{
  return jsMediaRef();
}

DomElementType WAudio::domElementType() const
{
  return DomElementType::AUDIO;
}
