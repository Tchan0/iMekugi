/*******************************************************************************
* SCSI Generic (sg) functions - Linux specific
*
*  Note that this is dependent of the following scripts
*   - lsscsi,          to be installed via  sudo apt install lsscsi
*   - rescan-scsi-bus 
*
* Note that these functions need to be run as administrator (sudo)
*******************************************************************************/
#include <stdio.h> //needed for popen()
#include <stdlib.h> //for calloc

#include "sgi.h"

#define SG_HOST_STATUS_ERR_STRING_MAX 12
SCSI_CODE_DESC sgHostStatusErrorDesc[SG_HOST_STATUS_ERR_STRING_MAX] = {
 { SG_ERR_DID_OK,          "NO error"},
 { SG_ERR_DID_NO_CONNECT,  "Couldn't connect before timeout period"},
 { SG_ERR_DID_BUS_BUSY,    "BUS stayed busy through time out period"},
 { SG_ERR_DID_TIME_OUT,    "TIMED OUT for other reason"},
 { SG_ERR_DID_BAD_TARGET,  "BAD target, device not responding?"},
 { SG_ERR_DID_ABORT,       "Told to abort for some other reason"},
 { SG_ERR_DID_PARITY,      "Parity error"},
 { SG_ERR_DID_ERROR,       "Internal error [DMA underrun on aic7xxx]"},
 { SG_ERR_DID_RESET,       "Reset by somebody."},
 { SG_ERR_DID_BAD_INTR,    "Got an interrupt we weren't expecting."},
 { SG_ERR_DID_PASSTHROUGH, "Force command past mid-layer"},
 { SG_ERR_DID_SOFT_ERROR,  "The low level driver wants a retry"}
};

#define SG_DRIVER_STATUS_ERR_STRING_MAX 14
SCSI_CODE_DESC sgDriverStatusErrorDesc[SG_DRIVER_STATUS_ERR_STRING_MAX] = {
 { SG_ERR_DRIVER_OK,      "OK - NO error"},
 { SG_ERR_DRIVER_BUSY,    "Busy"},
 { SG_ERR_DRIVER_SOFT,    "Soft"},
 { SG_ERR_DRIVER_MEDIA,   "Media"},
 { SG_ERR_DRIVER_ERROR,   "Error"},
 { SG_ERR_DRIVER_INVALID, "Invalid"},
 { SG_ERR_DRIVER_TIMEOUT, "Timeout"},
 { SG_ERR_DRIVER_HARD,    "Hard"},

 { SG_ERR_DRIVER_SENSE,   "Sense"}, /* Implies sense_buffer output. This is 'or'ed with one of the following suggestions */
 { SG_ERR_SUGGEST_RETRY,  "Retry"},
 { SG_ERR_SUGGEST_ABORT,  "Abort"},
 { SG_ERR_SUGGEST_REMAP,  "Remap"},
 { SG_ERR_SUGGEST_DIE,    "Die"},
 { SG_ERR_SUGGEST_SENSE,  "Sense"},

};

#define LSSCSI_KNOWN_DEVICE_TYPES_MAX 32
static const char * lsscsi_short_device_types[LSSCSI_KNOWN_DEVICE_TYPES_MAX] = {//Order is compatible with SCSI INQUIRY Peripheral device type
	"disk   ", "tape   ", "printer", "process", "worm   ", "cd/dvd ",
	"scanner", "optical", "mediumx", "comms  ", "(0xa)  ", "(0xb)  ",
	"storage", "enclosu", "sim dsk", "opti rd", "bridge ", "osd    ",
	"adi    ", "sec man", "zbc    ", "(0x15) ", "(0x16) ", "(0x17) ",
	"(0x18) ", "(0x19) ", "(0x1a) ", "(0x1b) ", "(0x1c) ", "(0x1e) ",
	"wlun   ", "no dev "};

