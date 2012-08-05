///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: service.cpp 54 2006-03-27 16:07:44Z epocman $
//	Subject : IBPP, Service class implementation
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

using namespace ibpp_internals;

#ifdef IBPP_UNIX
#include <unistd.h>
#define Sleep(x) usleep(x)
#endif

//	(((((((( OBJECT INTERFACE IMPLEMENTATION ))))))))

void ServiceImpl::Connect()
{
	if (mHandle	!= 0) return;	// Already connected
	
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mUserName.empty())
		throw LogicExceptionImpl("Service::Connect", _("Unspecified user name."));
	if (mUserPassword.empty())
		throw LogicExceptionImpl("Service::Connect", _("Unspecified user password."));

	// Attach to the Service Manager
	IBS status;
	SPB spb;
	std::string connect;

	// Build a SPB based on	the	properties
	spb.Insert(isc_spb_version);
	spb.Insert(isc_spb_current_version);
	spb.InsertString(isc_spb_user_name, 1, mUserName.c_str());
	spb.InsertString(isc_spb_password, 1, mUserPassword.c_str());

	if (! mServerName.empty())
	{
		connect = mServerName;
		connect += ":";
	}

	connect += "service_mgr";

	(*gds.Call()->m_service_attach)(status.Self(), (short)connect.size(), (char*)connect.c_str(),
		&mHandle, spb.Size(), spb.Self());
	if (status.Errors())
	{
		mHandle	= 0;		// Should be, but better be	sure...
		throw SQLExceptionImpl(status, "Service::Connect", _("isc_service_attach failed"));
	}
}

void ServiceImpl::Disconnect()
{
	if (mHandle	== 0) return; // Already disconnected
	
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));

	IBS status;

	// Detach from the service manager
	(*gds.Call()->m_service_detach)(status.Self(), &mHandle);

	// Set mHandle to 0 now, just in case we need to throw, because Disconnect()
	// is called from Service destructor and we want to maintain a coherent state.
	mHandle	= 0;
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::Disconnect", _("isc_service_detach failed"));
}

void ServiceImpl::GetVersion(std::string& version)
{
	// Based on a patch provided by Torsten Martinsen (SourceForge 'bullestock')

	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle == 0)
		throw LogicExceptionImpl("Service::GetVersion", _("Service is not connected."));

	IBS status;
	SPB spb;
	RB result(250);

	spb.Insert(isc_info_svc_server_version);

	(*gds.Call()->m_service_query)(status.Self(), &mHandle, 0, 0, 0, spb.Size(), spb.Self(),
		result.Size(), result.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::GetVersion", _("isc_service_query failed"));

	result.GetString(isc_info_svc_server_version, version);
}

