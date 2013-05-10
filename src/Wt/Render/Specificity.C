#include "Specificity.h"

using namespace Wt::Render;

Specificity::Specificity(bool valid)
  : value_("     ")
{setA(0); setB(0); setC(0); setD(0);setValid(valid);}

Specificity::Specificity(int a, int b, int c, int d)
  : value_("     ")
{setA(a); setB(b); setC(c); setD(d);setValid(true);}

void Specificity::setValid(bool b){value_[0] = b ? (char)1 : (char)0;}
void Specificity::setA    (int  a){value_[1] = (char)(a % 256);}
void Specificity::setB    (int  b){value_[2] = (char)(b % 256);}
void Specificity::setC    (int  c){value_[3] = (char)(c % 256);}
void Specificity::setD    (int  d){value_[4] = (char)(d % 256);}

bool Specificity::isValid() const { return value_[0] == (char)1; }

bool Specificity::isSmallerThen(const Specificity& other) const
{
  return value_ < other.value_;
}

bool Specificity::isGreaterThen(const Specificity& other) const
{
  return value_ > other.value_;
}

bool Specificity::isSmallerOrEqualThen(const Specificity& other) const
{
  return !(isGreaterThen(other));
}

bool Specificity::isGreaterOrEqualThen(const Specificity& other) const
{
  return !(isSmallerThen(other));
}