/*******************************************************************************
* This method uses command "lsscsi -g" as an easy way to find SCSI devices & their SG mappings.
* Example output of this command:
	[1:0:0:0]    disk    ATA      Samsung SSD 850  2B6Q  /dev/sda   /dev/sg0 
	[2:0:0:0]    disk    ATA      WDC WD60EFRX-68L 0A82  /dev/sdb   /dev/sg1 
	[3:0:0:0]    disk    ATA      Samsung SSD 850  2B6Q  /dev/sdc   /dev/sg2 
	[4:0:0:0]    cd/dvd  PIONEER  BD-RW   BDR-208M 1.10  /dev/sr0   /dev/sg3 
	[10:0:6:0]   process SEGA OA  Saturn CartDev   C69   -          /dev/sg4 
*
* Note: not used: other possibilities to find the SCSI devices were:
*  		cat /proc/scsi/sg/device_strs
*		cat /proc/scsi/sg/device_hdr
*		cat /proc/scsi/sg/devices
*		/sys/bus/scsi/devices/10:0:6:0/scsi_generic contains a directory with the sg /dev name: /sg4
*******************************************************************************/
PMSDEVLIST scsiGetDeviceList (void)
{
 FILE* pf;
 char str[1024];
 uint32_t i = 0;
 uint32_t k;
 MAPPEDSCSIDEVICE devs[128];
 PMSDEVLIST pDevList;

 pf = popen("lsscsi -g", "r");
 if (!pf) return (NULL);

 while (fgets((char*)str, 1024, pf)){
	memcpy ((void*)devs[i].vendor,  (void*)&str[21], 8);  devs[i].vendor[8]   = 0x00;
	memcpy ((void*)devs[i].model,   (void*)&str[30], 16); devs[i].model[16]   = 0x00;
	memcpy ((void*)devs[i].revision,(void*)&str[47], 4);  devs[i].revision[4] = 0x00;

	for (k=19; k >= 13; k--){//trim trailing spaces
		if (str[k] != 0x20) break;
		str[k] = 0x00;
	}
	devs[i].type = 0x1F;//DTYPE_UNKNOWN
	for (k=0; k <= LSSCSI_KNOWN_DEVICE_TYPES_MAX; k++){
		if (!strcmp((char*)&lsscsi_short_device_types[k], (char*)&str[13])){
			devs[i].type = k;
			break;
		}
	}

	memcpy ((void*)devs[i].deviceOpenStr, (void*)&str[64], 9);
	if (devs[i].deviceOpenStr[8] == 0x20) devs[i].deviceOpenStr[8] = 0x00; else devs[i].deviceOpenStr[9] = 0x00;
	sscanf ((char*)&str[1], "%d:%d:%d:%d]", &devs[i].real_hostId, &devs[i].real_channelId, &devs[i].real_targetId, &devs[i].real_lunId);
	devs[i].hostId = devs[i].real_hostId;
	//devs[i].channelId = devs[i].real_channelId;
	devs[i].targetId = devs[i].real_targetId;
	devs[i].lunId = devs[i].real_lunId;
	devs[i].timeout = 20000; //in millisecs
	fprintf(stdout, "Real device detected (%d,%d,%d,%d): %s, %s, %s\n",
		devs[i].real_hostId, devs[i].real_channelId, devs[i].real_targetId, devs[i].real_lunId,
		devs[i].vendor, devs[i].model, devs[i].revision);
	i++;
 }

 if (!i) return (NULL);
 
 //copy over all found devices to an allocated structure that the client app can use & free after usage
 pDevList = copyMappedSCSIDeviceToList ((PMSDEV)&devs[0], i);

 fclose (pf);
 return (pDevList);
}

/*******************************************************************************
* 
*******************************************************************************/
char* sgGetDriverStatusDesc (char status)
{
 return (scsiGetCodeDesc(status, sgDriverStatusErrorDesc, SG_DRIVER_STATUS_ERR_STRING_MAX, "Driver status code unknown"));
}

/*******************************************************************************
* 
*******************************************************************************/
char* sgGetHostStatusDesc (char status)
{
 return (scsiGetCodeDesc(status, sgHostStatusErrorDesc, SG_HOST_STATUS_ERR_STRING_MAX, "Host status code unknown"));
}

/*******************************************************************************
* get time-out value
*******************************************************************************/
int scsiGetTimeouts (PMSDEV pDev, uint32_t* pseconds)
{
 /*if (!pDev || !pDev->fd || !pseconds) return (0);
 jiffies = ioctl (pDev->fd, SG_GET_TIMEOUT, NULL);
 if (jiffies < 0) return (0);
 			else  *pseconds = (uint32_t)(jiffies / 60);*/
 if (!pDev || !pseconds) return (0);

 *pseconds = pDev->timeout / 1000;//no real action needed, since the timeout is passed with every deviceio command

 return 1;
}

