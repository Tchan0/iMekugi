/*******************************************************************************
* Header file for all aspi16 and aspi32 definitions
*
*******************************************************************************/
#ifndef __SRB_H_DEFINED__
#define __SRB_H_DEFINED__

/*******************************************************************************
* Some handy defines specific to MekugiAspi - not part of the ASPI standard
*******************************************************************************/
#define HOST_ADAPTER_COUNT		8 //this allows us to expose 7 SCSI Targets - Note: was 7 before MekugiAspi 1.0
#define HOST_ADAPTER_SCSI_ID	7 //SCSI adapters usually use ID 7 as their SCSI ID

/*******************************************************************************
* 
*******************************************************************************/
#ifndef WIN32   //_WINDOWS_
 typedef uint8_t/*unsigned char*/  BYTE;
 typedef uint16_t/*unsigned short*/ WORD;
 typedef uint32_t/*unsigned int*/  DWORD;
 #define VOID void			//typedef void VOID;
 typedef void* LPVOID; //4 byte generic pointer. Used in SRB fields which require either a pointer to a function or a Win32 handle (for example, SRB_PostProc).
 typedef uint8_t/*unsigned char*/* LPBYTE;// Pointer to an array of BYTEs. Mainly used as a buffer pointer.
#endif

/*******************************************************************************
* 
*******************************************************************************/
#define SENSE_LEN					14 // Default sense buffer length

/*******************************************************************************
* SCSI I/O requests are done via a SRB (SCSI Request Block) 
* 
*******************************************************************************/
//The top of every SRB Structure has the same fields:
typedef struct 	__attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// Command code				(input)
	BYTE	SRB_Status;				// Status of the command	(output)
	BYTE	SRB_HaId;				// Host adapter number		(input)
	BYTE	SRB_Flags;				// SCSI request flags		(input)
	DWORD	SRB_Hdr_Rsvd;			// Reserved (only used by DOS for SC_HA_INQUIRY - in & output)
} SRB_Header, *LPSRB; //100% compatible with aspidos

//SRB_Header-SRB_Cmd
#define SC_HA_INQUIRY				0x00 // Queries ASPI for information on specific host adapters		(synchronous)
#define SC_GET_DEV_TYPE				0x01 // Requests the SCSI device type for a specific SCSI target	(synchronous)
#define SC_EXEC_SCSI_CMD			0x02 // Sends a SCSI command (arbitrary CDB) to a SCSI target		(asynchronous)
#define SC_ABORT_SRB				0x03 // Requests that ASPI cancel a previously submitted request	(synchronous)
#define SC_RESET_DEV				0x04 // Sends a BUS DEVICE RESET message to a SCSI target			(asynchronous)
#define SC_SET_HA_PARMS				0x05 // Set HA parameters 											(synchronous)
#define SC_GET_DISK_INFO			0x06 // Returns BIOS information for a SCSI target (Win95 only)		(synchronous)
//all the above commands are 100% compatible with aspidos - the following ones were "reserved for future expansion":
#define SC_RESCAN_SCSI_BUS			0x07 // Requests a rescan of a host adapter's SCSI bus				(synchronous)
#define SC_GETSET_TIMEOUTS			0x08 // Sets SRB timeouts for specific SCSI targets					(synchronous)

typedef struct SRB_CODE_DESC{
	char  code;
	char* desc;
}SRB_CODE_DESC;

#define SRB_COMMAND_CODE_MAX 9
SRB_CODE_DESC srbCommandCodeDesc[SRB_COMMAND_CODE_MAX] = {
 { SC_HA_INQUIRY,				"SC_HA_INQUIRY"},      //0x00
 { SC_GET_DEV_TYPE,				"SC_GET_DEV_TYPE"},    //0x01
 { SC_EXEC_SCSI_CMD, 			"SC_EXEC_SCSI_CMD"},   //0x02
 { SC_ABORT_SRB, 				"SC_ABORT_SRB"},       //0x03
 { SC_RESET_DEV, 				"SC_RESET_DEV"},       //0x04
 { SC_SET_HA_PARMS,				"SC_SET_HA_PARMS"},	   //0x05	//Note: only in aspidos, NOT in aspi32 !
 { SC_GET_DISK_INFO,			"SC_GET_DISK_INFO"},   //0x06
 { SC_RESCAN_SCSI_BUS,			"SC_RESCAN_SCSI_BUS"}, //0x07
 { SC_GETSET_TIMEOUTS,			"SC_GETSET_TIMEOUTS"}, //0x08
};

