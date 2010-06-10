// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WHTML5Audio"
#include "DomElement.h"

using namespace Wt;

WHTML5Audio::WHTML5Audio(WContainerWidget *parent):
  WHTML5Media(parent)
{
}

DomElement *WHTML5Audio::createMediaDomElement()
{
  return DomElement::createNew(DomElement_AUDIO);
}

std::string WHTML5Audio::jsAudioRef() const
{
  return jsMediaRef();
}

DomElementType WHTML5Audio::domElementType() const
{
  return DomElement_AUDIO;
}
