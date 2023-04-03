/*******************************************************************************
* SCSI definitions & declarations
*   
*******************************************************************************/
#ifndef __SCSI_H_DEFINED__
#define __SCSI_H_DEFINED__

#if defined( __linux__ ) 
	#include "sgi.h" //SCSI generic (sg) driver (Linux)
#else //Windows _WIN32 _WIN64
	#include "spti.h" //SCSI Pass Through Interface via DeviceIoControl
#endif

#if (defined _MSC_VER) || (defined __MINGW32__)
	typedef unsigned __int32 uint32_t;//for older VS versions, including stdint.h doesn't work...
	typedef unsigned __int8  uint8_t;//for older VS versions, including stdint.h doesn't work...
#else
	#include <stdint.h>
#endif

typedef struct SCSI_CODE_DESC{
	char  code;
	char* desc;
}SCSI_CODE_DESC;

/*******************************************************************************
* SCSI COMMANDS that apply to all device types - MANDATORY
*******************************************************************************/
/*#define SCSI_CMD_TEST_UNIT_READY				0x00
#define SCSI_CMD_REQUEST_SENSE					0x03	//also: device-specific implementation)
#define SCSI_CMD_INQUIRY						0x12
*/
/*******************************************************************************
* SCSI COMMANDS that apply to all device types - OPTIONAL
*******************************************************************************/
/*//#define SCSI_CMD_REPORT DEVICE IDENTIFIER		0xA3 / 0x05
//#define SCSI_CMD_SET_DEVICE_IDENTIFIER		0xA4 / 0x06
#define SCSI_CMD_RECEIVE_DIAGNOSTIC_RESULTS		0x1C
#define SCSI_CMD_READ_BUFFER					0x3C
#define SCSI_CMD_LOG_SELECT						0x4C
#define SCSI_CMD_LOG_SENSE						0x4D
#define SCSI_CMD_EXTENDED_COPY					0x83	
#define SCSI_CMD_RECEIVE_COPY_RESULTS			0x84
#define SCSI_CMD_REPORT_DEVICE_IDENTIFIER		0xA3 // 0x05
//#define SCSI_CMD_SET_DEVICE_IDENTIFIER		0xA4 / 0x06
*/
/*******************************************************************************
* SCSI COMMANDS that apply to all device types - device-specific implementation
*******************************************************************************/
/*#define SCSI_CMD_MODE_SELECT_6					0x15
#define SCSI_CMD_RESERVE_6						0x16
#define SCSI_CMD_RELEASE_6						0x17
#define SCSI_CMD_MODE_SENSE_6					0x1A
#define SCSI_CMD_SEND_DIAGNOSTIC				0x1D
#define SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define SCSI_CMD_WRITE_BUFFER					0x3B
#define SCSI_CMD_MODE_SELECT_10					0x55
#define SCSI_CMD_RESERVE_10						0x56
#define SCSI_CMD_RELEASE_10						0x57
#define SCSI_CMD_MODE_SENSE_10					0x5A
#define SCSI_CMD_PERSISTENT_RESERVE_IN			0x5E
#define SCSI_CMD_PERSISTENT_RESERVE_OUT			0x5F
#define SCSI_CMD_MOVE_MEDIUM_ATTACHED			0xA7
#define SCSI_CMD_READ_ELEMENT_STATUS_ATTACHED	0xB4
*/
/*******************************************************************************
* SCSI COMMANDS that apply to all device types - Command implementation requirements given in reference subclause of this standard.
*******************************************************************************/
//#define SCSI_CMD_REPORT_LUNS					0xA0

/*******************************************************************************
* SCSI COMMANDS that apply to all device types - OBSOLETE in SPC-2
*******************************************************************************/
//Obsolete						0x18	
//Obsolete						0x39	
//Obsolete						0x3A	
//Obsolete						0x40	