//SRB_Header-SRB_Status
#define SS_PENDING					0x00 // SRB being processed
#define SS_COMP						0x01 // SRB completed without error
#define SS_ABORTED					0x02 // SRB aborted by host
#define SS_ABORT_FAIL				0x03 // Unable to abort SRB			//NOT present in aspidos !
#define SS_ERR						0x04 // SRB completed with error
#define SS_INVALID_CMD				0x80 // Invalid SCSI Request
#define SS_INVALID_HA				0x81 // Invalid host adapter number
#define SS_NO_DEVICE				0x82 // SCSI device not installed
//all the above commands are 100% compatible with aspidos - the following ones were "reserved for future expansion":
#define SS_INVALID_SRB				0xE0 // Invalid parameter set in SRB
#define SS_OLD_MANAGER				0xE1 // ASPI manager doesn't support Windows
#define SS_BUFFER_ALIGN				0xE1 // Buffer not aligned (replaces OLD_MANAGER in Win32)
#define SS_ILLEGAL_MODE				0xE2 // Unsupported Windows mode
#define SS_NO_ASPI					0xE3 // No ASPI managers resident
#define SS_FAILED_INIT				0xE4 // ASPI for windows failed init
#define SS_ASPI_IS_BUSY				0xE5 // No resources available to execute cmd
#define SS_BUFFER_TO_BIG			0xE6 // Buffer size to big to handle!
#define SS_MISMATCHED_COMPONENTS	0xE7 // The DLLs/EXEs of ASPI don't version check
#define SS_NO_ADAPTERS				0xE8 // No host adapters to manage
#define SS_INSUFFICIENT_RESOURCES	0xE9 // Couldn't allocate resources needed to init
#define SS_ASPI_IS_SHUTDOWN			0xEA // Call came to ASPI after PROCESS_DETACH
#define SS_BAD_INSTALL				0xEB // The DLL or other components are installed wrong

#define SRB_STATUS_CODE_MAX 21
SRB_CODE_DESC srbStatusCodeDesc[SRB_STATUS_CODE_MAX] = {
 { SS_PENDING,					"SS_PENDING"},					//0x00
 { SS_COMP,						"SS_COMP"},						//0x01
 { SS_ABORTED,					"SS_ABORTED"},					//0x02
 { SS_ABORT_FAIL,				"SS_ABORT_FAIL"},				//0x03
 { SS_ERR,						"SS_ERR"},						//0x04
 { SS_INVALID_CMD,				"SS_INVALID_CMD"},				//0x80
 { SS_INVALID_HA,				"SS_INVALID_HA"},				//0x81
 { SS_NO_DEVICE,				"SS_NO_DEVICE"},				//0x82
 { SS_INVALID_SRB,				"SS_INVALID_SRB"},				//0xE0
 { SS_OLD_MANAGER,				"SS_OLD_MANAGER"},				//0xE1
 { SS_BUFFER_ALIGN,				"SS_BUFFER_ALIGN"},				//0xE1
 { SS_ILLEGAL_MODE,				"SS_ILLEGAL_MODE"},				//0xE2
 { SS_NO_ASPI,					"SS_NO_ASPI"},					//0xE3
 { SS_FAILED_INIT,				"SS_FAILED_INIT"},				//0xE4
 { SS_ASPI_IS_BUSY,				"SS_ASPI_IS_BUSY"},				//0xE5
 { SS_BUFFER_TO_BIG,			"SS_BUFFER_TO_BIG"},			//0xE6
 { SS_MISMATCHED_COMPONENTS,	"SS_MISMATCHED_COMPONENTS"},	//0xE7
 { SS_NO_ADAPTERS,				"SS_NO_ADAPTERS"},				//0xE8
 { SS_INSUFFICIENT_RESOURCES,	"SS_INSUFFICIENT_RESOURCES"},	//0xE9
 { SS_ASPI_IS_SHUTDOWN,			"SS_ASPI_IS_SHUTDOWN"},			//0xEA
 { SS_BAD_INSTALL,				"SS_BAD_INSTALL"},				//0xEB
};

