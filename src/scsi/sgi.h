/*******************************************************************************
* SCSI generic (sg) driver defines
*
*******************************************************************************/
#ifndef SGI_H_DEFINED__
#define SGI_H_DEFINED__

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <scsi/sg.h> /* take care: fetches glibc's /usr/include/scsi/sg.h */

/*******************************************************************************
* HOST STATUS
*******************************************************************************/
#define SG_ERR_DID_OK          0x00 /* NO error                                */
#define SG_ERR_DID_NO_CONNECT  0x01 /* Couldn't connect before timeout period  */
#define SG_ERR_DID_BUS_BUSY    0x02 /* BUS stayed busy through time out period */
#define SG_ERR_DID_TIME_OUT    0x03 /* TIMED OUT for other reason              */
#define SG_ERR_DID_BAD_TARGET  0x04 /* BAD target, device not responding?      */
#define SG_ERR_DID_ABORT       0x05 /* Told to abort for some other reason     */
#define SG_ERR_DID_PARITY      0x06 /* Parity error                            */
#define SG_ERR_DID_ERROR       0x07 /* Internal error [DMA underrun on aic7xxx]*/
#define SG_ERR_DID_RESET       0x08 /* Reset by somebody.                      */
#define SG_ERR_DID_BAD_INTR    0x09 /* Got an interrupt we weren't expecting.  */
#define SG_ERR_DID_PASSTHROUGH 0x0a /* Force command past mid-layer            */
#define SG_ERR_DID_SOFT_ERROR  0x0b /* The low level driver wants a retry      */

/*******************************************************************************
* DRIVER STATUS
*******************************************************************************/
#define SG_ERR_DRIVER_OK           0x00 /* Typically no suggestion */
#define SG_ERR_DRIVER_BUSY         0x01
#define SG_ERR_DRIVER_SOFT         0x02
#define SG_ERR_DRIVER_MEDIA        0x03
#define SG_ERR_DRIVER_ERROR        0x04
#define SG_ERR_DRIVER_INVALID      0x05
#define SG_ERR_DRIVER_TIMEOUT      0x06
#define SG_ERR_DRIVER_HARD         0x07
#define SG_ERR_DRIVER_SENSE        0x08 /* Implies sense_buffer output */
    /* above status 'or'ed with one of the following suggestions */
#define SG_ERR_SUGGEST_RETRY       0x10
#define SG_ERR_SUGGEST_ABORT       0x20
#define SG_ERR_SUGGEST_REMAP       0x30
#define SG_ERR_SUGGEST_DIE         0x40
#define SG_ERR_SUGGEST_SENSE       0x80

/*******************************************************************************
* TARGET STATUS
*******************************************************************************/
#define SG_TARGET_STATUS_GOOD                 0x00
#define SG_TARGET_STATUS_CHECK_CONDITION      0x01
#define SG_TARGET_STATUS_CONDITION_GOOD       0x02
#define SG_TARGET_STATUS_BUSY                 0x04
#define SG_TARGET_STATUS_INTERMEDIATE_GOOD    0x08
#define SG_TARGET_STATUS_INTERMEDIATE_C_GOOD  0x0a
#define SG_TARGET_STATUS_RESERVATION_CONFLICT 0x0c
#define SG_TARGET_STATUS_COMMAND_TERMINATED   0x11
#define SG_TARGET_STATUS_QUEUE_FULL           0x14

#endif