#define SCSI_CMD_TEST_UNIT_READY				0x00
#define SCSI_CMD_REWIND 						0x01
#define SCSI_CMD_REQUEST_SENSE 					0x03
#define SCSI_CMD_FORMAT							0x04
#define SCSI_CMD_READ_BLOCK_LIMITS				0x05
#define SCSI_CMD_REASSIGN_BLOCKS				0x07 //TODO: also SCSI_CMD_INITIALIZE ELEMENT STATUS 0x07 ?
#define SCSI_CMD_READ_6							0x08
#define SCSI_CMD_WRITE_6						0x0A
#define SCSI_CMD_SEEK_6							0x0B
#define SCSI_CMD_READ_REVERSE_6					0x0F
#define SCSI_CMD_WRITE_FILEMARKS_6				0x10
#define SCSI_CMD_SPACE_6						0x11
#define SCSI_CMD_INQUIRY						0x12
#define SCSI_CMD_VERIFY_6						0x13
#define SCSI_CMD_RECOVER_BUFFERED_DATA			0x14
#define SCSI_CMD_MODE_SELECT_6					0x15
#define SCSI_CMD_RESERVE_6						0x16
#define SCSI_CMD_RELEASE_6						0x17
#define SCSI_CMD_COPY							0x18
#define SCSI_CMD_ERASE_6						0x19
#define SCSI_CMD_MODE_SENSE_6					0x1A
#define SCSI_CMD_START_STOP_UNIT 				0x1B //TODO: also ? #define SCSI_CMD_LOAD UNLOAD 0x1B
#define SCSI_CMD_RECEIVE_DIAGNOSTIC_RESULTS		0x1C
#define SCSI_CMD_SEND_DIAGNOSTIC 				0x1D
#define SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define SCSI_CMD_READ_FORMAT_CAPACITIES			0x23
#define SCSI_CMD_READ_CAPACITY_10				0x25
#define SCSI_CMD_READ_10						0x28
#define SCSI_CMD_READ_GENERATION				0x29
#define SCSI_CMD_WRITE_10						0x2A
#define SCSI_CMD_SEEK_10						0x2B
#define SCSI_CMD_LOCATE_10						0x2B
#define SCSI_CMD_ERASE_10						0x2C
#define SCSI_CMD_READ_UPDATED_BLOCK				0x2D
#define SCSI_CMD_WRITE_AND_VERIFY_10			0x2E
#define SCSI_CMD_VERIFY_10						0x2F
#define SCSI_CMD_SET_LIMITS_10					0x33
#define SCSI_CMD_PRE_FETCH_10					0x34 //TODO: ? also SCSI_CMD_READ POSITION 0x34
#define SCSI_CMD_SYNCHRONIZE_CACHE_10			0x35
#define SCSI_CMD_LOCK_UNLOCK_CACHE_10			0x36
#define SCSI_CMD_READ_DEFECT_DATA_10			0x37 //TODO ? also #define SCSI_CMD_INITIALIZE ELEMENT STATUS WITH RANGE 0x37
#define SCSI_CMD_MEDIUM_SCAN					0x38
#define SCSI_CMD_COMPARE						0x39
#define SCSI_CMD_COPY_AND_VERIFY				0x3A
#define SCSI_CMD_WRITE_BUFFER					0x3B
#define SCSI_CMD_READ_BUFFER					0x3C
#define SCSI_CMD_UPDATE_BLOCK					0x3D
#define SCSI_CMD_READ_LONG_10					0x3E
#define SCSI_CMD_WRITE_LONG_10					0x3F
#define SCSI_CMD_CHANGE_DEFINITION				0x40
#define SCSI_CMD_WRITE_SAME_10					0x41
#define SCSI_CMD_UNMAP							0x42
#define SCSI_CMD_READ_TOC_PMA_ATIP				0x43
#define SCSI_CMD_REPORT_DENSITY_SUPPORT			0x44
#define SCSI_CMD_PLAY_AUDIO_10					0x45
#define SCSI_CMD_GET_CONFIGURATION				0x46
#define SCSI_CMD_PLAY_AUDIO_MSF					0x47
#define SCSI_CMD_SANITIZE						0x48
#define SCSI_CMD_GET_EVENT_STATUS_NOTIFICATION	0x4A
#define SCSI_CMD_PAUSE_RESUME					0x4B
#define SCSI_CMD_LOG_SELECT						0x4C
#define SCSI_CMD_LOG_SENSE						0x4D
#define SCSI_CMD_XDWRITE_10						0x50
#define SCSI_CMD_XPWRITE_10						0x51 //TODO ? also #define SCSI_CMD_READ DISC INFORMATION 0x51
#define SCSI_CMD_XDREAD_10						0x52
#define SCSI_CMD_XDWRITEREAD_10					0x53
#define SCSI_CMD_SEND_OPC_INFORMATION			0x54
#define SCSI_CMD_MODE_SELECT_10					0x55
#define SCSI_CMD_RESERVE_10						0x56
#define SCSI_CMD_RELEASE_10						0x57
#define SCSI_CMD_REPAIR_TRACK					0x58
#define SCSI_CMD_MODE_SENSE_10					0x5A
#define SCSI_CMD_CLOSE_TRACK_SESSION			0x5B
#define SCSI_CMD_READ_BUFFER_CAPACITY			0x5C
#define SCSI_CMD_SEND_CUE_SHEET					0x5D
#define SCSI_CMD_PERSISTENT_RESERVE_IN			0x5E
#define SCSI_CMD_PERSISTENT_RESERVE_OUT			0x5F
#define SCSI_CMD_EXTENDED_CDB					0x7E
#define SCSI_CMD_VARIABLE_LENGTH_CDB			0x7F
#define SCSI_CMD_XDWRITE_EXTENDED_16			0x80 //TODO ? also #define SCSI_CMD_WRITE FILEMARKS_16 0x80
#define SCSI_CMD_READ_REVERSE_16				0x81
#define SCSI_CMD_THIRD_PARTY_COPY_OUT_COMMANDS	0x83
#define SCSI_CMD_THIRD_PARTY_COPY_IN_COMMANDS	0x84
#define SCSI_CMD_ATA_PASS_THROUGH_16			0x85
#define SCSI_CMD_ACCESS_CONTROL_IN				0x86
#define SCSI_CMD_ACCESS_CONTROL_OUT				0x87
#define SCSI_CMD_READ_16						0x88
#define SCSI_CMD_COMPARE_AND_WRITE				0x89
#define SCSI_CMD_WRITE_16						0x8A
#define SCSI_CMD_ORWRITE						0x8B
#define SCSI_CMD_READ_ATTRIBUTE					0x8C
#define SCSI_CMD_WRITE_ATTRIBUTE				0x8D
#define SCSI_CMD_WRITE_AND_VERIFY_16			0x8E
#define SCSI_CMD_VERIFY_16						0x8F
#define SCSI_CMD_PRE_FETCH_16					0x90
#define SCSI_CMD_SYNCHRONIZE_CACHE_16			0x91 //TODO ? also #define SCSI_CMD_SPACE_16 0x91
#define SCSI_CMD_LOCK_UNLOCK_CACHE_16			0x92 //TODO ? also #define SCSI_CMD_LOCATE_16 0x92
#define SCSI_CMD_WRITE_SAME_16					0x93 //TODO ? also #define SCSI_CMD_ERASE_16 0x93
#define SCSI_CMD_SERVICE_ACTION_BIDIRECTIONAL	0x9D
#define SCSI_CMD_SERVICE_ACTION_IN_16			0x9E
#define SCSI_CMD_SERVICE_ACTION_OUT_16			0x9F
#define SCSI_CMD_REPORT_LUNS					0xA0
#define SCSI_CMD_ATA_PASS_THROUGH_12			0xA1
#define SCSI_CMD_SECURITY_PROTOCOL_IN			0xA2
#define SCSI_CMD_MAINTENANCE_IN					0xA3
#define SCSI_CMD_MAINTENANCE_OUT				0xA4 // TODO ? also #define SCSI_CMD_REPORT KEY 0xA4
#define SCSI_CMD_MOVE_MEDIUM					0xA5 //TODO ? also #define SCSI_CMD_PLAY AUDIO 12 0xA5
#define SCSI_CMD_EXCHANGE_MEDIUM				0xA6
#define SCSI_CMD_MOVE_MEDIUM_ATTACHED			0xA7
#define SCSI_CMD_READ_12						0xA8
#define SCSI_CMD_SERVICE_ACTION_OUT_12			0xA9
#define SCSI_CMD_WRITE_12						0xAA
#define SCSI_CMD_SERVICE_ACTION_IN_12			0xAB
#define SCSI_CMD_ERASE_12						0xAC
#define SCSI_CMD_READ_DVD_STRUCTURE				0xAD
#define SCSI_CMD_WRITE_AND_VERIFY_12			0xAE
#define SCSI_CMD_VERIFY_12						0xAF
#define SCSI_CMD_SEARCH_DATA_HIGH_12			0xB0
#define SCSI_CMD_SEARCH_DATA_EQUAL_12			0xB1
#define SCSI_CMD_SEARCH_DATA_LOW_12				0xB2
#define SCSI_CMD_SET_LIMITS_12					0xB3
#define SCSI_CMD_READ_ELEMENT_STATUS_ATTACHED	0xB4
#define SCSI_CMD_SECURITY_PROTOCOL_OUT			0xB5
#define SCSI_CMD_SEND_VOLUME_TAG				0xB6
#define SCSI_CMD_READ_DEFECT_DATA_12			0xB7
#define SCSI_CMD_READ_ELEMENT_STATUS			0xB8
#define SCSI_CMD_READ_CD_MSF					0xB9
#define SCSI_CMD_REDUNDANCY_GROUP_IN			0xBA
#define SCSI_CMD_REDUNDANCY_GROUP_OUT			0xBB
#define SCSI_CMD_SPARE_IN						0xBC
#define SCSI_CMD_SPARE_OUT						0xBD
#define SCSI_CMD_VOLUME_SET_IN					0xBE
#define SCSI_CMD_VOLUME_SET_OUT					0xBF