/*******************************************************************************
* full SRB command structures per SRB Command Code
* 
*******************************************************************************/
///////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) { //60 bytes
	BYTE	SRB_Cmd;				// SC_HA_INQUIRY
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// must be 0
	WORD	SRB_Ext_Req_Sig;		// Extended Request Signature (not in wnaspi?)		(in- & output)
	WORD	SRB_Ext_Buf_Len;		// Extended Buffer Length (not in wnaspi?)			(in- & output)
	BYTE	HA_Count;				// Number of host adapters present					(output)
	BYTE	HA_SCSI_ID;				// SCSI ID of host adapter							(output)
	BYTE	HA_ManagerId[16];		// String describing the manager					(output)
	BYTE	HA_Identifier[16];		// String describing the host adapter				(output)
	BYTE	HA_Unique[16];			// Host Adapter Unique parameters					(output)
/*WIN:
Host adapter unique parameters as follows.
Size Offset Description
WORD 0 Buffer alignment mask. The host adapter requires data buffer alignmentspecified by this 16-bit value. A value of 0x0000 indicates no boundary requirements (e.g. byte alignment), 0x0001 indicates word alignment,0x0003 indicates double-word, 0x0007 indicates 8-byte alignment, etc.The 16-bit value allows data buffer alignments of up to 65536-byteboundaries. Alignment of buffers can be tested by logical ANDing (‘&’ in‘C’) this mask with the buffer address. If the result is 0 the buffer isproperly aligned.
BYTE 2 Residual byte count. Set to 0x01 if residual byte counting is supported,0x00 if not. See “Remarks” below for more information.
BYTE 3 Maximum SCSI targets. Indicates the maximum number of targets (SCSI IDs) the adapter supports. If this value is not set to 8 or 16, then it shouldbe assumed by the application that the maximum target count is 8.
DWORD 4 Maximum transfer length. DWORD count indicating the maximum transfer size the host adapter supports. If this number is less than 64KB then the application should assume a maximum transfer count of 64KB.*/
	WORD	HA_Sup_Ext;				// Supported Extensions (reserved field in wnaspi)	(output)
/*DOS: 
                      Bit 15-4  Reserved
                      Bit 3     0 = Not a Wide SCSI 32-bit host adapter
                                1 = Wide SCSI 32-bit host adapter
                      Bit 2     0 = Not a Wide SCSI 16-bit host adapter
                                1 = Wide SCSI 16-bit host adapter
                      Bit 1     0 = Residual byte length not reported
                                1 = Residual byte length reported. See
                                    section on Residual Byte Length below.
                      Bit 0     Reserved
*/
} SRB_HAInquiry, *PSRB_HAInquiry; //100% compatible with aspidos

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_GET_DEV_TYPE
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// must be 0
	DWORD	SRB_Hdr_Rsvd;
	BYTE	SRB_Target;				// Target's SCSI ID						(input)
	BYTE	SRB_Lun;				// Target's LUN number					(input)
	BYTE	SRB_DeviceType;			// Target's peripheral device type		(output)
	BYTE	SRB_Rsvd1;				// Reserved, MUST = 0
} SRB_GDEVBlock, *PSRB_GDEVBlock; // NOT compatible with aspidos !! -> cfr hereunder

typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_GET_DEV_TYPE
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// must be 0
	DWORD	SRB_Hdr_Rsvd;
	BYTE	SRB_Target;				// Target's SCSI ID						(input)
	BYTE	SRB_Lun;				// Target's LUN number					(input)
	BYTE	SRB_DeviceType;			// Target's peripheral device type		(output)
} SRB_GDEVBlock_DOS, *PSRB_GDEVBlock_DOS;

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_EXEC_SCSI_CMD
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// flags can be set
	DWORD	SRB_Hdr_Rsvd;

	BYTE	SRB_Target;				// Target's SCSI ID				(input)
	BYTE	SRB_Lun;				// Target's LUN number			(input)
	WORD	SRB_Rsvd1;				// Reserved for Alignment
	DWORD	SRB_BufLen;				// Data Buffer Length			(input)
	
	LPBYTE	SRB_BufPointer;			// Data Buffer Pointer			(input)
	BYTE	SRB_SenseLen;			// Sense Allocation Length		(input)
	BYTE	SRB_CDBLen;				// CDB Length					(input - This value is typically 6, 10, or 12)
	BYTE	SRB_HaStat;				// Host Adapter Status			(output - only if there was an error, this value should be checked by the client app)
	BYTE	SRB_TargStat;			// Target Status				(output - only if there was an error, this value should be checked by the client app)
	
	LPVOID	SRB_PostProc;			// Post routine					(input - used for posting or event notification)
	BYTE	SRB_Rsvd2[20];			// Reserved, MUST = 0
	BYTE	CDBByte[16];			// SCSI CDB						(input)
	BYTE	SenseArea[SENSE_LEN+2];	// Request Sense buffer			(output - if status = error, contains sense data to be analysed by client app)
} SRB_ExecSCSICmd, *PSRB_ExecSCSICmd; //aspidos version hereunder, quite different

typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_EXEC_SCSI_CMD
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// flags can be set
	DWORD	SRB_Hdr_Rsvd;

	BYTE	SRB_Target;				// Target's SCSI ID				(input)
	BYTE	SRB_Lun;				// Target's LUN number			(input)
	DWORD	SRB_BufLen;				// Data Buffer Length			(input)
	BYTE	SRB_SenseLen;			// Sense Allocation Length		(input)
	WORD	SRB_BufPointer_Offset;	// Data Buffer Pointer (Offset) (input)
	WORD	SRB_BufPointer_Segment;	// Data Buffer Pointer (Segment)(input)	
	WORD	SRB_Link_Pointer_Offset;// SRB Link Pointer (Offset)(pointer to the next SRB in a chain)
	WORD	SRB_Link_Pointer_Segment;// SRB Link Pointer (Segment)(pointer to the next SRB in a chain)
	BYTE	SRB_CDBLen;				// CDB Length					(input - This value is typically 6, 10, or 12)
	BYTE	SRB_HaStat;				// Host Adapter Status			(output - only if there was an error, this value should be checked by the client app)
	BYTE	SRB_TargStat;			// Target Status				(output - only if there was an error, this value should be checked by the client app)
	WORD	SRB_PostProc_Offset;	// Post routine	(Offset)				(input - used for posting or event notification)
	WORD	SRB_PostProc_Segment;	// Post routine (Segment)				(input - used for posting or event notification)
	BYTE	SRB_Rsvd2[34];			// Reserved
	BYTE	CDBAndSenseBytes[1];	// SCSI CDB - dynamic length, based on 	SRB_CDBLen	(input), followed by sense bytes
} SRB_ExecSCSICmd_DOS, *PSRB_ExecSCSICmd_DOS;

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_ABORT_SRB
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// must be 0
	DWORD	SRB_Hdr_Rsvd;
	LPSRB	SRB_ToAbort;			// Pointer to SRB to abort		(input) NOTE-TODO: if this is compiled as a 64bit app, not sure it will work, since it is supposed to be a 4 byte pointer
} SRB_Abort, *PSRB_Abort; //aspidos version hereunder - same size, but last ptr is split in 2

typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_ABORT_SRB
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// must be 0
	DWORD	SRB_Hdr_Rsvd;
	WORD	SRB_ToAbort_Offset;		// Pointer to SRB to abort (Offset)	 (input)
	WORD	SRB_ToAbort_Segment;	// Pointer to SRB to abort (Segment) (input)
} SRB_Abort_DOS, *PSRB_Abort_DOS;

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_RESET_DEV
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// Win: must be 0. DOS: Posting flag supported
	DWORD 	SRB_Hdr_Rsvd;
	
	BYTE	SRB_Target;				// Target's SCSI ID				(input)
	BYTE	SRB_Lun;				// Target's LUN number			(input)
	BYTE	SRB_Rsvd1[12];			// Reserved, MUST = 0
	BYTE	SRB_HaStat;				// Host Adapter Status			(output)
	BYTE	SRB_TargStat;			// Target Status				(output)
	LPVOID	SRB_PostProc;			// Post routine					(input)
	BYTE	SRB_Rsvd2[36];			// Reserved, MUST = 0
} SRB_BusDeviceReset, *PSRB_BusDeviceReset; // NOT compatible with aspidos, cfr hereunder

typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_RESET_DEV
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// Win: must be 0. DOS: Posting flag supported
	DWORD 	SRB_Hdr_Rsvd;

	BYTE	SRB_Target;				// Target's SCSI ID				(input)
	BYTE	SRB_Lun;				// Target's LUN number			(input)
	BYTE	SRB_Rsvd1[14];			// Reserved, MUST = 0					//NOTE: this is 14, win32 version is 12 !
	BYTE	SRB_HaStat;				// Host Adapter Status			(output)
	BYTE	SRB_TargStat;			// Target Status				(output)
	WORD	SRB_PostProc_Offset;	// Post routine address (Offset)(input)
	WORD	SRB_PostProc_Segment;	// Post routine address (Segment)(input)
	BYTE	SRB_Rsvd2[2];			// Reserved, MUST = 0
} SRB_BusDeviceReset_DOS, *PSRB_BusDeviceReset_DOS;

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_SET_HA_PARMS
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// unknown
	DWORD	SRB_Hdr_Rsvd;
	BYTE    HA_UniqueParams[16];	// Host Adapter Unique Parameters
} SRB_SetHostAdapterParams_DOS, *PSRB_SetHostAdapterParams_DOS; //Note: only in aspidos, NOT in aspi32 !

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_GET_DISK_INFO
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// must be 0
	DWORD	SRB_Hdr_Rsvd;
	BYTE	SRB_Target;				// Target's SCSI ID
	BYTE	SRB_Lun;				// Target's LUN number
	BYTE	SRB_DriveFlags;			// Drive flags
	BYTE	SRB_Int13HDriveInfo;	// Int 13 drive nr (Valid Int 13 drive numbers range for 00-FFh.)
	BYTE	SRB_Heads;				// Preferred number of heads translation
	BYTE	SRB_Sectors;			// Preferred number of sectors translation
	BYTE	SRB_Rsvd1[10];			// Reserved
} SRB_GetDiskInfo, *PSRB_GetDiskInfo; //100% compatible with aspidos

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_RESCAN_SCSI_BUS
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// must be 0
	DWORD	SRB_Hdr_Rsvd;
} SRB_RescanPort, *PSRB_RescanPort;// NOT in aspidos - this cmd code was "reserved for future expansion"

////////////////////////////////////////////////////////////////////////////////
typedef struct __attribute__ ((__packed__)) {
	BYTE	SRB_Cmd;				// SC_GETSET_TIMEOUTS
	BYTE	SRB_Status;
	BYTE	SRB_HaId;
	BYTE	SRB_Flags;				// direction IN = GET TIMEOUT, direction OUT = SET TIMEOUT
	DWORD	SRB_Hdr_Rsvd;
	BYTE	SRB_Target;				// Target's SCSI ID
	BYTE	SRB_Lun;				// Target's LUN number
	DWORD	SRB_Timeout;			// Timeout in half seconds
} SRB_GetSetTimeouts, *PSRB_GetSetTimeouts;// NOT in aspidos - this cmd code was "reserved for future expansion"

/*******************************************************************************
* Function pointer types used by the SRB dispatcher
*******************************************************************************/
/*typedef DWORD (*PFNHAINQUIRY)(PSRB_HAInquiry);
typedef DWORD (*PFNGETDEVICETYPE)(PSRB_GDEVBlock);
typedef DWORD (*PFNEXECSCSICMD)(PSRB_ExecSCSICmd);
typedef DWORD (*PFNABORTSRB)(PSRB_Abort);
typedef DWORD (*PFNRESETDEVICE)(PSRB_BusDeviceReset);
typedef DWORD (*PFNSETHAPARAMETERS)(PSRB_SetHostAdapterParams_DOS);
typedef DWORD (*PFNGETDISKINFO)(PSRB_GetDiskInfo);
typedef DWORD (*PFNRESCANSCSIBUS)(PSRB_RescanPort);
typedef DWORD (*PFNGETSETTIMEOUTS)(PSRB_GetSetTimeouts);*/

