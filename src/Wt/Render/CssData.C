#include "CssData.h"


#include <map>
#include "Wt/Render/Block.h"
#include "Wt/Render/CssData_p.h"
#include "web/WebUtils.h"

using namespace Wt::Render;

void Wt::Render::Term::setValue(const std::string& value)
{
  value_ = value;
}

///////////////////////////////////////////////////////////////////////////////
///// isMatch                                                             /////
///////////////////////////////////////////////////////////////////////////////

bool Wt::Render::Match::isMatch(const Block* block, const SimpleSelector& s)
{
  const DomElementType tag = block->type();
  const std::string id = block->id();
  const std::vector<std::string>& classes = block->classes();
  const std::vector<std::string>& requiredClasses = s.classes();

  //cout << "isMatch? block:" << tag << " " << id
  //               << "; s: " << s.elementName() << " " << s.hashId();

  // Match tagname?
  if (!s.elementName().empty() &&
      s.elementName() != "*" &&
      tag != s.elementType())
  {
    //cout << "; NO! tag" << endl;
    return false;
  }

  // Match Id?
  if (s.hashId().size() && id != s.hashId())
  {
    //cout << "; NO! id" << endl;
    return false;
  }

  // Match all classes?
  for (unsigned int i = 0; i < requiredClasses.size(); ++i)
  {
    if (Wt::Utils::indexOf(classes, requiredClasses[i]) == -1)
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
    bool matchFound = false;
    while(parent)
    {
      matchFound = isMatch(parent, selector.at(i));
      parent = parent->parent();
      if(matchFound)
        break;
    }

    if(!matchFound && !parent)
      return Specificity(false);
  }
  return selector.specificity();
}


