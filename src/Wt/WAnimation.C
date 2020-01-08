/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAnimation.h"

namespace Wt {

WAnimation::WAnimation()
  : effects_(),
    timing_(TimingFunction::Linear),
    duration_(250)
{ }

WAnimation::WAnimation(WFlags<AnimationEffect> effects, TimingFunction timing,
		       int duration)
  : effects_(effects),
    timing_(timing),
    duration_(duration)
{ }

#ifdef WT_TARGET_JAVA

WAnimation::WAnimation(AnimationEffect effect, TimingFunction timing,
		       int duration)
  : effects_(effect),
    timing_(timing),
    duration_(duration)
{ }

WAnimation::WAnimation(AnimationEffect effect1, AnimationEffect effect2,
		       TimingFunction timing, int duration)
  : effects_(effect1 | effect2),
    timing_(timing),
    duration_(duration)
{ }

WAnimation WAnimation::clone() const
{
  WAnimation result;
  result.effects_ = effects_;
  result.duration_ = duration_;
  return result;
}
#endif // WT_TARGET_JAVA

bool WAnimation::operator==(const WAnimation& animation) const
{
  return animation.effects_ == effects_ &&
    animation.duration_ == duration_;
}

bool WAnimation::operator!=(const WAnimation& animation) const
{
  return !(*this == animation);
}

void WAnimation::setEffects(WFlags<AnimationEffect> effects)
{
  effects_ = effects;
}

void WAnimation::setDuration(int msecs)
{
  duration_ = msecs;
}

void WAnimation::setTimingFunction(TimingFunction tf)
{
  timing_ = tf;
}

bool WAnimation::empty() const
{
  return duration_ == 0 || !effects_;
}

}