typedef DWORD (*PFNSRBDISPATCH)(LPSRB, int);
typedef VOID (*PFNSRBCALLBACK)(LPSRB);
typedef DWORD (*PFNSRBTHREADSTART)(LPSRB);

/*******************************************************************************
* 
*******************************************************************************/
//SRB_HaStat - Host Adapter Status - used in SC_EXEC_SCSI_CMD & SC_RESET_DEV
#define HASTAT_OK					0x00	// Host adapter did not detect an error
#define HASTAT_TIMEOUT				0x09	// The time allocated for a bus transaction ran out
#define HASTAT_COMMAND_TIMEOUT		0x0B	// SRB expired while waiting to be processed
#define HASTAT_MESSAGE_REJECT		0x0D	// MESSAGE REJECT received while processing SRB
#define HASTAT_BUS_RESET			0x0E	// A bus reset was detected
#define HASTAT_PARITY_ERROR			0x0F	// A parity error was detected
#define HASTAT_REQUEST_SENSE_FAILED	0x10	// The adapter failed in issuing a Request Sense after a check condition was reported by the target device
#define HASTAT_SEL_TO				0x11	// Selection of target timed out
#define HASTAT_DO_DU				0x12	// Data overrun/underrun
#define HASTAT_BUS_FREE				0x13	// Unexpected Bus Free
#define HASTAT_PHASE_ERR			0x14	// Target Bus phase sequence failure

//SRB_TargStat - Target Status - used in SC_EXEC_SCSI_CMD & SC_RESET_DEV
#define STATUS_GOOD					0x00	// No target status
#define STATUS_CHKCOND				0x02	// Check status (sense data is in SenseArea)
#define STATUS_BUSY					0x08	// Specified Target/LUN is busy
#define STATUS_RESCONF				0x18	// Reservation conflict
	
//SRB_DriveFlags - Drive flags - used in SC_GET_DISK_INFO
#define DISK_NOT_INT13				0x00	// Device is not controlled by Int 13h services. SRB_Int13HDriveInfo is not valid
#define DISK_INT13_AND_DOS			0x01	// Device is under Int 13h control and is claimed by DOS. SRB_Int13HDriveInfo contains valid info
#define DISK_INT13					0x02	// Device is under Int 13h control but not claimed by DOS. SRB_Int13HDriveInfo contains valid info

//SRB_Flags - used in SC_EXEC_SCSI_CMD, SC_RESET_DEV, SC_GETSET_TIMEOUTS
#define SRB_POSTING					0x01	// Enable posting. This flag and SRB_EVENT_NOTIFY are mutually exclusive.
#define SRB_LINKING					0x02	// Linking enabled. This bit should be cleared if scatter/gather is enabled.
#define SRB_ENABLE_RESIDUAL_COUNT	0x04	// Enables residual byte counting assuming it is supported.
#define SRB_DIR_IN					0x08	// Data transfer is from SCSI target to host. Mutually exclusive with SRB_DIR_OUT.
#define SRB_DIR_OUT					0x10	// Data transfer is from host to SCSI target. Mutually exclusive with SRB_DIR_IN.
		//note: if nor SRB_DIR_IN or SRB_DIR_OUT are specified, the Transfer direction determined by SCSI command
#define SRB_EVENT_NOTIFY			0x40	// Enable event notification. This flag and SRB_POSTING are mutually exclusive.

/*******************************************************************************
* GetASPI32Buffer - FreeASPI32Buffer related stuff
*******************************************************************************/
typedef struct __attribute__ ((__packed__)) {
	LPBYTE	AB_BufPointer;	// pointer to the buffer allocated by ASPI		(output)
	DWORD	AB_BufLen;		// desired size in bytes						(input, must be <= 512KB, should be >64KB)
	DWORD	AB_ZeroFill;	// set to 1 if buffer should be zeroed			(input)
	DWORD	AB_Reserved;	// must be 0									(input)
} ASPI32BUFF, *PASPI32BUFF;



/*******************************************************************************
* Function Declarations
*******************************************************************************/
DWORD srbGetSize (LPSRB pSRB, int isWin32Call);
char* srbGetCommandDesc (char command);
char* srbGetStatusDesc (char status);

#endif