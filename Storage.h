/*
 * File:   Storage.h
 * Author: root
 *
 * Created on February 19, 2013, 7:44 AM
 */

#ifndef STORAGE_H
#define	STORAGE_H

#ifndef MAX_DRIVERS
#define MAX_DRIVERS 1 // must be 1 to 4
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
// this lines must be copied into your new driver, to calculate maximum partitions possible.
#define REDO 10
// USB specific
static USB Usb;

static BulkOnly *Bulk[MAX_DRIVERS] = {
        &(BulkOnly(&Usb))
#if MAX_DRIVERS > 1
        , &(BulkOnly(&Usb))
#endif
#if MAX_DRIVERS > 2
        , &(BulkOnly(&Usb))
#endif
#if MAX_DRIVERS > 3
        , &(BulkOnly(&Usb))
#endif
#if MAX_DRIVERS > 4
        , &(BulkOnly(&Usb))
#endif
};
typedef struct Pvt {
        uint8_t lun;
        int B; // which "BulkOnly" instance
} pvt_t;

// to get _VOLUMES definition...
#include <FAT/FatFS/src/ffconf.h>

pvt_t info[_VOLUMES];

bool PStatus(storage_t *sto) {
        return (Bulk[((pvt_t *) sto->private_data)->B]->WriteProtected(((pvt_t *) sto->private_data)->lun));
        //return 0;
}

int PRead(uint32_t LBA, uint8_t *buf, storage_t *sto) {
        uint8_t x = 0;
        int tries = REDO;
        int z;
        //uint8_t sb[64];
        while(tries) {
                tries--;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Read(((pvt_t *) sto->private_data)->lun, LBA, (sto->SectorSize), 1, buf));
#if 0
                if(UsbDEBUGlvl >= 0x64) {
                        if(!x) {
                                if(UsbDEBUGlvl > 0x68) {
                                        for(int i = 0; i < sto->SectorSize; i++) {
                                                printf("%02x  ", buf[i]);
                                        }
                                        printf("\r\n");
                                }
                        } else {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
#endif
                if(!x) break;
        }
#if 0
        if(!x && UsbDEBUGlvl >= 0x64) printf("READ Success after %u tries\r\n", REDO - tries);
#endif
        int y = x;
        return y;
}

int PWrite(uint32_t LBA, uint8_t *buf, storage_t *sto) {
        //uint8_t sb[64];
        int x = 0;
        int tries = REDO;
        while(tries) {
                tries--;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Write(((pvt_t *) sto->private_data)->lun, LBA, sto->SectorSize, 1, buf));
                if(x == MASS_ERR_WRITE_PROTECTED) break;
#if 0
                if(UsbDEBUGlvl > 0x64) {
                        if(x) {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
#endif
                if(!x) break;
        }
#if 0
        if(!x && UsbDEBUGlvl >= 0x64) printf("Write Success after %u tries\r\n", REDO - tries);
#endif
        int y = x;
        return y;
}

int PReads(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        //uint8_t sb[64];
        //printf("PReads LBA=%8.8X, count=%i size=%i\r\n", LBA, count, (sto->SectorSize));
        //for(uint16_t t=0; t<(sto->SectorSize * count); t++) {
        //        buf[t] = 0x11;
        //}
        uint8_t x = 0;
        int tries = REDO;
        while(tries) {
                tries--;
                int z;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Read(((pvt_t *) sto->private_data)->lun, LBA, (sto->SectorSize), count, buf));
#if 0
                if(UsbDEBUGlvl >= 0x64) {
                        if(!x) {
                                if(UsbDEBUGlvl > 0x68) {
                                        for(int i = 0; i < sto->SectorSize; i++) {
                                                printf("%02x  ", buf[i]);
                                        }
                                        printf("\r\n");
                                }
                        } else {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
#endif
                if(!x) break;
                delay(200);
        }
#if 0

        if(!x && UsbDEBUGlvl >= 0x64) printf("Reads Success after %u tries\r\n", REDO - tries);
#endif
        int y = x;
        return y;
}

int PWrites(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        //uint8_t sb[64];
        int x = 0;
        int tries = REDO;
        while(tries) {
                tries--;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Write(((pvt_t *) sto->private_data)->lun, LBA, sto->SectorSize, count, buf));
                if(x == MASS_ERR_WRITE_PROTECTED) break;
#if 0
                if(UsbDEBUGlvl > 0x64) {
                        if(x) {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
#endif
                if(!x) break;
                delay(200);
        }
#if 0
        if(!x && UsbDEBUGlvl >= 0x64) printf("Writes Success after %u tries\r\n", REDO - tries);
#endif
        int y = x;
        return y;
}

#endif
// Your stuff here...


#endif	/* STORAGE_H */