#define SCSI_COMMAND_CODE_MAX 133
SCSI_CODE_DESC scsiCommandCodeDesc[SCSI_COMMAND_CODE_MAX] = {
 { SCSI_CMD_TEST_UNIT_READY,				"TEST_UNIT_READY"},//0x00
 { SCSI_CMD_REWIND,							"REWIND"},//0x01
 { SCSI_CMD_REQUEST_SENSE,					"REQUEST_SENSE"},//0x03
 { SCSI_CMD_FORMAT,							"FORMAT"},//0x04
 { SCSI_CMD_READ_BLOCK_LIMITS,				"READ_BLOCK_LIMITS"},//0x05
 { SCSI_CMD_REASSIGN_BLOCKS,				"REASSIGN_BLOCKS"},//0x07 //TODO: also SCSI_CMD_INITIALIZE ELEMENT STATUS 0x07 ?
 { SCSI_CMD_READ_6,							"READ_6"},//0x08
 { SCSI_CMD_WRITE_6,						"WRITE_6"},//0x0A
 { SCSI_CMD_SEEK_6,							"SEEK_6"},//0x0B
 { SCSI_CMD_READ_REVERSE_6,					"READ_REVERSE_6"},//0x0F
 { SCSI_CMD_WRITE_FILEMARKS_6,				"WRITE_FILEMARKS_6"},//0x10
 { SCSI_CMD_SPACE_6,						"SPACE_6"},//0x11
 { SCSI_CMD_INQUIRY,						"INQUIRY"},//0x12
 { SCSI_CMD_VERIFY_6,						"VERIFY_6"},//0x13
 { SCSI_CMD_RECOVER_BUFFERED_DATA,			"RECOVER_BUFFERED_DATA"},//0x14
 { SCSI_CMD_MODE_SELECT_6,					"MODE_SELECT_6"},//0x15
 { SCSI_CMD_RESERVE_6,						"RESERVE_6"},//0x16
 { SCSI_CMD_RELEASE_6,						"RELEASE_6"},//0x17
 { SCSI_CMD_COPY,							"COPY"},//0x18
 { SCSI_CMD_ERASE_6,						"ERASE_6"},//0x19
 { SCSI_CMD_MODE_SENSE_6,					"MODE_SENSE_6"},//0x1A
 { SCSI_CMD_START_STOP_UNIT,				"START_STOP_UNIT"},//0x1B //TODO: also ?  { SCSI_CMD_LOAD UNLOAD 0x1B
 { SCSI_CMD_RECEIVE_DIAGNOSTIC_RESULTS,		"RECEIVE_DIAGNOSTIC_RESULTS"},//0x1C
 { SCSI_CMD_SEND_DIAGNOSTIC,				"SEND_DIAGNOSTIC"},//0x1D
 { SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL,	"PREVENT_ALLOW_MEDIUM_REMOVAL"},//0x1E
 { SCSI_CMD_READ_FORMAT_CAPACITIES,			"READ_FORMAT_CAPACITIES"},//0x23
 { SCSI_CMD_READ_CAPACITY_10,				"READ_CAPACITY_10"},//0x25
 { SCSI_CMD_READ_10,						"READ_10"},//0x28
 { SCSI_CMD_READ_GENERATION,				"READ_GENERATION"},//0x29
 { SCSI_CMD_WRITE_10,						"WRITE_10"},//0x2A
 { SCSI_CMD_SEEK_10,						"SEEK_10"},//0x2B
 { SCSI_CMD_LOCATE_10,						"LOCATE_10"},//0x2B
 { SCSI_CMD_ERASE_10,						"ERASE_10"},//0x2C
 { SCSI_CMD_READ_UPDATED_BLOCK,				"READ_UPDATED_BLOCK"},//0x2D
 { SCSI_CMD_WRITE_AND_VERIFY_10,			"WRITE_AND_VERIFY_10"},//0x2E
 { SCSI_CMD_VERIFY_10,						"VERIFY_10"},//0x2F
 { SCSI_CMD_SET_LIMITS_10,					"SET_LIMITS_10"},//0x33
 { SCSI_CMD_PRE_FETCH_10,					"PRE_FETCH_10"},//0x34 //TODO: ? also SCSI_CMD_READ POSITION 0x34
 { SCSI_CMD_SYNCHRONIZE_CACHE_10,			"SYNCHRONIZE_CACHE_10"},//0x35
 { SCSI_CMD_LOCK_UNLOCK_CACHE_10,			"LOCK_UNLOCK_CACHE_10"},//0x36
 { SCSI_CMD_READ_DEFECT_DATA_10,			"READ_DEFECT_DATA_10"},//0x37 //TODO ? also  { SCSI_CMD_INITIALIZE ELEMENT STATUS WITH RANGE 0x37
 { SCSI_CMD_MEDIUM_SCAN,					"MEDIUM_SCAN"},//0x38
 { SCSI_CMD_COMPARE,						"COMPARE"},//0x39
 { SCSI_CMD_COPY_AND_VERIFY,				"COPY_AND_VERIFY"},//0x3A
 { SCSI_CMD_WRITE_BUFFER,					"WRITE_BUFFER"},//0x3B
 { SCSI_CMD_READ_BUFFER,					"READ_BUFFER"},//0x3C
 { SCSI_CMD_UPDATE_BLOCK,					"UPDATE_BLOCK"},//0x3D
 { SCSI_CMD_READ_LONG_10,					"READ_LONG_10"},//0x3E
 { SCSI_CMD_WRITE_LONG_10,					"WRITE_LONG_10"},//0x3F
 { SCSI_CMD_CHANGE_DEFINITION,				"CHANGE_DEFINITION"},//0x40
 { SCSI_CMD_WRITE_SAME_10,					"WRITE_SAME_10"},//0x41
 { SCSI_CMD_UNMAP,							"UNMAP"},//0x42
 { SCSI_CMD_READ_TOC_PMA_ATIP,				"READ_TOC_PMA_ATIP"},//0x43
 { SCSI_CMD_REPORT_DENSITY_SUPPORT,			"REPORT_DENSITY_SUPPORT"},//0x44
 { SCSI_CMD_PLAY_AUDIO_10,					"PLAY_AUDIO_10"},//0x45
 { SCSI_CMD_GET_CONFIGURATION,				"GET_CONFIGURATION"},//0x46
 { SCSI_CMD_PLAY_AUDIO_MSF,					"PLAY_AUDIO_MSF"},//0x47
 { SCSI_CMD_SANITIZE,						"SANITIZE"},//0x48
 { SCSI_CMD_GET_EVENT_STATUS_NOTIFICATION,	"GET_EVENT_STATUS_NOTIFICATION"},//0x4A
 { SCSI_CMD_PAUSE_RESUME,					"PAUSE_RESUME"},//0x4B
 { SCSI_CMD_LOG_SELECT,						"LOG_SELECT"},//0x4C
 { SCSI_CMD_LOG_SENSE,						"LOG_SENSE"},//0x4D
 { SCSI_CMD_XDWRITE_10,						"XDWRITE_10"},//0x50
 { SCSI_CMD_XPWRITE_10,						"XPWRITE_10"},//0x51 //TODO ? also  { SCSI_CMD_READ DISC INFORMATION 0x51
 { SCSI_CMD_XDREAD_10,						"XDREAD_10"},//0x52
 { SCSI_CMD_XDWRITEREAD_10,					"XDWRITEREAD_10"},//0x53
 { SCSI_CMD_SEND_OPC_INFORMATION,			"SEND_OPC_INFORMATION"},//0x54
 { SCSI_CMD_MODE_SELECT_10,					"MODE_SELECT_10"},//0x55
 { SCSI_CMD_RESERVE_10,						"RESERVE_10"},//0x56
 { SCSI_CMD_RELEASE_10,						"RELEASE_10"},//0x57
 { SCSI_CMD_REPAIR_TRACK,					"REPAIR_TRACK"},//0x58
 { SCSI_CMD_MODE_SENSE_10,					"MODE_SENSE_10"},//0x5A
 { SCSI_CMD_CLOSE_TRACK_SESSION,			"CLOSE_TRACK_SESSION"},//0x5B
 { SCSI_CMD_READ_BUFFER_CAPACITY,			"READ_BUFFER_CAPACITY"},//0x5C
 { SCSI_CMD_SEND_CUE_SHEET,					"SEND_CUE_SHEET"},//0x5D
 { SCSI_CMD_PERSISTENT_RESERVE_IN,			"PERSISTENT_RESERVE_IN"},//0x5E
 { SCSI_CMD_PERSISTENT_RESERVE_OUT,			"PERSISTENT_RESERVE_OUT"},//0x5F
 { SCSI_CMD_EXTENDED_CDB,					"EXTENDED_CDB"},//0x7E
 { SCSI_CMD_VARIABLE_LENGTH_CDB,			"VARIABLE_LENGTH_CDB"},//0x7F
 { SCSI_CMD_XDWRITE_EXTENDED_16,			"XDWRITE_EXTENDED_16"},//0x80 //TODO ? also  { SCSI_CMD_WRITE FILEMARKS_16 0x80
 { SCSI_CMD_READ_REVERSE_16,				"READ_REVERSE_16"},//	0x81
 { SCSI_CMD_THIRD_PARTY_COPY_OUT_COMMANDS,	"THIRD_PARTY_COPY_OUT_COMMANDS"},//0x83
 { SCSI_CMD_THIRD_PARTY_COPY_IN_COMMANDS,	"THIRD_PARTY_COPY_IN_COMMANDS"},//0x84
 { SCSI_CMD_ATA_PASS_THROUGH_16,			"ATA_PASS_THROUGH_16"},//0x85
 { SCSI_CMD_ACCESS_CONTROL_IN,				"ACCESS_CONTROL_IN"},//0x86
 { SCSI_CMD_ACCESS_CONTROL_OUT,				"ACCESS_CONTROL_OUT"},//0x87
 { SCSI_CMD_READ_16,						"READ_16"},//0x88
 { SCSI_CMD_COMPARE_AND_WRITE,				"COMPARE_AND_WRITE"},//0x89
 { SCSI_CMD_WRITE_16,						"WRITE_16"},//0x8A
 { SCSI_CMD_ORWRITE,						"ORWRITE"},//0x8B
 { SCSI_CMD_READ_ATTRIBUTE,					"READ_ATTRIBUTE"},//0x8C
 { SCSI_CMD_WRITE_ATTRIBUTE,				"WRITE_ATTRIBUTE"},//0x8D
 { SCSI_CMD_WRITE_AND_VERIFY_16,			"WRITE_AND_VERIFY_16"},//0x8E
 { SCSI_CMD_VERIFY_16,						"VERIFY_16"},//0x8F
 { SCSI_CMD_PRE_FETCH_16,					"PRE_FETCH_16"},//0x90
 { SCSI_CMD_SYNCHRONIZE_CACHE_16,			"SYNCHRONIZE_CACHE_16"},//0x91 //TODO ? also  { SCSI_CMD_SPACE_16 0x91
 { SCSI_CMD_LOCK_UNLOCK_CACHE_16,			"LOCK_UNLOCK_CACHE_16"},//0x92 //TODO ? also  { SCSI_CMD_LOCATE_16 0x92
 { SCSI_CMD_WRITE_SAME_16,					"WRITE_SAME_16"},//0x93 //TODO ? also  { SCSI_CMD_ERASE_16 0x93
 { SCSI_CMD_SERVICE_ACTION_BIDIRECTIONAL,	"SERVICE_ACTION_BIDIRECTIONAL"},//0x9D
 { SCSI_CMD_SERVICE_ACTION_IN_16,			"SERVICE_ACTION_IN_16"},//0x9E
 { SCSI_CMD_SERVICE_ACTION_OUT_16,			"SERVICE_ACTION_OUT_16"},//0x9F
 { SCSI_CMD_REPORT_LUNS,					"REPORT_LUNS"},//0xA0
 { SCSI_CMD_ATA_PASS_THROUGH_12,			"ATA_PASS_THROUGH_12"},//	0xA1
 { SCSI_CMD_SECURITY_PROTOCOL_IN,			"SECURITY_PROTOCOL_IN"},//0xA2
 { SCSI_CMD_MAINTENANCE_IN,					"MAINTENANCE_IN"},//0xA3
 { SCSI_CMD_MAINTENANCE_OUT,				"MAINTENANCE_OUT"},//0xA4 // TODO ? also  { SCSI_CMD_REPORT KEY 0xA4
 { SCSI_CMD_MOVE_MEDIUM,					"MOVE_MEDIUM"},//0xA5 //TODO ? also  { SCSI_CMD_PLAY AUDIO 12 0xA5
 { SCSI_CMD_EXCHANGE_MEDIUM,				"EXCHANGE_MEDIUM"},//0xA6
 { SCSI_CMD_MOVE_MEDIUM_ATTACHED,			"MOVE_MEDIUM_ATTACHED"},//0xA7
 { SCSI_CMD_READ_12,						"READ_12"},//0xA8
 { SCSI_CMD_SERVICE_ACTION_OUT_12,			"SERVICE_ACTION_OUT_12"},//0xA9
 { SCSI_CMD_WRITE_12,						"WRITE_12"},//0xAA
 { SCSI_CMD_SERVICE_ACTION_IN_12,			"SERVICE_ACTION_IN_12"},//0xAB
 { SCSI_CMD_ERASE_12,						"ERASE_12"},//0xAC
 { SCSI_CMD_READ_DVD_STRUCTURE,				"READ_DVD_STRUCTURE"},//0xAD
 { SCSI_CMD_WRITE_AND_VERIFY_12,			"WRITE_AND_VERIFY_12"},//0xAE
 { SCSI_CMD_VERIFY_12,						"VERIFY_12"},//0xAF
 { SCSI_CMD_SEARCH_DATA_HIGH_12,			"SEARCH_DATA_HIGH_12"},//0xB0
 { SCSI_CMD_SEARCH_DATA_EQUAL_12,			"SEARCH_DATA_EQUAL_12"},//0xB1
 { SCSI_CMD_SEARCH_DATA_LOW_12,				"SEARCH_DATA_LOW_12"},//0xB2
 { SCSI_CMD_SET_LIMITS_12,					"SET_LIMITS_12"},//0xB3
 { SCSI_CMD_READ_ELEMENT_STATUS_ATTACHED,	"READ_ELEMENT_STATUS_ATTACHED"},//0xB4
 { SCSI_CMD_SECURITY_PROTOCOL_OUT,			"SECURITY_PROTOCOL_OUT"},//0xB5
 { SCSI_CMD_SEND_VOLUME_TAG,				"SEND_VOLUME_TAG"},//0xB6
 { SCSI_CMD_READ_DEFECT_DATA_12,			"READ_DEFECT_DATA_12"},//0xB7
 { SCSI_CMD_READ_ELEMENT_STATUS,			"READ_ELEMENT_STATUS"},//0xB8
 { SCSI_CMD_READ_CD_MSF,					"READ_CD_MSF"},//0xB9
 { SCSI_CMD_REDUNDANCY_GROUP_IN,			"REDUNDANCY_GROUP_IN"},//0xBA
 { SCSI_CMD_REDUNDANCY_GROUP_OUT,			"REDUNDANCY_GROUP_OUT"},//0xBB
 { SCSI_CMD_SPARE_IN,						"SPARE_IN"},//0xBC
 { SCSI_CMD_SPARE_OUT,						"SPARE_OUT"},//0xBD
 { SCSI_CMD_VOLUME_SET_IN,					"VOLUME_SET_IN"},//0xBE
 { SCSI_CMD_VOLUME_SET_OUT,					"VOLUME_SET_OUT"},//0xBF
};