/*******************************************************************************
* set time-out value - cfr https://elixir.bootlin.com/linux/latest/source/drivers/scsi/sg.c#L1134
*******************************************************************************/
int scsiSetTimeouts (PMSDEV pDev, uint32_t seconds)
{
/* uint32_t jiffies;

 if (!pDev || !pDev->fd) return (0);
 /*jiffies = seconds * 60;//Jiffy: 1/60th Second.
 if (ioctl (pDev->fd, SG_SET_TIMEOUT, &jiffies) < 0) return (0);
//TODO: check if this is needed, //no real action needed, since the timeout is passed with every deviceio command*/
 if (!pDev) return 0;
 
 pDev->timeout = seconds * 1000;//no real action needed, since the timeout is passed with every deviceio command
 
 return 1;
}

/*******************************************************************************
* simulates a (sudo) rescan-scsi-bus -r   
*******************************************************************************/
int scsiRescan (uint32_t HaId)
{
 FILE* pf;
 //TODO: full bus scan for now, but should be only 1 adapter rescan
 pf = popen ("rescan-scsi-bus -r", "r");//(server should have been launched with sudo, so should work ?)
 fclose (pf);//pclose() function waits for the process to end

 //TODO: redo the device mapping to expose / remove devices
 return (1);
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiResetTarget (PMSDEV pDev)
{
 int i = SG_SCSI_RESET_DEVICE;

 if (!pDev || !pDev->fd) return (0);

 if (ioctl (pDev->fd, SG_SCSI_RESET, &i) < 0) return (0);

 return (1);
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiSend (PMSDEV pDev, uint8_t* pCDB, uint8_t cdbLength, void* pResultBuf, uint32_t resultBufLength, uint8_t senseLen, uint8_t* pSenseArea)
{
 sg_io_hdr_t io_hdr;
 int k;
 unsigned char sense_buffer[32];//TODO ? remove this - use the input sense buffer

 if (!pDev || !pDev->fd) return (0);

 memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
 io_hdr.interface_id    = 'S';                  //'S' for SCSI generic (required)
 io_hdr.timeout         = pDev->timeout;       // in millisecs
 //CDB
 io_hdr.cmdp            = pCDB;                 //points to command to perform
 io_hdr.cmd_len         = cdbLength;            //io_hdr.cmd_len = sizeof(inqCmdBlk);   //SCSI command length ( <= 16 bytes) 
 //extra data to be exchanged
 io_hdr.dxfer_direction= (pCDB[0] == SCSI_CMD_WRITE_BUFFER) ? SG_DXFER_TO_DEV : SG_DXFER_FROM_DEV;//data transfer direction TODO TEMP HACK -> make generic srb function ? also used in spti.c
 io_hdr.dxferp          = pResultBuf;           //points to data transfer memory or scatter gather list
 io_hdr.dxfer_len       = resultBufLength;      //byte count of data transfer
 //Sense Info
 io_hdr.sbp             = sense_buffer;         //points to sense_buffer memory
 io_hdr.mx_sb_len       = sizeof(sense_buffer); // sizeof(sense_buffer); //max length to write to sbp
 //io_hdr.sbp             = pSenseArea;         //points to sense_buffer memory
 //io_hdr.mx_sb_len       = senseLen; // sizeof(sense_buffer); //max length to write to sbp

 //io_hdr.iovec_count   = 0;                    //0 implies no scatter gather - memset takes care of this
 //io_hdr.flags         = 0;                    // take defaults: indirect IO, etc  //0 -> default, see SG_FLAG...
 //io_hdr.pack_id       = 0;                    //unused internally (normally)
 //io_hdr.usr_ptr       = NULL;                 //unused internally

 //Try to execute the command
 if (ioctl(pDev->fd, SG_IO, &io_hdr) < 0) {
	LOGPRINT_CORE((PLOGGER)&pDev->logger, "(SG) %s (0x%02x): ioctl() error: %s (0x%02x):\n", scsiGetCommandDesc(pCDB[0]), 
	(unsigned char)pCDB[0], strerror(errno), errno);
  	//fprintf (st dout, "CDBLen: %d, SenseLen: %d, BufTXLen: %d\n", cdbLength, pescsi->SenseLen, resultBufLength);
    return 0;//TODO: send a bad status
 }

 //check the status
 if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
	LOGPRINT_CORE((PLOGGER)&pDev->logger, "(SG) command 0x%02x (%s): ", (unsigned char)pCDB[0], scsiGetCommandDesc(pCDB[0]));
	if (io_hdr.masked_status){
		LOGPRINT_CORE((PLOGGER)&pDev->logger, "\t(ERR) %s (0x%02x)\n", scsiGetStatusDesc(io_hdr.masked_status<<1), io_hdr.masked_status<<1);//masked status is SCSI status without the (potential) vendor bits, shifted right 1 bit
		if ((io_hdr.status >> 5) & 0x3)
		    LOGPRINT_CORE((PLOGGER)&pDev->logger, "\t(SG)(ERR) SCSI status vendor bits=0x%02x\n", (io_hdr.status >> 5) & 0x3);//masked status is SCSI status without the (potential) vendor bits
	}
    if (io_hdr.host_status)
		LOGPRINT_CORE((PLOGGER)&pDev->logger, "\t(ERR) host_status = %s (0x%02x)\n", sgGetHostStatusDesc(io_hdr.host_status), io_hdr.host_status);
    if (io_hdr.driver_status)
		LOGPRINT_CORE((PLOGGER)&pDev->logger, "\t(ERR) driver_status = %s (0x%02x)\n", sgGetDriverStatusDesc(io_hdr.driver_status), io_hdr.driver_status);

    //SENSE DATA AVAILABLE
	if (io_hdr.sb_len_wr > 0) {
        //fprintf(stderr, "\t(SG) SENSE data: %s", scsiGetSenseKeyDesc ((sense_buffer[2]) & 0x0F));
		LOGPRINT_CORE((PLOGGER)&pDev->logger, "\t(SG) SENSE data: %s", scsiGetSenseKeyDesc ((io_hdr.sbp[2]) & 0x0F));
        for (k = 0; k < io_hdr.sb_len_wr; k++) {
            if (0 == (k % 10)) printf("\n\t");
            //printf("0x%02x ", sense_buffer[k]);
			printf("0x%02x ", io_hdr.sbp[k]);
			if (k < senseLen) pSenseArea[k] = io_hdr.sbp[k];
        }
        printf("\n");
    }

	return 0;
 }

/*unsigned char status;       // [o] scsi status 
  unsigned char masked_status;// [o] shifted, masked scsi status 
  unsigned char msg_status;   // [o] messaging level data (optional) 
  unsigned char sb_len_wr;    // [o] byte count actually written to sbp */
 LOGPRINT_LOW((PLOGGER)&pDev->logger, "(SG) command %s (0x%02x): GOOD (=%u msecs, resid=%d, aux info=%u)\n",
 	scsiGetCommandDesc(pCDB[0]), (unsigned char)pCDB[0], io_hdr.duration, io_hdr.resid, io_hdr.info);

 return 1;
}

/*******************************************************************************
* 
*******************************************************************************/
void scsiClose (PMSDEV pDev, int isAppShutdown)
{
 if (!pDev || !pDev->fd) return;
 
 close (pDev->fd);
 pDev->fd = 0;
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiIsOpen (PMSDEV pDev)
{
 if (!pDev) return (0);

 return (pDev->fd ? 1 : 0);
}

/*******************************************************************************
* 
*******************************************************************************/
int scsiOpen (PMSDEV pDev)
{
 int fd;
 int k;

 if (!pDev) return (0);

 fd = open(pDev->deviceOpenStr, O_RDONLY);//TODO Note that most SCSI commands require the O_RDWR flag to be set
 if (fd < 0){
	 fprintf(stderr, "(SG) error opening device %s\n", pDev->deviceOpenStr);
	 return (0);
 }

 /* It is prudent to check we have a sg device by trying an ioctl */
 if ((ioctl(fd, SG_GET_VERSION_NUM, &k) < 0) || (k < 30000)) {
    fprintf(stderr, "(SG) %s is not a sg device, or is using an old sg driver\n", pDev->deviceOpenStr);
	close (fd);
    return (0);
 }

 pDev->fd = fd;

 return (1);
}
