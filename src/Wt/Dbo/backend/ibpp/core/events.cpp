///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: _epb.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, internal EventsImpl class implementation
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
//	* Tabulations should be set every four characters when editing this file.
//
//	SPECIAL WARNING COMMENT (by Olivier Mascia, 2000 Nov 12)
//	The way this source file handles events is not publicly documented, in
//	the ibase.h header file or in the IB 6.0 documentation. This documentation
//	suggests to use the API isc_event_block to construct vectors of events.
//	Unfortunately, this API takes a variable number of parameters to specify
//	the list of event names. In addition, the documentation warn on not using
//	more than 15 names. This is a sad limitation, partly because the maximum
//	number of parameters safely processed in such an API is very compiler
//	dependant and also because isc_event_counts() is designed to return counts
//	through the IB status vector which is a vector of 20 32-bits integers !
//	From reverse engineering of the isc_event_block() API in
//	source file jrd/alt.c (as available on fourceforge.net/project/InterBase as
//	of 2000 Nov 12), it looks like the internal format of those EPB is simple.
//	An EPB starts by a byte with value 1. A version identifier of some sort.
//	Then a small cluster is used for each event name. The cluster starts with
//	a byte for the length of the event name (no final '\0'). Followed by the N
//	characters of the name itself (no final '\0'). The cluster ends with 4 bytes
//	preset to 0.
//
//	SPECIAL CREDIT (July 2004) : this is a complete re-implementation of this
//	class, directly based on work by Val Samko.
//	The whole event handling has then be completely redesigned, based on the old
//	EPB class to bring up the current IBPP::Events implementation.
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

using namespace ibpp_internals;

const size_t EventsImpl::MAXEVENTNAMELEN = 127;

//	(((((((( OBJECT INTERFACE IMPLEMENTATION ))))))))

void EventsImpl::Add(const std::string& eventname, IBPP::EventInterface* objref)
{
	if (eventname.size() == 0)
		throw LogicExceptionImpl("Events::Add", _("Zero length event names not permitted"));
	if (eventname.size() > MAXEVENTNAMELEN)
		throw LogicExceptionImpl("Events::Add", _("Event name is too long"));
	if ((mEventBuffer.size() + eventname.length() + 5) > 32766)	// max signed 16 bits integer minus one
		throw LogicExceptionImpl("Events::Add",
			_("Can't add this event, the events list would overflow IB/FB limitation"));

	Cancel();

	// 1) Alloc or grow the buffers
	size_t prev_buffer_size = mEventBuffer.size();
	size_t needed = ((prev_buffer_size==0) ? 1 : 0) + eventname.length() + 5;
	// Initial alloc will require one more byte, we need 4 more bytes for
	// the count itself, and one byte for the string length prefix

	mEventBuffer.resize(mEventBuffer.size() + needed);
	mResultsBuffer.resize(mResultsBuffer.size() + needed);
	if (prev_buffer_size == 0)
		mEventBuffer[0] = mResultsBuffer[0] = 1; // First byte is a 'one'. Documentation ??

	// 2) Update the buffers (append)
	{
		Buffer::iterator it = mEventBuffer.begin() +
				((prev_buffer_size==0) ? 1 : prev_buffer_size); // Byte after current content
		*(it++) = static_cast<char>(eventname.length());
		it = std::copy(eventname.begin(), eventname.end(), it);
		// We initialize the counts to (uint32_t)(-1) to initialize properly, see FireActions()
		*(it++) = -1; *(it++) = -1; *(it++) = -1; *it = -1;
	}

	// copying new event to the results buffer to keep event_buffer_ and results_buffer_ consistant,
	// otherwise we might get a problem in `FireActions`
	// Val Samko, val@digiways.com
	std::copy(mEventBuffer.begin() + prev_buffer_size,
		mEventBuffer.end(), mResultsBuffer.begin() + prev_buffer_size);

	// 3) Alloc or grow the objref array and update the objref array (append)
	mObjectReferences.push_back(objref);

	Queue();
}