/*******************************************************************************
* SCSI MASKED STATUS - cfr SAM-2 spec           = Target Status (match with to wnaspi32's HASTAT_* values)
* Status shall be sent from the device server to the application client
* whenever a command ends with a service response of TASK COMPLETE or LINKED COMMAND COMPLETE .
*******************************************************************************/
#define SCSI_STATUSCODE_GOOD                 0x00 // task was completed succesfully
#define SCSI_STATUSCODE_CHECK_CONDITION      0x02 // error trying to execute a scsi command (CA/ACA). Autosense data might be delivered. initiator usually then issues a SCSI Request Sense command in order to obtain a Key Code Qualifier (KCQ) from the target.
#define SCSI_STATUSCODE_CONDITION_MET        0x04 // successful completion of a Pre-fetch Command. 
#define SCSI_STATUSCODE_BUSY                 0x08 // logical unit is busy, temporarily unable to accept a command.
#define SCSI_STATUSCODE_INTERMEDIATE		 0x10 // (obsolete) returned for each successfully completed command in a series of linked commands (except the last command)
#define SCSI_STATUSCODE_INTERMEDIATE_CONDITION_MET  0x14	// returned for each successfully completed command in a series of linked commands (except the last command)
#define SCSI_STATUSCODE_RESERVATION_CONFLICT 0x18 // initiator port attempts to access a logical unit or an element of a logical unit in a way that conflicts with an existing reservation.
#define SCSI_STATUSCODE_COMMAND_TERMINATED   0x22 // (obsolete) target has to terminate the current I/O process because it received a Terminate I/O Process message.
#define SCSI_STATUSCODE_TASK_SET_FULL		 0x28 // logical unit lacks the resources to accept a received task
#define SCSI_STATUSCODE_ACA_ACTIVE			 0x30 // an auto-contingent allegiance condition has occurred.
#define SCSI_STATUSCODE_TASK_ABORTED		 0x40 // a task is aborted

