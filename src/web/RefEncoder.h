// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef REF_ENCODER_H_
#define REF_ENCODER_H_

#include <Wt/WFlags.h>

namespace Wt {

class WString;

enum RefEncoderOption {
  EncodeInternalPaths = 0x1,
  EncodeRedirectTrampoline = 0x2
};

W_DECLARE_OPERATORS_FOR_FLAGS(RefEncoderOption)

extern WString EncodeRefs(const WString& text,
			  WFlags<RefEncoderOption> options);

}

#endif // REF_ENCODER_H_