void EventsImpl::Drop(const std::string& eventname)
{
	if (eventname.size() == 0)
		throw LogicExceptionImpl("EventsImpl::Drop", _("Zero length event names not permitted"));
	if (eventname.size() > MAXEVENTNAMELEN)
		throw LogicExceptionImpl("EventsImpl::Drop", _("Event name is too long"));

	if (mEventBuffer.size() <= 1) return;	// Nothing to do, but not an error

	Cancel();

	// 1) Find the event in the buffers
	typedef EventBufferIterator<Buffer::iterator> EventIterator;
	EventIterator eit(mEventBuffer.begin()+1);
	EventIterator rit(mResultsBuffer.begin()+1);

	for (ObjRefs::iterator oit = mObjectReferences.begin();
			oit != mObjectReferences.end();
				++oit, ++eit, ++rit)
	{
		if (eventname != eit.get_name()) continue;
		
		// 2) Event found, remove it
		mEventBuffer.erase(eit.begin(), eit.end());
		mResultsBuffer.erase(rit.begin(), rit.end());
		mObjectReferences.erase(oit);
		break;
	}

	Queue();
}

void EventsImpl::List(std::vector<std::string>& events)
{
	events.clear();
	
	if (mEventBuffer.size() <= 1) return;	// Nothing to do, but not an error

	typedef EventBufferIterator<Buffer::iterator> EventIterator;
	EventIterator eit(mEventBuffer.begin()+1);

	for (ObjRefs::iterator oit = mObjectReferences.begin();
			oit != mObjectReferences.end();
				++oit, ++eit)
	{
		events.push_back(eit.get_name());
	}
}

void EventsImpl::Clear()
{
	Cancel();
	
	mObjectReferences.clear();
	mEventBuffer.clear();
	mResultsBuffer.clear();
}

void EventsImpl::Dispatch()
{
	// If no events registered, nothing to do of course.
	if (mEventBuffer.size() == 0) return;

	// Let's fire the events actions for all the events which triggered, if any, and requeue.
	FireActions();
	Queue();
}

IBPP::Database EventsImpl::DatabasePtr() const
{
	if (mDatabase == 0) throw LogicExceptionImpl("Events::DatabasePtr",
			_("No Database is attached."));
	return mDatabase;
}

IBPP::IEvents* EventsImpl::AddRef()
{
	ASSERTION(mRefCount >= 0);
	++mRefCount;
	return this;
}

void EventsImpl::Release()
{
	// Release cannot throw, except in DEBUG builds on assertion
	ASSERTION(mRefCount >= 0);
	--mRefCount;
	try { if (mRefCount <= 0) delete this; }
		catch (...) { }
}

//	(((((((( OBJECT INTERNAL METHODS ))))))))

void EventsImpl::Queue()
{
	if (! mQueued)
	{
		if (mDatabase->GetHandle() == 0)
			throw LogicExceptionImpl("EventsImpl::Queue",
				  _("Database is not connected"));

		IBS vector;
		mTrapped = false;
		mQueued = true;
		(*gds.Call()->m_que_events)(vector.Self(), mDatabase->GetHandlePtr(), &mId,
			short(mEventBuffer.size()), &mEventBuffer[0],
				(isc_callback)EventHandler, (char*)this);

		if (vector.Errors())
		{
			mId = 0;	// Should be, but better be safe
			mQueued = false;
			throw SQLExceptionImpl(vector, "EventsImpl::Queue",
				_("isc_que_events failed"));
		}
	}
}

void EventsImpl::Cancel()
{
	if (mQueued)
	{
		if (mDatabase->GetHandle() == 0) throw LogicExceptionImpl("EventsImpl::Cancel",
			_("Database is not connected"));

		IBS vector;

		// A call to cancel_events will call *once* the handler routine, even
		// though no events had fired. This is why we first set mEventsQueued
		// to false, so that we can be sure to dismiss those unwanted callbacks
		// subsequent to the execution of isc_cancel_events().
		mTrapped = false;
		mQueued = false;
		(*gds.Call()->m_cancel_events)(vector.Self(), mDatabase->GetHandlePtr(), &mId);

	    if (vector.Errors())
		{
			mQueued = true;	// Need to restore this as cancel failed
	    	throw SQLExceptionImpl(vector, "EventsImpl::Cancel",
	    		_("isc_cancel_events failed"));
		}

		mId = 0;	// Should be, but better be safe
	}
}