#define SCSI_STATUS_CODE_MAX 11
SCSI_CODE_DESC scsiStatusCodeDesc[SCSI_STATUS_CODE_MAX] = {
 { SCSI_STATUSCODE_GOOD,                         "GOOD"}, // task was completed succesfully
 { SCSI_STATUSCODE_CHECK_CONDITION,              "CHECK CONDITION"}, // error trying to execute a scsi command (CA/ACA). Autosense data might be delivered. initiator usually then issues a SCSI Request Sense command in order to obtain a Key Code Qualifier (KCQ) from the target.
 { SCSI_STATUSCODE_CONDITION_MET,                "CONDITION MET"}, // successful completion of a Pre-fetch Command. 
 { SCSI_STATUSCODE_BUSY,                         "BUSY"}, // logical unit is busy, temporarily unable to accept a command.
 { SCSI_STATUSCODE_INTERMEDIATE,                 "INTERMEDIATE"}, // (obsolete) returned for each successfully completed command in a series of linked commands (except the last command)
 { SCSI_STATUSCODE_INTERMEDIATE_CONDITION_MET,   "INTERMEDIATE CONDITION MET"}, // returned for each successfully completed command in a series of linked commands (except the last command)
 { SCSI_STATUSCODE_RESERVATION_CONFLICT,         "RESERVATION CONFLICT"}, // initiator port attempts to access a logical unit or an element of a logical unit in a way that conflicts with an existing reservation.
 { SCSI_STATUSCODE_COMMAND_TERMINATED,           "COMMAND TERMINATED"}, // (obsolete) target has to terminate the current I/O process because it received a Terminate I/O Process message.
 { SCSI_STATUSCODE_TASK_SET_FULL,                "TASK SET FULL"}, // logical unit lacks the resources to accept a received task
 { SCSI_STATUSCODE_ACA_ACTIVE,                   "ACA ACTIVE"}, // an auto-contingent allegiance condition has occurred.
 { SCSI_STATUSCODE_TASK_ABORTED,                 "TASK ABORTED"}, // a task is aborted
};

