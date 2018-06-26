#ifndef RENDER_CSSDATA_H_
#define RENDER_CSSDATA_H_

#include <Wt/WDllDefs.h>
#include <Wt/WString.h>
#include <Wt/WWebWidget.h>
#include "Wt/Render/Specificity.h"


namespace Wt{
namespace Render{

class Block;

class SimpleSelector
{
public:
  virtual ~SimpleSelector(){}
  virtual const std::string& elementName() const = 0;
  virtual DomElementType elementType() const = 0;
  virtual const std::string& hashId()      const = 0;
  virtual const std::vector<std::string>& classes() const = 0;
};

class Selector
{
public:
  virtual ~Selector(){}
  virtual unsigned int          size       ()      const = 0;
  virtual const SimpleSelector& at         (int i) const = 0;
  virtual Specificity           specificity()      const = 0;
};

class Term
{
public:
  Term() { }
  void setValue(const std::string& s);
  std::string value_;
};

class DeclarationBlock
{
public:
  virtual ~DeclarationBlock(){}
  virtual Term value(const std::string& property) const = 0;
  virtual const std::string& declarationString() const = 0;
};

class Ruleset
{
public:
  virtual ~Ruleset(){}
  virtual const Selector&         selector        () const = 0;
  virtual const DeclarationBlock& declarationBlock() const = 0;
};

class StyleSheet
{
public:
  virtual ~StyleSheet(){}
  virtual unsigned int   rulesetSize()    const = 0;
  virtual const Ruleset& rulesetAt(int i) const = 0;
};

// exported for test.exe
class WT_API Match
{
public:
  static bool        isMatch(const Block* block, const SimpleSelector& s );
  static Specificity isMatch(const Block* block, const Selector&       s );
};


}
}





#endif // CSSDATA_H_
