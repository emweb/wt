///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: _dpb.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, internal DPB class implementation
//
///////////////////////////////////////////////////////////////////////////////
//
//	(C) Copyright 2000-2006 T.I.P. Group S.A. and the IBPP Team (www.ibpp.org)
//
//	The contents of this file are subject to the IBPP License (the "License");
//	you may not use this file except in compliance with the License.  You may
//	obtain a copy of the License at http://www.ibpp.org or in the 'license.txt'
//	file which must have been distributed along with this file.
//
//	This software, distributed under the License, is distributed on an "AS IS"
//	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
//	License for the specific language governing rights and limitations
//	under the License.
//
///////////////////////////////////////////////////////////////////////////////
//
//	COMMENTS
//	* DPB == Database Parameter Block/Buffer, see Interbase 6.0 C-API
//	* Tabulations should be set every four characters when editing this file.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable: 4786 4996)
#ifndef _DEBUG
#pragma warning(disable: 4702)
#endif
#endif

#include "_ibpp.h"

#ifdef HAS_HDRSTOP
#pragma hdrstop
#endif

#include <string.h>

using namespace ibpp_internals;

const int DPB::BUFFERINCR = 128;

void DPB::Grow(int needed)
{
	if (mBuffer == 0) ++needed;	// Initial alloc will require one more byte
	if ((mSize + needed) > mAlloc)
	{
		// We need to grow the buffer. We use increments of BUFFERINCR bytes.
		needed = (needed / BUFFERINCR + 1) * BUFFERINCR;
		char* newbuffer = new char[mAlloc + needed];
		if (mBuffer == 0)
		{
			// Initial allocation, initialize the version tag
			newbuffer[0] = isc_dpb_version1;
			mSize = 1;
		}
		else
		{
			// Move the old buffer content to the new one
			memcpy(newbuffer, mBuffer, mSize);
			delete [] mBuffer;
		}
		mBuffer = newbuffer;
		mAlloc += needed;
	}
}

void DPB::Insert(char type, const char* data)
{
	int len = (int)strlen(data);
	Grow(len + 2);
    mBuffer[mSize++] = type;
	mBuffer[mSize++] = char(len);
    strncpy(&mBuffer[mSize], data, len);
    mSize += len;
}

void DPB::Insert(char type, int16_t data)
{
	Grow(2 + 2);
    mBuffer[mSize++] = type;
	mBuffer[mSize++] = char(2);
    *(int16_t*)&mBuffer[mSize] = int16_t((*gds.Call()->m_vax_integer)((char*)&data, 2));
    mSize += 2;
}

void DPB::Insert(char type, bool data)
{
	Grow(2 + 1);
    mBuffer[mSize++] = type;
	mBuffer[mSize++] = char(1);
    mBuffer[mSize++] = char(data ? 1 : 0);
}

void DPB::Insert(char type, char data)
{
	Grow(2 + 1);
    mBuffer[mSize++] = type;
	mBuffer[mSize++] = char(1);
    mBuffer[mSize++] = data;
}

void DPB::Reset()
{
	if (mAlloc != 0)
    {
    	delete [] mBuffer;
        mBuffer = 0;
        mSize = 0;
		mAlloc = 0;
    }
}

//
//	EOF
//