void ServiceImpl::AddUser(const IBPP::User& user)
{
	if (gds.Call()->mGDSVersion >= 60 && mHandle == 0)
		throw LogicExceptionImpl("Service::AddUser", _("Service is not connected."));
	if (user.username.empty())
		throw LogicExceptionImpl("Service::AddUser", _("Username required."));
	if (user.password.empty())
		throw LogicExceptionImpl("Service::AddUser", _("Password required."));

	IBS status;
	SPB spb;
	spb.Insert(isc_action_svc_add_user);
	spb.InsertString(isc_spb_sec_username, 2, user.username.c_str());
	spb.InsertString(isc_spb_sec_password, 2, user.password.c_str());
	if (! user.firstname.empty())
			spb.InsertString(isc_spb_sec_firstname, 2, user.firstname.c_str());
	if (! user.middlename.empty())
			spb.InsertString(isc_spb_sec_middlename, 2, user.middlename.c_str());
	if (! user.lastname.empty())
			spb.InsertString(isc_spb_sec_lastname, 2, user.lastname.c_str());
	if (user.userid != 0)
			spb.InsertQuad(isc_spb_sec_userid, (int32_t)user.userid);
	if (user.groupid != 0)
			spb.InsertQuad(isc_spb_sec_groupid, (int32_t)user.groupid);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::AddUser", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::ModifyUser(const IBPP::User& user)
{
	if (gds.Call()->mGDSVersion >= 60 && mHandle == 0)
		throw LogicExceptionImpl("Service::ModifyUser", _("Service is not connected."));
	if (user.username.empty())
		throw LogicExceptionImpl("Service::ModifyUser", _("Username required."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_modify_user);
	spb.InsertString(isc_spb_sec_username, 2, user.username.c_str());
	if (! user.password.empty())
			spb.InsertString(isc_spb_sec_password, 2, user.password.c_str());
	if (! user.firstname.empty())
			spb.InsertString(isc_spb_sec_firstname, 2, user.firstname.c_str());
	if (! user.middlename.empty())
			spb.InsertString(isc_spb_sec_middlename, 2, user.middlename.c_str());
	if (! user.lastname.empty())
			spb.InsertString(isc_spb_sec_lastname, 2, user.lastname.c_str());
	if (user.userid != 0)
			spb.InsertQuad(isc_spb_sec_userid, (int32_t)user.userid);
	if (user.groupid != 0)
			spb.InsertQuad(isc_spb_sec_groupid, (int32_t)user.groupid);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::ModifyUser", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::RemoveUser(const std::string& username)
{

	if (gds.Call()->mGDSVersion >= 60 && mHandle == 0)
		throw LogicExceptionImpl("Service::RemoveUser", _("Service is not connected."));
	if (username.empty())
		throw LogicExceptionImpl("Service::RemoveUser", _("Username required."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_delete_user);
	spb.InsertString(isc_spb_sec_username, 2, username.c_str());

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::RemoveUser", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::GetUser(IBPP::User& user)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle == 0)
		throw LogicExceptionImpl("Service::GetUser", _("Service is not connected."));
	if (user.username.empty())
		throw LogicExceptionImpl("Service::GetUser", _("Username required."));

	SPB spb;
	spb.Insert(isc_action_svc_display_user);
	spb.InsertString(isc_spb_sec_username, 2, user.username.c_str());

	IBS status;
	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::GetUser", _("isc_service_start failed"));

	RB result(8000);
	char request[] = {isc_info_svc_get_users};
	status.Reset();
	(*gds.Call()->m_service_query)(status.Self(), &mHandle, 0, 0, 0,
		sizeof(request), request, result.Size(), result.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::GetUser", _("isc_service_query failed"));

	char* p = result.Self();
	if (*p != isc_info_svc_get_users)
		throw SQLExceptionImpl(status, "Service::GetUser", _("isc_service_query returned unexpected answer"));

	p += 3;	// Skips the 'isc_info_svc_get_users' and its total length
	user.clear();
	while (*p != isc_info_end)
	{
		if (*p == isc_spb_sec_userid)
		{
			user.userid = (uint32_t)(*gds.Call()->m_vax_integer)(p+1, 4);
			p += 5;
		}
		else if (*p == isc_spb_sec_groupid)
		{
			user.groupid = (uint32_t)(*gds.Call()->m_vax_integer)(p+1, 4);
			p += 5;
		}
		else
		{
			unsigned short len = (unsigned short)(*gds.Call()->m_vax_integer)(p+1, 2);
			switch (*p)
			{
			case isc_spb_sec_username :
				// For each user, this is the first element returned
				if (len != 0) user.username.assign(p+3, len);
				break;
			case isc_spb_sec_password :
				if (len != 0) user.password.assign(p+3, len);
				break;
			case isc_spb_sec_firstname :
				if (len != 0) user.firstname.assign(p+3, len);
				break;
			case isc_spb_sec_middlename :
				if (len != 0) user.middlename.assign(p+3, len);
				break;
			case isc_spb_sec_lastname :
				if (len != 0) user.lastname.assign(p+3, len);
				break;
			}
			p += (3 + len);
		}
    }
}

void ServiceImpl::GetUsers(std::vector<IBPP::User>& users)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle == 0)
		throw LogicExceptionImpl("Service::GetUsers", _("Service is not connected."));

	SPB spb;
	spb.Insert(isc_action_svc_display_user);

	IBS status;
	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::GetUsers", _("isc_service_start failed"));

	RB result(8000);
	char request[] = {isc_info_svc_get_users};
	status.Reset();
	(*gds.Call()->m_service_query)(status.Self(), &mHandle, 0, 0, 0,
		sizeof(request), request, result.Size(), result.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::GetUsers", _("isc_service_query failed"));

	users.clear();
	char* p = result.Self();
	if (*p != isc_info_svc_get_users)
		throw SQLExceptionImpl(status, "Service::GetUsers", _("isc_service_query returned unexpected answer"));

	p += 3;	// Skips the 'isc_info_svc_get_users' and its total length
	IBPP::User user;
	while (*p != isc_info_end)
	{
		if (*p == isc_spb_sec_userid)
		{
			user.userid = (uint32_t)(*gds.Call()->m_vax_integer)(p+1, 4);
			p += 5;
		}
		else if (*p == isc_spb_sec_groupid)
		{
			user.groupid = (uint32_t)(*gds.Call()->m_vax_integer)(p+1, 4);
			p += 5;
		}
		else
		{
			unsigned short len = (unsigned short)(*gds.Call()->m_vax_integer)(p+1, 2);
			switch (*p)
			{
			case isc_spb_sec_username :
				// For each user, this is the first element returned
				if (! user.username.empty()) users.push_back(user);	// Flush previous user
				user.clear();
				if (len != 0) user.username.assign(p+3, len);
				break;
			case isc_spb_sec_password :
				if (len != 0) user.password.assign(p+3, len);
				break;
			case isc_spb_sec_firstname :
				if (len != 0) user.firstname.assign(p+3, len);
				break;
			case isc_spb_sec_middlename :
				if (len != 0) user.middlename.assign(p+3, len);
				break;
			case isc_spb_sec_lastname :
				if (len != 0) user.lastname.assign(p+3, len);
				break;
			}
			p += (3 + len);
		}
    }
	if (! user.username.empty()) users.push_back(user);	// Flush last user
}

void ServiceImpl::SetPageBuffers(const std::string& dbfile, int buffers)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::SetPageBuffers", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::SetPageBuffers", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_properties);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	spb.InsertQuad(isc_spb_prp_page_buffers, buffers);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::SetPageBuffers", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::SetSweepInterval(const std::string& dbfile, int sweep)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::SetSweepInterval", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::SetSweepInterval", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_properties);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	spb.InsertQuad(isc_spb_prp_sweep_interval, sweep);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::SetSweepInterval", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::SetSyncWrite(const std::string& dbfile, bool sync)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::SetSyncWrite", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::SetSyncWrite", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_properties);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	if (sync) spb.InsertByte(isc_spb_prp_write_mode, (char)isc_spb_prp_wm_sync);
	else spb.InsertByte(isc_spb_prp_write_mode, (char)isc_spb_prp_wm_async);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::SetSyncWrite", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::SetReadOnly(const std::string& dbfile, bool readonly)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::SetReadOnly", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::SetReadOnly", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_properties);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	if (readonly) spb.InsertByte(isc_spb_prp_access_mode, (char)isc_spb_prp_am_readonly);
	else spb.InsertByte(isc_spb_prp_access_mode, (char)isc_spb_prp_am_readwrite);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::SetReadOnly", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::SetReserveSpace(const std::string& dbfile, bool reserve)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::SetReserveSpace", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::SetReserveSpace", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_properties);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	if (reserve) spb.InsertByte(isc_spb_prp_reserve_space, (char)isc_spb_prp_res);
	else spb.InsertByte(isc_spb_prp_reserve_space, (char)isc_spb_prp_res_use_full);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::SetReserveSpace", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::Shutdown(const std::string& dbfile, IBPP::DSM mode, int sectimeout)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::Shutdown", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::Shutdown", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_properties);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	switch (mode)
	{
		case IBPP::dsDenyAttach :
			spb.InsertQuad(isc_spb_prp_deny_new_attachments, sectimeout);
			break;
		case IBPP::dsDenyTrans :
			spb.InsertQuad(isc_spb_prp_deny_new_transactions, sectimeout);
			break;
		case IBPP::dsForce :
			spb.InsertQuad(isc_spb_prp_shutdown_db, sectimeout);
			break;
	}

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::Shutdown", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::Restart(const std::string& dbfile)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::Restart", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::Restart", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_properties);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	spb.InsertQuad(isc_spb_options, isc_spb_prp_db_online);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::Restart", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::Sweep(const std::string& dbfile)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::Sweep", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::Sweep", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_repair);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	spb.InsertQuad(isc_spb_options, isc_spb_rpr_sweep_db);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::Sweep", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::Repair(const std::string& dbfile, IBPP::RPF flags)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::Repair", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::Repair", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_repair);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());

	unsigned int mask;
	if (flags & IBPP::rpValidateFull) mask = (isc_spb_rpr_full | isc_spb_rpr_validate_db);
	else if (flags & IBPP::rpValidatePages) mask = isc_spb_rpr_validate_db;
	else if (flags & IBPP::rpMendRecords) mask = isc_spb_rpr_mend_db;
	else throw LogicExceptionImpl("Service::Repair",
		_("One of rpMendRecords, rpValidatePages, rpValidateFull is required."));

	if (flags & IBPP::rpReadOnly)			mask |= isc_spb_rpr_check_db;
	if (flags & IBPP::rpIgnoreChecksums)	mask |= isc_spb_rpr_ignore_checksum;
	if (flags & IBPP::rpKillShadows)		mask |= isc_spb_rpr_kill_shadows;
	
	spb.InsertQuad(isc_spb_options, mask);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::Repair", _("isc_service_start failed"));

	Wait();
}

