/*
  Copyright (C) 2003, 2005, 2008 GraphicsMagick Group
  Copyright (C) 2002 ImageMagick Studio
 
  This program is covered by multiple licenses, which are described in
  Copyright.txt. You should have received a copy of Copyright.txt with this
  package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
 
  GraphicsMagick Alpha Composite Methods.
*/
#ifndef _MAGICK_ALPHA_COMPOSITE_H
#define _MAGICK_ALPHA_COMPOSITE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* defined(__cplusplus) || defined(c_plusplus) */

#if defined(MAGICK_IMPLEMENTATION)

static inline magick_uint32_t BlendQuantumOpacity(magick_uint32_t q,
  const magick_uint32_t opacity)
{
  magick_uint32_t
    result = 0U;

  if (opacity != 0U)
    {
#if QuantumDepth > 16
      result = ((magick_uint64_t) opacity*q)/MaxRGB;
#else
      result = ((magick_uint32_t) opacity*q)/MaxRGB;
#endif
    }
  return result;
}

static inline void BlendCompositePixel(PixelPacket *composite,
                                       const PixelPacket *p,
                                       const PixelPacket *q,
                                       const double alpha)
{
  double
    color;

  color=((double) p->red*(MaxRGBDouble-alpha)+q->red*alpha)/MaxRGBDouble;
  composite->red=RoundDoubleToQuantum(color);

  color=((double) p->green*(MaxRGBDouble-alpha)+q->green*alpha)/MaxRGBDouble;
  composite->green=RoundDoubleToQuantum(color);

  color=((double) p->blue*(MaxRGBDouble-alpha)+q->blue*alpha)/MaxRGBDouble;
  composite->blue=RoundDoubleToQuantum(color);

  composite->opacity=p->opacity;
}

/*
  Alpha compose pixel 'change' over pixel 'source'.

  The result will be the union of the two image shapes, with
  opaque areas of change-image obscuring base-image in the
  region of overlap.
*/
#define  MagickAlphaCompositeQuantum(change,change_alpha,base,base_alpha) \
   (1.0-(change_alpha/MaxRGBDouble))*(double) change+(1.0-(base_alpha/MaxRGBDouble))*(double) base*(change_alpha/MaxRGBDouble)

static inline void AlphaCompositePixel(PixelPacket *composite, const PixelPacket *change,
                                       const double change_alpha,const PixelPacket *base,
                                       const double base_alpha)
{
  if (change_alpha == (double) TransparentOpacity)
    {
      if (composite != base)
	*composite=*base;
    }
  else
    {
      double
        delta,
        value;

      delta=1.0-(change_alpha/MaxRGBDouble)*(base_alpha/MaxRGBDouble);
      
      value=MaxRGBDouble*(1.0-delta);
      composite->opacity=RoundDoubleToQuantum(value);
      
      delta=1.0/(delta <= MagickEpsilon ? 1.0 : delta);
      
      value=delta*MagickAlphaCompositeQuantum(change->red,change_alpha,base->red,base_alpha);
      composite->red=RoundDoubleToQuantum(value);
      
      value=delta*MagickAlphaCompositeQuantum(change->green,change_alpha,base->green,base_alpha);
      composite->green=RoundDoubleToQuantum(value);
      
      value=delta*MagickAlphaCompositeQuantum(change->blue,change_alpha,base->blue,base_alpha);
      composite->blue=RoundDoubleToQuantum(value);
    }
}

/*
  The result is the same shape as base-image, with change-image
  obscuring base-image where the image shapes overlap. Note this
  differs from over because the portion of change-image outside
  base-image's shape does not appear in the result.
*/
static inline void AtopCompositePixel(PixelPacket *composite,
                                      const PixelPacket *base,
                                      const PixelPacket *change)
{
  double
    color,
    opacity;

  opacity=((double)(MaxRGBDouble-change->opacity)*
           (MaxRGBDouble-base->opacity)+(double) change->opacity*
           (MaxRGBDouble-base->opacity))/MaxRGBDouble;
  
  color=((double) (MaxRGBDouble-change->opacity)*
         (MaxRGBDouble-base->opacity)*change->red/MaxRGBDouble+(double)
         change->opacity*(MaxRGBDouble-base->opacity)*
         base->red/MaxRGBDouble)/opacity;
  composite->red=RoundDoubleToQuantum(color);
  
  color=((double) (MaxRGBDouble-change->opacity)*
         (MaxRGBDouble-base->opacity)*change->green/MaxRGBDouble+(double)
         change->opacity*(MaxRGBDouble-base->opacity)*
         base->green/MaxRGBDouble)/opacity;
  composite->green=RoundDoubleToQuantum(color);
  
  color=((double) (MaxRGBDouble-change->opacity)*
         (MaxRGBDouble-base->opacity)*change->blue/MaxRGBDouble+(double)
         change->opacity*(MaxRGBDouble-base->opacity)*
         base->blue/MaxRGBDouble)/opacity;
  composite->blue=RoundDoubleToQuantum(color);
  
  composite->opacity=MaxRGB-RoundDoubleToQuantum(opacity);
}


#endif /* defined(MAGICK_IMPLEMENTATION) */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* defined(__cplusplus) || defined(c_plusplus) */

#endif /* _MAGICK_ALPHA_COMPOSITE_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * fill-column: 78
 * End:
 */
