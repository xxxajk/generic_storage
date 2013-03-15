/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file   (C)ChaN, 2013
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl fucntion */

#include <FAT/FatFS/src/integer.h>
#include <FAT/FatFS/src/ffconf.h>

        /* Status of Disk Functions */
        typedef BYTE DSTATUS;

        /* Results of Disk Functions */
        typedef enum {
                RES_OK = 0, /* 0: Successful */
                RES_ERROR, /* 1: R/W Error */
                RES_WRPRT, /* 2: Write Protected */
                RES_NOTRDY, /* 3: Not Ready */
                RES_PARERR /* 4: Invalid Parameter */
        } DRESULT;


        /*---------------------------------------*/
        /* Prototypes for disk control functions */

#ifndef DISK_INITIALIZE
        DSTATUS disk_initialize(BYTE pdrv);
#define DISK_INITIALIZE disk_initialize
#endif
#ifndef DISK_STATUS
        DSTATUS disk_status(BYTE pdrv);
#define DISK_STATUS disk_status
#endif
#ifndef DISK_READ
        DRESULT disk_read(BYTE pdrv, BYTE*buff, DWORD sector, BYTE count);
#define DISK_READ disk_read
#endif
#ifndef DISK_WRITE
        DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, BYTE count);
#define DISK_WRITE disk_write
#endif
#ifndef DISK_IOCTL
        DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);
#define DISK_IOCTL disk_ioctl
#endif

        /* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


        /* Command code for disk_ioctrl fucntion */

        /* Generic command (used by FatFs) */
#define CTRL_SYNC			0	/* Flush disk cache (for write functions) */
#define GET_SECTOR_COUNT	1	/* Get media size (for only f_mkfs()) */
#define GET_SECTOR_SIZE		2	/* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (for only f_mkfs()) */
#define CTRL_ERASE_SECTOR	4	/* Force erased a block of sectors (for only _USE_ERASE) */

        /* Generic command (not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

        /* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */

        /* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */


        /* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */


#if _USB_ == 1
struct PFAT;
DSTATUS CPP_PFAT_disk_initialize(struct PFAT*, BYTE pdrv);
#define DISK_INITIALIZE(a) CPP_PFAT_disk_initialize(fs->pfat,a)
DSTATUS CPP_PFAT_disk_status(struct PFAT*, BYTE pdrv);
#define DISK_STATUS(a) CPP_PFAT_disk_status(fs->pfat,a)
DRESULT CPP_PFAT_disk_read(struct PFAT*, BYTE pdrv, BYTE* buff, DWORD sector, BYTE count);
#define DISK_READ(a,b,c,d) CPP_PFAT_disk_read(fs->pfat,a,b,c,d)
DRESULT CPP_PFAT_disk_write(struct PFAT*, BYTE pdrv, const BYTE* buff, DWORD sector, BYTE count);
#define DISK_WRITE(a,b,c,d) CPP_PFAT_disk_write(fs->pfat,a,b,c,d)
DRESULT CPP_PFAT_disk_ioctl(struct PFAT*, BYTE pdrv, BYTE cmd, void* buff);
#define DISK_IOCTL(a,b,c) CPP_PFAT_disk_ioctl(fs->pfat,a,b,c)
DWORD CPP_PFAT_get_fattime(struct PFAT*);
#define GET_FATTIME() CPP_PFAT_get_fattime(fs->pfat)
#else
#define GET_FATTIME get_fattime
#endif



#ifdef __cplusplus
}
#endif

#endif