void ServiceImpl::StartBackup(const std::string& dbfile,
	const std::string& bkfile, IBPP::BRF flags)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::Backup", _("Service is not connected."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::Backup", _("Main database file must be specified."));
	if (bkfile.empty())
		throw LogicExceptionImpl("Service::Backup", _("Backup file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_backup);
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	spb.InsertString(isc_spb_bkp_file, 2, bkfile.c_str());
	if (flags & IBPP::brVerbose) spb.Insert(isc_spb_verbose);

	unsigned int mask = 0;
	if (flags & IBPP::brIgnoreChecksums)	mask |= isc_spb_bkp_ignore_checksums;
	if (flags & IBPP::brIgnoreLimbo)		mask |= isc_spb_bkp_ignore_limbo;
	if (flags & IBPP::brMetadataOnly)		mask |= isc_spb_bkp_metadata_only;
	if (flags & IBPP::brNoGarbageCollect)	mask |= isc_spb_bkp_no_garbage_collect;
	if (flags & IBPP::brNonTransportable)	mask |= isc_spb_bkp_non_transportable;
	if (flags & IBPP::brConvertExtTables)	mask |= isc_spb_bkp_convert;
	if (mask != 0) spb.InsertQuad(isc_spb_options, mask);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::Backup", _("isc_service_start failed"));
}

void ServiceImpl::StartRestore(const std::string& bkfile, const std::string& dbfile,
	int	pagesize, IBPP::BRF flags)
{
	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));
	if (mHandle	== 0)
		throw LogicExceptionImpl("Service::Restore", _("Service is not connected."));
	if (bkfile.empty())
		throw LogicExceptionImpl("Service::Restore", _("Backup file must be specified."));
	if (dbfile.empty())
		throw LogicExceptionImpl("Service::Restore", _("Main database file must be specified."));

	IBS status;
	SPB spb;

	spb.Insert(isc_action_svc_restore);
	spb.InsertString(isc_spb_bkp_file, 2, bkfile.c_str());
	spb.InsertString(isc_spb_dbname, 2, dbfile.c_str());
	if (flags & IBPP::brVerbose) spb.Insert(isc_spb_verbose);
	if (pagesize !=	0) spb.InsertQuad(isc_spb_res_page_size, pagesize);

	unsigned int mask;
	if (flags & IBPP::brReplace) mask = isc_spb_res_replace;
		else mask = isc_spb_res_create;	// Safe default mode

	if (flags & IBPP::brDeactivateIdx)	mask |= isc_spb_res_deactivate_idx;
	if (flags & IBPP::brNoShadow)		mask |= isc_spb_res_no_shadow;
	if (flags & IBPP::brNoValidity)		mask |= isc_spb_res_no_validity;
	if (flags & IBPP::brPerTableCommit)	mask |= isc_spb_res_one_at_a_time;
	if (flags & IBPP::brUseAllSpace)	mask |= isc_spb_res_use_all_space;
	if (mask != 0) spb.InsertQuad(isc_spb_options, mask);

	(*gds.Call()->m_service_start)(status.Self(), &mHandle, 0, spb.Size(), spb.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "Service::Restore", _("isc_service_start failed"));
}

