#include <cstring>
#include "Specificity.h"

using namespace Wt::Render;

Specificity::Specificity(bool valid)
  : value_(0),
    valid_(valid)
{ }

Specificity::Specificity(int a, int b, int c, int d)
  : value_(0),
    valid_(true)
{
  setA(a);
  setB(b); 
  setC(c);
  setD(d);
}

bool Specificity::operator==(const Specificity& other) const
{
  return valid_ == other.valid_ && value_ == other.value_;
}

void Specificity::setA(int a)
{
  value_ = (value_ & ~0xFF000000) | ((a & 0xFF) << 24);
}

void Specificity::setB(int b)
{
  value_ = (value_ & ~0x00FF0000) | ((b & 0xFF) << 16);
}

void Specificity::setC(int c)
{
  value_ = (value_ & ~0x0000FF00) | ((c & 0xFF) << 8);
}

void Specificity::setD(int d)
{
  value_ = (value_ & ~0x000000FF) | (d & 0xFF);
}

bool Specificity::isSmallerThen(const Specificity& other) const
{
  return !valid_ ? true : value_ < other.value_;
}

bool Specificity::isGreaterThen(const Specificity& other) const
{
  return !other.valid_ ? true : value_ > other.value_;
}

bool Specificity::isSmallerOrEqualThen(const Specificity& other) const
{
  return !isGreaterThen(other);
}

bool Specificity::isGreaterOrEqualThen(const Specificity& other) const
{
  return !isSmallerThen(other);
}