void EventsImpl::FireActions()
{
	if (mTrapped)
	{
		typedef EventBufferIterator<Buffer::iterator> EventIterator;
		EventIterator eit(mEventBuffer.begin()+1);
		EventIterator rit(mResultsBuffer.begin()+1);

		for (ObjRefs::iterator oit = mObjectReferences.begin();
			 oit != mObjectReferences.end();
				 ++oit, ++eit, ++rit)
		{
			if (eit == EventIterator(mEventBuffer.end())
				  || rit == EventIterator(mResultsBuffer.end()))
				throw LogicExceptionImpl("EventsImpl::FireActions", _("Internal buffer size error"));
			uint32_t vnew = rit.get_count();
			uint32_t vold = eit.get_count();
			if (vnew > vold)
			{
				// Fire the action
				try
				{
					(*oit)->ibppEventHandler(this, eit.get_name(), (int)(vnew - vold));
				}
				catch (...)
				{
					std::copy(rit.begin(), rit.end(), eit.begin());
					throw;
				}
				std::copy(rit.begin(), rit.end(), eit.begin());
			}
			// This handles initialization too, where vold == (uint32_t)(-1)
			// Thanks to M. Hieke for this idea and related initialization to (-1)
			if (vnew != vold)
 				std::copy(rit.begin(), rit.end(), eit.begin());
		}
	}
}

// This function must keep this prototype to stay compatible with
// what isc_que_events() expects

void EventsImpl::EventHandler(const char* object, short size, const char* tmpbuffer)
{
	// >>>>> This method is a STATIC member !! <<<<<
	// Consider this method as a kind of "interrupt handler". It should do as
	// few work as possible as quickly as possible and then return.
	// Never forget: this is called by the Firebird client code, on *some*
	// thread which might not be (and won't probably be) any of your application
	// thread. This function is to be considered as an "interrupt-handler" of a
	// hardware driver.

	// There can be spurious calls to EventHandler from FB internal. We must
	// dismiss those calls.
	if (object == 0 || size == 0 || tmpbuffer == 0) return;
		
	EventsImpl* evi = (EventsImpl*)object;	// Ugly, but wanted, c-style cast

	if (evi->mQueued)
	{
		try
		{
			char* rb = &evi->mResultsBuffer[0];
			if (evi->mEventBuffer.size() < (unsigned)size) size = (short)evi->mEventBuffer.size();
			for (int i = 0; i < size; i++)
				rb[i] = tmpbuffer[i];
			evi->mTrapped = true;
			evi->mQueued = false;
		}
		catch (...) { }
	}
}

void EventsImpl::AttachDatabaseImpl(DatabaseImpl* database)
{
	if (database == 0) throw LogicExceptionImpl("EventsImpl::AttachDatabase",
			_("Can't attach a null Database object."));

	if (mDatabase != 0) mDatabase->DetachEventsImpl(this);
	mDatabase = database;
	mDatabase->AttachEventsImpl(this);
}

void EventsImpl::DetachDatabaseImpl()
{
	if (mDatabase == 0) return;

	mDatabase->DetachEventsImpl(this);
	mDatabase = 0;
}

EventsImpl::EventsImpl(DatabaseImpl* database)
	: mRefCount(0)
{
	mDatabase = 0;
	mId = 0;
	mQueued = mTrapped = false;
	AttachDatabaseImpl(database);
}

EventsImpl::~EventsImpl()
{
	try { Clear(); }
		catch (...) { }
	
	try { if (mDatabase != 0) mDatabase->DetachEventsImpl(this); }
		catch (...) { }
}

//
//	EOF
//
