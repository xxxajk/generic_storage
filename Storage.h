/*
 * File:   Storage.h
 * Author: root
 *
 * Created on February 19, 2013, 7:44 AM
 */

#ifndef STORAGE_H
#define	STORAGE_H

#ifndef MAX_USB_MS_DRIVERS
#define MAX_USB_MS_DRIVERS 1 // must be 1 to 4
#endif

/*
 * Notes:
 * Read and Write do not care about sector counts, or how many to read.
 * The passed method needs to just read and write one full sector.
 * Reads and Writes are for multiple sectors.
 *
 * In order to assist these calls, a pointer to the Storage struct is also passed,
 * so that your driver can get at its own private information, if used.
 * The private_data pointer can point to anything that you need for your driver,
 * or nothing at all if you do not need any. Nothing in the code here will use it.
 *
 * Also, "sector" is the Logical Block Address on the media, starting at ZERO.
 * C/H/S is not, and will not be used!
 * Any translations from the LBA must be done on the storage driver side, if needed.
 * Translations include any sector offsets, logical device numbers, etc.
 * The buffer size is guaranteed to be correct providing that you set SectorSize properly.
 *
 * Success from Read and Write should return ZERO,
 * Errors should return a non-Zero integer meaningful to the storage caller.
 * Negative One is returned by this layer to indicate some other error.
 *
 *
 */


struct Storage {
        int (*Read)(uint32_t, uint8_t *, struct Storage *); // single sector read
        int (*Write)(uint32_t, uint8_t *, struct Storage *); // single sector write
        int (*Reads)(uint32_t, uint8_t *, struct Storage *, uint8_t); // multiple sector read
        int (*Writes)(uint32_t, uint8_t *, struct Storage *, uint8_t); // multiple sector write
        bool (*Status)(struct Storage *);
        uint16_t SectorSize; // physical or translated size on the physical media
        uint32_t TotalSectors; // Total sector count. Used to guard against illegal access.
        void *private_data; // Anything you need, or nothing at all.
};

typedef Storage storage_t;
#ifdef _usb_h_

extern USB Usb;

#define FAT_MAX_ERROR_RETRIES 10

extern BulkOnly *Bulk[MAX_USB_MS_DRIVERS];

typedef struct Pvt {
        uint8_t lun;
        int B; // which "BulkOnly" instance
} pvt_t;

bool PStatus(storage_t *sto);
int PRead(uint32_t LBA, uint8_t *buf, storage_t *sto);
int PWrite(uint32_t LBA, uint8_t *buf, storage_t *sto);
int PReads(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count);
int PWrites(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count);
#endif
// Your stuff here...

#endif	/* STORAGE_H */

