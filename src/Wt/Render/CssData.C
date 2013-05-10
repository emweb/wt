#include "CssData.h"


#include <boost/bind.hpp>
#include <map>
#include "Wt/Render/Block.h"
#include "Wt/Render/CssData_p.h"
#include <web/WebUtils.h>

using namespace Wt::Render;

void Term::setUnit(Unit u)
{
  unit_ = u;
  if(u <= Ex)
    type_ = Font;
  else if(u <= Pc)
    type_ = Length;
  else if(u <= Grad)
    type_ = Angle;
  else if(u <= Seconds)
    type_ = Time;
  else if(u <= Khz)
    type_ = Frequency;
  else if(u <= Percentage)
    type_ = OtherNumber;
  else
    type_ = Invalid;
}

void Term::setQuotedString(const std::string& s)
{
  quotedString_ = s;
  type_ = QuotedString;
}

void Term::setIdentifier(const std::string& id)
{
  identifier_ = id;
  type_ = Identifier;
}

void Term::setHash (const std::string& hash)
{
  hash_ = hash;
  type_ = Hash;
}

void Term::setUri(const std::string& uri)
{
  uri_ = uri;
  type_ = Uri;
}


///////////////////////////////////////////////////////////////////////////////
///// isMatch                                                             /////
///////////////////////////////////////////////////////////////////////////////



bool Wt::Render::Match::isMatch(const Block* block, const SimpleSelector& s)
{
  const DomElementType tag = block->type();
  const std::string id = block->id();
  const std::vector<std::string> classes = block->classes();
  const std::vector<std::string> requiredClasses = s.classes();

  //cout << "isMatch? block:" << tag << " " << id
  //               << "; s: " << s.elementName() << " " << s.hashId();

  // Match tagname?
  if(s.elementName().size()
     && s.elementName() != "*"
     && tag != Wt::DomElement::parseTagName(s.elementName()))
  {
    //cout << "; NO! tag" << endl;
    return false;
  }

  // Match Id?
  if(s.hashId().size() && id != s.hashId())
  {
    //cout << "; NO! id" << endl;
    return false;
  }

  // Match all classes?
  for(unsigned int i = 0; i < requiredClasses.size(); ++i)
  {
    if(Wt::Utils::indexOf(classes, requiredClasses[i]) == -1)
    {
      //cout << "; NO! classes" << r << endl;
      return false;
    }
  }

  // Done!
  //cout << "; YES!" << endl;
  return true;
}

Specificity Wt::Render::Match::isMatch(const Block* block, const Selector& selector)
{
  if(!selector.size())
    return Specificity(false);

  if(!isMatch(block, selector.at(selector.size()-1)))
    return Specificity(false);
  const Block* parent = block->parent();
  for(int i = selector.size()-2; i >= 0; --i)
  {
    while(parent)
    {
      if(isMatch(parent, selector.at(i)))
        break;
      else
        parent = parent->parent();
    }

    if(!parent)
      return Specificity(false);
  }
  return selector.specificity();
}


