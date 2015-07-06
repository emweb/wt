#include <cstring>
#include "Specificity.h"

using namespace Wt::Render;

Specificity::Specificity(bool valid)
{ std::memset(value_, 0, SPECIFICITY_LEN); setValid(valid); }

Specificity::Specificity(int a, int b, int c, int d)
{
  setA(a); setB(b); setC(c); setD(d); setValid(true);
}

void Specificity::setValid(bool b){value_[0] = b ? 1 : 0;}
void Specificity::setA    (int  a){value_[1] = a;}
void Specificity::setB    (int  b){value_[2] = b;}
void Specificity::setC    (int  c){value_[3] = c;}
void Specificity::setD    (int  d){value_[4] = d;}

bool Specificity::isValid() const { return value_[0] == 1; }

bool Specificity::isSmallerThen(const Specificity& other) const
{
  return std::memcmp(value_, other.value_, SPECIFICITY_LEN) < 0;
}

bool Specificity::isGreaterThen(const Specificity& other) const
{
  return std::memcmp(value_, other.value_, SPECIFICITY_LEN) > 0;
}

bool Specificity::isSmallerOrEqualThen(const Specificity& other) const
{
  return !(isGreaterThen(other));
}

bool Specificity::isGreaterOrEqualThen(const Specificity& other) const
{
  return !(isSmallerThen(other));
}

