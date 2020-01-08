// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WANIMATION_H_
#define WANIMATION_H_

#include <Wt/WGlobal.h>

namespace Wt {

/*! \brief An enumeration describing an animation effect
 *
 * An animation effect can be the combination of a motion and an optional
 * fade effect, e.g:
 * \code
 *  AnimationEffect::SlideInFromRight
 *  AnimationEffect::SlideInFromTop | AnimationEffect::Fade
 * \endcode
 *
 * You can specify only one motion effect.
 *
 * \sa setEffects()
 */
enum class AnimationEffect {
  SlideInFromLeft = 0x1,   //!< Slides right to show, left to hide
  SlideInFromRight = 0x2,  //!< Slides left to show, right to hide
  SlideInFromBottom = 0x3, //!< Slides up to show, down to hide
  SlideInFromTop = 0x4,    //!< Slides down to show, up to hide
  Pop = 0x5,               //!< Pops up to show, pops away to hide
  Fade = 0x100             //!< Fade effect
};

/*! \brief A timing function
 *
 * The timing function defines how the animation effects are animated
 * during the total duration of the animation.
 */
enum class TimingFunction {
  Ease,        //!< Slow start and slow finish
  Linear,      //!< Linear throughout
  EaseIn,      //!< Slow start
  EaseOut,     //!< Slow finish
  EaseInOut,   //!< Slow start and even slower finish
  CubicBezier  //!< (Currently unsupported)
};

/*! \class WAnimation Wt/WAnimation.h Wt/WAnimation.h
 *  \brief A value class that defines a transition effect.
 *
 * This class defines an animation used as a transition to show or
 * hide a widget.
 *
 * The animation can be defined as a motion effect (e.g. sliding in or
 * out), optionally combined with a fade effect. A timing function
 * defines how the effects(s) are animated during the total duration of
 * the animation.
 *
 * \sa WWidget::animateShow(), WWidget::animateHide(), WWidget::setHidden()
 */
class WT_API WAnimation
{
public:
  /*! \brief Typedef for enum Wt::AnimationEffect */
  typedef AnimationEffect Effect;

  /*! \brief Default constructor.
   *
   * Creates an animation that actually represent <i>no</i> animation.
   * (effects() == 0).
   */
  WAnimation();

  /*! \brief Creates an animation.
   *
   * An animation is created with given effects, timing and duration.
   */
  WAnimation(WFlags<AnimationEffect> effects,
	     TimingFunction timing = TimingFunction::Linear,
	     int duration = 250);

#ifdef WT_TARGET_JAVA
  /*! \brief Creates an animation.
   *
   * An animation is created with one effect, timing and duration.
   */
  WAnimation(AnimationEffect effect, 
	     TimingFunction timing = TimingFunction::Linear,
	     int duration = 250);

  /*! \brief Creates an animation.
   *
   * An animation is created with two effects (a motion and Fade).
   */
  WAnimation(AnimationEffect effect1, AnimationEffect effect2,
	     TimingFunction timing = TimingFunction::Linear,
	     int duration = 250);
  
  /*! \brief Clone method.
   *
   * Clones this animation object.
   */
  WAnimation clone() const;
#endif

  /*! \brief Sets the animation effects.
   *
   * A motion effect (\link AnimationEffect::SlideInFromLeft
   * SlideInFromLeft\endlink, \link AnimationEffect::SlideInFromRight
   * SlideInFromRight\endlink, \link AnimationEffect::SlideInFromBottom
   * SlideInFromBottom\endlink, \link AnimationEffect::SlideInFromTop
   * SlideInFromTop\endlink or \link AnimationEffect::Pop Pop\endlink) can be combined
   * with a fade effect (\link AnimationEffect::Fade Fade\endlink).
   *
   * When effects are 0, the animation does not actually specify an
   * animation, but instead an instant transition.
   */
  void setEffects(WFlags<AnimationEffect> effects);

  /*! \brief Returns animation effects.
   *
   * \sa setEffects()
   */
  WFlags<AnimationEffect> effects() const { return effects_; }

  /*! \brief Comparison operator.
   *
   * Returns \c true if the transitions are exactly the same.
   */
  bool operator==(const WAnimation& other) const;

  /*! \brief Comparison operator.
   *
   * Returns \c true if the transitions are different.
   */
  bool operator!=(const WAnimation& other) const;

  /*! \brief Sets the duration.
   *
   * The default animation duration is 250 ms.
   *
   * \sa duration()
   */
  void setDuration(int msecs);

  /*! \brief Returns the duration.
   *
   * \sa setDuration()
   */
  int duration() const { return duration_; }

  /*! \brief Sets a timing function.
   *
   * The default timinig function is TimingFunction::Linear.
   */
  void setTimingFunction(TimingFunction function);

  /*! \brief Returns the timing function.
   *
   * \sa setTimingFunction()
   */
  TimingFunction timingFunction() const { return timing_; }

  /*
  void setTimingFunction(double x1, double y1, double x2, double y2);
  const double[] timingFunction();
  */

  /*! \brief Returns whether the animation is empty.
   *
   * An animation is empty (meaning the transition is instant), if the
   * duration is 0, or if no effects are defined.
   */
  bool empty() const;

private:
  WFlags<AnimationEffect> effects_;
  TimingFunction          timing_;
  int                     duration_;
};

W_DECLARE_OPERATORS_FOR_FLAGS(AnimationEffect)

}

#endif // WANIMATION_H_
