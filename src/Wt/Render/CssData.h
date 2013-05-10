#ifndef RENDER_CSSDATA_H_
#define RENDER_CSSDATA_H_

#include <Wt/WDllDefs.h>
#include <Wt/WString>
#include "Wt/Render/Specificity.h"

namespace Wt{
namespace Render{

class Block;

class SimpleSelector
{
public:
  virtual ~SimpleSelector(){}
  virtual std::string elementName() const = 0;
  virtual std::string hashId()      const = 0;
  virtual std::vector<std::string> classes() const = 0;
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
  enum Type {
    Font, Length, Angle, Time, Frequency, OtherNumber,
    QuotedString, Identifier, Hash, Uri, Invalid };
  enum Unit {
    Em, Ex,                  // Font
    Px, Cm, Mm, In, Pt, Pc,  // Length
    Deg, Rad, Grad,          // Angle
    Ms, Seconds,             // Time
    Hz, Khz,                 // Frequency
    Percentage,              // OtherNumber
    InvalidUnit
  };

  Term() : unit_(InvalidUnit){}
  void setUnit        (Unit u);
  void setQuotedString(const std::string& s);
  void setIdentifier  (const std::string& id);
  void setHash        (const std::string& hash);
  void setUri         (const std::string& uri);

  Type type() const;

  double value_;
  std::string quotedString_;
  std::string identifier_;
  std::string hash_;
  std::string uri_;
  Unit unit_;
  Type type_;
};

class DeclarationBlock
{
public:
  virtual ~DeclarationBlock(){}
  virtual Term value(const std::string& property) const = 0;
  virtual std::string declarationString() const = 0;
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

class Match
{
public:
  static bool        isMatch(const Block* block, const SimpleSelector& s );
  static Specificity isMatch(const Block* block, const Selector&       s );
};


}
}





#endif // CSSDATA_H_
