///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: _ibpp.h 69 2006-04-11 13:18:22Z epocman $
//	Subject : IBPP internal declarations
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
//
//	* 'Internal declarations' means everything used to implement ibpp. This
//	  file and its contents is NOT needed by users of the library. All those
//	  declarations are wrapped in a namespace : 'ibpp_internals'.
//	* Tabulations should be set every four characters when editing this file.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __INTERNAL_IBPP_H__
#define __INTERNAL_IBPP_H__

#include "ibpp.h"

#if defined(__BCPLUSPLUS__) || defined(_MSC_VER) || defined(__DMC__)
#define HAS_HDRSTOP
#endif

#if (defined(__GNUC__) && defined(IBPP_WINDOWS))
//	Setting flags for ibase.h -- using GCC/Cygwin/MinGW on Win32
#ifndef _MSC_VER
#define _MSC_VER 1299
#endif
#ifndef _WIN32
#define _WIN32   1
#endif
#endif

#include "ibase.h"		// From Firebird 1.x or InterBase 6.x installation

#if (defined(__GNUC__) && defined(IBPP_WINDOWS))
//	UNSETTING flags used above for ibase.h -- Huge conflicts with libstdc++ !
#undef _MSC_VER
#undef _WIN32
#endif

#ifdef IBPP_WINDOWS
#include <windows.h>
#endif

#include <limits>
#include <string>
#include <vector>
#include <sstream>
#include <cstdarg>

#ifdef _DEBUG
#define ASSERTION(x)	{if (!(x)) {throw LogicExceptionImpl("ASSERTION", \
							"'"#x"' is not verified at %s, line %d", \
								__FILE__, __LINE__);}}
#else
#define ASSERTION(x)	/* x */
#endif

// Fix to famous MSVC 6 variable scope bug
#if defined(_MSC_VER) && (_MSC_VER < 1300)	// MSVC 6 should be < 1300
#define for if(true)for
#endif

namespace ibpp_internals
{

enum flush_debug_stream_type {fds};

#ifdef _DEBUG

struct DebugStream : public std::stringstream
{
	// next two operators fix some g++ and vc++ related problems
	std::ostream& operator<< (const char* p)
		{ static_cast<std::stringstream&>(*this)<< p; return *this; }

	std::ostream& operator<< (const std::string& p)
		{ static_cast<std::stringstream&>(*this)<< p; return *this; }