/*******************************************************************************
* SCSI SENSE KEYS = 3rd byte of sense data, 4 rightmost bits
*******************************************************************************/
#define SCSI_SENSEKEY_NO_SENSE            0x00
#define SCSI_SENSEKEY_RECOVERED_ERROR     0x01
#define SCSI_SENSEKEY_NOT_READY           0x02
#define SCSI_SENSEKEY_MEDIUM_ERROR        0x03
#define SCSI_SENSEKEY_HARDWARE_ERROR      0x04
#define SCSI_SENSEKEY_ILLEGAL_REQUEST     0x05
#define SCSI_SENSEKEY_UNIT_ATTENTION      0x06
#define SCSI_SENSEKEY_DATA_PROTECT        0x07
#define SCSI_SENSEKEY_BLANK_CHECK         0x08
#define SCSI_SENSEKEY_VENDOR_SPECIFIC     0x09
#define SCSI_SENSEKEY_COPY_ABORTED        0x0a
#define SCSI_SENSEKEY_ABORTED_COMMAND     0x0b
#define SCSI_SENSEKEY_OBSOLETE            0x0c
#define SCSI_SENSEKEY_VOLUME_OVERFLOW     0x0d
#define SCSI_SENSEKEY_MISCOMPARE          0x0e
#define SCSI_SENSEKEY_RESERVED            0x0f

#define SCSI_SENSE_KEY_MAX 16
SCSI_CODE_DESC scsiSenseKeyDesc[SCSI_SENSE_KEY_MAX] = {
 { SCSI_SENSEKEY_NO_SENSE,			"NO_SENSE" },		//0x00
 { SCSI_SENSEKEY_RECOVERED_ERROR,	"RECOVERED_ERROR" },//0x01
 { SCSI_SENSEKEY_NOT_READY,			"NOT_READY" },		//0x02
 { SCSI_SENSEKEY_MEDIUM_ERROR,		"MEDIUM_ERROR" },	//0x03
 { SCSI_SENSEKEY_HARDWARE_ERROR,	"HARDWARE_ERROR" },	//0x04
 { SCSI_SENSEKEY_ILLEGAL_REQUEST,	"ILLEGAL_REQUEST" },//0x05
 { SCSI_SENSEKEY_UNIT_ATTENTION,	"UNIT_ATTENTION" },	//0x06
 { SCSI_SENSEKEY_DATA_PROTECT,		"DATA_PROTECT" },	//0x07
 { SCSI_SENSEKEY_BLANK_CHECK,		"BLANK_CHECK" },	//0x08
 { SCSI_SENSEKEY_VENDOR_SPECIFIC,	"VENDOR_SPECIFIC" },//0x09
 { SCSI_SENSEKEY_COPY_ABORTED,		"COPY_ABORTED" },	//0x0a
 { SCSI_SENSEKEY_ABORTED_COMMAND,	"ABORTED_COMMAND" },//0x0b
 { SCSI_SENSEKEY_OBSOLETE,			"OBSOLETE" },		//0x0c
 { SCSI_SENSEKEY_VOLUME_OVERFLOW,	"VOLUME_OVERFLOW" },//0x0d
 { SCSI_SENSEKEY_MISCOMPARE,		"MISCOMPARE" },		//0x0e
 { SCSI_SENSEKEY_RESERVED,			"RESERVED" },		//0x0f

};




//#include <stdint.h>
#include <stddef.h> //for NULL

typedef unsigned char  BYTE; // uint8_t; //#define BYTE char
typedef unsigned short WORD; // uint16_t; #define WORD short

/* Command Descriptor Blocks (CDB)*/

/*Typical CDB for 6-byte commands*/
typedef struct SCSI_CDB6 {
	char  operation_code;
	char  misc_and_MSB; //3 bits misc cdb info 	5 bits MSB
	short logical_block_address;// if required
	char  length; //transfer length, parameter list length or allocation length
	char  control;
} SCSI_CDB6;

/*Typical CDB for 10-byte commands*/
typedef struct SCSI_CDB10 {
	char  operation_code;
	char  misc_and_service_action; //3 bits misc cdb info 	5 bits service action (if required)
	int   logical_block_address;// if required  4 bytes
	char  misc_cdb_info;
	short length; //transfer length, parameter list length or allocation length
	char  control;
} SCSI_CDB10;

/*Typical CDB for 12-byte commands*/
typedef struct SCSI_CDB12 {
	char  operation_code;
	char  misc_and_service_action; //3 bits misc cdb info 	5 bits service action (if required)
	int   logical_block_address;// if required  4 bytes
	int   length; //transfer length, parameter list length or allocation length
	char  misc_cdb_info;
	char  control;
} SCSI_CDB12;

/*Typical CDB for 16-byte commands*/
typedef struct SCSI_CDB16 {
	char  operation_code;
	char  misc_cdb_info; //
	long  logical_block_address;// if required  8 bytes
	int   length; //transfer length, parameter list length or allocation length
	char  misc_cdb_info2;
	char  control;
} SCSI_CDB16;

/*TODO: variable length CDB*/