const char* ServiceImpl::WaitMsg()
{
	IBS status;
	SPB req;
	RB result(1024);

	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));

	req.Insert(isc_info_svc_line);	// Request one line of textual output

	// _service_query will only block until a line of result is available
	// (or until the end of the task if it does not report information)
	(*gds.Call()->m_service_query)(status.Self(), &mHandle, 0, 0, 0,
		req.Size(),	req.Self(),	result.Size(), result.Self());
	if (status.Errors())
		throw SQLExceptionImpl(status, "ServiceImpl::Wait", _("isc_service_query failed"));

	// If message length is	zero bytes,	task is	finished
	if (result.GetString(isc_info_svc_line,	mWaitMessage) == 0) return 0;

	// Task	is not finished, but we	have something to report
	return mWaitMessage.c_str();
}

void ServiceImpl::Wait()
{
	IBS status;
	SPB spb;
	RB result(1024);
	std::string msg;

	if (gds.Call()->mGDSVersion < 60)
		throw LogicExceptionImpl("Service", _("Requires the version 6 of GDS32.DLL"));

	spb.Insert(isc_info_svc_line);
	for (;;)
	{
		// Sleeps 1 millisecond upfront. This will release the remaining
		// timeslot of the thread. Doing so will give a good chance for small
		// services tasks to finish before we check if they are still running.
		// The deal is to limit (in that particular case) the number of loops
		// polling _service_query that will happen.

		Sleep(1);

		// _service_query will only block until a line of result is available
		// (or until the end of the task if it does not report information) 
		(*gds.Call()->m_service_query)(status.Self(), &mHandle, 0, 0,	0,
			spb.Size(),	spb.Self(),	result.Size(), result.Self());
		if (status.Errors())
			throw SQLExceptionImpl(status, "ServiceImpl::Wait", _("isc_service_query failed"));

		// If message length is	zero bytes,	task is	finished
		if (result.GetString(isc_info_svc_line,	msg) ==	0) return;

		status.Reset();
		result.Reset();
	}
}