	DebugStream& operator=(const DebugStream&) {return *this;}
	DebugStream(const DebugStream&) {}
	DebugStream() {}
};
std::ostream& operator<< (std::ostream& a, flush_debug_stream_type);

#else

struct DebugStream
{
	template<class T> DebugStream& operator<< (const T&) { return *this; }
	// for manipulators
	DebugStream& operator<< (std::ostream&(*)(std::ostream&)) { return *this; }
};

#endif	// _DEBUG

class DatabaseImpl;
class TransactionImpl;
class StatementImpl;
class BlobImpl;
class ArrayImpl;
class EventsImpl;

//	Native data types
typedef enum {ivArray, ivBlob, ivDate, ivTime, ivTimestamp, ivString,
			ivInt16, ivInt32, ivInt64, ivFloat, ivDouble,
			ivBool, ivDBKey, ivByte} IITYPE;

//
//	Those are the Interbase C API prototypes that we use
//	Taken 'asis' from IBASE.H, prefix 'isc_' replaced with 'proto_',
//	and 'typedef' preprended...
//

typedef ISC_STATUS ISC_EXPORT proto_create_database (ISC_STATUS *,
					    short,
					    char *,
					    isc_db_handle *,
					    short,
					    char *,
					    short);

typedef ISC_STATUS ISC_EXPORT proto_attach_database (ISC_STATUS *,
					    short,
					    char *,
					    isc_db_handle *,
					    short,
					    char *);

typedef ISC_STATUS  ISC_EXPORT proto_detach_database (ISC_STATUS *,
					    isc_db_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_drop_database (ISC_STATUS *,
					  isc_db_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_database_info (ISC_STATUS *,
					  isc_db_handle *,
					  short,
					  char *,
					  short,
					  char *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_execute_immediate (ISC_STATUS *,
						   isc_db_handle *,
						   isc_tr_handle *,
						   unsigned short,
						   char *,
						   unsigned short,
						   XSQLDA *);

typedef ISC_STATUS  ISC_EXPORT proto_open_blob2 (ISC_STATUS *,
				       isc_db_handle *,
				       isc_tr_handle *,
				       isc_blob_handle *,
				       ISC_QUAD *,
				       short,
				       char *);

typedef ISC_STATUS  ISC_EXPORT proto_create_blob2 (ISC_STATUS *,
					isc_db_handle *,
					isc_tr_handle *,
					isc_blob_handle *,
					ISC_QUAD *,
					short,
					char *);

typedef ISC_STATUS  ISC_EXPORT proto_close_blob (ISC_STATUS *,
				       isc_blob_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_cancel_blob (ISC_STATUS *,
				        isc_blob_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_get_segment (ISC_STATUS *,
				        isc_blob_handle *,
				        unsigned short *,
				        unsigned short,
				        char *);

typedef ISC_STATUS  ISC_EXPORT proto_put_segment (ISC_STATUS *,
					isc_blob_handle *,
					unsigned short,
					char *);

typedef ISC_STATUS  ISC_EXPORT proto_blob_info (ISC_STATUS *,
				      isc_blob_handle *,
				      short,
 				      char *,
				      short,
				      char *);

typedef ISC_STATUS  ISC_EXPORT proto_array_lookup_bounds (ISC_STATUS *,
						isc_db_handle *,
						isc_tr_handle *,
						char *,
						char *,
						ISC_ARRAY_DESC *);

typedef ISC_STATUS  ISC_EXPORT proto_array_get_slice (ISC_STATUS *,
					    isc_db_handle *,
					    isc_tr_handle *,
					    ISC_QUAD *,
					    ISC_ARRAY_DESC *,
					    void *,
					    ISC_LONG *);

typedef ISC_STATUS  ISC_EXPORT proto_array_put_slice (ISC_STATUS *,
					    isc_db_handle *,
					    isc_tr_handle *,
					    ISC_QUAD *,
					    ISC_ARRAY_DESC *,
					    void *,
					    ISC_LONG *);

typedef ISC_LONG    ISC_EXPORT proto_vax_integer (char *,
					short);

typedef ISC_LONG    ISC_EXPORT proto_sqlcode (ISC_STATUS *);

typedef void        ISC_EXPORT proto_sql_interprete (short,
					   char *,
					   short);

typedef ISC_STATUS  ISC_EXPORT proto_interprete (char *,
				       ISC_STATUS * *);

typedef ISC_STATUS  ISC_EXPORT proto_que_events (ISC_STATUS *,
				       isc_db_handle *,
				       ISC_LONG *,
				       short,
				       char *,
				       isc_callback,
				       void *);

typedef ISC_STATUS  ISC_EXPORT proto_cancel_events (ISC_STATUS *,
					  isc_db_handle *,
					  ISC_LONG *);

typedef ISC_STATUS  ISC_EXPORT proto_start_multiple (ISC_STATUS *,
					   isc_tr_handle *,
					   short,
					   void *);

typedef ISC_STATUS  ISC_EXPORT proto_commit_transaction (ISC_STATUS *,
					       isc_tr_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_commit_retaining (ISC_STATUS *,
					     isc_tr_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_rollback_transaction (ISC_STATUS *,
						 isc_tr_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_rollback_retaining (ISC_STATUS *,
						 isc_tr_handle *);

///////////
typedef ISC_STATUS  ISC_EXPORT proto_dsql_allocate_statement (ISC_STATUS *,
						    isc_db_handle *,
						    isc_stmt_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_describe (ISC_STATUS *,
					  isc_stmt_handle *,
					  unsigned short,
					  XSQLDA *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_describe_bind (ISC_STATUS *,
					       isc_stmt_handle *,
					       unsigned short,
					       XSQLDA *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_execute (ISC_STATUS *,
					 isc_tr_handle *,
					 isc_stmt_handle *,
					 unsigned short,
					 XSQLDA *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_execute2 (ISC_STATUS *,
					  isc_tr_handle *,
					  isc_stmt_handle *,
					  unsigned short,
					  XSQLDA *,
					  XSQLDA *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_fetch (ISC_STATUS *,
				       isc_stmt_handle *,
				       unsigned short,
				       XSQLDA *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_free_statement (ISC_STATUS *,
						isc_stmt_handle *,
						unsigned short);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_prepare (ISC_STATUS *,
					 isc_tr_handle *,
					 isc_stmt_handle *,
					 unsigned short,
					 char *,
					 unsigned short,
				 	 XSQLDA *);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_set_cursor_name (ISC_STATUS *,
						 isc_stmt_handle *,
						 char *,
						 unsigned short);

typedef ISC_STATUS  ISC_EXPORT proto_dsql_sql_info (ISC_STATUS *,
					  isc_stmt_handle *,
					  short,
					  char *,
					  short,
					  char *);

typedef void        ISC_EXPORT proto_decode_date (ISC_QUAD *,
					void *);

typedef void        ISC_EXPORT proto_encode_date (void *,
					ISC_QUAD *);

typedef int			ISC_EXPORT proto_add_user (ISC_STATUS *, USER_SEC_DATA *);
typedef int			ISC_EXPORT proto_delete_user (ISC_STATUS *, USER_SEC_DATA *);
typedef int			ISC_EXPORT proto_modify_user (ISC_STATUS *, USER_SEC_DATA *);

//
//	Those API are only available in versions 6.x of the GDS32.DLL
//

typedef ISC_STATUS  ISC_EXPORT proto_service_attach (ISC_STATUS *,
					   unsigned short,
					   char *,
					   isc_svc_handle *,
					   unsigned short,
					   char *);

typedef ISC_STATUS  ISC_EXPORT proto_service_detach (ISC_STATUS *,
					   isc_svc_handle *);

typedef ISC_STATUS  ISC_EXPORT proto_service_query (ISC_STATUS *,
					  isc_svc_handle *,
                      		          isc_resv_handle *,
					  unsigned short,
					  char *,
					  unsigned short,
					  char *,
					  unsigned short,
					  char *);

typedef ISC_STATUS ISC_EXPORT proto_service_start (ISC_STATUS *,
    					 isc_svc_handle *,
                         		 isc_resv_handle *,
    					 unsigned short,
    					 char*);

typedef void        ISC_EXPORT proto_decode_sql_date (ISC_DATE *,
					void *);

typedef void        ISC_EXPORT proto_decode_sql_time (ISC_TIME *,
					void *);

typedef void        ISC_EXPORT proto_decode_timestamp (ISC_TIMESTAMP *,
					void *);

typedef void        ISC_EXPORT proto_encode_sql_date (void *,
					ISC_DATE *);

typedef void        ISC_EXPORT proto_encode_sql_time (void *,
					ISC_TIME *);

typedef void        ISC_EXPORT proto_encode_timestamp (void *,
					ISC_TIMESTAMP *);

//
//	Internal binding structure to the GDS32 DLL
//

struct GDS
{
	// Attributes
	bool mReady;
	int mGDSVersion; 		// Version of the GDS32.DLL (50 for 5.0, 60 for 6.0)

#ifdef IBPP_WINDOWS
	HMODULE mHandle;			// The GDS32.DLL HMODULE
	std::string mSearchPaths;	// Optional additional search paths
#endif

	GDS* Call();

	// GDS32 Entry Points
	proto_create_database*			m_create_database;
	proto_attach_database*			m_attach_database;
	proto_detach_database*			m_detach_database;
	proto_drop_database*			m_drop_database;
	proto_database_info*			m_database_info;
	proto_dsql_execute_immediate*	m_dsql_execute_immediate;
	proto_open_blob2*				m_open_blob2;
	proto_create_blob2*				m_create_blob2;
	proto_close_blob*				m_close_blob;
	proto_cancel_blob*				m_cancel_blob;
	proto_get_segment*				m_get_segment;
	proto_put_segment*				m_put_segment;
	proto_blob_info*				m_blob_info;
	proto_array_lookup_bounds*		m_array_lookup_bounds;
	proto_array_get_slice*			m_array_get_slice;
	proto_array_put_slice*			m_array_put_slice;

	proto_vax_integer*				m_vax_integer;
	proto_sqlcode*					m_sqlcode;
	proto_sql_interprete*			m_sql_interprete;
	proto_interprete*				m_interprete;
	proto_que_events*				m_que_events;
	proto_cancel_events* 			m_cancel_events;
	proto_start_multiple*			m_start_multiple;
	proto_commit_transaction*		m_commit_transaction;
	proto_commit_retaining*			m_commit_retaining;
	proto_rollback_transaction*		m_rollback_transaction;
	proto_rollback_retaining*		m_rollback_retaining;
	proto_dsql_allocate_statement*	m_dsql_allocate_statement;
	proto_dsql_describe*			m_dsql_describe;
	proto_dsql_describe_bind*		m_dsql_describe_bind;
	proto_dsql_prepare*				m_dsql_prepare;
	proto_dsql_execute*				m_dsql_execute;
	proto_dsql_execute2*			m_dsql_execute2;
	proto_dsql_fetch*				m_dsql_fetch;
	proto_dsql_free_statement*		m_dsql_free_statement;
	proto_dsql_set_cursor_name*		m_dsql_set_cursor_name;
	proto_dsql_sql_info* 			m_dsql_sql_info;
	//proto_decode_date*				m_decode_date;
	//proto_encode_date*				m_encode_date;
	//proto_add_user*					m_add_user;
	//proto_delete_user*				m_delete_user;
	//proto_modify_user*				m_modify_user;

	proto_service_attach*			m_service_attach;
	proto_service_detach*			m_service_detach;
	proto_service_start*			m_service_start;
	proto_service_query*			m_service_query;
	//proto_decode_sql_date*			m_decode_sql_date;
	//proto_decode_sql_time*			m_decode_sql_time;
	//proto_decode_timestamp*			m_decode_timestamp;
	//proto_encode_sql_date*			m_encode_sql_date;
	//proto_encode_sql_time*			m_encode_sql_time;
	//proto_encode_timestamp*			m_encode_timestamp;

	// Constructor (No need for a specific destructor)
	GDS()
	{
		mReady = false;
		mGDSVersion = 0;
#ifdef IBPP_WINDOWS
		mHandle = 0;
#endif
	};
};

extern GDS gds;

//
//	Service Parameter Block (used to define a service)
//

class SPB
{
	static const int BUFFERINCR;

	char* mBuffer;				// Dynamically allocated SPB structure
	int mSize;  				// Its used size in bytes
	int mAlloc;					// Its allocated size in bytes

	void Grow(int needed);		// Alloc or grow the mBuffer

public:
	void Insert(char);			// Insert a single byte code
	void InsertString(char, int, const char*);	// Insert a string, len can be defined as 1 or 2 bytes
	void InsertByte(char type, char data);
	void InsertQuad(char type, int32_t data);
	void Reset();			// Clears the SPB
	char* Self() { return mBuffer; }
	short Size() { return (short)mSize; }

	SPB() : mBuffer(0), mSize(0), mAlloc(0) { }
	~SPB() { Reset(); }
};

//
//	Database Parameter Block (used to define a database)
//

class DPB
{
	static const int BUFFERINCR;

	char* mBuffer;				// Dynamically allocated DPB structure
	int mSize;  				// Its used size in bytes
	int mAlloc;					// Its allocated size in bytes

	void Grow(int needed);		// Allocate or grow the mBuffer, so that
								// 'needed' bytes can be written (at least)

public:
	void Insert(char, const char*);	// Insert a new char* 'cluster'
	void Insert(char, int16_t);		// Insert a new int16_t 'cluster'
	void Insert(char, bool);   		// Insert a new bool 'cluster'
	void Insert(char, char);   		// Insert a new byte 'cluster'
	void Reset();				// Clears the DPB
	char* Self() { return mBuffer; }
	short Size() { return (short)mSize; }

	DPB() : mBuffer(0), mSize(0), mAlloc(0) { }
	~DPB() { Reset(); }
};

//
//	Transaction Parameter Block (used to define a transaction)
//

class TPB
{
	static const int BUFFERINCR;

	char* mBuffer;					// Dynamically allocated TPB structure
	int mSize;						// Its used size in bytes
	int mAlloc;						// Its allocated size

	void Grow(int needed);			// Alloc or re-alloc the mBuffer

public:
	void Insert(char);				// Insert a flag item
	void Insert(const std::string& data); // Insert a string (typically table name)
	void Reset();				// Clears the TPB
	char* Self() { return mBuffer; }
	int Size() { return mSize; }

	TPB() : mBuffer(0), mSize(0), mAlloc(0) { }
	~TPB() { Reset(); }
};

//
//	Used to receive (and process) a results buffer in various API calls
//

class RB
{
	char* mBuffer;
	int mSize;

	char* FindToken(char token);
	char* FindToken(char token, char subtoken);

public:
	void Reset();
	int GetValue(char token);
	int GetCountValue(char token);
	int GetValue(char token, char subtoken);
	bool GetBool(char token);
	int GetString(char token, std::string& data);

	char* Self() { return mBuffer; }
	short Size() { return (short)mSize; }

	RB();
	RB(int Size);
	~RB();
};

//
//	Used to receive status info from API calls
//

class IBS
{
	mutable ISC_STATUS mVector[20];
	mutable std::string mMessage;

public:
	ISC_STATUS* Self() { return mVector; }
	bool Errors() { return (mVector[0] == 1 && mVector[1] > 0) ? true : false; }
	const char* ErrorMessage() const;
	int SqlCode() const;
	int EngineCode() const { return (mVector[0] == 1) ? (int)mVector[1] : 0; }
	void Reset();

	IBS();
	IBS(IBS&);	// Copy Constructor
	~IBS();
};

///////////////////////////////////////////////////////////////////////////////
//
//	Implementation of the "hidden" classes associated with their public
//	counterparts. Their private data and methods can freely change without
//	breaking the compatibility of the DLL. If they receive new public methods,
//	and those methods are reflected in the public class, then the compatibility
//	is broken.
//
///////////////////////////////////////////////////////////////////////////////

//
// Hidden implementation of Exception classes.
//

/*
                         std::exception
                                |
                         IBPP::Exception
                       /                 \
                      /                   \
  IBPP::LogicException    ExceptionBase    IBPP::SQLException
        |        \         /   |     \     /
        |   LogicExceptionImpl |   SQLExceptionImpl
        |                      |
    IBPP::WrongType            |
               \               |
              IBPP::WrongTypeImpl
*/

class ExceptionBase
{
	//	(((((((( OBJECT INTERNALS ))))))))

protected:
	std::string mContext; 			// Exception context ("IDatabase::Drop")
	std::string mWhat;				// Full formatted message

	void buildErrorMessage(const char* message);
	void raise(const std::string& context, const char* message, va_list argptr);

public:
	// The following constructors are small and could be inlined, but for object
	// code compacity of the library it is much better to have them non-inlined.
	// The amount of code generated by compilers for a throw is well-enough.

	ExceptionBase() throw();
	ExceptionBase(const ExceptionBase& copied) throw();
	ExceptionBase& operator=(const ExceptionBase& copied) throw();
	ExceptionBase(const std::string& context, const char* message = 0, ...) throw();

	virtual ~ExceptionBase() throw();

	//	(((((((( OBJECT INTERFACE ))))))))

    virtual const char* Origin() const throw();
    virtual const char* ErrorMessage() const throw();
	virtual const char* what() const throw();
};

class LogicExceptionImpl : public IBPP::LogicException, public ExceptionBase
{
	//	(((((((( OBJECT INTERNALS ))))))))

public:
	// The following constructors are small and could be inlined, but for object
	// code compacity of the library it is much better to have them non-inlined.
	// The amount of code generated by compilers for a throw is well-enough.

	LogicExceptionImpl() throw();
	LogicExceptionImpl(const LogicExceptionImpl& copied) throw();
	LogicExceptionImpl& operator=(const LogicExceptionImpl& copied) throw();
	LogicExceptionImpl(const std::string& context, const char* message = 0, ...) throw();

	virtual ~LogicExceptionImpl() throw ();

	//	(((((((( OBJECT INTERFACE ))))))))
	//
	//	The object public interface is partly implemented by inheriting from
	//	the ExceptionBase class.

public:
    virtual const char* Origin() const throw();
    virtual const char* ErrorMessage() const throw();
	virtual const char* what() const throw();
};

class SQLExceptionImpl : public IBPP::SQLException, public ExceptionBase
{
	//	(((((((( OBJECT INTERNALS ))))))))

private:
	int mSqlCode;
	int mEngineCode;

public:
	// The following constructors are small and could be inlined, but for object
	// code compacity of the library it is much better to have them non-inlined.
	// The amount of code generated by compilers for a throw is well-enough.

	SQLExceptionImpl() throw();
	SQLExceptionImpl(const SQLExceptionImpl& copied) throw();
	SQLExceptionImpl& operator=(const SQLExceptionImpl& copied) throw();
	SQLExceptionImpl(const IBS& status, const std::string& context,
						const char* message = 0, ...) throw();

	virtual ~SQLExceptionImpl() throw ();

	//	(((((((( OBJECT INTERFACE ))))))))
	//
	//	The object public interface is partly implemented by inheriting from
	//	the ExceptionBase class.

public:
    virtual const char* Origin() const throw();
    virtual const char* ErrorMessage() const throw();
	virtual const char* what() const throw();
	virtual int SqlCode() const throw();
	virtual int EngineCode() const throw();
};

class WrongTypeImpl : public IBPP::WrongType, public ExceptionBase
{
	//	(((((((( OBJECT INTERNALS ))))))))

public:
	// The following constructors are small and could be inlined, but for object
	// code compacity of the library it is much better to have them non-inlined.
	// The amount of code generated by compilers for a throw is well-enough.

	WrongTypeImpl() throw();
	WrongTypeImpl(const WrongTypeImpl& copied) throw();
	WrongTypeImpl& operator=(const WrongTypeImpl& copied) throw();
	WrongTypeImpl(const std::string& context, int sqlType, IITYPE varType,
					const char* message = 0, ...) throw();

	virtual ~WrongTypeImpl() throw ();

	//	(((((((( OBJECT INTERFACE ))))))))
	//
	//	The object public interface is partly implemented by inheriting from
	//	the ExceptionBase class.

public:
    virtual const char* Origin() const throw();
    virtual const char* ErrorMessage() const throw();
	virtual const char* what() const throw();
};

class ServiceImpl : public IBPP::IService
{
	//	(((((((( OBJECT INTERNALS ))))))))

private:
	int mRefCount;				// Reference counter
    isc_svc_handle mHandle;		// InterBase API Service Handle
	std::string mServerName;	// Nom du serveur
    std::string mUserName;		// Nom de l'utilisateur
    std::string mUserPassword;	// Mot de passe de l'utilisateur
	std::string mWaitMessage;	// Progress message returned by WaitMsg()

	isc_svc_handle* GetHandlePtr() { return &mHandle; }
	void SetServerName(const char*);
	void SetUserName(const char*);
	void SetUserPassword(const char*);

public:
	isc_svc_handle GetHandle() { return mHandle; }

	ServiceImpl(const std::string& ServerName, const std::string& UserName,
					const std::string& UserPassword);
    ~ServiceImpl();

	//	(((((((( OBJECT INTERFACE ))))))))

public:
    void Connect();
	bool Connected() { return mHandle == 0 ? false : true; }
	void Disconnect();

	void GetVersion(std::string& version);

	void AddUser(const IBPP::User&);
	void GetUser(IBPP::User&);
	void GetUsers(std::vector<IBPP::User>&);
	void ModifyUser(const IBPP::User&);
	void RemoveUser(const std::string& username);

	void SetPageBuffers(const std::string& dbfile, int buffers);
	void SetSweepInterval(const std::string& dbfile, int sweep);
	void SetSyncWrite(const std::string& dbfile, bool);
	void SetReadOnly(const std::string& dbfile, bool);
	void SetReserveSpace(const std::string& dbfile, bool);

	void Shutdown(const std::string& dbfile, IBPP::DSM mode, int sectimeout);
	void Restart(const std::string& dbfile);
	void Sweep(const std::string& dbfile);
	void Repair(const std::string& dbfile, IBPP::RPF flags);

	void StartBackup(const std::string& dbfile, const std::string& bkfile,
		IBPP::BRF flags = IBPP::BRF(0));
	void StartRestore(const std::string& bkfile, const std::string& dbfile,
		int pagesize, IBPP::BRF flags = IBPP::BRF(0));

	const char* WaitMsg();
	void Wait();

	IBPP::IService* AddRef();
	void Release();
};

class DatabaseImpl : public IBPP::IDatabase
{
	//	(((((((( OBJECT INTERNALS ))))))))

	int mRefCount;				// Reference counter
    isc_db_handle mHandle;		// InterBase API Session Handle
	std::string mServerName;	// Server name
    std::string mDatabaseName;	// Database name (path/file)
    std::string mUserName;	  	// User name
    std::string mUserPassword;	// User password
    std::string mRoleName;	  	// Role used for the duration of the connection
	std::string mCharSet;	  	// Character Set used for the connection
	std::string mCreateParams;	// Other parameters (creation only)

	int mDialect;							// 1 if IB5, 1 or 3 if IB6/FB1
	std::vector<TransactionImpl*> mTransactions;// Table of Transaction*
	std::vector<StatementImpl*> mStatements;// Table of Statement*
	std::vector<BlobImpl*> mBlobs;			// Table of Blob*
	std::vector<ArrayImpl*> mArrays;		// Table of Array*
	std::vector<EventsImpl*> mEvents;		// Table of Events*

public:
	isc_db_handle* GetHandlePtr() { return &mHandle; }
	isc_db_handle GetHandle() { return mHandle; }

	void AttachTransactionImpl(TransactionImpl*);
	void DetachTransactionImpl(TransactionImpl*);
	void AttachStatementImpl(StatementImpl*);
	void DetachStatementImpl(StatementImpl*);
	void AttachBlobImpl(BlobImpl*);
	void DetachBlobImpl(BlobImpl*);
	void AttachArrayImpl(ArrayImpl*);
	void DetachArrayImpl(ArrayImpl*);
	void AttachEventsImpl(EventsImpl*);
	void DetachEventsImpl(EventsImpl*);

	DatabaseImpl(const std::string& ServerName, const std::string& DatabaseName,
				const std::string& UserName, const std::string& UserPassword,
				const std::string& RoleName, const std::string& CharSet,
				const std::string& CreateParams);
    ~DatabaseImpl();

	//	(((((((( OBJECT INTERFACE ))))))))

public:
	const char* ServerName() const		{ return mServerName.c_str(); }
	const char* DatabaseName() const	{ return mDatabaseName.c_str(); }
	const char* Username() const		{ return mUserName.c_str(); }
	const char* UserPassword() const	{ return mUserPassword.c_str(); }
	const char* RoleName() const		{ return mRoleName.c_str(); }
	const char* CharSet() const			{ return mCharSet.c_str(); }
	const char* CreateParams() const	{ return mCreateParams.c_str(); }

	void Info(int* ODSMajor, int* ODSMinor,
		int* PageSize, int* Pages, int* Buffers, int* Sweep,
		bool* SyncWrites, bool* Reserve);
	void Statistics(int* Fetches, int* Marks, int* Reads, int* Writes);
	void Counts(int* Insert, int* Update, int* Delete,
		int* ReadIdx, int* ReadSeq);
	void Users(std::vector<std::string>& users);
	int Dialect() { return mDialect; }

    void Create(int dialect);
	void Connect();
	bool Connected() { return mHandle == 0 ? false : true; }
	void Inactivate();
	void Disconnect();
    void Drop();

	IBPP::IDatabase* AddRef();
	void Release();
};

class TransactionImpl : public IBPP::ITransaction
{
	//	(((((((( OBJECT INTERNALS ))))))))

private:
	int mRefCount;					// Reference counter
    isc_tr_handle mHandle;			// Transaction InterBase

	std::vector<DatabaseImpl*> mDatabases;   	// Tableau de IDatabase*
	std::vector<StatementImpl*> mStatements;	// Tableau de IStatement*
	std::vector<BlobImpl*> mBlobs;				// Tableau de IBlob*
	std::vector<ArrayImpl*> mArrays;			// Tableau de Array*
	std::vector<TPB*> mTPBs;					// Tableau de TPB

	void Init();			// A usage exclusif des constructeurs

public:
	isc_tr_handle* GetHandlePtr() { return &mHandle; }
	isc_tr_handle GetHandle() { return mHandle; }

	void AttachStatementImpl(StatementImpl*);
	void DetachStatementImpl(StatementImpl*);
	void AttachBlobImpl(BlobImpl*);
	void DetachBlobImpl(BlobImpl*);
	void AttachArrayImpl(ArrayImpl*);
	void DetachArrayImpl(ArrayImpl*);
    void AttachDatabaseImpl(DatabaseImpl* dbi, IBPP::TAM am = IBPP::amWrite,
			IBPP::TIL il = IBPP::ilConcurrency,
			IBPP::TLR lr = IBPP::lrWait, IBPP::TFF flags = IBPP::TFF(0));
    void DetachDatabaseImpl(DatabaseImpl* dbi);

	TransactionImpl(DatabaseImpl* db, IBPP::TAM am = IBPP::amWrite,
		IBPP::TIL il = IBPP::ilConcurrency,
		IBPP::TLR lr = IBPP::lrWait, IBPP::TFF flags = IBPP::TFF(0));
    ~TransactionImpl();

	//	(((((((( OBJECT INTERFACE ))))))))

public:
    void AttachDatabase(IBPP::Database db, IBPP::TAM am = IBPP::amWrite,
			IBPP::TIL il = IBPP::ilConcurrency,
			IBPP::TLR lr = IBPP::lrWait, IBPP::TFF flags = IBPP::TFF(0));
    void DetachDatabase(IBPP::Database db);
	void AddReservation(IBPP::Database db,
			const std::string& table, IBPP::TTR tr);

    void Start();
	bool Started() { return mHandle == 0 ? false : true; }
    void Commit();
    void Rollback();
    void CommitRetain();
	void RollbackRetain();

	IBPP::ITransaction* AddRef();
	void Release();
};

class RowImpl : public IBPP::IRow
{
	//	(((((((( OBJECT INTERNALS ))))))))

private:
	int mRefCount;					// Reference counter

	XSQLDA* mDescrArea;				// XSQLDA descriptor itself
	std::vector<double> mNumerics;	// Temporary storage for Numerics
	std::vector<float> mFloats;	 	// Temporary storage for Floats
	std::vector<int64_t> mInt64s;	// Temporary storage for 64 bits
	std::vector<int32_t> mInt32s;	// Temporary storage for 32 bits
	std::vector<int16_t> mInt16s;	// Temporary storage for 16 bits
	std::vector<char> mBools;		// Temporary storage for Bools
	std::vector<std::string> mStrings;	// Temporary storage for Strings
	std::vector<bool> mUpdated;		// Which columns where updated (Set()) ?

	int mDialect;					// Related database dialect
	DatabaseImpl* mDatabase;		// Related Database (important for Blobs, ...)
	TransactionImpl* mTransaction;	// Related Transaction (same remark)

	void SetValue(int, IITYPE, const void* value, int = 0);
	void* GetValue(int, IITYPE, void* = 0);

public:
	void Free();
	short AllocatedSize() { return mDescrArea->sqln; }
	void Resize(int n);
	void AllocVariables();
	bool MissingValues();		// Returns wether one of the mMissing[] is true
	XSQLDA* Self() { return mDescrArea; }

	RowImpl& operator=(const RowImpl& copied);
	RowImpl(const RowImpl& copied);
	RowImpl(int dialect, int size, DatabaseImpl* db, TransactionImpl* tr);
    ~RowImpl();

	//	(((((((( OBJECT INTERFACE ))))))))

public:
	void SetNull(int);
	void Set(int, bool);
	void Set(int, const char*);				// c-strings
	void Set(int, const void*, int);		// byte buffers
	void Set(int, const std::string&);
	void Set(int, int16_t);
	void Set(int, int32_t);
	void Set(int, int64_t);
	void Set(int, float);
	void Set(int, double);
	void Set(int, const IBPP::Timestamp&);
	void Set(int, const IBPP::Date&);
	void Set(int, const IBPP::Time&);
	void Set(int, const IBPP::DBKey&);
	void Set(int, const IBPP::Blob&);
	void Set(int, const IBPP::Array&);

	bool IsNull(int);
	bool Get(int, bool&);
	bool Get(int, char*);  		// c-strings, len unchecked
	bool Get(int, void*, int&);	// byte buffers
	bool Get(int, std::string&);
	bool Get(int, int16_t&);
	bool Get(int, int32_t&);
	bool Get(int, int64_t&);
	bool Get(int, float&);
	bool Get(int, double&);
	bool Get(int, IBPP::Timestamp&);
	bool Get(int, IBPP::Date&);
	bool Get(int, IBPP::Time&);
	bool Get(int, IBPP::DBKey&);
	bool Get(int, IBPP::Blob&);
	bool Get(int, IBPP::Array&);

	bool IsNull(const std::string&);
	bool Get(const std::string&, bool&);
	bool Get(const std::string&, char*);	// c-strings, len unchecked
	bool Get(const std::string&, void*, int&);	// byte buffers
	bool Get(const std::string&, std::string&);
	bool Get(const std::string&, int16_t&);
	bool Get(const std::string&, int32_t&);
	bool Get(const std::string&, int64_t&);
	bool Get(const std::string&, float&);
	bool Get(const std::string&, double&);
	bool Get(const std::string&, IBPP::Timestamp&);
	bool Get(const std::string&, IBPP::Date&);
	bool Get(const std::string&, IBPP::Time&);
	bool Get(const std::string&, IBPP::DBKey&);
	bool Get(const std::string&, IBPP::Blob&);
	bool Get(const std::string&, IBPP::Array&);

	int ColumnNum(const std::string&);
	const char* ColumnName(int);
	const char* ColumnAlias(int);
	const char* ColumnTable(int);
	IBPP::SDT ColumnType(int);
	int ColumnSubtype(int);
	int ColumnSize(int);
	int ColumnScale(int);
	int Columns();

	bool ColumnUpdated(int);
	bool Updated();

	IBPP::Database DatabasePtr() const;
	IBPP::Transaction TransactionPtr() const;

	IBPP::IRow* Clone();
	IBPP::IRow* AddRef();
	void Release();
};

class StatementImpl : public IBPP::IStatement
{
	//	(((((((( OBJECT INTERNALS ))))))))

private:
	friend class TransactionImpl;

	int mRefCount;				// Reference counter
	isc_stmt_handle mHandle;	// Statement Handle

	DatabaseImpl* mDatabase;		// Attached database
	TransactionImpl* mTransaction;	// Attached transaction
	RowImpl* mInRow;
	//bool* mInMissing;			// Quels paramètres n'ont pas été spécifiés
	RowImpl* mOutRow;
	bool mResultSetAvailable;	// Executed and result set is available
	bool mCursorOpened;			// dsql_set_cursor_name was called
	IBPP::STT mType;			// Type de requète
	std::string mSql;			// Last SQL statement prepared or executed

	// Internal Methods
	void CursorFree();

public:
	// Properties and Attributes Access Methods
	isc_stmt_handle GetHandle() { return mHandle; }

	void AttachDatabaseImpl(DatabaseImpl*);
	void DetachDatabaseImpl();
	void AttachTransactionImpl(TransactionImpl*);
	void DetachTransactionImpl();

	StatementImpl(DatabaseImpl*, TransactionImpl*, const std::string&);
    ~StatementImpl();

	//	(((((((( OBJECT INTERFACE ))))))))

public:
	void Prepare(const std::string& sql);
	void Execute(const std::string& sql);
	inline void Execute()	{ Execute(std::string()); }
	void ExecuteImmediate(const std::string&);
	void CursorExecute(const std::string& cursor, const std::string& sql);
	inline void CursorExecute(const std::string& cursor)	{ CursorExecute(cursor, std::string()); }
	bool Fetch();
	bool Fetch(IBPP::Row&);
	int AffectedRows();
	void Close();	// Free resources, attachments maintained
	std::string& Sql() { return mSql; }
	IBPP::STT Type() { return mType; }

	void SetNull(int);
	void Set(int, bool);
	void Set(int, const char*);				// c-strings
	void Set(int, const void*, int);		// byte buffers
	void Set(int, const std::string&);
	void Set(int, int16_t);
	void Set(int, int32_t);
	void Set(int, int64_t);
	void Set(int, float);
	void Set(int, double);
	void Set(int, const IBPP::Timestamp&);
	void Set(int, const IBPP::Date&);
	void Set(int, const IBPP::Time&);
	void Set(int, const IBPP::DBKey&);
	void Set(int, const IBPP::Blob&);
	void Set(int, const IBPP::Array&);

	bool IsNull(int);
	bool Get(int, bool*);
	bool Get(int, bool&);
	bool Get(int, char*);				// c-strings, len unchecked
	bool Get(int, void*, int&);			// byte buffers
	bool Get(int, std::string&);
	bool Get(int, int16_t*);
	bool Get(int, int16_t&);
	bool Get(int, int32_t*);
	bool Get(int, int32_t&);
	bool Get(int, int64_t*);
	bool Get(int, int64_t&);
	bool Get(int, float*);
	bool Get(int, float&);
	bool Get(int, double*);
	bool Get(int, double&);
	bool Get(int, IBPP::Timestamp&);
	bool Get(int, IBPP::Date&);
	bool Get(int, IBPP::Time&);
	bool Get(int, IBPP::DBKey&);
	bool Get(int, IBPP::Blob&);
	bool Get(int, IBPP::Array&);

	bool IsNull(const std::string&);
	bool Get(const std::string&, bool*);
	bool Get(const std::string&, bool&);
	bool Get(const std::string&, char*);		// c-strings, len unchecked
	bool Get(const std::string&, void*, int&);	// byte buffers
	bool Get(const std::string&, std::string&);
	bool Get(const std::string&, int16_t*);
	bool Get(const std::string&, int16_t&);
	bool Get(const std::string&, int32_t*);
	bool Get(const std::string&, int32_t&);
	bool Get(const std::string&, int64_t*);
	bool Get(const std::string&, int64_t&);
	bool Get(const std::string&, float*);
	bool Get(const std::string&, float&);
	bool Get(const std::string&, double*);
	bool Get(const std::string&, double&);
	bool Get(const std::string&, IBPP::Timestamp&);
	bool Get(const std::string&, IBPP::Date&);
	bool Get(const std::string&, IBPP::Time&);
	bool Get(const std::string&, IBPP::DBKey&);
	bool Get(const std::string&, IBPP::Blob&);
	bool Get(const std::string&, IBPP::Array&);

	int ColumnNum(const std::string&);
    int ColumnNumAlias(const std::string&);
	const char* ColumnName(int);
	const char* ColumnAlias(int);
	const char* ColumnTable(int);
	IBPP::SDT ColumnType(int);
	int ColumnSubtype(int);
	int ColumnSize(int);
	int ColumnScale(int);
	int Columns();

	IBPP::SDT ParameterType(int);
	int ParameterSubtype(int);
	int ParameterSize(int);
	int ParameterScale(int);
	int Parameters();

	void Plan(std::string&);

	IBPP::Database DatabasePtr() const;
	IBPP::Transaction TransactionPtr() const;

	IBPP::IStatement* AddRef();
	void Release();
};

class BlobImpl : public IBPP::IBlob
{
	//	(((((((( OBJECT INTERNALS ))))))))

private:
	friend class RowImpl;

	int mRefCount;
	bool					mIdAssigned;
	ISC_QUAD				mId;
	isc_blob_handle			mHandle;
	bool					mWriteMode;
	DatabaseImpl*  			mDatabase;		// Belongs to this database
	TransactionImpl*		mTransaction;	// Belongs to this transaction

	void Init();
	void SetId(ISC_QUAD*);
	void GetId(ISC_QUAD*);

public:
	void AttachDatabaseImpl(DatabaseImpl*);
	void DetachDatabaseImpl();
	void AttachTransactionImpl(TransactionImpl*);
	void DetachTransactionImpl();

	BlobImpl(const BlobImpl&);
	BlobImpl(DatabaseImpl*, TransactionImpl* = 0);
	~BlobImpl();

	//	(((((((( OBJECT INTERFACE ))))))))

public:
	void Create();
	void Open();
	void Close();
	void Cancel();
	int Read(void*, int size);
	void Write(const void*, int size);
	void Info(int* Size, int* Largest, int* Segments);

	void Save(const std::string& data);
	void Load(std::string& data);

	IBPP::Database DatabasePtr() const;
	IBPP::Transaction TransactionPtr() const;

	IBPP::IBlob* AddRef();
	void Release();
};

class ArrayImpl : public IBPP::IArray
{
	//	(((((((( OBJECT INTERNALS ))))))))

private:
	friend class RowImpl;

	int					mRefCount;		// Reference counter
	bool				mIdAssigned;
	ISC_QUAD			mId;
	bool				mDescribed;
	ISC_ARRAY_DESC		mDesc;
	DatabaseImpl*  		mDatabase;		// Database attachée
	TransactionImpl*	mTransaction;	// Transaction attachée
	void*				mBuffer;		// Buffer for native data
	int					mBufferSize;	// Size of this buffer in bytes
	int					mElemCount;		// Count of elements in this array
	int					mElemSize;		// Size of an element in the buffer

	void Init();
	void SetId(ISC_QUAD*);
	void GetId(ISC_QUAD*);
	void ResetId();
	void AllocArrayBuffer();

public:
	void AttachDatabaseImpl(DatabaseImpl*);
	void DetachDatabaseImpl();
	void AttachTransactionImpl(TransactionImpl*);
	void DetachTransactionImpl();

	ArrayImpl(const ArrayImpl&);
	ArrayImpl(DatabaseImpl*, TransactionImpl* = 0);
	~ArrayImpl();

	//	(((((((( OBJECT INTERFACE ))))))))

public:
	void Describe(const std::string& table, const std::string& column);
	void ReadTo(IBPP::ADT, void*, int);
	void WriteFrom(IBPP::ADT, const void*, int);
	IBPP::SDT ElementType();
	int ElementSize();
	int ElementScale();
	int Dimensions();
	void Bounds(int dim, int* low, int* high);
	void SetBounds(int dim, int low, int high);

	IBPP::Database DatabasePtr() const;
	IBPP::Transaction TransactionPtr() const;

	IBPP::IArray* AddRef();
	void Release();
};

//
//	EventBufferIterator: used in EventsImpl implementation.
//

template<class It>
struct EventBufferIterator
{
	It mIt;

public:
	EventBufferIterator& operator++()
		{ mIt += 1 + static_cast<int>(*mIt) + 4; return *this; }

	bool operator == (const EventBufferIterator& i) const { return i.mIt == mIt; }
	bool operator != (const EventBufferIterator& i) const { return i.mIt != mIt; }

#ifdef __BCPLUSPLUS__
#pragma warn -8027
#endif
	std::string get_name() const
	{
		return std::string(mIt + 1, mIt + 1 + static_cast<int32_t>(*mIt));
	}
#ifdef __BCPLUSPLUS__
#pragma warn .8027
#endif

	uint32_t get_count() const
	{
		return (*gds.Call()->m_vax_integer)
			(const_cast<char*>(&*(mIt + 1 + static_cast<int>(*mIt))), 4);
	}

	// Those container like begin() and end() allow access to the underlying type
	It begin()	{ return mIt; }
	It end()	{ return mIt + 1 + static_cast<int>(*mIt) + 4; }

	EventBufferIterator() {}
	EventBufferIterator(It it) : mIt(it) {}
};

class EventsImpl : public IBPP::IEvents
{
	static const size_t MAXEVENTNAMELEN;
	static void EventHandler(const char*, short, const char*);

	typedef std::vector<IBPP::EventInterface*> ObjRefs;
	ObjRefs mObjectReferences;

	typedef std::vector<char> Buffer;
	Buffer mEventBuffer;
	Buffer mResultsBuffer;

	int mRefCount;		// Reference counter

	DatabaseImpl* mDatabase;
	ISC_LONG mId;			// Firebird internal Id of these events
	bool mQueued;			// Has isc_que_events() been called?
	bool mTrapped;			// EventHandled() was called since last que_events()

	void FireActions();
	void Queue();
	void Cancel();

	EventsImpl& operator=(const EventsImpl&);
	EventsImpl(const EventsImpl&);

public:
	void AttachDatabaseImpl(DatabaseImpl*);
	void DetachDatabaseImpl();
	
	EventsImpl(DatabaseImpl* dbi);
	~EventsImpl();
		
	//	(((((((( OBJECT INTERFACE ))))))))

public:
	void Add(const std::string&, IBPP::EventInterface*);
	void Drop(const std::string&);
	void List(std::vector<std::string>&);
	void Clear();				// Drop all events
	void Dispatch();			// Dispatch NON async events

	IBPP::Database DatabasePtr() const;

	IBPP::IEvents* AddRef();
	void Release();
};

void encodeDate(ISC_DATE& isc_dt, const IBPP::Date& dt);
void decodeDate(IBPP::Date& dt, const ISC_DATE& isc_dt);

void encodeTime(ISC_TIME& isc_tm, const IBPP::Time& tm);
void decodeTime(IBPP::Time& tm, const ISC_TIME& isc_tm);

void encodeTimestamp(ISC_TIMESTAMP& isc_ts, const IBPP::Timestamp& ts);
void decodeTimestamp(IBPP::Timestamp& ts, const ISC_TIMESTAMP& isc_ts);

struct consts	// See _ibpp.cpp for initializations of these constants
{
	static const double dscales[19];
	static const int Dec31_1899;
	static const int16_t min16;
	static const int16_t max16;
	static const int32_t min32;
	static const int32_t max32;
};

}	// namespace ibpp_internal

#endif // __INTERNAL_IBPP_H__

//
//	Eof
//