/*******************************************************************************
* SCSI_CMD_INQUIRY - VITAL PRODUCT DATA Pages
*******************************************************************************/
#define VPD_SUPPORTED_PAGES_LIST               0x00 //List of Supported VPD pages 5.4.18
#define VPD_ASCII_INFO_MIN                     0x01 //Ascii Information 5.4.2
#define VPD_ASCII_INFO_MAX                     0x7F //Ascii Information 5.4.2
#define VPD_UNIT_SERIAL_NUMBER                 0x80 //Unit Serial Number 5.4.19
#define VPD_DEVICE_IDENTIFICATION              0x83 //Device Identification 5.4.11
#define VPD_SOFTWARE_IDENTIFICATION            0x84 //Software Interface Identification n/s
#define VPD_MGMT_NETWORK_ADDRESSES             0x85 //Management Network Addresses n/s
#define VPD_EXTENDED_INQUIRY_DATA              0x86 //Extended INQUIRY Data 5.4.9
#define VPD_MODE_PAGE_POLICY                   0x87 //Mode Page Policy 5.4.14
#define VPD_SCSI_PORTS                         0x88 //SCSI Ports 5.4.17
#define VPD_POWER_CONDITION                    0x8A //Power Condition 5.4.15
#define VPD_DEVICE_CONSTITUENTS                0x8B //Device Constituents n/s
#define VPD_CFA_PROFILE_INFORMATION            0x8C //CFA Profile Information n/s
#define VPD_POWER_CONSUMPTION                  0x8D //Power Consumption 5.4.16
#define VPD_BLOCK_LIMITS                       0xB0 //Block Limits 5.4.5
#define VPD_BLOCK_DEVICE_CHARACTERISTICS       0xB1 //Block Device Characteristics 5.4.3
#define VPD_LOGICAL_BLOCK_PROVISIONING         0xB2 //Logical Block Provisioning 5.4.13
#define VPD_REFERRALS                          0xB3 //Referrals n/s
#define VPD_SUPPORTED_BLOCK_LENGTHS_AND_PROT   0xB4 //Supported Block Lengths and Protection Types n/s
#define VPD_BLOCK_DEVICE_CHARS_EXTENSION       0xB5 //Block Device Characteristics Extension 5.4.4
#define VPD_ZONED_BLOCK_DEVICE_CHARACTERISTICS 0xB6 //Zoned Block Device Characteristics 5.4.20
#define VPD_BLOCK_LIMITS_EXTENSION             0xB7 //Block Limits Extension 5.4.6
#define VPD_FIRMWARE_NUMBERS_PAGE              0xC0 //Firmware Numbers page 5.4.10
#define VPD_DATE_CODE_PAGE                     0xC1 //Date Code page 5.4.7
#define VPD_JUMPER_SETTINGS_PAGE               0xC2 //Jumper Settings page 5.4.12
#define VPD_DEVICE_BEHAVIOR PAGE               0xC3 //Device Behavior page 5.4.8

char* scsiGetCodeDesc (char code, SCSI_CODE_DESC* descArray, int max_entries, char* szDefaultDesc);
char* scsiGetCommandDesc (char command);
char* scsiGetStatusDesc (char status);
char* scsiGetSenseKeyDesc (char senseKey);





/*******************************************************************************
* SCSI Commands                        http://en.wikipedia.org/wiki/SCSI_command
*******************************************************************************/
//#define SCSI_CMD_TST_U_RDY             0x00 // Test Unit Ready (MANDATORY)
//#define SCSI_CMD_RECALIBRATE_REWIND  0x01 // RECALIBRATE or REWIND
//#define SCSI_CMD_REQ_SENSE           0x03 // REQUEST SENSE (mandatory except RBC)
//#define SCSI_CMD_FORMAT_UNIT         0x04 // FORMAT UNIT
//#define SCSI_CMD_READ_BLOCK_LIMITS   0x05 // READ BLOCK LIMITS
//#define SCSI_CMD_REASSIGN_BLOCKS     0x07 // REASSIGN BLOCKS
//#define SCSI_CMD_INIT_ELEMENT_STATUS 0x07 // INITIALIZE ELEMENT STATUS
//#define SCSI_CMD_READ6               0x08 // READ (6)
//#define SCSI_CMD_WRITE6              0x0A // WRITE (6)
//#define SCSI_CMD_SEEK6               0x0B // SEEK (6)
//#define SCSI_CMD_READ_REVERSE6       0x0F // READ REVERSE (6)
//#define SCSI_CMD_WRITE_FILEMARKS6    0x10 // WRITE FILEMARKS (6)
//#define SCSI_CMD_SPACE6              0x11 // SPACE (6)
//#define SCSI_CMD_INQUIRY               0x12 // Inquiry         (MANDATORY)
/*
13 	VERIFY(6)
14 	RECOVER BUFFERED DATA
15 	MODE SELECT (6)
16 	RESERVE (6)
17 	RELEASE (6)
18 	COPY
19 	ERASE (6)
1A 	MODE SENSE (6)
1B 	START/STOP UNIT
1B 	LOAD UNLOAD
1C 	RECEIVE DIAGNOSTIC RESULTS
1D 	SEND DIAGNOSTIC
1E 	PREVENT/ALLOW MEDIUM REMOVAL
23 	READ FORMAT CAPACITIES (MMC)
24 	SET WINDOW
25 	READ CAPACITY (10)
28 	READ (10)
29 	READ GENERATION
2A 	WRITE (10)
2B 	SEEK (10)
2C 	ERASE (10)
2D 	READ UPDATED BLOCK
2E 	WRITE AND VERIFY (10)
2F 	VERIFY (10)
30 	SEARCH DATA HIGH (10)
31 	SEARCH DATA EQUAL (10)
32 	SEARCH DATA LOW (10)
33 	SET LIMITS (10)
34 	PRE-FETCH (10)
35 	SYNCHRONIZE CACHE (10)
36 	LOCK/UNLOCK CACHE (10)
37 	READ DEFECT DATA (10)
37 	INITIALIZE ELEMENT STATUS WITH RANGE
38 	MEDIUM SCAN
39 	COMPARE
3A 	COPY AND VERIFY*/
//#define SCSI_CMD_WRITE_BUFF            0x3B // Write Buffer    (OPTIONAL)
//#define SCSI_CMD_READ_BUFF             0x3C // Read Buffer     (OPTIONAL)
/*
3D 	UPDATE BLOCK
3E 	READ LONG
3F 	WRITE LONG
40 	CHANGE DEFINITION
41 	WRITE SAME (10)
44 	REPORT DENSITY SUPPORT
45 	PLAY AUDIO (10)
46 	GET CONFIGURATION
47 	PLAY AUDIO MSF
48 	AUDIO TRACK INDEX (not mentioned in T10 overview)
49 	AUDIO TRACK RELATIVE 10 (not mentioned in T10 overview)
4A 	GET EVENT STATUS NOTIFICATION
4B 	PAUSE / RESUME
4C 	LOG SELECT
4D 	LOG SENSE
50 	XDWRITE (10)
51 	XPWRITE (10) //READ DISC INFORMATION CDB (in mmc5r02c 6.22.2)
52 	XDREAD (10)
53 	XDWRITEREAD (10)
54 	SEND OPC INFORMATION
55 	MODE SELECT (10)
56 	RESERVE (10)
57 	RELEASE (10)
58 	REPAIR TRACK
5A 	MODE SENSE (10)
5B 	CLOSE TRACK / SESSION
5C 	READ BUFFER CAPACITY
5D 	SEND CUE SHEET
5E 	PERSISTENT RESERVE IN
5F 	PERSISTENT RESERVE OUT
7E 	EXTENDED CDB
7F 	VARIABLE LENGTH CDB
80 	XDWRITE EXTENDED (16)
80 	WRITE FILEMARKS (16)
81 	REBUILD (16)
81 	READ REVERSE (16)
82 	REGENERATE (16)
83 	EXTENDED COPY
84 	RECEIVE COPY RESULTS
85 	ATA COMMAND PASS THROUGH (16)
86 	ACCESS CONTROL IN
87 	ACCESS CONTROL OUT
88 	READ (16)
89 	COMPARE AND WRITE
8A 	WRITE (16)
8B 	ORWRITE
8C 	READ ATTRIBUTE
8D 	WRITE ATTRIBUTE
8E 	WRITE AND VERIFY (16)
8F 	VERIFY (16)
90 	PRE-FETCH (16)
91 	SYNCHRONIZE CACHE (16)
91 	SPACE (16)
92 	LOCK UNLOCK CACHE (16)
93 	WRITE SAME (16)
9E 	SERVICE ACTION IN (16)
9F 	SERVICE ACTION OUT (16)
A0 	REPORT LUNS
A1 	ATA COMMAND PASS THROUGH (12)
A2 	SECURITY PROTOCOL IN
A2 	SEND EVENT (not mentioned in T10 overview)
A3 	MAINTENANCE IN
A4 	MAINTENANCE OUT (REPORT_KEY)
A5 	MOVE MEDIUM
A5 	PLAY AUDIO 12 (not mentioned in T10 overview)
A6 	EXCHANGE MEDIUM
A7 	MOVE MEDIUM ATTACHED
A8 	READ (12)
A9 	SERVICE ACTION OUT (12)
A9 	AUDIO TRACK RELATIVE 12 (not mentioned in T10 overview)
AA 	WRITE (12)
AB 	SERVICE ACTION IN (12)
AC 	ERASE (12)
AD 	READ DVD STRUCTURE
AE 	WRITE AND VERIFY (12)
AF 	VERIFY (12)
B0 	SEARCH DATA HIGH (12)
B1 	SEARCH DATA EQUAL (12)
B2 	SEARCH DATA LOW (12)
B3 	SET LIMITS (12)
B4 	READ ELEMENT STATUS ATTACHED
B5 	SECURITY PROTOCOL OUT
B6 	SEND VOLUME TAG
B7 	READ DEFECT DATA (12)
B8 	READ ELEMENT STATUS
B9 	READ CD MSF
BA 	REDUNDANCY GROUP (IN)
BB 	REDUNDANCY GROUP (OUT)
BC 	SPARE (IN)
BC 	PLAY CD (not mentioned in T10 overview)
BD 	SPARE (OUT)
BE 	VOLUME SET (IN)
BF 	VOLUME SET (OUT)
*/