IBPP::IService* ServiceImpl::AddRef()
{
	ASSERTION(mRefCount >= 0);
	++mRefCount;
	return this;
}

void ServiceImpl::Release()
{
	// Release cannot throw, except in DEBUG builds on assertion
	ASSERTION(mRefCount >= 0);
	--mRefCount;
	try { if (mRefCount <= 0) delete this; }
		catch (...) { }
}

//	(((((((( OBJECT INTERNAL METHODS ))))))))

void ServiceImpl::SetServerName(const char* newName)
{
	if (newName == 0) mServerName.erase();
	else mServerName = newName;
}

void ServiceImpl::SetUserName(const char* newName)
{
	if (newName == 0) mUserName.erase();
	else mUserName = newName;
}

void ServiceImpl::SetUserPassword(const char* newPassword)
{
	if (newPassword == 0) mUserPassword.erase();
	else mUserPassword = newPassword;
}

ServiceImpl::ServiceImpl(const std::string& ServerName,
			const std::string& UserName, const std::string& UserPassword)
	:	mRefCount(0), mHandle(0),
		mServerName(ServerName), mUserName(UserName), mUserPassword(UserPassword)
{
}

ServiceImpl::~ServiceImpl()
{
	try { if (Connected()) Disconnect(); }
		catch (...) { }
}

//
//	Eof
//
