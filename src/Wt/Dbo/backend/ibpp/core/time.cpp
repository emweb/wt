///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: time.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, Time class implementation
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

#include <time.h>		// Can't use <ctime> thanks to MSVC6 buggy library

using namespace ibpp_internals;

void IBPP::Time::Now()
{
	time_t systime = time(0);
	tm* loctime = localtime(&systime);
	IBPP::itot(&mTime, loctime->tm_hour, loctime->tm_min, loctime->tm_sec, 0);
}

void IBPP::Time::SetTime(int tm)
{
	if (tm < 0 || tm > 863999999)
		throw LogicExceptionImpl("Time::SetTime", _("Invalid time value"));
	mTime = tm;
}

void IBPP::Time::SetTime(int hour, int minute, int second, int tenthousandths)
{
	if (hour < 0 || hour > 23 ||
		minute < 0 || minute > 59 ||
			second < 0 || second > 59 ||
				tenthousandths < 0 || tenthousandths > 9999)
					throw LogicExceptionImpl("Time::SetTime",
						_("Invalid hour, minute, second values"));
	IBPP::itot(&mTime, hour, minute, second, tenthousandths);
}

void IBPP::Time::GetTime(int& hour, int& minute, int& second) const
{
	IBPP::ttoi(mTime, &hour, &minute, &second, 0);
}

void IBPP::Time::GetTime(int& hour, int& minute, int& second, int& tenthousandths) const
{
	IBPP::ttoi(mTime, &hour, &minute, &second, &tenthousandths);
}

int IBPP::Time::Hours() const
{
	int hours;
	IBPP::ttoi(mTime, &hours, 0, 0, 0);
	return hours;
}

int IBPP::Time::Minutes() const
{
	int minutes;
	IBPP::ttoi(mTime, 0, &minutes, 0, 0);
	return minutes;
}

int IBPP::Time::Seconds() const
{
	int seconds;
	IBPP::ttoi(mTime, 0, 0, &seconds, 0);
	return seconds;
}

int IBPP::Time::SubSeconds() const	// Actually tenthousandths of seconds
{
	int tenthousandths;
	IBPP::ttoi(mTime, 0, 0, 0, &tenthousandths);
	return tenthousandths;
}

IBPP::Time::Time(int hour, int minute, int second, int tenthousandths)
{
	SetTime(hour, minute, second, tenthousandths);
}

IBPP::Time::Time(const IBPP::Time& copied)
{
	mTime = copied.mTime;
}

IBPP::Time& IBPP::Time::operator=(const IBPP::Timestamp& assigned)
{
	mTime = assigned.GetTime();
	return *this;
}

IBPP::Time& IBPP::Time::operator=(const IBPP::Time& assigned)
{
	mTime = assigned.mTime;
	return *this;
}

//	Time calculations. Internal format is the number of seconds elapsed since
//	midnight. Splits such a time in its hours, minutes, seconds components.

void IBPP::ttoi(int itime, int *h, int *m, int *s, int* t)
{
	int hh, mm, ss, tt;

	hh = (int) (itime / 36000000);	itime = itime - hh * 36000000;
	mm = (int) (itime / 600000);	itime = itime - mm * 600000;
	ss = (int) (itime / 10000);
	tt = (int) (itime - ss * 10000);

	if (h != 0) *h = hh;
	if (m != 0) *m = mm;
	if (s != 0) *s = ss;
	if (t != 0) *t = tt;

	return;
}

//	Get the internal time format, given hour, minute, second.

void IBPP::itot (int *ptime, int hour, int minute, int second, int tenthousandths)
{
	*ptime = hour * 36000000 + minute * 600000 + second * 10000 + tenthousandths;
	return;
}

namespace ibpp_internals
{

//
//	The following functions are helper conversions functions between IBPP
//	Date, Time, Timestamp and ISC_DATE, ISC_TIME and ISC_TIMESTAMP.
//	(They must be maintained if the encoding used by Firebird evolve.)
//	These helper functions are used from row.cpp and from array.cpp.
//

void encodeDate(ISC_DATE& isc_dt, const IBPP::Date& dt)
{
	// There simply has a shift of 15019 between the native Firebird
	// date model and the IBPP model.
	isc_dt = (ISC_DATE)(dt.GetDate() + 15019);
}

void decodeDate(IBPP::Date& dt, const ISC_DATE& isc_dt)
{
	// There simply has a shift of 15019 between the native Firebird
	// date model and the IBPP model.
	dt.SetDate((int)isc_dt - 15019);
}

void encodeTime(ISC_TIME& isc_tm, const IBPP::Time& tm)
{
	isc_tm = (ISC_TIME)tm.GetTime();
}

void decodeTime(IBPP::Time& tm, const ISC_TIME& isc_tm)
{
	tm.SetTime((int)isc_tm);
}

void encodeTimestamp(ISC_TIMESTAMP& isc_ts, const IBPP::Timestamp& ts)
{
	encodeDate(isc_ts.timestamp_date, ts);
	encodeTime(isc_ts.timestamp_time, ts);
}

void decodeTimestamp(IBPP::Timestamp& ts, const ISC_TIMESTAMP& isc_ts)
{
	decodeDate(ts, isc_ts.timestamp_date);
	decodeTime(ts, isc_ts.timestamp_time);
}

}

//
//	EOF
//