//***************************************************************************
//                          %%% TARGET STATUS VALUES %%%
//***************************************************************************
/*#define SCSI_STATUS_GOOD     0x00    // Status Good
#define SCSI_STATUS_CHKCOND  0x02    // Check Condition
#define SCSI_STATUS_CONDMET  0x04    // Condition Met
#define SCSI_STATUS_BUSY     0x08    // Busy
#define SCSI_STATUS_INTERM   0x10    // Intermediate
#define SCSI_STATUS_INTCDMET 0x14    // Intermediate-condition met
#define SCSI_STATUS_RESCONF  0x18    // Reservation conflict
#define SCSI_STATUS_COMTERM  0x22    // Command Terminated
#define SCSI_STATUS_QFULL    0x28    // Queue full
*/


/*******************************************************************************
* Used for SCSI device enumeration & mapping
*******************************************************************************/
typedef struct {
  char			vendor[9];		//cfr SCSI INQUIRY
  char			model[17];		//cfr SCSI INQUIRY
  char			revision[5];	//cfr SCSI INQUIRY
  uint32_t		type;			//cfr SCSI INQUIRY

  uint32_t		hostId;			//the mapped one, not the physical one
  //TODO: add channel ?
  uint32_t		targetId;		//the mapped one, not the physical one
  uint32_t		lunId;			//the mapped one, not the physical one
  uint32_t      timeout;        //timeout value in millisecs
  uint32_t		async_delay;

  uint32_t		real_hostId;	//the physical one
  uint32_t		real_channelId; //the physical one
  uint32_t		real_targetId;	//the physical one
  uint32_t		real_lunId;		//the physical one
#if defined( __linux__ ) 
  int fd;       //used in the ioctl of sg
#else
  int hasDriveLetter; //Windows SPTI only - those are accessed with their own deviceOpenStr, not the deviceOpenStr of the host adapter
  HANDLE hand;
#endif
  char			deviceOpenStr[1024]; //OS-specific string used when opening a device for read/write access
  LOGGER_STRUCT logger;
} MAPPEDSCSIDEVICE;
typedef MAPPEDSCSIDEVICE* PMSDEV;

typedef struct {
	uint32_t num_devices;
	uint32_t num_devices_allocated;
	MAPPEDSCSIDEVICE  dev[1];
} MAPPEDSCSIDEVICELIST;

typedef MAPPEDSCSIDEVICELIST* PMSDEVLIST;

/*******************************************************************************
* Function declarations
*******************************************************************************/
char* scsiGetCommandDesc (char command);
char* scsiGetStatusDesc (char status);
char* scsiGetSenseKeyDesc (char senseKey);
//device mapping
PMSDEVLIST copyMappedSCSIDeviceToList (PMSDEV pDev, int numDevices);

//open-close devices, will call the OS-specific function
int scsiDeviceOpen   (PMSDEV pDev);
void scsiDeviceClose (PMSDEV pDev, int isAppShutdown);

//declared here, but defined per OS:
int scsiOpen (PMSDEV pDev);
void scsiClose (PMSDEV pDev, int isAppShutdown);
int scsiIsOpen (PMSDEV pDev);
PMSDEVLIST scsiGetDeviceList (void);
int scsiSend (PMSDEV pDev, uint8_t* pCDB, uint8_t cdbLength, void* pResultBuf, uint32_t resultBufLength, uint8_t senseLen, uint8_t* pSenseArea);
int scsiResetTarget (PMSDEV pDev);
int scsiRescan (uint32_t HaId);
int scsiGetTimeouts (PMSDEV pDev, uint32_t* pseconds);
int scsiSetTimeouts (PMSDEV pDev, uint32_t seconds);
PMSDEV scsiGetDeviceByMappedHTL (PMSDEVLIST pDevList, uint32_t hostId, uint32_t targetId, uint32_t lunId, int bOpenDevice);
PMSDEV scsiGetDeviceByVendorAndModel (PMSDEVLIST pDevList, PMSDEV pDev);
PMSDEV scsiGetDeviceByVendorAndModelStr (PMSDEVLIST pDevList, char* vendor, char* model);

#endif